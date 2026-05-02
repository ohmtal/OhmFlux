//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------

#pragma once
#include <SDL3/SDL.h>

#include "core/ResourceManagerBase.h"
#include "AudioType.h"

#include <unordered_map>
#include <vector>
#include <memory>


namespace FluxAudio {


    struct ResourceData {
       std::string fileName = "";
       std::vector<uint8_t> mRawData =  {};
       AudioType fileType = AudioType::UNKNOWN;

       bool enableLoop      = false;  // default for instance
       uint8_t maxInstances = 1;      // manager should check this to decide to spawn a new instance

       // WAV handling on load it will be stripped
       SDL_AudioSpec wavSrcSpec;

    };

    // -------------------------------------------------------------------------
    class ResourceManager : public OhmFlux::ResourceManagerBase {
        SDL_AudioDeviceID mAudioDevice = 0;
        // std::unordered_map<std::string, AudioResourceData> mResourceMap;
        std::unordered_map<std::string, std::unique_ptr<ResourceData>> mResourceMap;

        bool mInitialized = false;
        bool mShutDown = false;


        ResourceManager():
            mShutDown(false)
        {}
        ~ResourceManager() {
            Deinitialize();
        }

        // Delete copy/assignment for Singleton safety
        ResourceManager(const ResourceManager&) = delete;
        void operator=(const ResourceManager&) = delete;

        void Deinitialize();

    public:
        // Get the single instance
        static ResourceManager&  getInstance() {
            static ResourceManager instance;
            return instance;
        }



        bool Initialize();
        const bool isInitialized() { return mInitialized; }

        // Returns a const reference to the map to prevent copies and protect data
        const std::unordered_map<std::string, std::unique_ptr<ResourceData>>& getMap() const {
            return mResourceMap;
        }

        // const std::unordered_map<std::string, std::unique_ptr<AudioResourceData>> getMap() { return mResourceMap; }

        //-----------------------------------------------------------------------------
        bool add(std::string fileName, bool enableLoop = false, uint8_t maxInstances = 1);
        ResourceData* get(std::string fileName, bool noAutoLoad = false);


    private:

        bool LoadRawFile(ResourceData &data);


    }; //class
}; //namespace
#define AudioResourceManager FluxAudio::ResourceManager::getInstance()
