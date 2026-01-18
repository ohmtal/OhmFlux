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

    class Limiter : public Effect {
    private:
        float mCurrentGain = 1.0f;
        const float mThreshold = 30000.0f; // Limit just before 32767
        const float mAttack = 0.05f;      // How fast it turns down
        const float mRelease = 0.0005f;   // How slow it turns back up

    public:

        Limiter(bool switchOn = false) :
            Effect(switchOn)
            {}

        virtual void process(float* buffer, int numSamples) override {
            if (!isEnabled()) return;

            // Ensure mThreshold is in 0.0 to 1.0 range (e.g., 0.95f for a hard limiter)
            for (int i = 0; i < numSamples; i++) {
                // 1. Input is already float (-1.0 to 1.0)
                float input = buffer[i];
                float absInput = std::abs(input);

                // 2. Calculate Target Gain
                float targetGain = 1.0f;
                if (absInput > mThreshold) {
                    // Avoid division by zero with a tiny epsilon
                    targetGain = mThreshold / (absInput + 1e-9f);
                }

                // 3. Smooth the Gain (Attack/Release)
                // Note: mAttack/mRelease should be coefficients between 0.0 and 1.0
                if (targetGain < mCurrentGain) {
                    mCurrentGain += (targetGain - mCurrentGain) * mAttack;
                } else {
                    mCurrentGain += (targetGain - mCurrentGain) * mRelease;
                }

                // 4. Apply Gain
                // No clamping or static_cast needed.
                // The smoothing logic ensures the signal stays near the threshold.
                buffer[i] = input * mCurrentGain;
            }
        }


    };

} // namespace DSP
