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


class FluxFile {
public:
    // Loads a file into a vector of strings
    static bool LoadTextFile(const std::string& path, std::vector<std::string>& outLines) {
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
    static bool SaveTextFile(const std::string& path, const std::vector<std::string>& lines) {
        SDL_IOStream* io = SDL_IOFromFile(path.c_str(), "w");
        if (!io) return false;

        for (const auto& line : lines) {
            std::string lineWithNewline = line + "\n";
            SDL_WriteIO(io, lineWithNewline.c_str(), lineWithNewline.size());
        }

        SDL_CloseIO(io);
        return true;
    }



    bool SaveBufferToFile(const char* filename, const uint8_t* data, size_t size) {
        // Desktop: Standard file write
        FILE* file = fopen(filename, "wb");
        if (!file) return false;
        fwrite(data, 1, size, file);
        fclose(file);
        return true;
    }
};


