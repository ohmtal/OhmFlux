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

    // Sustain Compressor
    // std::atomic<float> compressor_threshold{0.1f};
    // std::atomic<float> compressor_ratio{4.0f};
    // std::atomic<float> compressor_attack_ms{5.0f};
    // std::atomic<float> compressor_release_ms{100.0f};
    // std::atomic<float> compressor_gain{1.0f};

    // rackKnob("SENSE", sConfig.autowah_sensitivity, {0.0f, 1.0f}, ksGreen);
    // ImGui::SameLine();
    // rackKnob("RES", sConfig.autowah_resonance, {0.0f, 1.0f}, ksBlack);
    // ImGui::SameLine();
    // rackKnob("RANGE", sConfig.autowah_range, {0.0f, 1.0f}, ksYellow);
    // ImGui::SameLine();
    // rackKnob("MIX", sConfig.autowah_mix, {0.0f, 1.0f}, ksBlue);


    class SustainCompressor {
    private:
        float envelope = 0.0f;
    public:
        float process(float input, float threshold, float ratio, float attack_ms, float release_ms, float output_gain, int sample_rate) {
            float abs_in = std::abs(input);

            // 1. Envelope follower (detect peak)
            float attack_alpha = 1.0f - std::exp(-1.0f / (attack_ms * 0.001f * sample_rate));
            float release_alpha = 1.0f - std::exp(-1.0f / (release_ms * 0.001f * sample_rate));

            if (abs_in > envelope) {
                envelope += attack_alpha * (abs_in - envelope);
            } else {
                envelope += release_alpha * (abs_in - envelope);
            }

            // 2. Gain calculation
            float gain = 1.0f;
            if (envelope > threshold && threshold > 0.0f) {
                // Compression in linear domain:
                // out = threshold + (envelope - threshold) / ratio
                // target_envelope = threshold + (envelope - threshold) / ratio
                // gain = target_envelope / envelope
                gain = (threshold + (envelope - threshold) / ratio) / envelope;
            }

            // 3. Apply gain and output makeup
            return input * gain * output_gain;
        }
    };


};};
