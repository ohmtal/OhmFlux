//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// STATUS: WIP
// New AudioResourceManager - i need to rename this because i already have a manager
// NO merge it with fluxAudio.h !
//  - Using AudioInstance to seperate data from playing and creating
//    a soundprofile and adding playing multiple instances
//  - implementing WAV/OGG/MP3
//  -
//-----------------------------------------------------------------------------
/*
 *  1. Initial
 *      [ ] cleanup and add AudioManager
 *      [ ] load raw data to vector and detect filetype
 *
 *  2. Action
 *      [ ] implement decoders
 *      [ ] add position panning
 *
 *
 *
 *
 *
 */


#pragma once
#include <SDL3/SDL.h>
#include <unordered_map>

#include <fstream>
#include <vector>
#include <string>
#include <stdexcept>
#include <type_traits>


#include "core/ResourceManagerBase.h"
#include "utils/errorlog.h"

namespace FluxAudio {


    enum class AudioType { UNKNOWN, WAV, OGG, MP3 };

    AudioType detectType(const std::vector<uint8_t>& data) {
        if (data.size() < 4) return AudioType::UNKNOWN;

        // WAV: "RIFF" .... "WAVE"
        if (data[0] == 'R' && data[1] == 'I' && data[2] == 'F' && data[3] == 'F') return AudioType::WAV;

        // OGG: "OggS"
        if (data[0] == 'O' && data[1] == 'g' && data[2] == 'g' && data[3] == 'S') return AudioType::OGG;

        // MP3: often has not "magic" but starts usually with 0xFF 0xFB (Frame Sync) or "ID3" (Metadata-Tag)
        if (data[0] == 'I' && data[1] == 'D' && data[2] == '3') return AudioType::MP3;
        if (data[0] == 0xFF && (data[1] & 0xE0) == 0xE0) return AudioType::MP3;

        return AudioType::UNKNOWN;
    }


    struct AudioResourceData {
       std::string fileName = "";
       std::vector<uint8_t> mRawData =  {};
       AudioType fileType = AudioType::UNKNOWN;
    };

    class AudioResourceManager : public OhmFlux::ResourceManagerBase {
        SDL_AudioDeviceID mAudioDevice = 0;
        std::unordered_map<std::string, AudioResourceData> mResourceMap;
        bool mInitialized = false;
        bool mShutDown = false;

    public:

        AudioResourceManager():
         mInitialized(false)
        ,mShutDown(false)
        {}
        ~AudioResourceManager() {}

        bool init();
        const bool isInitialized() { return mInitialized; }
        void shutDown();


        //-----------------------------------------------------------------------------
        bool add(std::string fileName);
        AudioResourceData* get(std::string fileName, bool noAutoLoad = false);


    private:

        bool LoadRawFile(AudioResourceData &data) {
            // Open file at the end to get size immediately
            std::ifstream ifs(data.fileName, std::ios::binary | std::ios::ate);

            if (!ifs.is_open()) {
                Log("[error] Load audio resource: Can't open File %s", data.fileName.c_str());
                return false;
            }

            try {
                std::streamsize size = ifs.tellg();
                ifs.seekg(0, std::ios::beg);

                // Reserve memory and read file content into the vector
                data.mRawData.resize(static_cast<size_t>(size));
                if (!ifs.read(reinterpret_cast<char*>(data.mRawData.data()), size)) {
                    Log("[error] Load audio resource: Failed to read data from %s", data.fileName.c_str());
                    return false;
                }
                ifs.close();

                // Identify file format based on magic bytes
                data.fileType = detectType(data.mRawData);

                if (data.fileType == AudioType::UNKNOWN) {
                    Log("[warning] Audio format not recognized for: %s", data.fileName.c_str());
                    return false;
                }

                return true;

            } catch (const std::exception& e) {
                Log("[error] Load audio resource exception: %s", e.what());
                return false;
            }
        }


    }; //class
}; //namespace
