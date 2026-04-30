//NOTE THIS IS NOT FOR USE ONLY FOR MERGING TO FluxAudio::Manager

#include "AudioResourceManager.h"
namespace FluxAudio {
    //--------------------------------------------------------------------------
    bool AudioResourceManager::init() {
        mInitialized = true;
        return mInitialized;
    }
    //--------------------------------------------------------------------------
    void AudioResourceManager::shutDown() {
        if (mShutDown) return;
        mShutDown = true;

        for (auto& [key, val] : mResourceMap) {
            val.mRawData.clear();
        }
        mResourceMap.clear();
        if (mAudioDevice != 0) {
            SDL_CloseAudioDevice(mAudioDevice);
            mAudioDevice = 0;
        }
    }
    //--------------------------------------------------------------------------
    AudioResourceData* AudioResourceManager::get(std::string fileName, bool noAutoLoad) {
        auto it = mResourceMap.find(fileName);

        if (it != mResourceMap.end()) {
            return &it->second;
        }

        if (noAutoLoad) return nullptr;

        // auto load
        // checked on add: if (isBlackListed(fileName)) return nullptr;
        if (!add(fileName)) return nullptr;

        // lookup again
        {
            auto it = mResourceMap.find(fileName);

            if (it != mResourceMap.end()) {
                return &it->second;
            }
        }

        return nullptr;
    }
    //--------------------------------------------------------------------------
    bool AudioResourceManager::add(std::string fileName)
    {
        if (!isInitialized()) return false;
        if (isBlackListed(fileName)) {
            SDL_Log("[error] denied eaudio resource - it's blacklisted: %s!", fileName.c_str());
            return false;
        }

        if (get(fileName, true) != nullptr) {
            SDL_Log("[error] denied audio resource -  double load: %s!", fileName.c_str());
            return false;
        }

        AudioResourceData resData;
        resData.fileName = fileName;
        LoadRawFile(resData);
        if (resData.fileType == AudioType::UNKNOWN) return false;

        mResourceMap[fileName] = resData;
        return true;
    }
    //--------------------------------------------------------------------------


}; //namespace
