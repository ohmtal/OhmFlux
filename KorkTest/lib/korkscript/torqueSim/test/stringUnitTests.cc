//-----------------------------------------------------------------------------
// Copyright (c) 2026 korkscript contributors.
// See AUTHORS file and git repository for contributor information.
//
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------

#include "core/stringUnit.h"

#include <catch2/catch.hpp>

#include <array>
#include <string>

TEST_CASE("StringUnit counts units across delimiter sets", "[StringUnit]") {
   REQUIRE(StringUnit::getUnitCount("", " ") == 0);
   REQUIRE(StringUnit::getUnitCount("alpha beta gamma", " ") == 3);
   REQUIRE(StringUnit::getUnitCount("alpha\tbeta\ngamma", " \t\n") == 3);
   REQUIRE(StringUnit::getUnitCount("single", " ") == 1);
}

TEST_CASE("StringUnit extracts individual units", "[StringUnit]") {
   std::array<char, 32> buffer{};

   // Use the caller-supplied buffer so the test does not depend on the shared static return buffer.
   REQUIRE(std::string(StringUnit::getUnit("red,green,blue", 0, ",", buffer.data(), buffer.size())) == "red");
   REQUIRE(std::string(StringUnit::getUnit("red,green,blue", 1, ",", buffer.data(), buffer.size())) == "green");
   REQUIRE(std::string(StringUnit::getUnit("red,green,blue", 2, ",", buffer.data(), buffer.size())) == "blue");
   REQUIRE(std::string(StringUnit::getUnit("red,green,blue", 3, ",", buffer.data(), buffer.size())).empty());

   // Whitespace-delimited strings are used heavily by the console helpers, so cover that common case too.
   REQUIRE(std::string(StringUnit::getUnit("alpha beta\tgamma", 1, " \t", buffer.data(), buffer.size())) == "beta");
}

TEST_CASE("StringUnit extracts unit ranges", "[StringUnit]") {
   REQUIRE(std::string(StringUnit::getUnits("one two three four", 1, 2, " ")) == "two three");
   REQUIRE(std::string(StringUnit::getUnits("one,two,three", 0, 0, ",")) == "one");
   REQUIRE(std::string(StringUnit::getUnits("one,two,three", 2, 1, ",")) .empty());
   REQUIRE(std::string(StringUnit::getUnits("one,two,three", 4, 5, ",")).empty());
}

TEST_CASE("StringUnit sets units in place-like copies", "[StringUnit]") {
   // Replacing a unit preserves the surrounding units and delimiter layout.
   REQUIRE(std::string(StringUnit::setUnit("alpha beta gamma", 1, "delta", " ")) == "alpha delta gamma");
   REQUIRE(std::string(StringUnit::setUnit("alpha beta gamma", 0, "omega", " ")) == "omega beta gamma");

   // Setting beyond the end pads with the delimiter before appending the new unit.
   REQUIRE(std::string(StringUnit::setUnit("alpha beta", 3, "omega", " ")) == "alpha beta  omega");
}

TEST_CASE("StringUnit removes units", "[StringUnit]") {
   REQUIRE(std::string(StringUnit::removeUnit("alpha beta gamma", 1, " ")) == "alpha gamma");
   REQUIRE(std::string(StringUnit::removeUnit("alpha beta gamma", 2, " ")) == "alpha beta");
   REQUIRE(std::string(StringUnit::removeUnit("alpha beta gamma", 5, " ")) == "alpha beta gamma");
   REQUIRE(std::string(StringUnit::removeUnit("solo", 0, " ")) .empty());
}
