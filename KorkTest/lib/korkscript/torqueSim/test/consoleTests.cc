//-----------------------------------------------------------------------------
// Copyright (c) 2026 korkscript contributors.
// See AUTHORS file and git repository for contributor information.
//
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------

#include "console/console.h"
#include "core/stringTable.h"
#include "platform/platformProcess.h"
#include "platform/platformString.h"

#include <catch2/catch.hpp>

#include <array>
#include <atomic>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

namespace
{
   static std::atomic<U32> gNextConsoleTestId{1};

   std::string uniqueName(const char* prefix)
   {
      return std::string(prefix) + "_" + std::to_string(gNextConsoleTestId.fetch_add(1));
   }

   struct CapturedLine
   {
      ConsoleLogEntry::Level level;
      std::string text;
   };

   void captureLine(U32 level, const char* text, void* userPtr)
   {
      auto* lines = static_cast<std::vector<CapturedLine>*>(userPtr);
      lines->push_back({ static_cast<ConsoleLogEntry::Level>(level), text ? text : "" });
   }

   std::string normalizePath(const std::filesystem::path& p)
   {
      return p.lexically_normal().generic_string();
   }
}

ConsoleFunction(consoleRuntimeFuncProbe, void, 1, 1, "")
{
}

TEST_CASE("Console consumer callbacks see severity and type", "[Console]") {
   std::vector<CapturedLine> lines;
   Con::addConsumer(captureLine, &lines);

   Con::printf("console consumer normal");
   Con::warnf(ConsoleLogEntry::Script, "console consumer warning");
   Con::errorf(ConsoleLogEntry::GUI, "console consumer error");

   Con::removeConsumer(captureLine, &lines);

   REQUIRE(lines.size() >= 3);
   REQUIRE(lines[lines.size() - 3].level == ConsoleLogEntry::Normal);
   REQUIRE(lines[lines.size() - 3].text.find("console consumer normal") != std::string::npos);
   REQUIRE(lines[lines.size() - 2].level == ConsoleLogEntry::Warning);
   REQUIRE(lines[lines.size() - 2].text.find("console consumer warning") != std::string::npos);
   REQUIRE(lines[lines.size() - 1].level == ConsoleLogEntry::Error);
   REQUIRE(lines[lines.size() - 1].text.find("console consumer error") != std::string::npos);
}

TEST_CASE("Console path expandos and path helpers", "[Console][Paths]") {
   const std::filesystem::path tempRoot = std::filesystem::temp_directory_path() / uniqueName("consolePaths");
   std::filesystem::create_directories(tempRoot);
   const std::string expandoName = uniqueName("ConsoleExpando");
   const std::filesystem::path expandoPath = tempRoot / "root" / "assets";
   std::filesystem::create_directories(expandoPath);
   const std::string normalizedExpandoPath = normalizePath(expandoPath);

   const U32 startCount = Con::getPathExpandoCount();
   Con::addPathExpando(expandoName.c_str(), (expandoPath.string() + "//").c_str());
   REQUIRE(Con::getPathExpandoCount() == startCount + 1);
   REQUIRE(Con::isPathExpando(expandoName.c_str()));
   REQUIRE(std::string(Con::getPathExpando(expandoName.c_str())) == normalizedExpandoPath);

   bool foundExpando = false;
   for (U32 i = 0; i < Con::getPathExpandoCount(); ++i)
   {
      const char* key = Con::getPathExpandoKey(i);
      const char* value = Con::getPathExpandoValue(i);
      if (key && std::string(key) == expandoName)
      {
         foundExpando = true;
         REQUIRE(value != nullptr);
         REQUIRE(std::string(value) == normalizedExpandoPath);
      }
   }
   REQUIRE(foundExpando);

   char expanded[1024];
   REQUIRE(Con::expandPath(expanded, sizeof(expanded), ("^" + expandoName + "//textures///diffuse.png").c_str()));
   REQUIRE(normalizePath(expanded) == normalizePath(expandoPath / "textures" / "diffuse.png"));

   char collapsed[1024];
   Con::collapsePath(collapsed, sizeof(collapsed), expanded);
   REQUIRE(std::string(collapsed) == "^" + expandoName + "/textures/diffuse.png");

   char cwdExpanded[1024];
   REQUIRE(Con::expandPath(cwdExpanded, sizeof(cwdExpanded), "levels//start.mis", tempRoot.generic_string().c_str()));
   REQUIRE(normalizePath(cwdExpanded) == normalizePath(tempRoot / "levels" / "start.mis"));

   char cwdCollapsed[1024];
   Con::collapsePath(cwdCollapsed, sizeof(cwdCollapsed), cwdExpanded, tempRoot.generic_string().c_str());
   REQUIRE(normalizePath(cwdCollapsed) == normalizePath(tempRoot / "levels" / "start.mis"));

   char ensured[1024];
   Con::ensureTrailingSlash(ensured, "alpha/beta");
   REQUIRE(std::string(ensured) == "alpha/beta/");

   char stripped[1024];
   REQUIRE(Con::stripRepeatSlashes(stripped, "alpha///beta//gamma", sizeof(stripped)));
   REQUIRE(std::string(stripped) == "alpha/beta/gamma");

   Con::removePathExpando(expandoName.c_str());
   REQUIRE_FALSE(Con::isPathExpando(expandoName.c_str()));
   REQUIRE(Con::getPathExpandoCount() == startCount);

   std::filesystem::remove_all(tempRoot);
}

