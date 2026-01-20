//-----------------------------------------------------------------------------
// Copyright (c) 2026 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Digital Sound Processing : 9 Band Equalizer
// ISO Standard Frequencies for the 9 bands (63Hz to 16kHz).
//-----------------------------------------------------------------------------
#pragma once
#define _USE_MATH_DEFINES // Required for M_PI on some systems (like Windows/MSVC)
#include <cmath>          // Provides pow, sin, cos, and math constants

#include <vector>
#include <cstdint>
#include <algorithm>
#include <cstring>

#include "DSP_Effect.h"
#include "DSP_Equilizer.h"

namespace DSP {

    struct Equalizer9BandSettings {
        std::array<float, 9> gains; // Gain for each band in dB
    };

    // Preset: Flat (No change)
    constexpr Equalizer9BandSettings FLAT_EQ = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
    // Preset: Bass Boost (Classic "V" shape but focused on low end)
    constexpr Equalizer9BandSettings BASS_BOOST_EQ =  { 5.5f, 4.0f, 2.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
    // Preset: "Loudness" / Smile (Boosted lows and highs, dipped mids)
    constexpr Equalizer9BandSettings SMILE_EQ = {4.5f, 3.0f, 0.0f, -2.0f, -3.5f, -2.0f, 0.0f, 3.0f, 4.5f };
    // Preset: Radio / Telephone (Cuts lows and highs, boosts mids)
    constexpr Equalizer9BandSettings RADIO_EQ = {-12.0f, -12.0f, -6.0f, 3.0f, 6.0f, 3.0f, -6.0f, -12.0f, -12.0f };
    // Preset: Air / Clarity (Subtle low cut, boost in the presence and high frequencies)
    constexpr Equalizer9BandSettings CLARITY_EQ = {-2.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.5f, 3.0f, 4.5f, 6.0f };



    struct FilterState {
        float x1 = 0, x2 = 0, y1 = 0, y2 = 0;
    };



    class Equalizer9Band : public DSP::Effect {
    private:
        Equalizer9BandSettings mSettings;
        static constexpr int NUM_BANDS = 9;
        // Standard ISO 9-band center frequencies
        const float mFrequencies[NUM_BANDS] = { 63.0f, 125.0f, 250.0f, 500.0f, 1000.0f, 2000.0f, 4000.0f, 8000.0f, 16000.0f };

        BiquadCoeffs mCoeffs[NUM_BANDS];
        FilterState mStateL[NUM_BANDS];
        FilterState mStateR[NUM_BANDS];
        float mSampleRate = 44100.0f;

        void calculateBand(int band) {
            float A = pow(10.0f, mSettings.gains[band] / 40.0f);
            float omega = 2.0f * M_PI * mFrequencies[band] / mSampleRate;
            float sn = sin(omega);
            float cs = cos(omega);
            float Q = 1.414f; // Steepness tuned for 9-band spacing
            float alpha = sn / (2.0f * Q);

            float a0 = 1.0f + alpha / A;
            mCoeffs[band].b0 = (1.0f + alpha * A) / a0;
            mCoeffs[band].b1 = (-2.0f * cs) / a0;
            mCoeffs[band].b2 = (1.0f - alpha * A) / a0;
            mCoeffs[band].a1 = (-2.0f * cs) / a0;
            mCoeffs[band].a2 = (1.0f - alpha / A) / a0;
        }

        void updateAllBands() {
            for (int i = 0; i < 9; i++) calculateBand(i);
        }

    public:
        Equalizer9Band(bool switchOn = false, float sampleRate = 44100.0f)
        : Effect(switchOn)
        , mSampleRate(sampleRate)
        {
            setSettings(FLAT_EQ);
            updateAllBands();
        }


        float getSampleRate() const { return mSampleRate; }

        void setSampleRate(float newRate) {
            if (newRate <= 0) return;
            mSampleRate = newRate;
            updateAllBands();
        }


        Equalizer9BandSettings getSettings() const { return mSettings; }

        // Setter for loading
        void setSettings(const Equalizer9BandSettings& s) {
            mSettings = s;
            updateAllBands();

        }

        // Update specific band
        void setGain(int band, float db) {
            if (band < 0 || band >= 9) return;
            mSettings.gains[band] = db;
            calculateBand(band);
        }

        float getGain(int band) const {
            if (band >= 0 && band < 9) return mSettings.gains[band];
            return 0.0f;
        }

        virtual void process(float* buffer, int numSamples) override {
            if (!isEnabled()) return;

            for (int i = 0; i < numSamples; i++) {
                float sample = buffer[i];
                bool isLeft = (i % 2 == 0);

                // Cascade the sample through all 9 filters
                for (int b = 0; b < NUM_BANDS; b++) {
                    FilterState& s = isLeft ? mStateL[b] : mStateR[b];
                    BiquadCoeffs& c = mCoeffs[b];

                    float out = c.b0 * sample + c.b1 * s.x1 + c.b2 * s.x2
                    - c.a1 * s.y1 - c.a2 * s.y2;

                    s.x2 = s.x1; s.x1 = sample;
                    s.y2 = s.y1; s.y1 = out;

                    sample = out; // Result of this band is input to next band
                }
                buffer[i] = sample;
            }
        }
    };

} // namespace DSP
