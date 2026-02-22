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

    // Tremolo
    // std::atomic<float> tremolo_rate{5.0f};  // 0.1f - 20.0f Hz
    // std::atomic<float> tremolo_depth{0.5f}; // 0.0 - 1.0
    // rackKnob("RATE", sConfig.tremolo_rate, {0.1f, 20.0f}, ksGreen);
    // ImGui::SameLine();
    // rackKnob("DEPTH", sConfig.tremolo_depth, {0.0f, 1.0f}, ksYellow);
    // ImGui::End();


    class Tremolo {
    private:
        float lfo_phase = 0.0f;
    public:

        float process(float input, float rate_hz, float depth, int sample_rate) {
            lfo_phase += rate_hz / (float)sample_rate;
            if (lfo_phase >= 1.0f) lfo_phase -= 1.0f;

            // Sine wave LFO: ranges from 0.0 to 1.0
            float lfo_val = 0.5f * (1.0f + sinf(lfo_phase * 2.0f * M_PI));

            // Modulate volume: 1.0 (no reduction) to (1.0 - depth)
            float multiplier = 1.0f - (depth * lfo_val);
            return input * multiplier;
        }
    };


};};
