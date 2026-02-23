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


    // Threshold : -30.f .. -40.f
    // Ratio : 4.f .. 10.f
    // Release:  100.f .. 200.f
    struct SustainCompressor {
        float envelope = 0.0f;

        float process(float input, float threshold_db, float ratio, float attack_ms, float release_ms, float makeup_gain_db, int sample_rate) {
            float abs_in = std::abs(input);

            float attack_alpha = 1.0f - std::exp(-1.0f / (attack_ms * 0.001f * sample_rate));
            float release_alpha = 1.0f - std::exp(-1.0f / (release_ms * 0.001f * sample_rate));

            if (abs_in > envelope) {
                envelope += attack_alpha * (abs_in - envelope);
            } else {
                envelope += release_alpha * (abs_in - envelope);
            }

            float env_db = 20.0f * std::log10(envelope + 1e-6f);

            float reduction_db = 0.0f;
            if (env_db > threshold_db) {
                float over_db = env_db - threshold_db;
                reduction_db = over_db * (1.0f - 1.0f / ratio);
            }
            float total_gain = std::pow(10.0f, (makeup_gain_db - reduction_db) / 20.0f);

            return input * total_gain;
        }
    };


};};
