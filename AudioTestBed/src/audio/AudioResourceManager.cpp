//NOTE THIS IS NOT FOR USE ONLY FOR MERGING TO FluxAudio::Manager

#include "AudioResourceManager.h"

#include <vector>
#include <fstream>
#include <string>
#include <stdexcept>
#include <type_traits>


#include "utils/errorlog.h"
#include "utils/fluxStr.h"

namespace FluxAudio {
    //--------------------------------------------------------------------------
    bool ResourceManager::Initialize() {
        mInitialized = true;
        return mInitialized;
    }
    //--------------------------------------------------------------------------
    void ResourceManager::Deinitialize() {
        if (mShutDown) return;
        mShutDown = true;


        for (auto& [key, val] : mResourceMap) {
            val->mRawData.clear();
        }
        mResourceMap.clear();
        if (mAudioDevice != 0) {
            SDL_CloseAudioDevice(mAudioDevice);
            mAudioDevice = 0;
        }

        mInitialized = false;
    }
    //--------------------------------------------------------------------------
    ResourceData* ResourceManager::get(std::string fileName, bool noAutoLoad) {
        // it can be loaded but added to blacklist (invalid) by AudioInstance!
        if (isBlackListed(fileName)) {
            return nullptr;
        }

        auto it = mResourceMap.find(fileName);

        if (it != mResourceMap.end()) {
            return it->second.get();
        }

        if (noAutoLoad) return nullptr;

        // auto load
        // checked on add: if (isBlackListed(fileName)) return nullptr;
        if (!add(fileName)) return nullptr;

        // lookup again
        {
            auto it = mResourceMap.find(fileName);

            if (it != mResourceMap.end()) {
                return it->second.get();
            }
        }

        return nullptr;
    }
    //--------------------------------------------------------------------------
    bool ResourceManager::add(std::string fileName, bool enableLoop , uint8_t maxInstances )
    {
        if (!isInitialized()) return false;
        if (isBlackListed(fileName)) {
            Log("[info] audio resource - it's blacklisted: %s!", fileName.c_str());
            return false;
        }

        if (get(fileName, true) != nullptr) {
            Log("[info] audio resource -  double load: %s!", fileName.c_str());
            return true; //we have it return true!
        }

        auto resData = std::make_unique<ResourceData>();
        resData->fileName = fileName;
        resData->enableLoop = enableLoop;
        resData->maxInstances = maxInstances;

        if (!LoadRawFile(*resData)) {
            blacklist(fileName);
            return false;
        }

        mResourceMap[fileName] = std::move(resData);
        return true;

    }
    //--------------------------------------------------------------------------
    bool ResourceManager::LoadRawFile(ResourceData& data) {
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

            // Fallback: If detection failed, check file extension for MP3
            if (data.fileType == AudioType::UNKNOWN) {
                std::string ext = FluxStr::extractFileExt(data.fileName, true);
                if ( ext == "mp3") {
                    data.fileType = AudioType::MP3;
                    Log("[info] Audio type MP3 detected via extension fallback for: %s", data.fileName.c_str());
                }
                else if (ext == "sfx" && data.mRawData.size() == 105) {
                    data.fileType = AudioType::SFX;
                    Log("[info] Audio type legacy SFX detected via extension fallback for: %s", data.fileName.c_str());
                }

            }
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



}; //namespace
