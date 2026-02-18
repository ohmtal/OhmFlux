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
//-----------------------------------------------------------------------------
// Tone Control :: Volume and Tone
// ------------
// Defaults:
//     std::atomic<float> tone_volume{1.0f}; // test simple volume add
//     std::atomic<float> tone_bass{0.0f};
//     std::atomic<float> tone_treble{0.0f};
//     std::atomic<float> tone_presence{0.0f};
// ------------
// Ranges:
//    rackKnob("VOLUME", sConfig.tone_volume, {0.f, 2.f}, ksBlack);
//    rackKnob("BASS", sConfig.tone_bass, {-15.0f, 15.0f}, ksBlack);
//    rackKnob("TREBLE", sConfig.tone_treble, {-15.0f, 15.0f}, ksBlack);
//    rackKnob("PRESENCE", sConfig.tone_presence, {-15.0f, 15.0f}, ksBlack);
//--------------------------------------------------------------------------
//     struct Biquad {
//         float b0 = 1, b1 = 0, b2 = 0, a1 = 0, a2 = 0;
//         float x1 = 0, x2 = 0, y1 = 0, y2 = 0;
//
//         void reset() { x1 = x2 = y1 = y2 = 0; }
//
//         // --- 1. PEAKING (For Presence) ---
//         void setupPeaking(float freq, float gainDb, float Q, float sampleRate) {
//             float A = std::pow(10.0f, gainDb / 40.0f);
//             float omega = 2.0f * M_PI * freq / sampleRate;
//             float sn = std::sin(omega);
//             float cs = std::cos(omega);
//             float alpha = sn / (2.0f * Q);
//
//             float a0 = 1.0f + alpha / A;
//             b0 = (1.0f + alpha * A) / a0;
//             b1 = (-2.0f * cs) / a0;
//             b2 = (1.0f - alpha * A) / a0;
//             a1 = (-2.0f * cs) / a0;
//             a2 = (1.0f - alpha / A) / a0;
//         }
//
//         // --- 2. LOW SHELF (For Bass) ---
//         void setupLowShelf(float freq, float gainDb, float Q, float sampleRate) {
//             float A = std::pow(10.0f, gainDb / 40.0f);
//             float omega = 2.0f * M_PI * freq / sampleRate;
//             float sn = std::sin(omega);
//             float cs = std::cos(omega);
//             float alpha = sn / (2.0f * Q);
//             float sq2A = 2.0f * std::sqrt(A) * alpha;
//
//             float a0 = (A + 1.0f) + (A - 1.0f) * cs + sq2A;
//             b0 = (A * ((A + 1.0f) - (A - 1.0f) * cs + sq2A)) / a0;
//             b1 = (2.0f * A * ((A - 1.0f) - (A + 1.0f) * cs)) / a0;
//             b2 = (A * ((A + 1.0f) - (A - 1.0f) * cs - sq2A)) / a0;
//             a1 = (-2.0f * ((A - 1.0f) * cs + (A + 1.0f))) / a0;
//             a2 = ((A + 1.0f) + (A - 1.0f) * cs - sq2A) / a0;
//         }
//
//         // --- 3. HIGH SHELF (For Treble) ---
//         void setupHighShelf(float freq, float gainDb, float Q, float sampleRate) {
//             float A = std::pow(10.0f, gainDb / 40.0f);
//             float omega = 2.0f * M_PI * freq / sampleRate;
//             float sn = std::sin(omega);
//             float cs = std::cos(omega);
//             float alpha = sn / (2.0f * Q);
//             float sq2A = 2.0f * std::sqrt(A) * alpha;
//
//             float a0 = (A + 1.0f) - (A - 1.0f) * cs + sq2A;
//             b0 = (A * ((A + 1.0f) + (A - 1.0f) * cs + sq2A)) / a0;
//             b1 = (-2.0f * A * ((A - 1.0f) + (A + 1.0f) * cs)) / a0;
//             b2 = (A * ((A + 1.0f) + (A - 1.0f) * cs - sq2A)) / a0;
//             a1 = (2.0f * ((A - 1.0f) * cs - (A + 1.0f))) / a0;
//             a2 = ((A + 1.0f) - (A - 1.0f) * cs - sq2A) / a0;
//         }
//
//         float process(float in) {
//             float out = b0 * in + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;
//             if (!std::isfinite(out)) { reset(); return 0.0f; } // Guard against Inf/NaN
//             x2 = x1; x1 = in;
//             y2 = y1; y1 = out;
//             return out;
//         }
//     };
//
//
//     class ToneControl {
//     public:
//         ToneControl() = default;
//
//
//         float process(float input, float volume, float bass, float treble, float presence, float sampleRate) {
//
//             // 1. Smooth the parameter values to prevent "Zipper Noise"
//             // Formula: y[n] = y[n-1] + factor * (target - y[n-1])
//             smoothed_bass += smoothing_factor * (bass - smoothed_bass);
//             smoothed_treble += smoothing_factor * (treble - smoothed_treble);
//             smoothed_presence += smoothing_factor * (presence - smoothed_presence);
//
//             // 2. Update filters only if smoothed values have moved significantly
//             // We use a small epsilon to avoid unnecessary trigonometric re-calculations
//             if (std::abs(smoothed_bass - last_bass) > 0.001f) {
//                 // LowShelf is more "amp-like" for bass
//                 bassFilter.setupLowShelf(120.0f, smoothed_bass, 0.707f, sampleRate);
//                 last_bass = smoothed_bass;
//             }
//
//             if (std::abs(smoothed_treble - last_treble) > 0.001f) {
//                 // HighShelf provides natural brilliance for treble
//                 trebleFilter.setupHighShelf(2200.0f, smoothed_treble, 0.707f, sampleRate);
//                 last_treble = smoothed_treble;
//             }
//
//             if (std::abs(smoothed_presence - last_presence) > 0.001f) {
//                 // Peaking at 3.5k - 4k hits the "bite" frequency of guitar speakers
//                 presenceFilter.setupPeaking(3800.0f, smoothed_presence, 0.6f, sampleRate);
//                 last_presence = smoothed_presence;
//             }
//
//             // 3. Sequential processing
//             float out = bassFilter.process(input);
//             out = trebleFilter.process(out);
//             out = presenceFilter.process(out);
//
//             // 4. Final gain stage and saturation
//             // Applying gain before clipping allows the EQ to shape the distortion character
//             return DSP::softClip(out * volume);
//         }
//
//
//         void reset () {
//             smoothed_bass = 0.0f;
//             smoothed_treble = 0.0f;
//             smoothed_presence = 0.0f;
//
//             bassFilter.reset();
//             trebleFilter.reset();
//             presenceFilter.reset();
//             last_bass = -999.0f;
//             last_treble = -999.0f;
//             last_presence = -999.0f;
//         }
//
//
//     private:
//         // Add these as class members
//         float smoothed_bass = 0.0f;
//         float smoothed_treble = 0.0f;
//         float smoothed_presence = 0.0f;
//         const float smoothing_factor = 0.001f; // Adjust between 0.0001 (slow) and 0.01 (fast)
//
//         Biquad bassFilter;
//         Biquad trebleFilter;
//         Biquad presenceFilter;
//         float last_bass = -999.0f, last_treble = -999.0f, last_presence = -999.0f;
//
//
//     };
//
// };


