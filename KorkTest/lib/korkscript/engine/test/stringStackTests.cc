//-----------------------------------------------------------------------------
// Copyright (c) 2026 korkscript contributors.
// See AUTHORS file and git repository for contributor information.
//
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------

#include "embed/api.h"
#include "embed/internalApi.h"
#include "console/stringStack.h"
#include "engine/test/testVmFixture.h"
#include "embed/compilerOpcodes.h"

#include <catch2/catch.hpp>

#include <array>
#include <cstring>
#include <string>

namespace
{
   struct StackHarness
   {
      std::array<void*, 4> funcBuffers{};
      std::array<char, 256> returnBuffer{};
      KorkApi::ConsoleValue::AllocBase allocBase{};
      KorkApi::Vector<KorkApi::TypeInfo>* typeInfos = nullptr;
      StringStack stack;

      StackHarness(KorkApi::Vector<KorkApi::TypeInfo>* types = nullptr)
         : allocBase{funcBuffers.data(), returnBuffer.data()},
           typeInfos(types),
           stack(&allocBase, typeInfos)
      {
         funcBuffers.fill(nullptr);
         stack.initForFiber(0);
      }
   };

}

TEST_CASE_METHOD(TestVmFixture, "StringStack buffer setup and value helpers", "[StringStack]") {
   KorkApi::VmAllocTLS::Scope scope(internal);
   StackHarness harness;

   // The stack should publish its backing buffer for the active fiber.
   REQUIRE(harness.funcBuffers[0] == harness.stack.mBuffer.data());

   harness.stack.setStringValue("hello");
   REQUIRE(std::strcmp(harness.stack.getStringValue(), "hello") == 0);
   REQUIRE(harness.stack.getHeadLength() == 5);

   harness.stack.setStringValue(nullptr);
   REQUIRE(harness.stack.getStringValue()[0] == '\0');
   REQUIRE(harness.stack.getHeadLength() == 0);

   harness.stack.setUnsignedValue(42);
   REQUIRE(harness.stack.getConsoleValue().isUnsigned());
   REQUIRE(harness.stack.getConsoleValue().getInt() == 42);

   harness.stack.setNumberValue(3.5);
   REQUIRE(harness.stack.getConsoleValue().isFloat());
   REQUIRE(harness.stack.getConsoleValue().getFloat() == Catch::Approx(3.5));

   harness.stack.setStringIntValue(1234);
   REQUIRE(std::strcmp(harness.stack.getStringValue(), "1234") == 0);

   harness.stack.setStringFloatValue(2.25);
   REQUIRE(std::strcmp(harness.stack.getStringValue(), "2.25") == 0);

   harness.stack.setTypedLen(KorkApi::ConsoleValue::TypeBeginCustom + 1, 7);
   REQUIRE(harness.stack.mType == KorkApi::ConsoleValue::TypeBeginCustom + 1);
   REQUIRE(harness.stack.mLen == 7);

   harness.stack.setConsoleValueSize(KorkApi::ConsoleValue::TypeBeginCustom + 2, 9);
   REQUIRE(harness.stack.mType == KorkApi::ConsoleValue::TypeBeginCustom + 2);
   REQUIRE(harness.stack.mLen == 9);
   REQUIRE(harness.stack.mValue == harness.stack.mStart);

   harness.stack.setConsoleValueValue(KorkApi::ConsoleValue::TypeBeginCustom + 3, 0x1234);
   REQUIRE(harness.stack.mType == KorkApi::ConsoleValue::TypeBeginCustom + 3);
   REQUIRE(harness.stack.mValue == 0x1234);
   REQUIRE(harness.stack.mLen == 0);
}

TEST_CASE_METHOD(TestVmFixture, "StringStack frame motion and stack rewinding", "[StringStack]") {
   KorkApi::VmAllocTLS::Scope scope(internal);
   StackHarness harness;

   harness.stack.setStringValue("outer");
   harness.stack.pushFrame();
   harness.stack.setStringValue("inner");
   REQUIRE(harness.stack.mNumFrames == 1);

   harness.stack.popFrame();
   REQUIRE(harness.stack.mNumFrames == 0);
   REQUIRE(harness.stack.mStartStackSize == 0);
   REQUIRE(harness.stack.mType == KorkApi::ConsoleValue::TypeInternalString);

   // Rewind should restore the previous item, while rewindTerminate should
   // also force a terminator into the current string head before popping.
   harness.stack.push();
   harness.stack.setStringValue("alpha");
   harness.stack.push();
   harness.stack.setStringValue("beta");
   harness.stack.rewind();
   REQUIRE(harness.stack.mType == KorkApi::ConsoleValue::TypeInternalString);

   harness.stack.push();
   harness.stack.setStringValue("gamma");
   harness.stack.advanceChar('!');
   harness.stack.rewindTerminate();
   REQUIRE(harness.stack.mType == KorkApi::ConsoleValue::TypeInternalString);
   REQUIRE(harness.stack.getHeadLength() == 6);

   harness.stack.push();
   harness.stack.setStringValue("match");
   harness.stack.push();
   harness.stack.setStringValue("MATCH");
   REQUIRE(harness.stack.compare() == 1);
   harness.stack.push();
   REQUIRE(harness.stack.getHeadLength() == 0);
}

