//-----------------------------------------------------------------------------
// Copyright (c) 2026 korkscript contributors.
// See AUTHORS file and git repository for contributor information.
//
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------

#include "platform/platform.h"
#include "embed/api.h"
#include "embed/internalApi.h"
#include "console/ast.h"
#include "console/codeBlock.h"
#include "core/memStream.h"
#include "engine/test/testVmFixture.h"

#include <catch2/catch.hpp>

#include <algorithm>
#include <cstring>
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

   class AllowTypesGuard
   {
   public:
      explicit AllowTypesGuard(Compiler::Resources* resources, bool value)
         : mResources(resources)
         , mPrev(mResources->allowTypes)
      {
         mResources->allowTypes = value;
      }

      ~AllowTypesGuard()
      {
         mResources->allowTypes = mPrev;
      }

   private:
      Compiler::Resources* mResources;
      bool mPrev;
   };

   bool hasLine(const std::vector<CapturedLine>& lines, const std::string& needle)
   {
      return std::any_of(lines.begin(), lines.end(), [&](const CapturedLine& line) {
         return line.text.find(needle) != std::string::npos;
      });
   }

   void compileScript(TestVmFixture& fx, CodeBlock& block, const char* filename, const char* script)
   {
      AllowTypesGuard typesGuard(fx.internal->mCompilerResources, true);
      (void)filename;
      (void)block.compileExec(nullptr, nullptr, script, true);
   }

   void compileScriptToStream(TestVmFixture& fx, CodeBlock& block, const char* filename, const char* script, bool allowTypes)
   {
      AllowTypesGuard typesGuard(fx.internal->mCompilerResources, allowTypes);

      std::vector<U8> scratch(1 << 20);
      MemStream stream((U32)scratch.size(), scratch.data(), true, true);

      REQUIRE(block.compileToStream(stream, fx.vm->internString(filename, false), script));
   }

   void populateSerializedBlock(TestVmFixture& fx, CodeBlock& block)
   {
      block.globalStringsMaxLen = 11;
      block.globalStrings = fx.internal->NewArray<char>(block.globalStringsMaxLen);
      std::memcpy(block.globalStrings, "alpha\0beta\0", block.globalStringsMaxLen);

      block.functionStringsMaxLen = 6;
      block.functionStrings = fx.internal->NewArray<char>(block.functionStringsMaxLen);
      std::memcpy(block.functionStrings, "fn\0x\0", block.functionStringsMaxLen);

      block.numGlobalFloats = 1;
      block.globalFloats = fx.internal->NewArray<F64>(1);
      block.globalFloats[0] = 3.25;

      block.numFunctionFloats = 1;
      block.functionFloats = fx.internal->NewArray<F64>(1);
      block.functionFloats[0] = 7.5;

      block.codeSize = 1;
      block.lineBreakPairCount = 1;
      block.code = fx.internal->NewArray<U32>(block.codeSize + block.lineBreakPairCount * 2);
      block.code[0] = Compiler::OP_RETURN;
      block.code[1] = 1;
      block.code[2] = 0;
      block.lineBreakPairs = block.code + block.codeSize;

      block.numIdentStrings = 1;
      block.identStrings = fx.internal->NewArray<StringTableEntry>(1);
      block.identStringOffsets = fx.internal->NewArray<U32>(1);
      block.identStrings[0] = fx.vm->internString("alpha", false);
      block.identStringOffsets[0] = 0;

      block.numFunctionCalls = 1;
      block.functionCalls = fx.internal->NewArray<void*>(1);
      block.functionCalls[0] = nullptr;

      block.startTypeStrings = 0;
      block.numTypeStrings = 0;
      block.typeStringMap = nullptr;
   }

   std::vector<U8> writeBlock(CodeBlock& block)
   {
      std::vector<U8> buffer(1 << 20);
      MemStream stream((U32)buffer.size(), buffer.data(), true, true);

      REQUIRE(block.write(stream));
      buffer.resize(stream.getPosition());
      return buffer;
   }

   void readBlock(TestVmFixture& fx, CodeBlock& block, const std::vector<U8>& buffer, const char* fileName, const char* modPath)
   {
      MemStream stream((U32)buffer.size(), const_cast<U8*>(buffer.data()), true, false);

      REQUIRE(block.read(fx.vm->internString(fileName, false),
                         fx.vm->internString(modPath, false),
                         stream,
                         0));
      REQUIRE(stream.getPosition() == buffer.size());
   }

   struct BreakPointInfo
   {
      U32 line;
      U32 ip;
      U32 opcode;
   };

   std::vector<BreakPointInfo> collectBreakPoints(CodeBlock& block)
   {
      std::vector<BreakPointInfo> out;
      out.reserve(block.lineBreakPairCount);

      for (U32 i = 0; i < block.lineBreakPairCount; ++i)
      {
         const U32 encoded = block.lineBreakPairs[i * 2];
         out.push_back({
            encoded >> 8,
            block.lineBreakPairs[i * 2 + 1],
            encoded & 0xFF
         });
      }

      return out;
   }

   std::vector<U32> collectUniqueLines(const std::vector<BreakPointInfo>& points)
   {
      std::vector<U32> lines;
      lines.reserve(points.size());

      for (const auto& point : points)
         lines.push_back(point.line);

      std::sort(lines.begin(), lines.end());
      lines.erase(std::unique(lines.begin(), lines.end()), lines.end());
      return lines;
   }
}

