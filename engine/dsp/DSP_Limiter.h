//-----------------------------------------------------------------------------
// Copyright (c) 2026 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Audio Reverb Digital
// Limiter
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


        virtual void process(int16_t* buffer, int numSamples) override {
            if (!inOn()) return;

            for (int i = 0; i < numSamples; i++) {
                // 1. Peak Detection: Get absolute level
                float input = static_cast<float>(buffer[i]);
                float absInput = std::abs(input);

                // 2. Calculate Target Gain
                float targetGain = 1.0f;
                if (absInput > mThreshold) {
                    targetGain = mThreshold / absInput;
                }

                // 3. Smooth the Gain (Attack/Release)
                // If we need to turn down, use Attack speed. If turning up, use Release.
                if (targetGain < mCurrentGain) {
                    mCurrentGain += (targetGain - mCurrentGain) * mAttack;
                } else {
                    mCurrentGain += (targetGain - mCurrentGain) * mRelease;
                }

                // 4. Apply Gain and Clamp
                float output = input * mCurrentGain;
                buffer[i] = static_cast<int16_t>(std::clamp(output, -32768.0f, 32767.0f));
            }
        }
    };

} // namespace DSP
