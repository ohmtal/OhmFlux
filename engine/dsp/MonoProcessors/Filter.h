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

    //--------------------------------------------------------------------------
    class LPFFilter {
    private:
        float last_out = 0.0f;
    public:
        float process(float input, float alpha) {
            float out = last_out + alpha * (input - last_out);
            last_out = out;
            return out;
        }
    };
    //--------------------------------------------------------------------------
    class HPFFilter {
    private:
        float last_input = 0.0f;
        float last_out = 0.0f;
    public:
        float process(float input, float alpha) {
            // Simple one-pole HPF: y[n] = alpha * (y[n-1] + x[n] - x[n-1])
            float out = alpha * (last_out + input - last_input);
            last_input = input;
            last_out = out;
            return out;
        }
    };
    //--------------------------------------------------------------------------
    class NoiseGate {
    private:
        float current_gain = 1.0f;
    public:
        float process(float input, float threshold, float release_samples, bool enabled) {
            if (!enabled) return input;
            float abs_input = std::abs(input);
            if (abs_input >= threshold) {
                current_gain = 1.0f;
            } else {
                current_gain -= 1.0f / release_samples;
                if (current_gain < 0.0f) current_gain = 0.0f;
            }
            return input * current_gain;
        }
    };

};};
