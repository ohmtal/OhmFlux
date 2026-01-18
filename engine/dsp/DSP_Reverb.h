//-----------------------------------------------------------------------------
// Copyright (c) 2026 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Audio Reverb Digital Sound Processing
//-----------------------------------------------------------------------------
// usage:
// Add to your class:  AudioReverb mReverb;
// SDL3: in audio_callback just before SDL_PutAudioStreamData
//          controller->mReverb.process(buffer, framesNeeded * 2);
//-----------------------------------------------------------------------------
#pragma once
#include <vector>
#include <cstdint>
#include <algorithm>
#include <cstring>

#include "DSP_Effect.h"

namespace DSP {

struct ReverbSettings {
    float decay;
    int sizeL;
    int sizeR;
    float wet;
};


//-----
constexpr ReverbSettings OFF_REVERB        = { 0.00f,    0,     0,   0.00f }; // No effect
//-----
constexpr ReverbSettings HALL_REVERB      = { 0.82f, 17640, 17201,  0.45f }; // Large, lush reflections Concert Hall
constexpr ReverbSettings CAVE_REVERB      = { 0.90f, 30000, 29800,  0.60f }; // Massive, long decay
constexpr ReverbSettings ROOM_REVERB      = { 0.40f,  4000,  3950,  0.25f }; // Short, tight reflections
constexpr ReverbSettings HAUNTED_REVERB   = { 0.88f, 22050, 21500,  0.60f }; // Haunted Corridor


class Reverb : public DSP::Effect {
private:
    std::vector<int16_t> mBufL;
    std::vector<int16_t> mBufR;
    int mPosL = 0;
    int mPosR = 0;
    ReverbSettings mSettings;

public:
    Reverb(bool switchOn = false) :
        Effect(switchOn)
    {
        // Allocate 1 second of buffer for 44.1kHz
        mBufL.assign(44100, 0);
        mBufR.assign(44100, 0);
        mSettings = ROOM_REVERB;
    }

    void setSettings(const ReverbSettings& s) {
        mSettings = s;
        // Optional: clear buffers on change to prevent "ghost" sounds
        std::fill(mBufL.begin(), mBufL.end(), 0);
        std::fill(mBufR.begin(), mBufR.end(), 0);
        mPosL = 0; mPosR = 0;
    }

    // Process interleaved stereo S16 buffer
    void process(int16_t* buffer, int numSamples) override {
        if (!inOn()) return;

        if (mSettings.wet <= 0.001f) return;

        for (int i = 0; i < numSamples; i++) {
            float dry = static_cast<float>(buffer[i]);
            float delayed;

            if (i % 2 == 0) { // Left
                delayed = static_cast<float>(mBufL[mPosL]);
                mBufL[mPosL] = static_cast<int16_t>(dry + (delayed * mSettings.decay));
                mPosL = (mPosL + 1) % mSettings.sizeL;
            } else { // Right
                delayed = static_cast<float>(mBufR[mPosR]);
                mBufR[mPosR] = static_cast<int16_t>(dry + (delayed * mSettings.decay));
                mPosR = (mPosR + 1) % mSettings.sizeR;
            }

            float mixed = (dry * (1.0f - mSettings.wet)) + (delayed * mSettings.wet);
            buffer[i] = static_cast<int16_t>(std::clamp(mixed, -32768.0f, 32767.0f));
        }
    }
};
}; //namespace
