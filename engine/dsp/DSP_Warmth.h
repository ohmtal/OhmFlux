//-----------------------------------------------------------------------------
// Copyright (c) 2026 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Audio Reverb Digital
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


        // even muffliger
        virtual void process(int16_t* buffer, int numSamples) override {
            if (!inOn()) return;

            if (mSettings.wet <= 0.001f) return;

            float alpha = std::clamp(mSettings.cutoff, 0.01f, 0.99f);

            for (int i = 0; i < numSamples; i++) {
                float dry = static_cast<float>(buffer[i]);

                // Point to the correct 4-pole array for this channel
                float* p = (i % 2 == 0) ? mPolesL : mPolesR;

                // --- 4-POLE CASCADE ---
                // Each pole acts as a -6dB filter. 4 x -6dB = -24dB/octave (Very steep!)
                p[0] = (dry  * alpha) + (p[0] * (1.0f - alpha));
                p[1] = (p[0] * alpha) + (p[1] * (1.0f - alpha));
                p[2] = (p[1] * alpha) + (p[2] * (1.0f - alpha));
                p[3] = (p[2] * alpha) + (p[3] * (1.0f - alpha));

                float filtered = p[3]; // The final muffled output is the last pole

                // Analog Saturation (Tanh)
                float x = (filtered * mSettings.drive) / 32768.0f;
                float saturated = x * (1.5f - (0.5f * x * x));
                saturated = std::clamp(saturated, -1.0f, 1.0f);

                float mixed = (dry * (1.0f - mSettings.wet)) + (saturated * 32768.0f * mSettings.wet);
                buffer[i] = static_cast<int16_t>(std::clamp(mixed, -32768.0f, 32767.0f));
            }
        }

        // virtual void process(int16_t* buffer, int numSamples) override {
        //     if (mSettings.wet <= 0.001f) return;
        //
        //     float alpha = std::clamp(mSettings.cutoff, 0.01f, 0.99f);
        //
        //     for (int i = 0; i < numSamples; i++) {
        //         float dry = static_cast<float>(buffer[i]);
        //
        //         // Use an array of 'last' samples to create 4 poles
        //         // Add float mPolesL[4] and mPolesR[4] to your class
        //         float* poles = (i % 2 == 0) ? mPolesL : mPolesR;
        //
        //         // 4-Pole Cascade (24dB/octave slope)
        //         // This is much more audible than a single pole
        //         poles[0] = (dry * alpha) + (poles[0] * (1.0f - alpha));
        //         poles[1] = (poles[0] * alpha) + (poles[1] * (1.0f - alpha));
        //         poles[2] = (poles[1] * alpha) + (poles[2] * (1.0f - alpha));
        //         poles[3] = (poles[2] * alpha) + (poles[3] * (1.0f - alpha));
        //
        //         float filtered = poles[3];
        //
        //         // Analog Saturation (Tanh)
        //         float x = (filtered * mSettings.drive) / 32768.0f;
        //         float saturated = x * (1.5f - (0.5f * x * x));
        //         saturated = std::clamp(saturated, -1.0f, 1.0f);
        //
        //         float finalWet = saturated * 32768.0f;
        //         float mixed = (dry * (1.0f - mSettings.wet)) + (finalWet * mSettings.wet);
        //
        //         buffer[i] = static_cast<int16_t>(std::clamp(mixed, -32768.0f, 32767.0f));
        //     }
        // }

    };


} // namespace DSP