TEST_CASE_METHOD(TestVmFixture, "CodeBlock compile, dump, and breakpoint helpers", "[CodeBlock]") {
   const char* fileName = "codeBlockExample.cs";
   const char* modPath = "codeBlockTests";

   CodeBlock source(internal, false);
   populateSerializedBlock(*this, source);

   const std::vector<U8> serialized = writeBlock(source);
   REQUIRE_FALSE(serialized.empty());

   CodeBlock loaded(internal, false);
   const CodeBlock* originalHead = internal->mCodeBlockList;
   readBlock(*this, loaded, serialized, fileName, modPath);

   REQUIRE(loaded.inList);
   REQUIRE(internal->mCodeBlockList == &loaded);
   REQUIRE(loaded.name == vm->internString(fileName, false));
   REQUIRE(loaded.fullPath == vm->internString(fileName, false));
   REQUIRE(loaded.modPath == vm->internString(modPath, false));
   REQUIRE(loaded.codeSize == 1);
   REQUIRE(loaded.lineBreakPairCount == 1);
   REQUIRE(loaded.numFunctionCalls == 1);

   // NSEntry storage should behave like a simple indexed pointer table.
   REQUIRE(loaded.getNSEntry(0) == nullptr);

   void* sentinel = reinterpret_cast<void*>(0x1234);
   loaded.setNSEntry(0, sentinel);
   REQUIRE(loaded.getNSEntry(0) == sentinel);
   REQUIRE_FALSE(loaded.didFlushFunctions);

   loaded.flushNSEntries();
   REQUIRE(loaded.didFlushFunctions);
   REQUIRE(loaded.getNSEntry(0) == nullptr);

   loaded.setNSEntry(loaded.numFunctionCalls, sentinel);
   REQUIRE(loaded.getNSEntry(loaded.numFunctionCalls) == nullptr);

   const std::vector<BreakPointInfo> points = collectBreakPoints(loaded);
   REQUIRE(points.size() == 1);
   REQUIRE(points.front().line == 1);
   REQUIRE(points.front().ip == 0);
   REQUIRE(points.front().opcode == Compiler::OP_RETURN);

   REQUIRE(loaded.findFirstBreakLine(1) == 1);
   REQUIRE(loaded.findFirstBreakLine(2) == 0);

   U32 line = 0;
   U32 inst = 0;
   loaded.findBreakLine(0, line, inst);
   REQUIRE(line == 1);
   REQUIRE(inst == Compiler::OP_RETURN);
   REQUIRE(std::string(loaded.getFileLine(0)) == std::string(fileName) + " (1)");

   std::vector<CapturedLine> dumpLines;
   {
      LogCaptureGuard logGuard(vm, [](U32 level, const char* line, void* user)
      {
         auto* sink = static_cast<std::vector<CapturedLine>*>(user);
         sink->push_back({ level, line ? line : "" });
      }, &dumpLines);
      loaded.dumpInstructions(0, false, false, true);
   }
   REQUIRE(hasLine(dumpLines, "# Line 1"));
   REQUIRE(hasLine(dumpLines, "OP_RETURN"));

   REQUIRE(loaded.setBreakpoint(1));
   REQUIRE(loaded.code[0] == Compiler::OP_BREAK);
   loaded.clearBreakpoint(1);
   REQUIRE(loaded.code[0] == Compiler::OP_RETURN);

   loaded.setAllBreaks();
   REQUIRE(loaded.code[0] == Compiler::OP_BREAK);
   loaded.clearAllBreaks();
   REQUIRE(loaded.code[0] == Compiler::OP_RETURN);

   REQUIRE_FALSE(loaded.setBreakpoint(0));

   // A rehydrated block should still serialize back out cleanly.
   const std::vector<U8> reserialized = writeBlock(loaded);
   REQUIRE_FALSE(reserialized.empty());

   // List management is split between file-backed code blocks and exec blocks.
   loaded.removeFromCodeList();
   REQUIRE_FALSE(loaded.inList);
      REQUIRE(internal->mCodeBlockList == originalHead);

   loaded.addToCodeList();
   REQUIRE(loaded.inList);
   REQUIRE(internal->mCodeBlockList == &loaded);
}

