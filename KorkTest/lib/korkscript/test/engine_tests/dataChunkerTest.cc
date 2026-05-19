//-----------------------------------------------------------------------------
// Copyright (c) 2023 tgemit contributors.
// Copyright (c) 2026 korkscript contributors.
// See AUTHORS file and git repository for contributor information.
//
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------

#include "platform/platform.h"
#include "core/dataChunker.h"
#include <vector>
#include <catch2/catch.hpp>

struct TestClassChunkerStruct
{
   U32 value;
   U32 value2;

   TestClassChunkerStruct()
   {
      value  = 0xC001B33F;
      value2 = 0x10101010;
   }
   
   ~TestClassChunkerStruct()
   {
      value  = 0;
      value2 = 0;
   }
};


TEST_CASE("BaseDataChunker should function correctly", "[BaseDataChunker]") {
   BaseDataChunker<TestClassChunkerStruct> testChunks(1024);
   BaseDataChunker<U32> testChunk4(1024);
   BaseDataChunker<U64> testChunk8(1024);

   REQUIRE(testChunks.countUsedBlocks() == 0);
   REQUIRE(testChunk4.countUsedBlocks() == 0);
   REQUIRE(testChunk8.countUsedBlocks() == 0);

   testChunks.alloc(1);
   testChunk4.alloc(1);
   testChunk8.alloc(1);

   REQUIRE(testChunks.countUsedBlocks() == 1);
   REQUIRE(testChunk4.countUsedBlocks() == 1);
   REQUIRE(testChunk8.countUsedBlocks() == 1);

   testChunks.alloc(1);
   testChunk4.alloc(1);
   testChunk8.alloc(1);

   REQUIRE(testChunks.countUsedBlocks() == 1);
   REQUIRE(testChunk4.countUsedBlocks() == 1);
   REQUIRE(testChunk8.countUsedBlocks() == 1);

   REQUIRE(testChunks.countUsedBytes() == (sizeof(TestClassChunkerStruct) * 2));
   REQUIRE(testChunk4.countUsedBytes() == (sizeof(U32) * 2));
   REQUIRE(testChunk8.countUsedBytes() == (sizeof(U64) * 2));

   testChunks.freeBlocks(true);
   testChunk4.freeBlocks(true);
   testChunk8.freeBlocks(true);
   
   REQUIRE(testChunks.countUsedBlocks() == 1);
   REQUIRE(testChunk4.countUsedBlocks() == 1);
   REQUIRE(testChunk8.countUsedBlocks() == 1);
   
   testChunks.freeBlocks(false);
   testChunk4.freeBlocks(false);
   testChunk8.freeBlocks(false);
   
   REQUIRE(testChunks.countUsedBlocks() == 0);
   REQUIRE(testChunk4.countUsedBlocks() == 0);
   REQUIRE(testChunk8.countUsedBlocks() == 0);

   testChunks.setChunkSize(sizeof(TestClassChunkerStruct));
   testChunks.alloc(1);
   REQUIRE(testChunks.countUsedBlocks() == 1);
   testChunks.alloc(1);
   REQUIRE(testChunks.countUsedBlocks() == 2);
}

TEST_CASE("DataChunker should function correctly", "[DataChunker]") {
   DataChunker<> testChunk(1024);

   testChunk.alloc(1024);

   REQUIRE(testChunk.countUsedBlocks() == 1);

   testChunk.alloc(1024);

   REQUIRE(testChunk.countUsedBlocks() == 2);

   testChunk.alloc(4096);
   
   REQUIRE(testChunk.countUsedBytes() == (1024+1024+4096));

   REQUIRE(testChunk.countUsedBlocks() == 3);

   testChunk.alloc(12);

   REQUIRE(testChunk.countUsedBlocks() == 4);

   testChunk.alloc(12);

   REQUIRE(testChunk.countUsedBlocks() == 4);
   
   U32 reqEls = AlignedBufferAllocator<uintptr_t>::calcRequiredElementSize(12) * sizeof(uintptr_t);
   
   REQUIRE(testChunk.countUsedBytes() == (1024+1024+4096+reqEls+reqEls));

   testChunk.freeBlocks(true);
   REQUIRE(testChunk.countUsedBlocks() == 1);
   testChunk.freeBlocks(false);
   REQUIRE(testChunk.countUsedBlocks() == 0);

   // Large block cases

   testChunk.alloc(8192);
   REQUIRE(testChunk.countUsedBlocks() == 1);
   testChunk.freeBlocks(true);
   REQUIRE(testChunk.countUsedBlocks() == 1);

   testChunk.alloc(8192);
   testChunk.alloc(1024);
   REQUIRE(testChunk.countUsedBlocks() == 2);
   testChunk.freeBlocks(true);
   REQUIRE(testChunk.countUsedBlocks() == 1);
   testChunk.freeBlocks(false);
   REQUIRE(testChunk.countUsedBlocks() == 0);
   
   // Instead using the chunk size
   
   for (U32 i=0; i<8; i++)
   {
      testChunk.alloc(1024);
   }
   REQUIRE(testChunk.countUsedBlocks() == 8);
   testChunk.freeBlocks(false);
   REQUIRE(testChunk.countUsedBlocks() == 0);
}

