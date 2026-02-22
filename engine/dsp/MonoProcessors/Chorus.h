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
    // ChorusLine
    // Rate: 0.1 - 5.0 Hz (Lush to fast Leslie-style)
    // Depth: 0.1 - 10.0 ms (Subtle to heavy pitch-bend)
    // Delay: 10.0 - 50.0 ms (Chorus sweet spot)
    // std::atomic<float> chorus_rate{0.5f};        // Speed: 0.5 Hz (Slow & lush)
    // std::atomic<float> chorus_depth{3.0f};       // Intensity: 3.0 ms swing
    // std::atomic<float> chorus_delayBase{25.0f};  // Offset: 25.0 ms (Classic sweet spot)
    // std::atomic<float> chorus_wet{0.5f};         // Mix: 0.5 (Equal dry/wet for max phase cancellation)

    // paddleHeader("CHORUS", IM_COL32(80,0,180,100), sConfig.chorus_enabled);
    // rackKnob(/*"Rate (Hz)"*/"RATE", sConfig.chorus_rate, {0.1f, 5.0f}, ksGreen);
    // ImGui::SameLine();
    // rackKnob(/*"Depth (ms)"*/ "DEPTH", sConfig.chorus_depth, {0.1f, 10.0f}, ksBlack);
    // ImGui::SameLine();
    // rackKnob(/*"Base Delay (ms)"*/"DELAY", sConfig.chorus_delayBase, {1.0, 50.0f}, ksBlack);
    // ImGui::SameLine();
    // rackKnob("MIX", sConfig.chorus_wet, {0.0f, 1.f}, ksBlue);



    struct ChorusLine {
    private:
        std::vector<float> buffer;
        size_t write_pos = 0;
        float lfo_phase = 0.0f;
        int sample_rate = 48000;
    public:
        void init(int rate, float max_delay_ms = 100.0f) {
            sample_rate = rate;
            // Allocate buffer with small safety margin for interpolation
            size_t size = static_cast<size_t>((max_delay_ms / 1000.0f) * rate) + 5;
            buffer.assign(size, 0.0f);
        }

        float process(float input, float rate_hz, float depth_ms, float delay_base_ms, float mix) {
            if (buffer.empty()) return input;

            // 1. Update LFO phase (Sine wave oscillator)
            lfo_phase += rate_hz / static_cast<float>(sample_rate);
            if (lfo_phase >= 1.0f) lfo_phase -= 1.0f;

            // Map 0..1 phase to -1..1 sine range
            float lfo_sin = sinf(lfo_phase * 2.0f * M_PI);

            // 2. Calculate modulated delay time in samples
            // Result is base offset + (modulation swing)
            float current_delay_ms = delay_base_ms + (lfo_sin * depth_ms);
            float delay_samples = (current_delay_ms / 1000.0f) * sample_rate;

            // 3. Calculate fractional read position
            float read_pos = static_cast<float>(write_pos) - delay_samples;
            while (read_pos < 0) read_pos += buffer.size();

            // 4. Linear Interpolation to avoid artifacts during modulation
            size_t idx1 = static_cast<size_t>(read_pos) % buffer.size();
            size_t idx2 = (idx1 + 1) % buffer.size();
            float frac = read_pos - std::floor(read_pos);

            float delayed_sample = buffer[idx1] * (1.0f - frac) + buffer[idx2] * frac;

            // 5. Update circular buffer and return wet/dry mix
            buffer[write_pos] = input;
            write_pos = (write_pos + 1) % buffer.size();

            return input * (1.0f - mix) + delayed_sample * mix;
        }
    };



};};
