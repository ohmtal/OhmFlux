//-----------------------------------------------------------------------------
// Copyright (c) 2026 korkscript contributors.
// See AUTHORS file and git repository for contributor information.
//
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------

#include "core/unicode.h"

#include <catch2/catch.hpp>

#include <array>
#include <cstdlib>
#include <cstring>
#include <string>

namespace
{
   constexpr UTF32 ReplacementChar = 0xFFFD;
}

TEST_CASE("Unicode buffer conversions preserve BMP code points", "[unicode]") {
   const UTF8 utf8[] = "A\xE2\x82\xAC";

   // The implementation is BMP-only, so this round-trip should preserve ASCII and U+20AC.
   UTF16* utf16 = convertUTF8toUTF16(utf8);
   REQUIRE(utf16 != nullptr);
   REQUIRE(utf16[0] == UTF16('A'));
   REQUIRE(utf16[1] == UTF16(0x20AC));
   REQUIRE(utf16[2] == UTF16(0));

   UTF32* utf32 = convertUTF8toUTF32(utf8);
   REQUIRE(utf32 != nullptr);
   REQUIRE(utf32[0] == UTF32('A'));
   REQUIRE(utf32[1] == UTF32(0x20AC));
   REQUIRE(utf32[2] == UTF32(0));

   UTF8* utf8RoundTrip = convertUTF16toUTF8(utf16);
   REQUIRE(utf8RoundTrip != nullptr);
   REQUIRE(std::strcmp(reinterpret_cast<const char*>(utf8RoundTrip), reinterpret_cast<const char*>(utf8)) == 0);

   UTF32* utf32From16 = convertUTF16toUTF32(utf16);
   REQUIRE(utf32From16 != nullptr);
   REQUIRE(utf32From16[0] == UTF32('A'));
   REQUIRE(utf32From16[1] == UTF32(0x20AC));
   REQUIRE(utf32From16[2] == UTF32(0));

   UTF16* utf16From32 = convertUTF32toUTF16(utf32);
   REQUIRE(utf16From32 != nullptr);
   REQUIRE(utf16From32[0] == UTF16('A'));
   REQUIRE(utf16From32[1] == UTF16(0x20AC));
   REQUIRE(utf16From32[2] == UTF16(0));

   UTF8* utf8From32 = convertUTF32toUTF8(utf32);
   REQUIRE(utf8From32 != nullptr);
   REQUIRE(std::strcmp(reinterpret_cast<const char*>(utf8From32), reinterpret_cast<const char*>(utf8)) == 0);

   free(utf16);
   free(utf32);
   free(utf8RoundTrip);
   free(utf32From16);
   free(utf16From32);
   free(utf8From32);
}

TEST_CASE("Unicode converts unsupported code points to the replacement character", "[unicode]") {
   const UTF32 aboveBmp[] = { 0x110000, 0 };
   const UTF16 surrogatePair[] = { 0xD83D, 0xDE00, 0 };
   std::array<UTF8, 4> encoded{};

   // Values beyond the supported scalar range are intentionally collapsed to U+FFFD.
   REQUIRE(oneUTF32toUTF16(0x110000) == UTF16(ReplacementChar));
   REQUIRE(oneUTF32toUTF8(0x110000, encoded.data()) == 3);
   REQUIRE(std::memcmp(encoded.data(), "\xEF\xBF\xBD", 3) == 0);

   UTF16* collapsed16 = convertUTF32toUTF16(aboveBmp);
   REQUIRE(collapsed16[0] == UTF16(ReplacementChar));
   REQUIRE(collapsed16[1] == UTF16(0));

   // This decoder currently combines surrogate pairs into a BMP code point.
   UTF32* collapsed32 = convertUTF16toUTF32(surrogatePair);
   REQUIRE(collapsed32[0] == UTF32(0xF600));
   REQUIRE(collapsed32[1] == UTF32(0));

   UTF8* collapsed8 = convertUTF16toUTF8(surrogatePair);
   REQUIRE(std::strcmp(reinterpret_cast<const char*>(collapsed8), "\xEF\x98\x80") == 0);

   free(collapsed16);
   free(collapsed32);
   free(collapsed8);
}

