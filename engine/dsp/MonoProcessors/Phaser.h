//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// FIXME SOUND!!!!
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


    // Phaser
    // std::atomic<float> phaser_rate{0.5f};
    // std::atomic<float> phaser_depth{0.5f};
    // std::atomic<float> phaser_feedback{0.5f};
    // std::atomic<float> phaser_mix{0.5f};

    //--------------------------------------------------------------------------
    class Phaser {
    private:
        float lfo_phase = 0.0f;
        float old_phi[6] = {0};

    public:
        float process(float input, float rate_hz, float depth, float feedback, float mix, int sample_rate) {
            lfo_phase += rate_hz / (float)sample_rate;
            if (lfo_phase >= 1.0f) lfo_phase -= 1.0f;

            float lfo = 0.5f * (1.0f + sinf(lfo_phase * 2.0f * M_PI));
            float freq = 400.0f + depth * 4000.0f * lfo;

            float omega = M_PI * freq / (float)sample_rate;
            float a1 = (1.0f - tanf(omega)) / (1.0f + tanf(omega));

            float out = input + feedback * old_phi[5];
            for (int i = 0; i < 6; ++i) {
                float y = a1 * out + old_phi[i];
                old_phi[i] = out - a1 * y;
                out = y;
            }

            return input * (1.0f - mix) + out * mix;
        }
    };


};};
