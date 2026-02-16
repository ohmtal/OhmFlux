//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Digital Sound Processing : Equalizer (single Band)
//-----------------------------------------------------------------------------
// * using ISettings
//-----------------------------------------------------------------------------

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

#include <cmath>

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
    // default: {100.f, 0.f, 0.707f};
    struct EQBandData {
        float frequency; // e.g., 100.0f for Bass, 3000.0f for Presence
        float gainDb;    // e.g., +6.0f for boost, -6.0f for cut
        float Q;         // 0.707 is standard; higher is narrower
    };


    struct EQBandSettings : public ISettings {
        AudioParam<float> frequency { "Frequency" , 100.f, 20.f,  16000.f, "%.1f" };
        AudioParam<float> gainDb    { "Gain", 0.f, -6.f,  6.f, "%.2f db" };
        AudioParam<float> Q         { "Q", 0.707, 0.f,  1.414f, "%.3f"}; //range??

        EQBandSettings() = default;
        REGISTER_SETTINGS(EQBandSettings, &frequency, &gainDb, &Q)

        EQBandData getData() const {
            return { frequency.get(), gainDb.get(), Q.get()};
        }

        void setData(const EQBandData& data) {
            frequency.set(data.frequency);
            gainDb.set(data.gainDb) ;
            Q.set(data.Q);
        }

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
        EQBandSettings mSettings;
        BiquadCoeffs mCoeffs;
        std::vector<BiquadState> mStates;

        // Previous samples for Left and Right (Required for IIR filtering)
        float x1L = 0, x2L = 0, y1L = 0, y2L = 0;
        float x1R = 0, x2R = 0, y1R = 0, y2R = 0;

        void calculateCoefficients() {
            // Standard Audio EQ Cookbook formula for a Peaking EQ
            float A = pow(10.0f, mSettings.gainDb.get() / 40.0f);
            float omega = 2.0f * M_PI * mSettings.frequency.get() / mSampleRate;
            float sn = sin(omega);
            float cs = cos(omega);
            float alpha = sn / (2.0f * mSettings.Q.get());

            float a0 = 1.0f + alpha / A;
            mCoeffs.b0 = (1.0f + alpha * A) / a0;
            mCoeffs.b1 = (-2.0f * cs) / a0;
            mCoeffs.b2 = (1.0f - alpha * A) / a0;
            mCoeffs.a1 = (-2.0f * cs) / a0;
            mCoeffs.a2 = (1.0f - alpha / A) / a0;
        }

    public:
        IMPLEMENT_EFF_CLONE(Equalizer)

        Equalizer(bool switchOn = true) : Effect(DSP::EffectType::Equalizer, switchOn)
                , mSettings() {
            //default stereo
            mStates = { {0.f,0.f,0.f,0.f} , {0.f,0.f,0.f,0.f} };
            calculateCoefficients();
        }
        //----------------------------------------------------------------------
        virtual std::string getName() const override { return "Equalizer Band";}
        //----------------------------------------------------------------------
        EQBandSettings& getSettings() { return mSettings; }
        //----------------------------------------------------------------------
        void setSettings(const EQBandSettings& s) {
            mSettings = s;
            calculateCoefficients();
        }
        //----------------------------------------------------------------------
        void updateSettings(float freq, float gain) {
            mSettings.frequency.set( freq ) ;
            mSettings.gainDb.set( gain );
            calculateCoefficients();
        }
        //----------------------------------------------------------------------
        void save(std::ostream& os) const override {
            Effect::save(os);              // Save mEnabled
            mSettings.save(os);       // Save Settings
        }
        //----------------------------------------------------------------------
        bool load(std::istream& is) override {
            if (!Effect::load(is)) return false; // Load mEnabled
            return mSettings.load(is);      // Load Settings
        }
        //----------------------------------------------------------------------
        //----------------------------------------------------------------------
        virtual void process(float* buffer, int numSamples, int numChannels) override {
            if (!isEnabled()) return;

            // Ensure we have a state object for every channel
            if (mStates.size() != static_cast<size_t>(numChannels)) {
                mStates.assign(numChannels, BiquadState());
            }

            int channel = 0;
            for (int i = 0; i < numSamples; i++) {

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

                if (++channel >= numChannels) channel = 0;
            }
        }

        #ifdef FLUX_ENGINE
        virtual ImVec4 getColor() const  override { return ImVec4(0.2f, 0.2f, 0.2f, 1.0f);}
        // we have not extra gui here it must be added manually since it's a single band !
        virtual void renderUIWide() override {};
        virtual void renderUI() override {};
        virtual void renderPaddle() override {};

        #endif
    }; //Class

}
