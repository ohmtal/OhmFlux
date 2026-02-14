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
    struct Biquad {
        float b0 = 1, b1 = 0, b2 = 0, a1 = 0, a2 = 0;
        float x1 = 0, x2 = 0, y1 = 0, y2 = 0;

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
                presenceFilter.setupPeaking(6000.0f, presence, 0.5f, sampleRate);
                last_presence = presence;
            }

            float out = bassFilter.process(input);
            out = trebleFilter.process(out);
            out = presenceFilter.process(out);


            float curVol = out;
            out =  DSP::clamp(volume * curVol, -1.f, 1.f);

            return out;
        }
    private:
        Biquad bassFilter;
        Biquad trebleFilter;
        Biquad presenceFilter;
        float last_bass = -999.0f, last_treble = -999.0f, last_presence = -999.0f;
    };

};}; //namespace
