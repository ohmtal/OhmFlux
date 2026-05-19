//-----------------------------------------------------------------------------
// Copyright (c) 2026 korkscript contributors.
// See AUTHORS file and git repository for contributor information.
//
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------

#include "platform/platform.h"
#include "console/stlTypes.h"
#include "embed/api.h"
#include "embed/internalApi.h"
#include "console/compiler.h"
#include "core/memStream.h"
#include "engine/test/testVmFixture.h"

#include <catch2/catch.hpp>

#include <string>
#include <vector>

namespace
{
   template <class T>
   std::vector<U8> writeToBuffer(T& table)
   {
      std::vector<U8> buffer(1024);
      MemStream stream((U32)buffer.size(), buffer.data(), true, true);
      table.write(stream);
      buffer.resize(stream.getPosition());
      return buffer;
   }
}

TEST_CASE_METHOD(TestVmFixture, "Compiler string table dedupes and serializes entries", "[Compiler][Tables]") {
   KorkApi::VmAllocTLS::Scope scope(internal);

   Compiler::CompilerStringTable table(internal->mCompilerResources);

   const U32 alpha = table.add("Alpha");
   const U32 alphaRepeat = table.add("Alpha");
   const U32 alphaCaseFold = table.add("alpha", false);
   const U32 alphaTagged = table.add("AlphaTag", true, true);
   const U32 number = table.addIntString(42);
   const U32 floating = table.addFloatString(3.5);

   REQUIRE(alpha == 0);
   REQUIRE(alphaRepeat == alpha);
   REQUIRE(alphaCaseFold == alpha);
   REQUIRE(alphaTagged != alpha);
   REQUIRE(number > alphaTagged);
   REQUIRE(floating > number);
   REQUIRE(table.totalLen > 0);

   const std::vector<U8> serialized = writeToBuffer(table);
   REQUIRE(serialized.size() > sizeof(U32));

   MemStream stream((U32)serialized.size(), const_cast<U8*>(serialized.data()), true, false);
   U32 totalLen = 0;
   REQUIRE(stream.read(&totalLen));
   REQUIRE(totalLen == table.totalLen);

   char* built = table.build();
   REQUIRE(built != nullptr);
   REQUIRE(std::string(built + alpha) == "Alpha");
   REQUIRE(std::string(built + alphaTagged) == "AlphaTag");
   REQUIRE(std::string(built + number) == "42");
   REQUIRE(std::string(built + floating) == "3.5");
   KorkApi::VMem::DeleteArray(built);
}

TEST_CASE_METHOD(TestVmFixture, "Compiler float table dedupes values and tracks insertion order", "[Compiler][Tables]") {
   KorkApi::VmAllocTLS::Scope scope(internal);

   Compiler::CompilerFloatTable table(internal->mCompilerResources);

   const U32 first = table.add(1.25);
   const U32 repeat = table.add(1.25);
   const U32 second = table.add(7.5);

   REQUIRE(first == 0);
   REQUIRE(repeat == first);
   REQUIRE(second == 1);
   REQUIRE(table.count == 2);

   F64* values = table.build();
   REQUIRE(values != nullptr);
   REQUIRE(values[0] == Catch::Approx(1.25));
   REQUIRE(values[1] == Catch::Approx(7.5));
   KorkApi::VMem::DeleteArray(values);

   const std::vector<U8> serialized = writeToBuffer(table);
   MemStream stream((U32)serialized.size(), const_cast<U8*>(serialized.data()), true, false);
   U32 count = 0;
   REQUIRE(stream.read(&count));
   REQUIRE(count == 2);

   F64 a = 0.0;
   F64 b = 0.0;
   REQUIRE(stream.read(&a));
   REQUIRE(stream.read(&b));
   REQUIRE(a == Catch::Approx(1.25));
   REQUIRE(b == Catch::Approx(7.5));
}

TEST_CASE_METHOD(TestVmFixture, "Compiler ident table tracks names and appends tables", "[Compiler][Tables]") {
   KorkApi::VmAllocTLS::Scope scope(internal);

   Compiler::Resources* res = internal->mCompilerResources;
   res->resetTables();

   Compiler::CompilerIdentTable table(res);

   const StringTableEntry alpha = vm->internString("alpha", false);
   const StringTableEntry beta = vm->internString("beta", false);
   const StringTableEntry gamma = vm->internString("gamma", false);
   const StringTableEntry delta = vm->internString("delta", false);

   REQUIRE(table.add(alpha, 10) == 0);
   REQUIRE(table.add(alpha, 20) == 0);
   REQUIRE(table.add(beta, 30) == 1);
   REQUIRE(table.addNoAddress(gamma) == 2);
   REQUIRE(table.numIdentStrings == 3);

   StringTableEntry* strings = nullptr;
   U32* stringOffsets = nullptr;
   U32 count = 0;
   table.build(&strings, &stringOffsets, &count);

   REQUIRE(count == 3);
   REQUIRE(strings[0] == alpha);
   REQUIRE(strings[1] == beta);
   REQUIRE(strings[2] == gamma);
   REQUIRE(stringOffsets[0] == 0);
   REQUIRE(stringOffsets[1] > stringOffsets[0]);
   REQUIRE(stringOffsets[2] > stringOffsets[1]);
   KorkApi::VMem::DeleteArray(strings);
   KorkApi::VMem::DeleteArray(stringOffsets);

   const std::vector<U8> serialized = writeToBuffer(table);
   MemStream stream((U32)serialized.size(), const_cast<U8*>(serialized.data()), true, false);

   U32 identCount = 0;
   REQUIRE(stream.read(&identCount));
   REQUIRE(identCount == 3);

   U32 offset = 0;
   U32 patchCount = 0;
   U32 ip = 0;

   REQUIRE(stream.read(&offset));
   REQUIRE(stream.read(&patchCount));
   REQUIRE(offset == 0);
   REQUIRE(patchCount == 2);
   REQUIRE(stream.read(&ip));
   REQUIRE(ip == 20);
   REQUIRE(stream.read(&ip));
   REQUIRE(ip == 10);

   REQUIRE(stream.read(&offset));
   REQUIRE(stream.read(&patchCount));
   REQUIRE(offset > 0);
   REQUIRE(patchCount == 1);
   REQUIRE(stream.read(&ip));
   REQUIRE(ip == 30);

   REQUIRE(stream.read(&offset));
   REQUIRE(stream.read(&patchCount));
   REQUIRE(offset > 0);
   REQUIRE(patchCount == 0);

   Compiler::CompilerIdentTable extra(res);
   REQUIRE(extra.add(delta, 40) == 0);

   REQUIRE(table.append(extra) == 3);
   REQUIRE(table.numIdentStrings == 4);

   table.build(&strings, &stringOffsets, &count);
   REQUIRE(count == 4);
   REQUIRE(strings[3] == delta);
   KorkApi::VMem::DeleteArray(strings);
   KorkApi::VMem::DeleteArray(stringOffsets);
}
