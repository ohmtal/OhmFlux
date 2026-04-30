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

#include "core/ResourceManagerBase.h"
#include <unordered_map>
#include <vector>
#include <memory>

namespace FluxAudio {

    //FIXME move type to fluxAudio.h when done
    enum class AudioType { UNKNOWN, WAV, OGG, MP3, SFX };

    inline AudioType detectType(const std::vector<uint8_t>& data) {
        if (data.size() < 4) return AudioType::UNKNOWN;

        // WAV: "RIFF" .... "WAVE"
        if (data[0] == 'R' && data[1] == 'I' && data[2] == 'F' && data[3] == 'F') return AudioType::WAV;

        // OGG: "OggS"
        if (data[0] == 'O' && data[1] == 'g' && data[2] == 'g' && data[3] == 'S') return AudioType::OGG;

        // MP3: often has not "magic" but starts usually with 0xFF 0xFB (Frame Sync) or "ID3" (Metadata-Tag)
        if (data[0] == 'I' && data[1] == 'D' && data[2] == '3') return AudioType::MP3;
        if (data[0] == 0xFF && (data[1] & 0xE0) == 0xE0) return AudioType::MP3;


        // FluxSFX << SFXGenerator Stereo!
        if (data.size() < 7) return AudioType::UNKNOWN;

        if (data[0] == 'F' && data[1] == 'l' && data[2] == 'u' && data[3] == 'x'
            && data[4] == 'S' && data[5] == 'F' && data[6] == 'X'
        ) return AudioType::SFX;


        return AudioType::UNKNOWN;
    }


    struct AudioResourceData {
       std::string fileName = "";
       std::vector<uint8_t> mRawData =  {};
       AudioType fileType = AudioType::UNKNOWN;
    };

    // -------------------------------------------------------------------------
    class AudioResourceManager : public OhmFlux::ResourceManagerBase {
        SDL_AudioDeviceID mAudioDevice = 0;
        // std::unordered_map<std::string, AudioResourceData> mResourceMap;
        std::unordered_map<std::string, std::unique_ptr<AudioResourceData>> mResourceMap;

        bool mInitialized = false;
        bool mShutDown = false;

    public:

        AudioResourceManager():
         mInitialized(false)
        ,mShutDown(false)
        {}
        ~AudioResourceManager() {}

        bool Initialize();
        const bool isInitialized() { return mInitialized; }
        void Deinitialize();

        // Returns a const reference to the map to prevent copies and protect data
        const std::unordered_map<std::string, std::unique_ptr<AudioResourceData>>& getMap() const {
            return mResourceMap;
        }

        // const std::unordered_map<std::string, std::unique_ptr<AudioResourceData>> getMap() { return mResourceMap; }

        //-----------------------------------------------------------------------------
        bool add(std::string fileName);
        AudioResourceData* get(std::string fileName, bool noAutoLoad = false);


    private:

        bool LoadRawFile(AudioResourceData &data);


    }; //class
}; //namespace
