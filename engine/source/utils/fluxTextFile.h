//-----------------------------------------------------------------------------
// Copyright (c) 2025 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Example:
//
// char* prefPath = SDL_GetPrefPath(game->mSettings.Company, game->mSettings.Caption ) ;
// std::string savePath = std::string(prefPath) + "savegame.txt";
//
// FluxTextFile::Save(savePath, myLines);
//
// SDL_free(prefPath); // Always free the prefPath string
//-----------------------------------------------------------------------------

#pragma once

#include <SDL3/SDL.h>
#include <string>
#include <vector>
#include <sstream>

class FluxTextFile {
public:
    // Loads a file into a vector of strings
    static bool Load(const std::string& path, std::vector<std::string>& outLines) {
        size_t size;
        void* data = SDL_LoadFile(path.c_str(), &size);
        if (!data) return false;

        std::string content(static_cast<char*>(data), size);
        SDL_free(data);

        std::stringstream ss(content);
        std::string line;
        while (std::getline(ss, line)) {
            // Handle Windows \r\n endings if necessary
            if (!line.empty() && line.back() == '\r') line.pop_back();
            outLines.push_back(line);
        }
        return true;
    }

    // Saves a vector of strings
    static bool Save(const std::string& path, const std::vector<std::string>& lines) {
        SDL_IOStream* io = SDL_IOFromFile(path.c_str(), "w");
        if (!io) return false;

        for (const auto& line : lines) {
            std::string lineWithNewline = line + "\n";
            SDL_WriteIO(io, lineWithNewline.c_str(), lineWithNewline.size());
        }

        SDL_CloseIO(io);
        return true;
    }
};


// #include <fstream>
// #include <string>
// #include <vector>
//
// class FluxTextFile {
// public:
//     // Saves a 2D-style vector of strings (rows) to a file
//     static bool Save(const std::string& path, const std::vector<std::string>& lines) {
//         std::ofstream file(path);
//         if (!file.is_open()) return false;
//         for (const auto& line : lines) {
//             file << line << "\n";
//         }
//         return true;
//     }
//
//     // Loads a file into a vector of strings
//     static bool Load(const std::string& path, std::vector<std::string>& outLines) {
//         std::ifstream file(path);
//         if (!file.is_open()) return false;
//         std::string line;
//         while (std::getline(file, line)) {
//             outLines.push_back(line);
//         }
//         return true;
//     }
// };
