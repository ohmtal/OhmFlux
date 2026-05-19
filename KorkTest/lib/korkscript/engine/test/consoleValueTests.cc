//-----------------------------------------------------------------------------
// Copyright (c) 2026 korkscript contributors.
// See AUTHORS file and git repository for contributor information.
//
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------

#include "platform/platform.h"
#include "console/consoleValue.h"

#include <catch2/catch.hpp>

#include <array>
#include <cstring>
#include <string>

namespace
{
   KorkApi::ConsoleValue::AllocBase makeAllocBase(void** func, void* arg)
   {
      KorkApi::ConsoleValue::AllocBase base;
      base.func = func;
      base.arg = arg;
      return base;
   }
}

TEST_CASE("ConsoleValue default construction and immediate values", "[ConsoleValue]") {
   // Default construction should behave like an empty string handle.
   KorkApi::ConsoleValue def;
   REQUIRE(def.typeId == KorkApi::ConsoleValue::TypeInternalString);
   REQUIRE(def.getZone() == KorkApi::ConsoleValue::ZoneExternal);
   REQUIRE(def.cvalue == 0);
   REQUIRE(def.isString());
   REQUIRE(def.isNull());
   REQUIRE_FALSE(def.isUnsigned());
   REQUIRE_FALSE(def.isFloat());
   REQUIRE_FALSE(def.isCustom());

   // Numeric values use the immediate payload, not pointer storage.
   KorkApi::ConsoleValue u = KorkApi::ConsoleValue::makeUnsigned(42);
   REQUIRE(u.isUnsigned());
   REQUIRE(u.getInt() == 42);
   REQUIRE(u.getFloat(123.0) == Catch::Approx(123.0));
   REQUIRE(u.quickCastToNumeric() == Catch::Approx(42.0));
   REQUIRE(u.canBePacked());
   REQUIRE(u.evaluatePtr() == nullptr);
   REQUIRE(u.evaluateFixedPtr() != nullptr);

   KorkApi::ConsoleValue f = KorkApi::ConsoleValue::makeNumber(3.5);
   REQUIRE(f.isFloat());
   REQUIRE(f.getFloat() == Catch::Approx(3.5));
   REQUIRE(f.getInt(999) == 999);
   REQUIRE(f.quickCastToNumeric() == Catch::Approx(3.5));
   REQUIRE(f.canBePacked());
   REQUIRE(f.evaluatePtr() == nullptr);
   REQUIRE(f.evaluateFixedPtr() != nullptr);

   KorkApi::ConsoleValue s = KorkApi::ConsoleValue::makeString("hello");
   REQUIRE(s.isString());
   REQUIRE_FALSE(s.isNull());
   REQUIRE(s.canBePacked() == false);
   REQUIRE(s.evaluatePtr() != nullptr);
   REQUIRE(std::string((const char*)s.evaluatePtr()) == "hello");

   KorkApi::ConsoleValue nullStr = KorkApi::ConsoleValue::makeString(nullptr);
   REQUIRE(nullStr.isString());
   REQUIRE(nullStr.isNull());
   REQUIRE(nullStr.evaluatePtr() == nullptr);
}

TEST_CASE("ConsoleValue mutators and pointer helpers", "[ConsoleValue]") {
   std::array<char, 32> externalStorage{};
   std::strcpy(externalStorage.data(), "sample");

   KorkApi::ConsoleValue stringValue;
   stringValue.setString(externalStorage.data());
   REQUIRE(stringValue.isString());
   REQUIRE(stringValue.evaluatePtr() == externalStorage.data());
   REQUIRE(stringValue.evaluateFixedPtr() == externalStorage.data());

   KorkApi::ConsoleValue dynStringValue;
   dynStringValue.setDynString(externalStorage.data());
   REQUIRE(dynStringValue.isString());
   REQUIRE(dynStringValue.evaluatePtr() == externalStorage.data());

   // Packed strings/custom values store the pointer in the CV payload itself.
   KorkApi::ConsoleValue packedString = KorkApi::ConsoleValue::makeString(externalStorage.data(), KorkApi::ConsoleValue::ZonePacked);
   REQUIRE(packedString.getZone() == KorkApi::ConsoleValue::ZonePacked);
   REQUIRE(packedString.evaluatePtr() == &packedString.cvalue);
   REQUIRE(*(const char**)packedString.evaluatePtr() == externalStorage.data());

   KorkApi::ConsoleValue customValue = KorkApi::ConsoleValue::makeTyped(externalStorage.data(), KorkApi::ConsoleValue::TypeBeginCustom + 1);
   REQUIRE(customValue.isCustom());
   REQUIRE(customValue.evaluatePtr() == externalStorage.data());

   KorkApi::ConsoleValue packedCustom = KorkApi::ConsoleValue::makeTyped(externalStorage.data(), KorkApi::ConsoleValue::TypeBeginCustom + 1, KorkApi::ConsoleValue::ZonePacked);
   REQUIRE(packedCustom.evaluatePtr() == &packedCustom.cvalue);
   REQUIRE(*(void**)packedCustom.evaluatePtr() == externalStorage.data());

   KorkApi::ConsoleValue rawValue = KorkApi::ConsoleValue::makeRaw(99, KorkApi::ConsoleValue::TypeBeginCustom + 2);
   REQUIRE(rawValue.isCustom());
   REQUIRE(rawValue.ptr() == reinterpret_cast<void*>(99));
}

