//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Drum Synth's:
//  - KickSynth
//  - SnareSynth
//  - HiHatSynth
//  - TomSynth
//  - CymbalsSynth
//-----------------------------------------------------------------------------
#pragma once

#include <vector>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <atomic>
#include <fstream>
#include <cstdlib>

#include "../DSP_Math.h" // fast_tanh




namespace DrumSynth {

    //-----------------------------------------------------------------------------
    // KickSynt
    // default values :
    //      std::atomic<float> kick_pitch{50.f};
    //      std::atomic<float> kick_decay{0.3f};
    //      std::atomic<float> kick_click{0.5f};
    //      std::atomic<float> kick_drive{1.5f}; * techno

    // Ranges:
    //     rackKnob("Pitch", sConfig.kick_pitch, {30.f, 100.0f}, ksGreen);
    //     rackKnob("Decay", sConfig.kick_decay, {0.01f, 2.0f}, ksGreen);
    //     rackKnob("Click", sConfig.kick_click, {0.0f, 1.0f}, ksGreen);
    //     rackKnob("Drive", sConfig.kick_drive, {1.0f, 5.0f}, ksRed); * techno

    class KickSynth {
    public:
        void trigger() {
            mPhase = 0.0f;
            mEnvelope = 1.0f; // full volume
            mPitchEnv = 1.0f; // highest pitch
            mActive = true;
        }

        // ------ normal processSample .........
        float processSample(float pitch, float decay, float click, float velocity, float sampleRate) {
            if (!mActive) return 0.0f;

            float dynamicClick = click * (0.5f + 0.5f * velocity);

            float currentFreq = pitch + (mPitchEnv * dynamicClick * 500.0f);

            // float phaseIncrement = (2.0f * (float)M_PI * currentFreq) / sampleRate;
            // mPhase += phaseIncrement;
            // if (mPhase >= 2.0f * (float)M_PI) mPhase -= 2.0f * (float)M_PI;
            // float signal = DSP::FastMath::fastSin(mPhase);
            // 1. Increment berechnen (0.0 bis 1.0 pro Kreislauf)
            float phaseIncrement = currentFreq / sampleRate;
            mPhase += phaseIncrement;
            if (mPhase >= 1.0f) mPhase -= 1.0f;
            float signal = DSP::FastMath::fastSin(mPhase);


            float tau = std::max(0.001f, decay);
            float multiplier = std::exp(-1.0f / (tau * sampleRate));
            mEnvelope *= multiplier;

            float pitchMultiplier = std::exp(-5.0f / (tau * sampleRate));
            mPitchEnv *= pitchMultiplier;

            if (mEnvelope < 0.0001f) {
                mEnvelope = 0.0f;
                mActive = false;
                mPitchEnv = 0.0f;
                mPhase = 0.0f;
            }

            return signal * mEnvelope * velocity * 0.5f;
        }

        // pre velocity
        // float processSample(float pitch, float decay, float click, float sampleRate) {
        //     if (!mActive) return 0.0f;
        //
        //     float currentFreq = pitch + (mPitchEnv * click * 500.0f);
        //
        //     float phaseIncrement = (2.0f * M_PI * currentFreq) / sampleRate;
        //     mPhase += phaseIncrement;
        //     if (mPhase >= 2.0f * M_PI) mPhase -= 2.0f * M_PI;
        //     float signal = std::sin(mPhase);
        //
        //     float tau = std::max(0.001f, decay);
        //     float multiplier = std::exp(-1.0f / (tau * sampleRate));
        //
        //     mEnvelope *= multiplier;
        //
        //     mPitchEnv *= std::exp(-5.0f / (tau * sampleRate));
        //
        //     if (mEnvelope < 0.0001f) {
        //         mEnvelope = 0.0f;
        //         mActive = false;
        //         mPitchEnv = 0.0f;
        //     }
        //
        //     return signal * mEnvelope * 0.5f;
        // }
        // ------ normal processSample with Drive (techno style).........

        float processSampleDrive(float pitch, float decay, float click, float drive, float velocity, float sampleRate) {
            if (!mActive) return 0.0f;

            // 1. Envelop
            float safeDecay = std::max(0.01f, decay);
            mPitchEnv *= std::exp(-15.0f / (safeDecay * sampleRate));
            mEnvelope *= std::exp(-1.0f / (safeDecay * sampleRate));

            // 2. Oszillator
            float dynamicClick = click * (0.5f + 0.5f * velocity);
            float currentFreq = pitch + (mPitchEnv * dynamicClick * 500.0f);

            float phaseIncrement = currentFreq / sampleRate;
            mPhase += phaseIncrement;
            if (mPhase >= 1.0f) mPhase -= 1.0f;
            float signal = DSP::FastMath::fastSin(mPhase);


            float saturatedSignal = std::tanh(signal * drive);

            // 4. Kill-Switch
            if (mEnvelope < 0.0001f) {
                mEnvelope = 0.0f; mPitchEnv = 0.0f; mActive = false; mPhase = 0.0f;
            }

            // 5. normalize
            float outputGain = 1.0f / (1.0f + (drive * 0.2f));

            return saturatedSignal * outputGain * velocity;
        }