struct Biquad {
    float b0 = 1, b1 = 0, b2 = 0, a1 = 0, a2 = 0;
    float x1 = 0, x2 = 0, y1 = 0, y2 = 0;

    void reset() { x1 = x2 = y1 = y2 = 0; }


    void setupPeaking(float freq, float gainDb, float Q, float sampleRate) {
        float A = std::pow(10.0f, gainDb / 40.0f);

        float omega = 2.0f * M_PI * freq / sampleRate;
        float sn = std::sin(omega);
        float cs = std::cos(omega);

        float alpha = sn / (2.0f * Q);

        float a0 = 1.0f + alpha / A;
        b0 = (1.0f + alpha * A) / a0;
        b1 = (-2.0f * cs) / a0;
        b2 = (1.0f - alpha * A) / a0;
        a1 = (-2.0f * cs) / a0;
        a2 = (1.0f - alpha / A) / a0;
    }

    float process(float in) {
        float out = b0 * in + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;
        x2 = x1; x1 = in;
        y2 = y1; y1 = out;
        return out;
    }
};


    class ToneControl {
    public:
        ToneControl() = default;
        float process(float input, float volume,  float bass, float treble, float presence, float sampleRate) {
            if (bass != last_bass) {
                bassFilter.setupPeaking(100.0f, bass, 0.5f, sampleRate);
                last_bass = bass;
            }
            if (treble != last_treble) {
                trebleFilter.setupPeaking(2500.0f, treble, 0.5f, sampleRate);
                last_treble = treble;
            }
            if (presence != last_presence) {
                presenceFilter.setupPeaking(4000.0f, presence, 0.5f, sampleRate);
                last_presence = presence;
            }

            float out = bassFilter.process(input);
            out = trebleFilter.process(out);
            out = presenceFilter.process(out);


            float curVol = out;

            // out =  DSP::clamp(volume * curVol, -1.f, 1.f);
            out =  DSP::softClip(volume * curVol);


            return out;
        }



    void reset () {
        bassFilter.reset();
        trebleFilter.reset();
        presenceFilter.reset();
        last_bass = -999.0f;
        last_treble = -999.0f;
        last_presence = -999.0f;
    }


    private:
        Biquad bassFilter;
        Biquad trebleFilter;
        Biquad presenceFilter;
        float last_bass = -999.0f, last_treble = -999.0f, last_presence = -999.0f;
    };

};

}; //namespace
