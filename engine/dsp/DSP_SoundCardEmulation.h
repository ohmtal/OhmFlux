//-----------------------------------------------------------------------------
// Copyright (c) 2026 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Digital Sound Processing : SoundCardEmulation (maybe obsolte since warmth)
//-----------------------------------------------------------------------------
#pragma once
#define _USE_MATH_DEFINES // Required for M_PI on some systems (like Windows/MSVC)
#include <cmath>
#include <vector>
#include <cstdint>
#include <algorithm>
#include <cstring>

#ifdef FLUX_ENGINE
#include <imgui.h>
#include <imgui_internal.h>
#include <gui/ImFlux.h>
#endif


#include "DSP_Effect.h"

namespace DSP {


    enum class RenderMode : uint32_t { // Explicitly set size to 4 bytes
        BLENDED, MODERN_LPF, SBPRO, SB_ORIGINAL, ADLIB_GOLD, CLONE_CARD
    };

    struct SoundCardEmulationSettings {
        RenderMode renderMode = RenderMode::BLENDED; // Default initialization

        static const uint8_t CURRENT_VERSION = 1;

        void getBinary(std::ostream& os) const {
            uint8_t ver = CURRENT_VERSION;
            os.write(reinterpret_cast<const char*>(&ver), sizeof(ver));
            os.write(reinterpret_cast<const char*>(this), sizeof(SoundCardEmulationSettings));
        }

        bool setBinary(std::istream& is) {
            uint8_t fileVersion = 0;
            is.read(reinterpret_cast<char*>(&fileVersion), sizeof(fileVersion));
            if (fileVersion != CURRENT_VERSION)
                return false;

            is.read(reinterpret_cast<char*>(this), sizeof(SoundCardEmulationSettings));

            // Safety check: Ensure the loaded enum value is valid
            if (renderMode > RenderMode::CLONE_CARD) {
                renderMode = RenderMode::BLENDED;
            }

            return is.good();
        }


        auto operator<=>(const SoundCardEmulationSettings&) const = default; //C++20 lazy way
    };


    constexpr SoundCardEmulationSettings BLENDED_MODE  = {  RenderMode::BLENDED };


class SoundCardEmulation : public DSP::Effect {


private:
    float mAlpha = 1.0f;
    float mGain = 1.0f;
    bool mUseBlending = true;

    SoundCardEmulationSettings mSettings;

    // State for filtering and blending
    float mPrevL = 0.0f;
    float mPrevR = 0.0f;

    float mFilterStateL = 0.0f;
    float mFilterStateR = 0.0f;
    float mPrevInputL = 0.0f;
    float mPrevInputR = 0.0f;

public:

    SoundCardEmulation(bool switchOn = true) : DSP::Effect(switchOn) {
        mSettings.renderMode = RenderMode::BLENDED;
    }
    DSP::EffectType getType() const override { return DSP::EffectType::SoundCardEmulation; }

    const SoundCardEmulationSettings& getSettings() { return mSettings; }

    void setSettings(const SoundCardEmulationSettings& s) {
        mSettings = s;
        setMode(s.renderMode);
    }

    RenderMode getMode() {
        return mSettings.renderMode;
    }

private:
    void setMode(RenderMode mode) {
        float cutoff = 20000.0f;
        mUseBlending = true;
        mGain = 1.0f;

        switch (mode) {
            case RenderMode::BLENDED:     cutoff = 20000.0f; break;
            case RenderMode::SBPRO:       cutoff = 3200.0f;  mGain = 1.2f; break;
            case RenderMode::SB_ORIGINAL: cutoff = 2800.0f;  mGain = 1.3f; break;
            case RenderMode::ADLIB_GOLD:  cutoff = 16000.0f; mGain = 1.05f; break;
            case RenderMode::MODERN_LPF:  cutoff = 12000.0f; break;
            case RenderMode::CLONE_CARD:  cutoff = 8000.0f;  mUseBlending = false; mGain = 0.9f; break;
        }

        // Standard Alpha calculation for 1-pole LPF
        float sampleRate = getSampleRateF();
        float dt = 1.0f / sampleRate;
        float rc = 1.0f / (2.0f * M_PI * cutoff);
        mAlpha = (cutoff >= 20000.0f) ? 1.0f : (dt / (rc + dt));

        mFilterStateL = 0.0f;
        mFilterStateR = 0.0f;
        mPrevInputL = 0.0f;
        mPrevInputR = 0.0f;
    }
public:

    void save(std::ostream& os) const override {
        Effect::save(os);              // Save mEnabled
        mSettings.getBinary(os);       // Save Settings
    }

    bool load(std::istream& is) override {
        if (!Effect::load(is)) return false; // Load mEnabled
        return mSettings.setBinary(is);      // Load Settings
    }

    void process(float* buffer, int numSamples) override {
        if (!mEnabled) return;

        for (int i = 0; i < numSamples; i += 2) {
            float inputL = buffer[i];
            float inputR = buffer[i + 1];

            // 1. Blending (Linear Interpolation)
            // We use the raw input and the raw previous input for blending
            float blendedL = mUseBlending ? (inputL + mPrevInputL) * 0.5f : inputL;
            float blendedR = mUseBlending ? (inputR + mPrevInputR) * 0.5f : inputR;

            // Save raw input for the next sample's blending
            mPrevInputL = inputL;
            mPrevInputR = inputR;

            // 2. Low-Pass Filter (One-Pole IIR)
            // Use a separate state variable for the filter (mFilterStateL/R)
            mFilterStateL = mFilterStateL + mAlpha * (blendedL - mFilterStateL);
            mFilterStateR = mFilterStateR + mAlpha * (blendedR - mFilterStateR);

            // 3. Apply Gain and Clamp to prevent clipping or NaN propagation
            float outL = std::clamp(mFilterStateL * mGain, -1.0f, 1.0f);
            float outR = std::clamp(mFilterStateR * mGain, -1.0f, 1.0f);

            // Check for NaN (safety for 2026 DSP standards)
            if (std::isnan(outL)) { mFilterStateL = 0.0f; outL = 0.0f; }
            if (std::isnan(outR)) { mFilterStateR = 0.0f; outR = 0.0f; }

            buffer[i] = outL;
            buffer[i + 1] = outR;
        }
    }


}; //class
}; //namespace
