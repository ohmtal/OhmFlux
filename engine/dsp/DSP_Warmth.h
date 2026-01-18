//-----------------------------------------------------------------------------
// Copyright (c) 2026 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Digital Sound Processing : Warmth
// Warmth - A simple One-Pole Low-Pass Filter mimics the "warm" analog
//          output of 90s sound cards.
//-----------------------------------------------------------------------------
#pragma once

#include <vector>
#include <cstdint>
#include <algorithm>
#include <cmath>

#include "DSP_Effect.h"

namespace DSP {

    struct WarmthSettings {
        float cutoff;   // 0.0 to 1.0 (Filter intensity, lower is muddier/warmer)
        float drive;    // 1.0 to 2.0 (Analog saturation/harmonic thickness)
        float wet;      // 0.0 to 1.0
    };

    // 2026 Standard Presets
    constexpr WarmthSettings OFF_WARMTH            = { 1.0f, 1.0f, 0.0f };
    constexpr WarmthSettings GENTLE_WARMTH         = { 0.85f, 1.1f, 0.5f };
    constexpr WarmthSettings ANALOGDESK_WARMTH     = { 0.70f, 1.3f, 0.8f };
    constexpr WarmthSettings TUBEAMP_WARMTH        = { 0.50f, 1.6f, 1.0f };
    constexpr WarmthSettings EXTREME_WARMTH        = { 0.10f, 2.0f, 1.0f };


    class Warmth : public Effect {
    private:
        WarmthSettings mSettings;
        float mPolesL[4];
        float mPolesR[4];

    public:
        Warmth(bool switchOn = false) :
            Effect(switchOn),
            mSettings(TUBEAMP_WARMTH)
        {
            std::memset(mPolesL, 0, sizeof(mPolesL));
            std::memset(mPolesR, 0, sizeof(mPolesR));
        }
        void setSettings(const WarmthSettings& s) {
            mSettings = s;
            std::memset(mPolesL, 0, sizeof(mPolesL));
            std::memset(mPolesR, 0, sizeof(mPolesR));
        }


        virtual void process(float* buffer, int numSamples) override {
            if (!inOn()) return;
            if (mSettings.wet <= 0.001f) return;

            // alpha represents the cutoff frequency (0.01 to 0.99)
            float alpha = std::clamp(mSettings.cutoff, 0.01f, 0.99f);

            for (int i = 0; i < numSamples; i++) {
                // 1. Input is already float (-1.0 to 1.0)
                float dry = buffer[i];

                // 2. Select poles for Left or Right channel
                // Ensure mPolesL/R are float[4] initialized to 0.0f
                float* p = (i % 2 == 0) ? mPolesL : mPolesR;

                // 3. 4-POLE CASCADE (-24dB/octave)
                p[0] = (dry  * alpha) + (p[0] * (1.0f - alpha));
                p[1] = (p[0] * alpha) + (p[1] * (1.0f - alpha));
                p[2] = (p[1] * alpha) + (p[2] * (1.0f - alpha));
                p[3] = (p[2] * alpha) + (p[3] * (1.0f - alpha));

                float filtered = p[3];

                // 4. Analog Saturation (Soft Clipping)
                // In the float pipeline, 'x' is already normalized.
                float x = filtered * mSettings.drive;

                // Polynomial approximation of Tanh for "warm" analog saturation
                float saturated = x * (1.5f - (0.5f * x * x));

                // 5. Final Mix and Clamp
                // We clamp here because saturation/drive can push values outside [-1, 1]
                float mixed = (dry * (1.0f - mSettings.wet)) + (saturated * mSettings.wet);
                buffer[i] = std::clamp(mixed, -1.0f, 1.0f);
            }
        }


    };


} // namespace DSP