        // float processSampleDrive(float pitch, float decay, float click,float drive, float velocity,float sampleRate) {
        //     if (!mActive) return 0.0f;
        //
        //     float dynamicClick = click * (0.5f + 0.5f * velocity);
        //
        //     float pitchEnvMult = std::exp(-15.0f / (std::max(0.01f, decay) * sampleRate));
        //     mPitchEnv *= pitchEnvMult;
        //
        //     float currentFreq = pitch + (mPitchEnv * dynamicClick * 500.0f);
        //
        //     float phaseIncrement = (2.0f * M_PI * currentFreq) / sampleRate;
        //     mPhase += phaseIncrement;
        //     if (mPhase >= 2.0f * M_PI) mPhase -= 2.0f * M_PI;
        //
        //     float rawOscillator = std::sin(mPhase);
        //
        //     float ampEnvMult = std::exp(-1.0f / (std::max(0.01f, decay) * sampleRate));
        //     mEnvelope *= ampEnvMult;
        //
        //     float signal = rawOscillator * mEnvelope;
        //
        //     float saturatedSignal = DSP::fast_tanh(signal * drive);
        //
        //     if (mEnvelope < 0.0001f) {
        //         mEnvelope = 0.0f;
        //         mPitchEnv = 0.0f;
        //         mActive = false;
        //         mPhase = 0.0f;
        //     }
        //
        //     float outputGain = 1.0f / std::sqrt(drive);
        //
        //     return saturatedSignal * outputGain * velocity;
        // }

        // ------ normal processSample_orig like processSample .........
        // maybe removed ... basiclly the same as processSample

        float processSample_variant(float pitch, float decay, float click, float velocity, float sampleRate) {
            if (!mActive) return 0.0f;

            float dynamicClick = click * (0.5f + 0.5f * velocity);

            float startFreq = pitch;
            float clickAmount = dynamicClick * 500.0f; // Extra-Punch in Hz
            float currentFreq = startFreq + (mPitchEnv * clickAmount);


            float phaseIncrement = (2.0f * M_PI * currentFreq) / sampleRate;
            mPhase += phaseIncrement;
            if (mPhase >= 2.0f * M_PI) mPhase -= 2.0f * M_PI;
            float signal = DSP::FastMath::fastSin(mPhase);


            float decayFactor = 1.0f / (decay * sampleRate);

            mEnvelope -= decayFactor;
            // mPitchEnv -= decayFactor * 4.0f;
            mPitchEnv = std::max(0.0f, mPitchEnv - (decayFactor * 4.0f));

            if (mEnvelope <= 0.0f) {
                mEnvelope = 0.0f;
                mActive = false;
            }

            return signal * mEnvelope * velocity * 0.5f;
        }

    protected:
        float mPhase = 0.0f;
        float mEnvelope = 0.0f;
        float mPitchEnv = 0.0f;
        bool mActive = false;
    };

    //-----------------------------------------------------------------------------
    // Snare Drum
    //   Default Values:
    //       std::atomic<float> snare_pitch{180.f};
    //       std::atomic<float> snare_decay{0.2f};
    //       std::atomic<float> snare_snappy{0.7f};
    //   Ranges:
    //       rackKnob("Pitch", sConfig.snare_pitch, {100.f, 400.0f}, ksGreen);
    //       rackKnob("Decay", sConfig.snare_decay, {0.01f, 1.0f}, ksGreen);
    //       rackKnob("Snappy", sConfig.snare_snappy, {0.0f, 1.0f}, ksYellow);
    //-----------------------------------------------------------------------------
    class SnareSynth {
    public:
        void trigger() {
            mEnvelope = 1.0f;
            mActive = true;
            mPhase = 0.0f;
        }