TEST_CASE("ChunkerFreeClassList should function correctly", "[ChunkerFreeClassList]") {
   TestClassChunkerStruct list[5];
   ChunkerFreeClassList<TestClassChunkerStruct> freeListTest;
   
   // Push & pop works as expected
   REQUIRE(freeListTest.isEmpty() == true);
   freeListTest.push((ChunkerFreeClassList<TestClassChunkerStruct>*)&list[0]);
   REQUIRE(freeListTest.isEmpty() == false);
   freeListTest.push((ChunkerFreeClassList<TestClassChunkerStruct>*)&list[4]);
   REQUIRE(freeListTest.pop() == &list[4]);
   REQUIRE(freeListTest.pop() == &list[0]);
   REQUIRE(freeListTest.pop() == NULL);
   
   // Reset clears list head
   freeListTest.push((ChunkerFreeClassList<TestClassChunkerStruct>*)&list[4]);
   freeListTest.reset();
   REQUIRE(freeListTest.pop() == NULL);
}


TEST_CASE("FreeListChunker should function correctly", "[FreeListChunker]") {
   FreeListChunker<TestClassChunkerStruct> testFreeList;
   
   TestClassChunkerStruct* s1 = testFreeList.alloc();
   TestClassChunkerStruct* s2 = testFreeList.alloc();
   
   // Allocation is sequential
   REQUIRE(s2 > s1);
   REQUIRE(((s2 - s1) == 1));
   
   testFreeList.free(s1);
   
   // But previous reallocations are reused
   TestClassChunkerStruct* s3 = testFreeList.alloc();
   TestClassChunkerStruct* s4 = testFreeList.alloc();
   
   REQUIRE(s1 == s3);
   REQUIRE(((s4 - s2) == 1)); // continues from previous free alloc
   
   // Check sharing
   
   FreeListChunker<TestClassChunkerStruct> sharedChunker(testFreeList.getChunker());
   
   s2 = testFreeList.alloc();
   REQUIRE(((s2 - s4) == 1));
}

TEST_CASE("ClassChunker should function correctly", "[ClassChunker]") {
   ClassChunker<TestClassChunkerStruct> testClassList;
   
   TestClassChunkerStruct* s1 = testClassList.alloc();
   TestClassChunkerStruct* s2 = testClassList.alloc();
   
   // Allocation is sequential
   REQUIRE(s2 > s1);
   REQUIRE(((s2 - s1) == 1));
   
   testClassList.free(s1);
   REQUIRE(s1->value == 0);
   REQUIRE(s1->value2 == 0);
   
   // But previous reallocations are reused
   TestClassChunkerStruct* s3 = testClassList.alloc();
   TestClassChunkerStruct* s4 = testClassList.alloc();
   
   REQUIRE(s1 == s3);
   REQUIRE(((s4 - s2) == 1)); // continues from previous free alloc
   
   // Values should be initialized correctly for all allocs at this point
   REQUIRE(s1->value == 0xC001B33F);
   REQUIRE(s1->value2 == 0x10101010);
   REQUIRE(s2->value == 0xC001B33F);
   REQUIRE(s2->value2 == 0x10101010);
   REQUIRE(s3->value == 0xC001B33F);
   REQUIRE(s3->value2 == 0x10101010);
   REQUIRE(s4->value == 0xC001B33F);
   REQUIRE(s4->value2 == 0x10101010);
   
   // Should still be valid if using freeBlocks
   testClassList.freeBlocks(true);
   REQUIRE(s1->value == 0xC001B33F);
   REQUIRE(s1->value2 == 0x10101010);
   REQUIRE(s2->value == 0xC001B33F);
   REQUIRE(s2->value2 == 0x10101010);
   REQUIRE(s3->value == 0xC001B33F);
   REQUIRE(s3->value2 == 0x10101010);
   REQUIRE(s4->value == 0xC001B33F);
   REQUIRE(s4->value2 == 0x10101010);
}