TEST_CASE_METHOD(TestVmFixture, "StringStack argc argv extraction and conversion", "[StringStack]") {
   KorkApi::VmAllocTLS::Scope scope(internal);
   StackHarness harness;

   const StringTableEntry functionName = vm->internString("stringStackFn", false);

   harness.stack.pushFrame();
   harness.stack.setStringValue("first");
   harness.stack.push();
   harness.stack.setStringValue("second");
   harness.stack.push();
   harness.stack.setStringValue("third");
   harness.stack.push();

   U32 argc = 0;
   KorkApi::ConsoleValue* argv = nullptr;
   harness.stack.getArgcArgv(functionName, &argc, &argv, false);

   REQUIRE(argc == 4);
   REQUIRE(argv[0].isString());
   REQUIRE(std::strcmp(vm->valueAsString(argv[0]), "stringStackFn") == 0);
   REQUIRE(argv[1].isString());
   REQUIRE(argv[1].evaluatePtr(harness.allocBase) != nullptr);
   REQUIRE(argv[2].isString());
   REQUIRE(argv[2].evaluatePtr(harness.allocBase) != nullptr);
   REQUIRE(argv[3].isString());
   REQUIRE(argv[3].evaluatePtr(harness.allocBase) != nullptr);

   const char** converted = nullptr;
   harness.stack.convertArgv(internal, argc, &converted);
   REQUIRE(converted[0] != nullptr);
   REQUIRE(converted[1] != nullptr);
   REQUIRE(converted[2] != nullptr);
   REQUIRE(converted[3] != nullptr);

   harness.stack.getArgcArgv(functionName, &argc, &argv, true);
   REQUIRE(argc == 4);
   REQUIRE(harness.stack.mNumFrames == 0);
}

TEST_CASE_METHOD(TestVmFixture, "StringStack fixed-size custom values copy into the stack buffer", "[StringStack]") {
   KorkApi::VmAllocTLS::Scope scope(internal);

   KorkApi::TypeInfo customInfo{};
   customInfo.name = vm->internString("stackCustom", false);
   customInfo.valueSize = sizeof(U32);
   customInfo.fieldSize = sizeof(U32);
   KorkApi::TypeId customTypeId = vm->registerType(customInfo);

   StackHarness harness(&internal->mTypes);

   U32 customValue = 0x11223344;
   KorkApi::ConsoleValue custom = KorkApi::ConsoleValue::makeTyped(&customValue, customTypeId);
   harness.stack.setConsoleValue(internal, custom);

   REQUIRE(harness.stack.getConsoleValue().typeId == customTypeId);
   REQUIRE(harness.stack.getConsoleValue().getZone() == KorkApi::ConsoleValue::ZoneFunc);
   REQUIRE(harness.stack.getHeadLength() == sizeof(U32));

   U32 storedValue = 0;
   std::memcpy(&storedValue, harness.stack.mBuffer.data() + harness.stack.mStart, sizeof(U32));
   REQUIRE(storedValue == customValue);
}

TEST_CASE_METHOD(TestVmFixture, "StringStack stack value accessors expose stored numeric items", "[StringStack]") {
   KorkApi::VmAllocTLS::Scope scope(internal);
   StackHarness harness;

   harness.stack.setUnsignedValue(5);
   harness.stack.push();
   harness.stack.setUnsignedValue(7);

   KorkApi::ConsoleValue current = harness.stack.getConsoleValue();
   REQUIRE(current.isUnsigned());
   REQUIRE(current.getInt() == 7);

   KorkApi::ConsoleValue prior = harness.stack.getStackConsoleValue(0);
   REQUIRE(prior.isUnsigned());
   REQUIRE(prior.getInt() == 5);
}

TEST_CASE_METHOD(TestVmFixture, "StringStack performOp consumes the right-hand operand from the stack", "[StringStack]") {
   KorkApi::VmAllocTLS::Scope scope(internal);
   StackHarness harness(&internal->mTypes);

   // Match the compiler's typed binary-op layout: push the right-hand value,
   // then leave the left-hand value at the top before the op runs.
   harness.stack.setUnsignedValue(5);
   harness.stack.push();
   harness.stack.setUnsignedValue(7);

   REQUIRE(harness.stack.mStartStackSize == 1);

   harness.stack.performOp(Compiler::OP_ADD, vm, internal->mTypes.data());

   REQUIRE(harness.stack.getConsoleValue().isFloat());
   REQUIRE(harness.stack.getConsoleValue().getFloat() == Catch::Approx(12.0));
   REQUIRE(harness.stack.mStartStackSize == 0);
}

TEST_CASE_METHOD(TestVmFixture, "StringStack performOpReverse uses the current value as the left operand", "[StringStack]") {
   KorkApi::VmAllocTLS::Scope scope(internal);
   StackHarness harness(&internal->mTypes);

   // Reverse op uses the current value as lhs and the stacked value as rhs.
   harness.stack.setUnsignedValue(5);
   harness.stack.push();
   harness.stack.setUnsignedValue(2);

   REQUIRE(harness.stack.mStartStackSize == 1);

   harness.stack.performOpReverse(Compiler::OP_SUB, vm, internal->mTypes.data());

   REQUIRE(harness.stack.getConsoleValue().isFloat());
   REQUIRE(harness.stack.getConsoleValue().getFloat() == Catch::Approx(-3.0));
   REQUIRE(harness.stack.mStartStackSize == 0);
}

TEST_CASE_METHOD(TestVmFixture, "StringStack performUnaryOp uses the current value twice", "[StringStack]") {
   KorkApi::VmAllocTLS::Scope scope(internal);
   StackHarness harness(&internal->mTypes);

   harness.stack.setUnsignedValue(7);

   harness.stack.performUnaryOp(Compiler::OP_NEG, vm, internal->mTypes.data());

   REQUIRE(harness.stack.getConsoleValue().isFloat());
   REQUIRE(harness.stack.getConsoleValue().getFloat() == Catch::Approx(-7.0));
   REQUIRE(harness.stack.mStartStackSize == 0);
}
