//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH) 
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Singleton
//-----------------------------------------------------------------------------
#pragma once

#include "SDL3/SDL.h"
#include "utils/errorlog.h"

namespace FluxAudio {


    inline const char* to_string(SDL_AudioFormat format) {
        switch (format) {
            case SDL_AUDIO_UNKNOWN: return "Unknown";
            case SDL_AUDIO_U8: return "Unsigned 8-bit";
            case SDL_AUDIO_S8: return "Signed 8-bit";
            case SDL_AUDIO_S16LE: return "Signed 16-bit Little Endian";
            case SDL_AUDIO_S16BE: return "Signed 16-bit Big Endian";
            case SDL_AUDIO_S32LE: return "Signed 32-bit Little Endian";
            case SDL_AUDIO_S32BE: return "Signed 32-bit Big Endian";
            case SDL_AUDIO_F32LE: return "32-bit Floating Point Little Endian";
            case SDL_AUDIO_F32BE: return "32-bit Floating Point Big Endian";
            default: return "Invalid Format";
        }
    }

    class Manager {
    private:
        Manager() : mAudioDevice(0) {}
        ~Manager() {
            if (mAudioDevice) SDL_CloseAudioDevice(mAudioDevice);
        }

        // Delete copy/assignment for Singleton safety
        Manager(const Manager&) = delete;
        void operator=(const Manager&) = delete;

        SDL_AudioDeviceID mAudioDevice;

        SDL_AudioSpec mOutputSpec;

    public:
        // Get the single instance
        static Manager&  getInstance() {
            static Manager instance;
            return instance;
        }

        bool init() {
            if (mAudioDevice != 0)
                return true;

            mAudioDevice = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr);

            if (SDL_GetAudioDeviceFormat(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &mOutputSpec, nullptr)) {
                Log("[info] Audio output Hardware: %d Hz, %d Channels, Format: 0x%x (%s)"
                    , mOutputSpec.freq
                    , mOutputSpec.channels
                    , mOutputSpec.format
                    , to_string(mOutputSpec.format)
                );
            }
            Log("Init Audiomanager ID:%d.", mAudioDevice);

            return mAudioDevice != 0;
        }

        SDL_AudioDeviceID getDeviceID() const { return mAudioDevice; }
        const SDL_AudioSpec getAudioSpec() { return mOutputSpec; }

        bool bindStream(SDL_AudioStream* stream)
        {
            if (!mAudioDevice || !stream)
                return false;
            SDL_AudioDeviceID currentDevice = SDL_GetAudioStreamDevice(stream);

            if (currentDevice == mAudioDevice) {
                return true;
            }
            if (currentDevice != 0) {
                SDL_UnbindAudioStream(stream);
            }
            return SDL_BindAudioStream(mAudioDevice, stream);
        }


        bool unBindStream(SDL_AudioStream* stream)
        {
            if (!mAudioDevice || !stream)
                return false;
            SDL_UnbindAudioStream( stream);
            return (SDL_GetAudioStreamDevice(stream) == 0);
        }

        // Global Master Volume
        float getMasterVolume() {
            return mAudioDevice ? SDL_GetAudioDeviceGain(mAudioDevice) : 1.0f;
        }

        bool setMasterVolume(float value) {
            if (mAudioDevice) {
                return SDL_SetAudioDeviceGain(mAudioDevice, value);
            }
            return false;
        }

    };

} // namespace FluxAudio
#define AudioManager FluxAudio::Manager::getInstance()
