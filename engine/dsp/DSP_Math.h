//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Digital Sound Processing : Math functions
//-----------------------------------------------------------------------------
#pragma once
#include <algorithm>
#include <cmath>

namespace DSP {

    template<typename T>
    inline T clamp(T val, T min, T max) {
        if (val < min) return min;
        if (val > max) return max;
        return val;
    }

    // fast tanh
    inline float fast_tanh(float x) {
        float x2 = x * x;
        float a = x * (135.0f + x2 * (15.0f + x2));
        float b = 135.0f + x2 * (45.0f + x2 * 5.0f);
        return a / b;
    }

    // Lineare Interpolation ( Delay/Chorus/Pitch)
    inline float lerp(float a, float b, float f) {
        return a + f * (b - a);
    }

    //
    inline float gainToDb(float gain) {
        return 20.0f * std::log10(gain + 1e-9f);
    }

    //
    inline float dbToGain(float db) {
        return std::pow(10.0f, db * 0.05f);
    }
}
