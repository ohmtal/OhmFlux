//-----------------------------------------------------------------------------
// Copyright (c) 2026 korkscript contributors.
// See AUTHORS file and git repository for contributor information.
//
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------

#include "platform/platform.h"
#include "embed/api.h"
#include "embed/internalApi.h"
#include "engine/test/testVmFixture.h"

#include <catch2/catch.hpp>

#include <algorithm>
#include <string>
#include <vector>

namespace
{
   struct CapturedLine
   {
      U32 level;
      std::string text;
   };

   class LogCaptureGuard
   {
   public:
      LogCaptureGuard(KorkApi::Vm* vm, KorkApi::ConsumerCallback cb, void* userPtr)
         : mVm(vm)
      {
         mVm->getLogConsumer(&mPrevLogFn, &mPrevLogUser);
         mVm->setLogConsumer(cb, userPtr);
      }

      ~LogCaptureGuard()
      {
         mVm->setLogConsumer(mPrevLogFn, mPrevLogUser);
      }

   private:
      KorkApi::Vm* mVm;
      KorkApi::ConsumerCallback mPrevLogFn = nullptr;
      void* mPrevLogUser = nullptr;
   };

   void captureLine(U32 level, const char* text, void* userPtr)
   {
      auto* lines = static_cast<std::vector<CapturedLine>*>(userPtr);
      lines->push_back({level, text ? text : ""});
   }

   bool hasLineContaining(const std::vector<CapturedLine>& lines, const std::string& needle)
   {
      return std::any_of(lines.begin(), lines.end(), [&](const CapturedLine& line) {
         return line.text.find(needle) != std::string::npos;
      });
   }

   const char* globalStringDoc(void*, void*, S32, const char**)
   {
      return "string-doc";
   }

   S32 globalIntDoc(void*, void*, S32, const char**)
   {
      return 17;
   }

   bool globalBoolDoc(void*, void*, S32, const char**)
   {
      return true;
   }

   void globalVoidDoc(void*, void*, S32, const char**)
   {
   }
}

TEST_CASE_METHOD(TestVmFixture, "ConsoleDoc emits namespace function documentation", "[ConsoleDoc][Functions]") {
   KorkApi::NamespaceId globalNs = vm->getGlobalNamespace();
   REQUIRE(globalNs != nullptr);

   const StringTableEntry stringName = vm->internString("docStringFunction", false);
   const StringTableEntry intName = vm->internString("docIntFunction", false);
   const StringTableEntry boolName = vm->internString("docBoolFunction", false);
   const StringTableEntry voidName = vm->internString("docVoidFunction", false);

   vm->addNamespaceFunction(globalNs, stringName, globalStringDoc, nullptr,
      "(string who)Greets a caller.\n@param who The person being greeted.\n@returns Greeting text.", 1, 1);
   vm->addNamespaceFunction(globalNs, intName, globalIntDoc, nullptr,
      "(int count)Counts items.", 1, 1);
   vm->addNamespaceFunction(globalNs, boolName, globalBoolDoc, nullptr,
      "(bool enabled)Returns whether the path is enabled.", 1, 1);
   vm->addNamespaceFunction(globalNs, voidName, globalVoidDoc, nullptr,
      "(void)A void-returning doc entry.", 0, 0);

   std::vector<CapturedLine> lines;
   {
      LogCaptureGuard guard(vm, captureLine, &lines);
      vm->dumpNamespaceFunctions(true, true);
   }

   // The global dump should document the namespace wrapper and the registered functions.
   REQUIRE(hasLineContaining(lines, "namespace Global {"));
   REQUIRE(hasLineContaining(lines, "Greets a caller."));
   REQUIRE(hasLineContaining(lines, "@param who The person being greeted."));
   REQUIRE(hasLineContaining(lines, "string docStringFunction(string who) {}"));
   REQUIRE(hasLineContaining(lines, "int docIntFunction(int count) {}"));
   REQUIRE(hasLineContaining(lines, "bool docBoolFunction(bool enabled) {}"));
   REQUIRE(hasLineContaining(lines, "void docVoidFunction(void) {}"));
}

TEST_CASE_METHOD(TestVmFixture, "ConsoleDoc emits namespace inheritance and usage documentation", "[ConsoleDoc][Namespaces]") {
   const StringTableEntry baseName = vm->internString("ConsoleDocProbeBase", false);
   const StringTableEntry childName = vm->internString("ConsoleDocProbeChild", false);
   const StringTableEntry baseFuncName = vm->internString("baseDocFunction", false);
   const StringTableEntry childFuncName = vm->internString("childDocFunction", false);

   KorkApi::NamespaceId baseNs = vm->findNamespace(baseName);
   KorkApi::NamespaceId childNs = vm->findNamespace(childName);
   REQUIRE(baseNs != nullptr);
   REQUIRE(childNs != nullptr);

   vm->addNamespaceFunction(baseNs, baseFuncName, globalVoidDoc, nullptr, "(void)Base doc function.", 0, 0);
   vm->addNamespaceFunction(childNs, childFuncName, globalVoidDoc, nullptr, "(void)Child doc function.", 0, 0);
   REQUIRE(vm->linkNamespaceById(baseNs, childNs));

   vm->setNamespaceUsage(baseNs, "Base console doc usage.");
   vm->setNamespaceUsage(childNs, "Child console doc usage.");

   std::vector<CapturedLine> lines;
   {
      LogCaptureGuard guard(vm, captureLine, &lines);
      vm->dumpNamespaceClasses(true, true);
   }

   // The dump should show the class-style namespace hierarchy and the registered methods/usage text.
   REQUIRE(hasLineContaining(lines, "class  ConsoleDocProbeBase"));
   REQUIRE(hasLineContaining(lines, "class  ConsoleDocProbeChild : public ConsoleDocProbeBase {"));
   REQUIRE(hasLineContaining(lines, "Base console doc usage."));
   REQUIRE(hasLineContaining(lines, "Child console doc usage."));
   REQUIRE(hasLineContaining(lines, "baseDocFunction"));
   REQUIRE(hasLineContaining(lines, "childDocFunction"));
}
