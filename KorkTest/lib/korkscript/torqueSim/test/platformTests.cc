//-----------------------------------------------------------------------------
// Copyright (c) 2026 korkscript contributors.
// See AUTHORS file and git repository for contributor information.
//
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------

#include "platform/platform.h"
#include "platform/platformProcess.h"
#include "core/stringTable.h"

#include <catch2/catch.hpp>

#include <filesystem>
#include <string>

namespace
{
}

TEST_CASE("Platform working directory helpers round-trip", "[Platform]") {
#ifdef TORQUE_USE_STD_FILESYSTEM
   const std::filesystem::path tempRoot = std::filesystem::temp_directory_path() / "korkscript-working-directory";
   std::filesystem::create_directories(tempRoot);
   const std::string previous = Platform::getWorkingDirectory() ? Platform::getWorkingDirectory() : "";

   REQUIRE(Platform::setWorkingDirectory(StringTable->insert(tempRoot.generic_string().c_str())));
   REQUIRE(Platform::getWorkingDirectory() != nullptr);
   REQUIRE(Platform::getCurrentDirectory() != nullptr);
   REQUIRE(std::filesystem::equivalent(std::filesystem::path(Platform::getWorkingDirectory()), tempRoot));
   REQUIRE(std::filesystem::equivalent(std::filesystem::path(Platform::getCurrentDirectory()), tempRoot));

   REQUIRE(Platform::setWorkingDirectory(StringTable->insert(previous.c_str())));
   REQUIRE(std::filesystem::exists(tempRoot));
   std::filesystem::remove_all(tempRoot);
#else
   REQUIRE(Platform::getWorkingDirectory() == nullptr);
   REQUIRE_FALSE(Platform::setWorkingDirectory("ignored"));
#endif
}

TEST_CASE("Platform working directory rejects invalid input", "[Platform]") {
#ifdef TORQUE_USE_STD_FILESYSTEM
   REQUIRE_FALSE(Platform::setWorkingDirectory(nullptr));
   REQUIRE_FALSE(Platform::setWorkingDirectory(""));
#else
   REQUIRE_FALSE(Platform::setWorkingDirectory(nullptr));
   REQUIRE_FALSE(Platform::setWorkingDirectory(""));
#endif
}
