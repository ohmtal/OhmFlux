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

    // Delay
    // std::atomic<float> delay_time_ms{300.0f};
    // std::atomic<float> delay_feedback{0.4f};
    // std::atomic<float> delay_mix{0.3f};
    // std::atomic<bool> delay_enabled{false};

    // paddleHeader("DELAY", IM_COL32(0,80,180,100), sConfig.delay_enabled);
    // rackKnob("TIME", sConfig.delay_time_ms, {10.0f, 2000.0f}, ksGreen);
    // rackKnob("FEEDBACK", sConfig.delay_feedback, {0.0f, 0.95f}, ksBlack);
    // rackKnob("MIX", sConfig.delay_mix, {0.0f, 1.f}, ksBlue);


    // Delay Buffer
    class DelayLine {
    private:
        std::vector<float> buffer;
        size_t write_pos = 0;
        int sample_rate = 48000;

    public:
        void init(int rate, float max_delay_ms) {
            sample_rate = rate;
            buffer.assign(static_cast<size_t>((max_delay_ms / 1000.0f) * rate), 0.0f);
        }

        float process(float input, float delay_ms, float feedback, float mix) {
            if (buffer.empty()) return input;

            size_t delay_samples = static_cast<size_t>((delay_ms / 1000.0f) * sample_rate);
            if (delay_samples >= buffer.size()) delay_samples = buffer.size() - 1;
            if (delay_samples < 1) delay_samples = 1;

            size_t read_pos = (write_pos + buffer.size() - delay_samples) % buffer.size();
            float delayed_sample = buffer[read_pos];

            buffer[write_pos] = input + delayed_sample * feedback;
            write_pos = (write_pos + 1) % buffer.size();

            return input * (1.0f - mix) + delayed_sample * mix;
        }
    };



};};
