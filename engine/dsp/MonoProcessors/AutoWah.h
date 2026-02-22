//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#pragma once

#include <vector>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <atomic>
#include <fstream>
#include <cstdlib>

#include "../DSP_Math.h"


namespace DSP {
namespace MonoProcessors {

    // Auto-Wah
    // Defaults:
    // std::atomic<float> autowah_sensitivity{0.5f}; // 0.f - 1.f
    // std::atomic<float> autowah_resonance{0.5f};   // 0.f - 1.f
    // std::atomic<float> autowah_range{0.5f};       // 0.f - 1.f
    // std::atomic<float> autowah_mix{0.5f};         // 0.f - 1.f

    // rackKnob("SENSE", sConfig.autowah_sensitivity, {0.0f, 1.0f}, ksGreen);
    // rackKnob("RES", sConfig.autowah_resonance, {0.0f, 1.0f}, ksBlack);
    // rackKnob("RANGE", sConfig.autowah_range, {0.0f, 1.0f}, ksYellow);
    // rackKnob("MIX", sConfig.autowah_mix, {0.0f, 1.0f}, ksBlue);
    //--------------------------------------------------------------------------
    class AutoWah {
    private:
        float mEnvelope = 0.0f;
        float mLow = 0.0f, mBand = 0.0f;
    public:

        float process(float input, float sensitivity, float resonance, float range, float mix, int sample_rate) {
            float abs_in = std::abs(input);
            float attack = 0.05f;
            float release = 0.005f;
            if (abs_in > mEnvelope) mEnvelope += attack * (abs_in - mEnvelope);
            else mEnvelope += release * (abs_in - mEnvelope);

            float freq = 300.0f + sensitivity * mEnvelope * range * 5000.0f;
            freq = DSP::clamp(freq, 100.0f, 10000.0f);

            float f = 2.0f * sinf(M_PI * freq / (float)sample_rate);
            float q = 1.0f - resonance * 0.95f;

            float high = input - mLow - q * mBand;
            mBand = f * high + mBand;
            mLow = f * mBand + mLow;

            return input * (1.0f - mix) + mBand * mix;
        }
    };


};};
