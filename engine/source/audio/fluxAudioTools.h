//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#pragma once

#include "SDL3/SDL.h"
#include "utils/errorlog.h"

namespace FluxAudio {

    bool saveWavFile(const std::string& filename, const std::vector<float>& data, int sampleRate)  {
        // Open the file for writing using SDL3's IO system
        SDL_IOStream* io = SDL_IOFromFile(filename.c_str(), "wb");
        if (!io) {
            LogFMT("ERROR:Failed to open file for writing: %s", SDL_GetError());
            return false;
        }

        uint32_t numChannels = 2;
        uint32_t bitsPerSample = 32; // 32 bits for float
        uint32_t dataSize = (uint32_t)(data.size() * sizeof(float));
        uint32_t fileSize = 36 + dataSize;
        uint32_t byteRate = sampleRate * numChannels * (bitsPerSample / 8);
        uint16_t blockAlign = (uint16_t)(numChannels * (bitsPerSample / 8));

        // Write the WAV Header
        SDL_WriteIO(io, "RIFF", 4);
        SDL_WriteU32LE(io, fileSize);
        SDL_WriteIO(io, "WAVE", 4);
        SDL_WriteIO(io, "fmt ", 4);
        SDL_WriteU32LE(io, 16);

        // IMPORTANT: AudioFormat 3 is IEEE Float (instead of 1 for PCM)
        SDL_WriteU16LE(io, 3);

        SDL_WriteU16LE(io, (uint16_t)numChannels);
        SDL_WriteU32LE(io, (uint32_t)sampleRate);
        SDL_WriteU32LE(io, byteRate);
        SDL_WriteU16LE(io, blockAlign);
        SDL_WriteU16LE(io, (uint16_t)bitsPerSample);

        SDL_WriteIO(io, "data", 4);
        SDL_WriteU32LE(io, dataSize);

        // Write the raw float data directly - no conversion needed!
        SDL_WriteIO(io, data.data(), dataSize);

        SDL_CloseIO(io);
        LogFMT("Successfully exported WAV {}", filename);

        return true;
    }


}
