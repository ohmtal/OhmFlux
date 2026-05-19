//-----------------------------------------------------------------------------
// Copyright (c) 2026 korkscript contributors.
// See AUTHORS file and git repository for contributor information.
//
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------

#include "console/console.h"
#include "console/consoleLogger.h"
#include "platform/platformProcess.h"

#include <catch2/catch.hpp>

#include <atomic>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

namespace
{
   namespace fs = std::filesystem;

   static std::atomic<U32> gNextConsoleLoggerTestId{0};

   fs::path makeLogPath(const char* prefix)
   {
      fs::path tempDir;
      if (const char* platformTempDir = Platform::getTemporaryDirectory())
         tempDir = fs::path(platformTempDir);
      else
      {
         std::error_code ec;
         tempDir = fs::temp_directory_path(ec);
         if (ec)
            tempDir = fs::current_path(ec);
      }

      return tempDir / (std::string("korkscript_console_logger_") + prefix + "_" + std::to_string(gNextConsoleLoggerTestId.fetch_add(1)) + ".log");
   }

   std::string pathString(const fs::path& path)
   {
      return path.string();
   }

   void writeTextFile(const fs::path& path, const std::string& contents)
   {
      std::ofstream out(path, std::ios::binary | std::ios::trunc);
      REQUIRE(out.good());
      out << contents;
      REQUIRE(out.good());
   }

   std::string readTextFile(const fs::path& path)
   {
      std::ifstream in(path, std::ios::binary);
      REQUIRE(in.good());
      std::ostringstream buffer;
      buffer << in.rdbuf();
      return buffer.str();
   }

   bool fileContains(const fs::path& path, const std::string& needle)
   {
      return readTextFile(path).find(needle) != std::string::npos;
   }
}

TEST_CASE("ConsoleLogger defaults and attach preconditions", "[ConsoleLogger]") {
   ConsoleLogger logger;

   REQUIRE(logger.mLevel == ConsoleLogEntry::Normal);
   REQUIRE_FALSE(logger.attach());
   REQUIRE_FALSE(logger.detach());
   REQUIRE_FALSE(logger.processArguments(0, nullptr));
}

TEST_CASE("ConsoleLogger routes messages by level", "[ConsoleLogger]") {
   const fs::path normalPath = makeLogPath("normal");
   const fs::path errorPath = makeLogPath("error");

   writeTextFile(normalPath, "stale-normal\n");
   writeTextFile(errorPath, "stale-error\n");

   const std::string normalPathString = pathString(normalPath);
   const std::string errorPathString = pathString(errorPath);
   ConsoleLogger normalLogger(normalPathString.c_str(), false);
   ConsoleLogger errorLogger(errorPathString.c_str(), false);
   normalLogger.mLevel = ConsoleLogEntry::Normal;
   errorLogger.mLevel = ConsoleLogEntry::Error;

   REQUIRE(normalLogger.attach());
   REQUIRE(errorLogger.attach());
   REQUIRE_FALSE(normalLogger.attach());
   REQUIRE_FALSE(errorLogger.attach());

   Con::printf("console logger normal message");
   Con::warnf("console logger warning message");
   Con::errorf("console logger error message");

   REQUIRE(normalLogger.detach());
   REQUIRE(errorLogger.detach());
   REQUIRE_FALSE(normalLogger.detach());
   REQUIRE_FALSE(errorLogger.detach());

   REQUIRE(fileContains(normalPath, "console logger normal message"));
   REQUIRE(fileContains(normalPath, "console logger warning message"));
   REQUIRE(fileContains(normalPath, "console logger error message"));
   REQUIRE_FALSE(fileContains(normalPath, "stale-normal"));

   REQUIRE_FALSE(fileContains(errorPath, "console logger normal message"));
   REQUIRE_FALSE(fileContains(errorPath, "console logger warning message"));
   REQUIRE(fileContains(errorPath, "console logger error message"));
   REQUIRE_FALSE(fileContains(errorPath, "stale-error"));

   fs::remove(normalPath);
   fs::remove(errorPath);
}

TEST_CASE("ConsoleLogger processArguments appends when requested", "[ConsoleLogger]") {
   const fs::path logPath = makeLogPath("append");
   writeTextFile(logPath, "seed-line\n");

   ConsoleLogger logger;
   const std::string logPathString = pathString(logPath);
   const char* argv[] = {logPathString.c_str(), "true"};
   REQUIRE(logger.processArguments(2, argv));

   Con::printf("appended line");
   REQUIRE(logger.detach());

   const std::string contents = readTextFile(logPath);
   REQUIRE(contents.find("seed-line") != std::string::npos);
   REQUIRE(contents.find("appended line") != std::string::npos);

   fs::remove(logPath);
}

TEST_CASE("ConsoleLogger destructor detaches automatically", "[ConsoleLogger]") {
   const fs::path logPath = makeLogPath("destructor");

   {
      const std::string logPathString = pathString(logPath);
      ConsoleLogger logger(logPathString.c_str(), false);
      REQUIRE(logger.attach());
      Con::printf("before destructor");
   }

   Con::printf("after destructor");

   const std::string contents = readTextFile(logPath);
   REQUIRE(contents.find("before destructor") != std::string::npos);
   REQUIRE(contents.find("after destructor") == std::string::npos);

   fs::remove(logPath);
}
