//-----------------------------------------------------------------------------
// Copyright (c) 2026 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Digital Sound Processing : Equalizer (single Band)
//-----------------------------------------------------------------------------
// USAGE Example:
// ==============
// DSP::Equalizer lowBand({100.0f, 3.0f, 0.707f});   // +3dB Bass boost
// DSP::Equalizer midBand({1000.0f, -2.0f, 0.707f}); // -2dB Mid cut
// DSP::Equalizer highBand({5000.0f, 4.0f, 0.707f});  // +4dB Treble boost
//
// // In your main audio loop:
// lowBand.process(buffer, size);
// midBand.process(buffer, size);
// highBand.process(buffer, size);
//-----------------------------------------------------------------------------
#pragma once

#define _USE_MATH_DEFINES // Required for M_PI on some systems (like Windows/MSVC)
#include <cmath>          // Provides pow, sin, cos, and math constants

#include <vector>
#include <cstdint>
#include <algorithm>
#include <cstring>

#ifdef FLUX_ENGINE
#include <imgui.h>
#include <imgui_internal.h>
#include <gui/ImFlux.h>
#endif


#include "DSP_Effect.h"



namespace DSP {


    struct EQBand {
        float frequency; // e.g., 100.0f for Bass, 3000.0f for Presence
        float gainDb;    // e.g., +6.0f for boost, -6.0f for cut
        float Q;         // 0.707 is standard; higher is narrower

        static const uint8_t CURRENT_VERSION = 1;
        void getBinary(std::ostream& os) const {
            uint8_t ver = CURRENT_VERSION;
            DSP_STREAM_TOOLS::write_binary(os, ver);

            DSP_STREAM_TOOLS::write_binary(os, frequency);
            DSP_STREAM_TOOLS::write_binary(os, gainDb);
            DSP_STREAM_TOOLS::write_binary(os, Q);

        }

        bool  setBinary(std::istream& is) {
            uint8_t fileVersion = 0;
            DSP_STREAM_TOOLS::read_binary(is, fileVersion);
            if (fileVersion != CURRENT_VERSION) return false;

            DSP_STREAM_TOOLS::read_binary(is, frequency);
            DSP_STREAM_TOOLS::read_binary(is, gainDb);
            DSP_STREAM_TOOLS::read_binary(is, Q);

            return  is.good();
        }

        auto operator<=>(const EQBand&) const = default; //C++20 lazy way

    };

    // Coefficients for the Biquad filter formula
    struct BiquadCoeffs {
        float b0, b1, b2, a1, a2;
    };

    class Equalizer : public DSP::Effect {
        struct BiquadState {
            float x1 = 0.0f, x2 = 0.0f; // Input history
            float y1 = 0.0f, y2 = 0.0f; // Output history
        };

    private:
        EQBand mSettings;
        BiquadCoeffs mCoeffs;

        std::vector<BiquadState> mStates;


        // Previous samples for Left and Right (Required for IIR filtering)
        float x1L = 0, x2L = 0, y1L = 0, y2L = 0;
        float x1R = 0, x2R = 0, y1R = 0, y2R = 0;

        void calculateCoefficients() {
            // Standard Audio EQ Cookbook formula for a Peaking EQ
            float sampleRate = getSampleRateF();
            float A = pow(10.0f, mSettings.gainDb / 40.0f);
            float omega = 2.0f * M_PI * mSettings.frequency / sampleRate;
            float sn = sin(omega);
            float cs = cos(omega);
            float alpha = sn / (2.0f * mSettings.Q);

            float a0 = 1.0f + alpha / A;
            mCoeffs.b0 = (1.0f + alpha * A) / a0;
            mCoeffs.b1 = (-2.0f * cs) / a0;
            mCoeffs.b2 = (1.0f - alpha * A) / a0;
            mCoeffs.a1 = (-2.0f * cs) / a0;
            mCoeffs.a2 = (1.0f - alpha / A) / a0;
        }

    public:
        IMPLEMENT_EFF_CLONE(Equalizer)

        Equalizer(bool switchOn = true) : Effect(switchOn) {
            mSettings = {100.f, 0.f, 0.707f};

            //default stereo
            mStates = { {0.f,0.f,0.f,0.f} , {0.f,0.f,0.f,0.f} };

            calculateCoefficients();
        }

        const EQBand& getSettings() { return mSettings; }

        void setSettings(const EQBand& s) {
            mSettings = s;
            calculateCoefficients();
        }


        void updateSettings(float freq, float gain) {
            mSettings.frequency = freq;
            mSettings.gainDb = gain;
            calculateCoefficients();
        }

        DSP::EffectType getType() const override { return DSP::EffectType::Equalizer; }

        void save(std::ostream& os) const override {
            Effect::save(os);              // Save mEnabled
            mSettings.getBinary(os);       // Save Settings
        }

        bool load(std::istream& is) override {
            if (!Effect::load(is)) return false; // Load mEnabled
            return mSettings.setBinary(is);      // Load Settings
        }


        virtual void process(float* buffer, int numSamples, int numChannels) override {
            if (!isEnabled()) return;

            // Ensure we have a state object for every channel
            if (mStates.size() != static_cast<size_t>(numChannels)) {
                mStates.assign(numChannels, BiquadState());
            }

            for (int i = 0; i < numSamples; i++) {
                int channel = i % numChannels;
                float in = buffer[i];

                // Access the state for the current channel
                BiquadState& s = mStates[channel];

                // 1. Calculate the Direct Form I Biquad equation
                float out = mCoeffs.b0 * in + mCoeffs.b1 * s.x1 + mCoeffs.b2 * s.x2
                - mCoeffs.a1 * s.y1 - mCoeffs.a2 * s.y2;

                // 2. Update state history for this channel
                s.x2 = s.x1;
                s.x1 = in;
                s.y2 = s.y1;
                s.y1 = out;

                // 3. Write back to buffer
                buffer[i] = out;
            }
        }

        // virtual void process(float* buffer, int numSamples, int numChannels) override {
        //     if (numChannels !=  2) { return;  }  //FIXME REWRITE from stereo TO variable CHANNELS
        //     if (!isEnabled()) return;
        //
        //     for (int i = 0; i < numSamples; i++) {
        //         float in = buffer[i];
        //         float out;
        //
        //         if (i % 2 == 0) { // Left Channel
        //             out = mCoeffs.b0 * in + mCoeffs.b1 * x1L + mCoeffs.b2 * x2L
        //             - mCoeffs.a1 * y1L - mCoeffs.a2 * y2L;
        //             x2L = x1L; x1L = in;
        //             y2L = y1L; y1L = out;
        //         } else { // Right Channel
        //             out = mCoeffs.b0 * in + mCoeffs.b1 * x1R + mCoeffs.b2 * x2R
        //             - mCoeffs.a1 * y1R - mCoeffs.a2 * y2R;
        //             x2R = x1R; x1R = in;
        //             y2R = y1R; y1R = out;
        //         }
        //         buffer[i] = out;
        //     }
        // }

        virtual std::string getName() const override { return "Equalizer Band";}
        #ifdef FLUX_ENGINE
        virtual ImVec4 getColor() const  override { return ImVec4(0.2f, 0.2f, 0.2f, 1.0f);}
        // we have not extra gui here it must be added manually since it's a single band !
        virtual void renderUIWide() override {};
        virtual void renderUI() override {};

        #endif
    }; //Class

}