TEST_CASE("Unicode one-codepoint helpers report consumed units", "[unicode]") {
   U32 walked = 0;
   const UTF8 utf8[] = "Z\xE2\x82\xAC";
   const UTF16 utf16[] = { UTF16('Z'), UTF16(0x20AC), 0 };

   REQUIRE(oneUTF8toUTF32(utf8, &walked) == UTF32('Z'));
   REQUIRE(walked == 1);
   REQUIRE(oneUTF8toUTF32(utf8 + 1, &walked) == UTF32(0x20AC));
   REQUIRE(walked == 3);

   REQUIRE(oneUTF16toUTF32(utf16, &walked) == UTF32('Z'));
   REQUIRE(walked == 1);
   REQUIRE(oneUTF16toUTF32(utf16 + 1, &walked) == UTF32(0x20AC));
   REQUIRE(walked == 1);

   std::array<UTF8, 4> encoded{};
   REQUIRE(oneUTF32toUTF8(0x20AC, encoded.data()) == 3);
   REQUIRE(std::memcmp(encoded.data(), "\xE2\x82\xAC", 3) == 0);
}

TEST_CASE("Unicode string length and comparison helpers behave as expected", "[unicode]") {
   const UTF16 utf16A[] = { UTF16('a'), UTF16('b'), UTF16('c'), 0 };
   const UTF16 utf16B[] = { UTF16('a'), UTF16('b'), UTF16('d'), 0 };
   const UTF32 utf32A[] = { UTF32('a'), UTF32('b'), UTF32('c'), 0 };
   const UTF32 utf32B[] = { UTF32('a'), UTF32('b'), UTF32('d'), 0 };

   REQUIRE(dStrlen(utf16A) == 3);
   REQUIRE(dStrlen(utf32A) == 3);
   REQUIRE(dStrncmp(utf16A, utf16A, 3) == 0);
   REQUIRE(dStrncmp(utf32A, utf32A, 3) == 0);
   REQUIRE(dStrncmp(utf16A, utf16B, 3) != 0);
   REQUIRE(dStrncmp(utf32A, utf32B, 3) != 0);
}

TEST_CASE("Unicode BOM helpers identify supported encodings", "[unicode]") {
   U8 utf8Bom[4] = { 0xEF, 0xBB, 0xBF, 0x41 };
   U8 utf16Bom[4] = { 0xFE, 0xFF, 0x00, 0x41 };
   U8 utf32Bom[4] = { 0xFF, 0xFE, 0x00, 0x00 };
   U8 invalidBom[4] = { 0x01, 0x02, 0x03, 0x04 };
   const char* bomName = nullptr;

   REQUIRE(isValidUTF8BOM(utf8Bom, &bomName));
   REQUIRE(std::string(bomName) == "UTF8");

   REQUIRE_FALSE(isValidUTF8BOM(utf16Bom, &bomName));
   REQUIRE(std::string(bomName) == "UTF16");

   REQUIRE_FALSE(isValidUTF8BOM(utf32Bom, &bomName));
   REQUIRE(std::string(bomName) == "UTF32");

   REQUIRE_FALSE(isValidUTF8BOM(invalidBom, nullptr));
   REQUIRE_FALSE(isValidUTF8BOM(invalidBom, &bomName));
   REQUIRE(bomName == nullptr);

   char* stripped = nullptr;
   REQUIRE(chompUTF8BOM(reinterpret_cast<const char*>(utf8Bom), &stripped));
   REQUIRE(stripped == reinterpret_cast<char*>(utf8Bom) + 3);
   REQUIRE_FALSE(chompUTF8BOM(nullptr, &stripped));
   REQUIRE_FALSE(chompUTF8BOM(reinterpret_cast<const char*>(utf8Bom), nullptr));
}

TEST_CASE("Unicode getNthCodepoint advances by code points rather than bytes", "[unicode]") {
   const UTF8 text[] = { UTF8('A'), UTF8(0xE2), UTF8(0x82), UTF8(0xAC), UTF8('B'), UTF8(0) };

   REQUIRE(getNthCodepoint(text, 0) == text);
   REQUIRE(getNthCodepoint(text, 1) == text + 1);
   REQUIRE(getNthCodepoint(text, 2) == text + 4);
   REQUIRE(getNthCodepoint(text, 3) == text + 5);
}