        float processSample(float pitch, float decay, float snappy, float sampleRate) {
            if (!mActive) return 0.0f;

            // 1. Noise part (snappy)
            float noise = ((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f;
            
            float phaseIncrement = pitch / sampleRate;
            mPhase += phaseIncrement;
            if (mPhase >= 1.0f) mPhase -= 1.0f;
            float body = DSP::FastMath::fastSin(mPhase);


            // 3. Envelope
            float tau = std::max(0.001f, decay);
            float multiplier = std::exp(-1.0f / (tau * sampleRate));
            mEnvelope *= multiplier;

            if (mEnvelope < 0.0001f) {
                mEnvelope = 0.0f;
                mActive = false;
            }

            // Mix body and noise
            float signal = (body * (1.0f - snappy) + noise * snappy) * mEnvelope;
            return signal * 0.5f;
        }

    private:
        float mEnvelope = 0.0f;
        float mPhase = 0.0f;
        bool mActive = false;
    };

    //-----------------------------------------------------------------------------
    // HiHat
    //     Default Values:
    //         std::atomic<float> hihat_decay{0.05f};
    //         std::atomic<float> hihat_pitch{3000.f};
    //     Ranges:
    //         rackKnob("Decay", sConfig.hihat_decay, {0.01f, 0.5f}, ksGreen);
    //         rackKnob("Pitch", sConfig.hihat_pitch, {1000.f, 8000.0f}, ksBlue);
    //-----------------------------------------------------------------------------
    class HiHatSynth {
    public:
        void trigger() {
            mEnvelope = 1.0f;
            mActive = true;
        }

        float processSample(float decay, float pitch, float sampleRate) {
            if (!mActive) return 0.0f;

            // Metallic noise (sum of high freq sines or just filtered noise)
            float noise = ((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f;
            
            // Simple high pass filter (one-pole)
            float alpha = 0.8f;
            mLastOut = alpha * (mLastOut + noise - mLastIn);
            mLastIn = noise;

            // Envelope
            float tau = std::max(0.001f, decay);
            float multiplier = std::exp(-1.0f / (tau * sampleRate));
            mEnvelope *= multiplier;

            if (mEnvelope < 0.0001f) {
                mEnvelope = 0.0f;
                mActive = false;
            }

            return mLastOut * mEnvelope * 0.4f;
        }

    private:
        float mEnvelope = 0.0f;
        float mLastOut = 0.0f;
        float mLastIn = 0.0f;
        bool mActive = false;
    };

    //-----------------------------------------------------------------------------
    // Tom Drum
    //     Default Values:
    //         std::atomic<float> tom_pitch{100.f};
    //         std::atomic<float> tom_decay{0.3f};
    //         std::atomic<float> tom_click{0.4f};
    //     Ranges:
    //         rackKnob("Pitch", sConfig.tom_pitch, {60.f, 300.0f}, ksGreen);
    //         rackKnob("Decay", sConfig.tom_decay, {0.01f, 2.0f}, ksGreen);
    //         rackKnob("Click", sConfig.tom_click, {0.0f, 1.0f}, ksGreen);
    //-----------------------------------------------------------------------------
    class TomSynth {
    public:
        void trigger() {
            mPhase = 0.0f;
            mEnvelope = 1.0f;
            mPitchEnv = 1.0f;
            mActive = true;
        }

        float processSample(float pitch, float decay, float click, float sampleRate) {
            if (!mActive) return 0.0f;

            float currentFreq = pitch + (mPitchEnv * click * 300.0f);


            // mPhase += (2.0f * M_PI * currentFreq) / sampleRate;
            // if (mPhase >= 2.0f * M_PI) mPhase -= 2.0f * M_PI;
            // float body = std::sin(mPhase);
            // replacement:
            float phaseIncrement = currentFreq / sampleRate;
            mPhase += phaseIncrement;
            if (mPhase >= 1.0f) mPhase -= 1.0f;
            float body = DSP::FastMath::fastSin(mPhase);


            float tau = std::max(0.001f, decay);
            float multiplier = std::exp(-1.0f / (tau * sampleRate));
            mEnvelope *= multiplier;
            mPitchEnv *= std::exp(-10.0f / (tau * sampleRate));

            if (mEnvelope < 0.0001f) {
                mEnvelope = 0.0f;
                mActive = false;
            }

            return body * mEnvelope * 0.5f;
        }

    private:
        float mPhase = 0.0f;
        float mEnvelope = 0.0f;
        float mPitchEnv = 0.0f;
        bool mActive = false;
    };

    //-----------------------------------------------------------------------------
    // Cymbals
    //     Default Values:
    //         std::atomic<float> cymbals_decay{0.5f};
    //         std::atomic<float> cymbals_pitch{4000.f};
    //     Ranges:
    //         rackKnob("Decay", sConfig.cymbals_decay, {0.01f, 2.0f}, ksGreen); //0..1?
    //         rackKnob("Pitch", sConfig.cymbals_pitch, {1000.0f, 8000.0f}, ksGreen);
    //-----------------------------------------------------------------------------
    class CymbalsSynth {
    public:
        void trigger() {
            mEnvelope = 1.0f;
            mActive = true;
        }

        float processSample(float decay, float pitch, float sampleRate) {
            if (!mActive) return 0.0f;

            float noise = ((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f;
            
            // HPF cutoff based on pitch
            float dt = 1.0f / sampleRate;
            float rc = 1.0f / (2.0f * M_PI * std::max(100.0f, pitch));
            float alpha = rc / (rc + dt);
            
            float out = alpha * (mLastOut + noise - mLastIn);
            mLastIn = noise;
            mLastOut = out;

            // Envelope
            float tau = std::max(0.01f, decay);
            float multiplier = std::exp(-1.0f / (tau * sampleRate));
            mEnvelope *= multiplier;

            if (mEnvelope < 0.0001f) {
                mEnvelope = 0.0f;
                mActive = false;
            }

            return mLastOut * mEnvelope * 0.4f;
        }

    private:
        float mEnvelope = 0.0f;
        float mLastOut = 0.0f;
        float mLastIn = 0.0f;
        bool mActive = false;
    };
    //-----------------------------------------------------------------------------

};

