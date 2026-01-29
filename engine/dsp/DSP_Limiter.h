//-----------------------------------------------------------------------------
// Copyright (c) 2026 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Digital Sound Processing : Limiter
//-----------------------------------------------------------------------------
#pragma once

#include <vector>
#include <cstdint>
#include <algorithm>
#include <cstring>


#include "DSP_Effect.h"

namespace DSP {


    struct LimiterSettings {
        float Threshold ;  // Limit just before 1.f
        float Attack;      // How fast it turns down
        float Release;     // How slow it turns back up
    };


    constexpr LimiterSettings LIMITER_DEFAULT = { 0.95f,  0.05f,  0.0000005f };
    constexpr LimiterSettings LIMITER_EIGHTY  = { 0.80f,  0.05f,  0.0000005f };
    constexpr LimiterSettings LIMITER_FIFTY   = { 0.50f,  0.05f,  0.0000005f };
    constexpr LimiterSettings LIMITER_LOWVOL  = { 0.25f,  0.05f,  0.0000005f };
    constexpr LimiterSettings LIMITER_EXTREM  = { 0.05f,  0.50f,  0.0000005f };

    class Limiter : public Effect {
    private:
        float mCurrentGain = 1.0f;
        LimiterSettings mSettings;

        // const float mThreshold =  0.98f; //mhhh we switched to float! 30000.0f; // Limit just before 32767
        // const float mAttack    = 0.05f;      // How fast it turns down
        // const float mRelease   = 0.0005f;   // How slow it turns back up

    public:

        Limiter(bool switchOn = false) :
            Effect(switchOn),
            mSettings(LIMITER_DEFAULT)
            {
            }

        void setSettings(const LimiterSettings& s) {
                mSettings = s;
        }

        LimiterSettings& getSettings() { return mSettings; }


        virtual void process(float* buffer, int numSamples) override {
            if (!isEnabled()) return;

            // Ensure mThreshold is in 0.0 to 1.0 range (e.g., 0.95f for a hard limiter)
            for (int i = 0; i < numSamples; i++) {
                // 1. Input is already float (-1.0 to 1.0)
                float input = buffer[i];
                float absInput = std::abs(input);

                // 2. Calculate Target Gain
                float targetGain = 1.0f;
                if (absInput > mSettings.Threshold) {
                    // Avoid division by zero with a tiny epsilon
                    targetGain = mSettings.Threshold / (absInput + 1e-9f);
                }

                // 3. Smooth the Gain (Attack/Release)
                // Note: mAttack/mRelease should be coefficients between 0.0 and 1.0
                if (targetGain < mCurrentGain) {
                    mCurrentGain += (targetGain - mCurrentGain) * mSettings.Attack;
                } else {
                    mCurrentGain += (targetGain - mCurrentGain) * mSettings.Release;
                }

                // 4. Apply Gain
                // No clamping or static_cast needed.
                // The smoothing logic ensures the signal stays near the threshold.
                buffer[i] = input * mCurrentGain;
            }
        }

        // virtual void process(float* buffer, int numSamples) override {
        //     if (!isEnabled()) return;
        //
        //     // Ensure mThreshold is in 0.0 to 1.0 range (e.g., 0.95f for a hard limiter)
        //     for (int i = 0; i < numSamples; i++) {
        //         // 1. Input is already float (-1.0 to 1.0)
        //         float input = buffer[i];
        //         float absInput = std::abs(input);
        //
        //         // 2. Calculate Target Gain
        //         float targetGain = 1.0f;
        //         if (absInput > mThreshold) {
        //             // Avoid division by zero with a tiny epsilon
        //             targetGain = mThreshold / (absInput + 1e-9f);
        //         }
        //
        //         // 3. Smooth the Gain (Attack/Release)
        //         // Note: mAttack/mRelease should be coefficients between 0.0 and 1.0
        //         if (targetGain < mCurrentGain) {
        //             mCurrentGain += (targetGain - mCurrentGain) * mAttack;
        //         } else {
        //             mCurrentGain += (targetGain - mCurrentGain) * mRelease;
        //         }
        //
        //         // 4. Apply Gain
        //         // No clamping or static_cast needed.
        //         // The smoothing logic ensures the signal stays near the threshold.
        //         buffer[i] = input * mCurrentGain;
        //     }
        // }


    };

} // namespace DSP
