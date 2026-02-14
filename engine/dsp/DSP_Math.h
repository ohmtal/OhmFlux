//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Digital Sound Processing : Math functions
//-----------------------------------------------------------------------------
#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <numbers>

namespace DSP {

    struct FastMath {
        static constexpr int LUT_SIZE = 1024;

        struct SinTable {
            std::array<float, LUT_SIZE> data;
            /*constexpr*/ SinTable() : data() {
                for (int i = 0; i < LUT_SIZE; ++i) {
                    data[i] = std::sin(2.0f * 3.1415926535f * (float)i / (float)LUT_SIZE);
                }
            }
        };
        static inline const SinTable& getTable() {
            static SinTable instance;
            return instance;
        }

        static inline float fastSin(float phase01) {
            if (phase01 >= 1.0f) phase01 -= 1.0f;
            if (phase01 < 0.0f) phase01 += 1.0f;
            int index = static_cast<int>(phase01 * (LUT_SIZE - 1)) & (LUT_SIZE - 1);
            return getTable().data[index];
        }

        static inline float fastCos(float phase01) {
            return fastSin(phase01 + 0.25f);
        }
    };


    template<typename T>
    inline constexpr T clamp(T val, T min, T max) {
        if (val < min) return min;
        if (val > max) return max;
        return val;
    }

    inline constexpr float clampFloat(float val, float min, float max) {
        return std::fmax(min, std::fmin(val, max));
    }

    // fast tanh
    inline constexpr float fast_tanh(float x) {
        float x2 = x * x;
        float a = x * (135.0f + x2 * (15.0f + x2));
        float b = 135.0f + x2 * (45.0f + x2 * 5.0f);
        return a / b;
    }

    // Lineare Interpolation ( Delay/Chorus/Pitch)
    inline constexpr float lerp(float a, float b, float f) {
        return a + f * (b - a);
    }

    //
    inline constexpr float gainToDb(float gain) {
        return 20.0f * std::log10(gain + 1e-9f);
    }

    //
    inline constexpr float dbToGain(float db) {
        return std::pow(10.0f, db * 0.05f);
    }
}
