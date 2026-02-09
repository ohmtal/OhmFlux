//-----------------------------------------------------------------------------
// Copyright (c) 2026 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Singleton
//-----------------------------------------------------------------------------
#pragma once

#include "SDL3/SDL.h"
#include "utils/errorlog.h"

namespace FluxAudio {

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
                Log("[info] Audio output Hardware: %d Hz, %d Channels, Format id: %d ", mOutputSpec.freq, mOutputSpec.channels, mOutputSpec.format);
            }
            Log("Init Audiomanager ID:%d.", mAudioDevice);

            return mAudioDevice != 0;
        }

        SDL_AudioDeviceID getDeviceID() const { return mAudioDevice; }

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