TEST_CASE_METHOD(TestVmFixture, "CodeBlock function-call metadata and type linking", "[CodeBlock]") {
   CodeBlock source(internal, false);
   source.numIdentStrings = 4;
   source.identStrings = internal->NewArray<StringTableEntry>(4);
   source.identStrings[0] = vm->internString("%Value", false);
   source.identStrings[1] = vm->internString("%Other", false);
   source.identStrings[2] = vm->internString("uint", false);
   source.identStrings[3] = vm->internString("float", false);

   source.codeSize = 10;
   source.code = internal->NewArray<U32>(source.codeSize);
   std::memset(source.code, 0, sizeof(U32) * source.codeSize);
   source.code[5] = 2;
   source.code[6] = 1;
   source.code[8] = 2;

   source.startTypeStrings = 2;
   source.numTypeStrings = 2;
   source.typeStringMap = internal->NewArray<S32>(2);
   source.typeStringMap[0] = -1;
   source.typeStringMap[1] = -1;

   // getFunctionArgs() should reconstruct the declaration parameter list from the opcode stream.
   char args[1024];
   source.getFunctionArgs(args, 0);
   REQUIRE(std::string(args) == "var Value, var Other");

   // linkTypes() should resolve built-in names that appear in the compiled signature.
   REQUIRE(source.numTypeStrings >= 2);
   REQUIRE(source.linkTypes());

   bool sawFloat = false;
   bool sawUint = false;
   for (U32 i = 0; i < source.numTypeStrings; ++i)
   {
      const StringTableEntry typeName = source.getTypeName(i);
      REQUIRE(typeName != nullptr);

      const S32 realTypeId = (S32)source.getRealTypeID(i);
      REQUIRE(realTypeId >= 0);
      REQUIRE(vm->getTypeInfo(realTypeId) != nullptr);
      REQUIRE(vm->getTypeInfo(realTypeId)->name == typeName);

      if (std::strcmp(typeName, "float") == 0)
      {
         sawFloat = true;
         REQUIRE(realTypeId == internal->lookupTypeId(typeName));
      }
      else if (std::strcmp(typeName, "uint") == 0)
      {
         sawUint = true;
         REQUIRE(realTypeId == internal->lookupTypeId(typeName));
      }
   }

   REQUIRE(sawFloat);
   REQUIRE(sawUint);
   REQUIRE(source.getTypeName(source.numTypeStrings) == nullptr);
   REQUIRE(source.getRealTypeID(source.numTypeStrings) == 0);
}

TEST_CASE_METHOD(TestVmFixture, "CodeBlock addToCodeList and removeFromCodeList handle exec blocks", "[CodeBlock]") {
   CodeBlock execBlock(internal, true);

   const CodeBlock* originalExecHead = internal->mExecCodeBlockList;

   // Exec blocks use a separate list from file-backed blocks.
   execBlock.addToCodeList();
   REQUIRE(execBlock.inList);
   REQUIRE(internal->mExecCodeBlockList == &execBlock);

   execBlock.removeFromCodeList();
   REQUIRE_FALSE(execBlock.inList);
   REQUIRE(internal->mExecCodeBlockList == originalExecHead);
}

TEST_CASE_METHOD(TestVmFixture, "CodeBlock linkTypes reports undefined type names", "[CodeBlock]") {
   CodeBlock source(internal, false);
   source.numTypeStrings = 1;
   source.startTypeStrings = 0;
   source.identStrings = internal->NewArray<StringTableEntry>(1);
   source.identStrings[0] = vm->internString("notAType", false);
   source.typeStringMap = internal->NewArray<S32>(1);
   source.typeStringMap[0] = -1;

   std::vector<CapturedLine> lines;
   {
      LogCaptureGuard logGuard(vm, [](U32 level, const char* line, void* user)
      {
         auto* sink = static_cast<std::vector<CapturedLine>*>(user);
         sink->push_back({ level, line ? line : "" });
      }, &lines);
      REQUIRE_FALSE(source.linkTypes());
   }

   REQUIRE(hasLine(lines, "Type notAType used in script is undefined"));
}