TEST_CASE("ConsoleValue zone-based pointer evaluation", "[ConsoleValue]") {
   // ZoneReturn resolves against the return buffer pointer stored in the alloc base.
   std::array<char, 16> returnStorage{};
   std::array<char, 16> funcStorage{};
   std::array<void*, 2> funcPtrs{ funcStorage.data(), nullptr };
   auto allocBase = makeAllocBase(funcPtrs.data(), returnStorage.data());

   KorkApi::ConsoleValue returnString;
   returnString.setTyped(4, KorkApi::ConsoleValue::TypeInternalString, KorkApi::ConsoleValue::ZoneReturn);
   REQUIRE(returnString.evaluatePtr(allocBase) == returnStorage.data() + 4);
   REQUIRE(returnString.evaluateFixedPtr(allocBase) == returnStorage.data() + 4);

   // ZoneFunc resolves against the per-fiber function buffer table.
   KorkApi::ConsoleValue funcString;
   funcString.setTyped(3, KorkApi::ConsoleValue::TypeInternalString, KorkApi::ConsoleValue::ZoneFunc);
   REQUIRE(funcString.evaluatePtr(allocBase) == funcStorage.data() + 3);
   REQUIRE(funcString.evaluateFixedPtr(allocBase) == funcStorage.data() + 3);

   // Relocation policy depends on the zone, not just the type.
   KorkApi::ConsoleValue reloc = KorkApi::ConsoleValue::makeString("x");
   reloc.setZone(KorkApi::ConsoleValue::ZoneVmHeap);
   REQUIRE(reloc.hasRelocatableStorage());
   reloc.setZone(KorkApi::ConsoleValue::ZonePacked);
   REQUIRE_FALSE(reloc.hasRelocatableStorage());

   KorkApi::ConsoleValue nonPtr = KorkApi::ConsoleValue::makeUnsigned(7);
   REQUIRE(nonPtr.evaluateFixedPtr(allocBase) != nullptr);
   REQUIRE(nonPtr.evaluatePtr(allocBase) == nullptr);
}

TEST_CASE("ConsoleValue packing helpers and classification", "[ConsoleValue]") {
   KorkApi::ConsoleValue u = KorkApi::ConsoleValue::makeUnsigned(12);
   KorkApi::ConsoleValue n = KorkApi::ConsoleValue::makeNumber(12.5);
   KorkApi::ConsoleValue s = KorkApi::ConsoleValue::makeString("plain");
   KorkApi::ConsoleValue c = KorkApi::ConsoleValue::makeTyped(reinterpret_cast<void*>(0x1234), KorkApi::ConsoleValue::TypeBeginCustom + 1);

   REQUIRE(u.canBePacked());
   REQUIRE(n.canBePacked());
   REQUIRE_FALSE(s.canBePacked());
   REQUIRE_FALSE(c.canBePacked());
   REQUIRE_FALSE(u.isCustom());
   REQUIRE_FALSE(n.isCustom());
   REQUIRE(c.isCustom());

   u.setNumber(9.25);
   REQUIRE(u.isFloat());
   REQUIRE(u.getFloat() == Catch::Approx(9.25));

   n.setUnsigned(44);
   REQUIRE(n.isUnsigned());
   REQUIRE(n.getInt() == 44);
}

TEST_CASE("ConsoleValue convertArgs helpers", "[ConsoleValue]") {
   std::array<char, 16> textBuffer{};
   std::strcpy(textBuffer.data(), "alpha");
   std::array<char, 16> customBuffer{};
   std::strcpy(customBuffer.data(), "beta");

   KorkApi::ConsoleValue inArgs[] = {
      KorkApi::ConsoleValue::makeString(textBuffer.data()),
      KorkApi::ConsoleValue::makeTyped(customBuffer.data(), KorkApi::ConsoleValue::TypeBeginCustom + 1),
      KorkApi::ConsoleValue::makeNumber(2.0)
   };

   const char* outArgs[3] = {};
   bool delegateCalled = false;
   KorkApi::ConsoleValue::convertArgs({}, 3, inArgs, outArgs,
      [&](const KorkApi::ConsoleValue& v)
      {
         delegateCalled = true;
         return static_cast<const char*>(v.evaluatePtr());
      });

   REQUIRE(delegateCalled);
   REQUIRE(std::string(outArgs[0]) == "alpha");
   REQUIRE(std::string(outArgs[1]) == "beta");
   REQUIRE(outArgs[2] == nullptr);

   KorkApi::ConsoleValue reverseArgs[2];
   const char* reverseIn[] = { "one", "two" };
   KorkApi::ConsoleValue::convertArgsReverse(2, reverseIn, reverseArgs);
   REQUIRE(reverseArgs[0].isString());
   REQUIRE(reverseArgs[1].isString());
   REQUIRE(std::string((const char*)reverseArgs[0].evaluatePtr()) == "one");
   REQUIRE(std::string((const char*)reverseArgs[1].evaluatePtr()) == "two");
}

TEST_CASE("ConsoleValue advancePtr updates the raw payload", "[ConsoleValue]") {
   std::array<char, 16> buffer{};
   KorkApi::ConsoleValue v = KorkApi::ConsoleValue::makeTyped(buffer.data(), KorkApi::ConsoleValue::TypeBeginCustom + 1);
   REQUIRE(v.ptr() == buffer.data());

   void* advanced = v.advancePtr(4);
   REQUIRE(advanced == buffer.data() + 4);
   REQUIRE(v.ptr() == buffer.data() + 4);
}
