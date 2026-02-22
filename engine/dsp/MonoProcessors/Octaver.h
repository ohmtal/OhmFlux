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

#include "Filter.h"
#include "../DSP_Math.h"

namespace DSP {
namespace MonoProcessors {

    // Octaver
    // std::atomic<float> octaver_dry{1.0f};
    // std::atomic<float> octaver_sub{0.5f};

    class Octaver {
    private:
        bool flip = false;
        float last_input = 0.0f;
        LPFFilter lpf; // pre-filter for better tracking
        LPFFilter post_lpf; // smooth the square-ish result

    public:
        float process(float input, float dry_level, float oct_level, int sample_rate) {
            // 1. Pre-filter for better tracking (fundamental)
            float tracked = lpf.process(input, 0.1f);

            // 2. Zero-crossing detection (positive going)
            if (tracked > 0 && last_input <= 0) {
                flip = !flip;
            }
            last_input = tracked;

            // 3. Generate octave (square wave modulated by input envelope)
            float sub = (flip ? 1.0f : -1.0f) * std::abs(input);

            // 4. Post-filter to make it more "bassy" and less "buzzy"
            float sub_filtered = post_lpf.process(sub, 0.05f);

            return (input * dry_level) + (sub_filtered * oct_level);
        }
    };



};};
