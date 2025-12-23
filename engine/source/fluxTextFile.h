//-----------------------------------------------------------------------------
// Copyright (c) 2025 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#pragma once
#include <fstream>
#include <string>
#include <vector>

class FluxTextFile {
public:
    // Saves a 2D-style vector of strings (rows) to a file
    static bool Save(const std::string& path, const std::vector<std::string>& lines) {
        std::ofstream file(path);
        if (!file.is_open()) return false;
        for (const auto& line : lines) {
            file << line << "\n";
        }
        return true;
    }

    // Loads a file into a vector of strings
    static bool Load(const std::string& path, std::vector<std::string>& outLines) {
        std::ifstream file(path);
        if (!file.is_open()) return false;
        std::string line;
        while (std::getline(file, line)) {
            outLines.push_back(line);
        }
        return true;
    }
};
