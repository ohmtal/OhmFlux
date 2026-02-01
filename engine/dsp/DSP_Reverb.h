//-----------------------------------------------------------------------------
// Copyright (c) 2026 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Digital Sound Processing : Reverb
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

    static const uint8_t CURRENT_VERSION = 1;
    void getBinary(std::ostream& os) const {
        uint8_t ver = CURRENT_VERSION;
        os.write(reinterpret_cast<const char*>(&ver), sizeof(ver));
        os.write(reinterpret_cast<const char*>(this), sizeof(ReverbSettings));
    }

    bool  setBinary(std::istream& is) {
        uint8_t fileVersion = 0;
        is.read(reinterpret_cast<char*>(&fileVersion), sizeof(fileVersion));
        if (fileVersion != CURRENT_VERSION) //Something is wrong !
            return false;
        is.read(reinterpret_cast<char*>(this), sizeof(ReverbSettings));
        return  is.good();
    }

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
    // CHANGED: Must be float to maintain precision in feedback loops
    std::vector<float> mBufL;
    std::vector<float> mBufR;
    int mPosL = 0;
    int mPosR = 0;
    ReverbSettings mSettings;

public:
    Reverb(bool switchOn = false) :
    Effect(switchOn)
    {
        // Allocate 1 second of buffer for 44.1kHz as float
        mBufL.assign(44100, 0.0f);
        mBufR.assign(44100, 0.0f);
        mSettings = ROOM_REVERB;
    }

    DSP::EffectType getType() const override { return DSP::EffectType::Reverb; }

    const ReverbSettings& getSettings() { return mSettings; }

    void setSettings(const ReverbSettings& s) {
        mSettings = s;
        // Ensure size settings don't exceed the allocated 44100
        // and reset positions to avoid clicks
        std::fill(mBufL.begin(), mBufL.end(), 0.0f);
        std::fill(mBufR.begin(), mBufR.end(), 0.0f);
        mPosL = 0;
        mPosR = 0;
    }

    void save(std::ostream& os) const override {
        Effect::save(os);              // Save mEnabled
        mSettings.getBinary(os);       // Save Settings
    }

    bool load(std::istream& is) override {
        if (!Effect::load(is)) return false; // Load mEnabled
        return mSettings.setBinary(is);      // Load Settings
    }


    virtual void process(float* buffer, int numSamples) override {
        if (!isEnabled()) return;
        if (mSettings.wet <= 0.001f) return;

        for (int i = 0; i < numSamples; i++) {
            float dry = buffer[i];
            float delayed;

            if (i % 2 == 0) { // Left Channel
                delayed = mBufL[mPosL];
                // FEEDBACK: High precision float math
                mBufL[mPosL] = dry + (delayed * mSettings.decay);
                // Wrap position based on current room size
                mPosL = (mPosL + 1) % mSettings.sizeL;
            } else { // Right Channel
                delayed = mBufR[mPosR];
                mBufR[mPosR] = dry + (delayed * mSettings.decay);
                mPosR = (mPosR + 1) % mSettings.sizeR;
            }

            // MIX: Linear interpolation between dry and wet
            buffer[i] = (dry * (1.0f - mSettings.wet)) + (delayed * mSettings.wet);
        }
    }
};
}; //namespace