TEST_CASE("Console script filename and module helpers", "[Console][Paths]") {
   char fileRelative[1024];
   REQUIRE(Con::expandScriptFilename(fileRelative, sizeof(fileRelative), "./child.cs", "mods/example/scripts/root.cs"));
   REQUIRE(std::string(fileRelative) == "mods/example/scripts/child.cs");

   char modRelative[1024];
   REQUIRE(Con::expandScriptFilename(modRelative, sizeof(modRelative), "~/child.cs", "mods/example/scripts/root.cs"));
   REQUIRE(std::string(modRelative) == "mods/child.cs");

   char passthrough[1024];
   REQUIRE(Con::expandScriptFilename(passthrough, sizeof(passthrough), "data/absolute.cs", nullptr));
   REQUIRE(std::string(passthrough) == "data/absolute.cs");

   REQUIRE(std::string(Con::getModNameFromPath("myMod/scripts/example.cs")) == "myMod");
   REQUIRE(Con::getModNameFromPath("example.cs") == nullptr);
   REQUIRE(Con::getModNameFromPath(nullptr) == nullptr);
}

TEST_CASE("Console global tab completion and variables", "[Console][TabComplete]") {
   const std::string varName = uniqueName("consoleRuntimeVar");
   const std::string varPrefix = "consoleRuntimeVar";
   const std::string funcPrefix = "consoleRuntimeFunc";
   const std::string funcName = "consoleRuntimeFuncProbe";

   REQUIRE(Con::isActive());
   REQUIRE(Con::isMainThread());

   Con::setVariable(varName.c_str(), "runtime-value");
   REQUIRE(std::string(Con::getVariable(varName.c_str())) == "runtime-value");

   char variableBuffer[128];
   std::snprintf(variableBuffer, sizeof(variableBuffer), "$%s", varPrefix.c_str());
   const U32 variableCursor = Con::tabComplete(variableBuffer, std::strlen(variableBuffer), sizeof(variableBuffer), true);
   REQUIRE(variableCursor == std::strlen(variableBuffer));
   REQUIRE(std::string(variableBuffer) == "$" + varName);

   char functionBuffer[128];
   std::snprintf(functionBuffer, sizeof(functionBuffer), "%s", funcPrefix.c_str());
   const U32 functionCursor = Con::tabComplete(functionBuffer, std::strlen(functionBuffer), sizeof(functionBuffer), true);
   REQUIRE(functionCursor == std::strlen(functionBuffer));
   REQUIRE(std::string(functionBuffer) == funcName);
   REQUIRE(Con::isFunction(funcName.c_str()));

   Con::removeVariable(varName.c_str());
}

TEST_CASE("Console color stripping removes control characters only", "[Console]") {
   char line[] = { char(0x01), 'A', char(0x02), 'B', '\t', 'C', '\n', 'D', '\r', 'E', char(0x0F), 'F', '\0' };
   Con::stripColorChars(line);
   const std::string stripped = line;
   REQUIRE(stripped.find(char(0x01)) == std::string::npos);
   REQUIRE(stripped.find(char(0x02)) == std::string::npos);
   REQUIRE(stripped.find(char(0x0F)) == std::string::npos);
   REQUIRE(stripped.find('\t') != std::string::npos);
   REQUIRE(stripped.find('\n') != std::string::npos);
   REQUIRE(stripped.find('\r') != std::string::npos);
}
