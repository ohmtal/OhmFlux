//-----------------------------------------------------------------------------
// Copyright (c) 2026 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Singleton
// - only used on FluxAudioStream at the moment
// FIXME sfx and OPL should be also bound to this to have a MasterVolume
//-----------------------------------------------------------------------------
#pragma once

#include "SDL3/SDL.h"

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
            Log("Init Audiomanager ID:%d", mAudioDevice);

            return mAudioDevice != 0;

        }

        SDL_AudioDeviceID getDeviceID() const { return mAudioDevice; }

        // Bind any new stream (OPL or OGG) to this master device
        bool bindStream(SDL_AudioStream* stream)
        {
            if (!mAudioDevice || !stream)
                return false;
            return SDL_BindAudioStream(mAudioDevice, stream);
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
