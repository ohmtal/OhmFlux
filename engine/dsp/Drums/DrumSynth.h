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



namespace DSP {
namespace DrumSynth {

    //-----------------------------------------------------------------------------
    // KickSynt
    // default values :
    //      std::atomic<float> kick_pitch{50.f};
    //      std::atomic<float> kick_decay{0.3f};
    //      std::atomic<float> kick_click{0.5f};
    //      std::atomic<float> kick_drive{1.5f};
    //      std::atomic<float> kick_velocity{0.f};

    // Ranges:
    //     rackKnob("Pitch", sConfig.kick_pitch, {30.f, 100.0f}, ksGreen);
    //     rackKnob("Decay", sConfig.kick_decay, {0.01f, 2.0f}, ksGreen);
    //     rackKnob("Click", sConfig.kick_click, {0.0f, 1.0f}, ksGreen);
    //     rackKnob("Drive", sConfig.kick_drive, {1.0f, 5.0f}, ksRed); * techno
    //     rackKnob("Velocity", sConfig.kick_velocity, {0.0f, 1.0f}, ksBlack);

    class KickSynth {
    public:
        void trigger() {
            mPhase = 0.0f;
            mEnvelope = 1.0f; // full volume
            mPitchEnv = 1.0f; // highest pitch
            mActive = true;
        }

        void stop() {
            // no chance :P
        }

        float processSample(float pitch, float decay, float click, float drive, float velocity, float sampleRate) {
            if (!mActive || velocity < 0.01f ) return 0.0f;

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

            // 6. Apply Envelope
            float finalSignal = saturatedSignal * mEnvelope;

            return finalSignal * outputGain * velocity;

        }

    protected:
        float mPhase = 0.0f;
        float mEnvelope = 0.0f;
        float mPitchEnv = 0.0f;
        bool mActive = false;
    };

    //-----------------------------------------------------------------------------
    // Snare Drum
    //-----------------------------------------------------------------------------
    //   Default Values:
    //       std::atomic<float> snare_pitch{180.f};
    //       std::atomic<float> snare_decay{0.2f};
    //       std::atomic<float> snare_noiseAmount{0.7f};
    //       std::atomic<float> snare_drive{1.5f};
    //       std::atomic<float> snare_velocity{1.f};
    //
    //   Ranges:
    //       rackKnob("Pitch", sConfig.snare_pitch, {100.f, 400.0f}, ksGreen);
    //       rackKnob("Decay", sConfig.snare_decay, {0.01f, 1.0f}, ksGreen);
    //       rackKnob("Snappy", sConfig.snare_noiseAmount, {0.0f, 1.0f}, ksYellow);
    //       rackKnob("Drive", sConfig.snare_drive, {1.0f, 10.0f}, ksYellow);
    //        rackKnob("velocity", sConfig.snare_velocity, {1.0f, 1.0f}, ksBlack);

    // Parameter-Mapping & Ranges
    // Argument	Knob-Label	Range	Default	Beschreibung
    // pitch	Pitch	100.f - 400.f	180.f	Basis-Frequenz des Sinus-Oszillators.
    // decay	Decay	0.01f - 1.0f	0.2f	Länge der Amplituden- und Pitch-Hüllkurve.
    // noiseAmount	Snappy	0.0f - 1.0f	0.7f	Anteil des "Teppich"-Rauschens am Gesamtsignal.
    // drive	Drive	1.0f - 10.0f	1.5f	Stärke der tanh-Sättigung (verdichtet den Sound).
    // velocity	(Intern)	0.0f - 1.0f	-	Anschlagstärke vom Sequencer.

    //-----------------------------------------------------------------------------
    class SnareSynth {
    public:
        void trigger() {
            mEnvelope = 1.0f;
            mActive = true;
            mPhase = 0.0f;
            mPitchEnv = 0.0f;
        }

        // hold
        void stop() {
            mPitchEnv = 0.f;
            mEnvelope = 0.0f;
            mPhase = 0.0f;
            mActive = false;
        }


        float processSample(float pitch, float decay, float noiseAmount, float drive, float velocity, float sampleRate) {
            if (!mActive || velocity < 0.01f ) return 0.0f;

            // 1. Envelopes (Snare needs a faster pitch drop than a kick)
            float safeDecay = std::max(0.01f, decay);
            mPitchEnv *= std::exp(-40.0f / (safeDecay * sampleRate)); // Fast snap
            mEnvelope *= std::exp(-2.5f / (safeDecay * sampleRate));  // Snare decay

            // 2. Oscillator (Body)
            float currentFreq = pitch + (mPitchEnv * 400.0f);
            mPhase += currentFreq / sampleRate;
            if (mPhase >= 1.0f) mPhase -= 1.0f;
            float body = DSP::FastMath::fastSin(mPhase);

            // 3. Noise (The "Sizzle")
            // Simple white noise: ((float)rand() / RAND_MAX) * 2.0f - 1.0f;
            float sizzle = ((float)std::rand() / (float)RAND_MAX * 2.0f - 1.0f);

            // High-pass filter for the noise (optional but recommended for "crispy" feel)
            // float filteredNoise = sizzle - mPrevNoise; mPrevNoise = sizzle; // Simple HP

            // 4. Mix & Saturate
            float mixed = (body * 0.5f) + (sizzle * noiseAmount);
            float saturated = std::tanh(mixed * drive * (0.5f + 0.5f * velocity));

            // 5. Kill-Switch (a bit higher for snares to avoid "static" sizzle tail)
            if (mEnvelope < 0.001f) {
                mActive = false; mEnvelope = 0.0f; mPhase = 0.0f;
            }

            // 6. Apply Envelope & Velocity
            return saturated * mEnvelope * velocity;
        }

    private:
        float mPitchEnv = 0.f;
        float mEnvelope = 0.0f;
        float mPhase = 0.0f;
        bool mActive = false;
    };




    //-----------------------------------------------------------------------------
    //  Variant 1
    //   Default Values:
    //       std::atomic<float> snare_pitch{180.f};
    //       std::atomic<float> snare_decay{0.2f};
    //       std::atomic<float> snare_snappy{0.7f};
    //   Ranges:
    //       rackKnob("Pitch", sConfig.snare_pitch, {100.f, 400.0f}, ksGreen);
    //       rackKnob("Decay", sConfig.snare_decay, {0.01f, 1.0f}, ksGreen);
    //       rackKnob("Snappy", sConfig.snare_snappy, {0.0f, 1.0f}, ksYellow);
    //-----------------------------------------------------------------------------
    // class SnareSynth_Variant1 {
    // public:
    //     void trigger() {
    //         mEnvelope = 1.0f;
    //         mActive = true;
    //         mPhase = 0.0f;
    //     }
    //
    //     float processSample(float pitch, float decay, float snappy, float sampleRate) {
    //         if (!mActive) return 0.0f;
    //
    //         // 1. Noise part (snappy)
    //         float noise = ((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f;
    //
    //         float phaseIncrement = pitch / sampleRate;
    //         mPhase += phaseIncrement;
    //         if (mPhase >= 1.0f) mPhase -= 1.0f;
    //         float body = DSP::FastMath::fastSin(mPhase);
    //
    //
    //         // 3. Envelope
    //         float tau = std::max(0.001f, decay);
    //         float multiplier = std::exp(-1.0f / (tau * sampleRate));
    //         mEnvelope *= multiplier;
    //
    //         if (mEnvelope < 0.0001f) {
    //             mEnvelope = 0.0f;
    //             mActive = false;
    //         }
    //
    //         // Mix body and noise
    //         float signal = (body * (1.0f - snappy) + noise * snappy) * mEnvelope;
    //         return signal * 0.5f;
    //     }
    //
    // private:
    //     float mEnvelope = 0.0f;
    //     float mPhase = 0.0f;
    //     bool mActive = false;
    // };

    //-----------------------------------------------------------------------------
    // HiHat

    // Parameter	Function Argument	Typical Range	Note
    // Pitch	pitch	5000.f - 18000.f	Controls the high-pass cutoff frequency.
    // Decay	decay	0.01f - 0.3f	0.05f is a perfect "Closed Hat".
    // Drive	drive	1.0f - 15.0f	Adds grit and digital sizzle.

    // Example:
    // Closed: processSample(14000.f, 0.05f, 5.f, velocity, sr) (Hell & kurz)
    // Open: processSample(8000.f, 0.5f, 3.f, velocity, sr) (Tiefer & lang)
    //-----------------------------------------------------------------------------
    class HiHatSynth {
    public:
        HiHatSynth() : mNoiseState(0xACE12345) {} // IMPORTANT: Seed must be non-zero

        void trigger() {
            mEnvelope = 1.0f;
            mActive = true;
        }

        // used for open hihat when a closed it triggered :D
        void stop() {
            mEnvelope = 0.0f;
            mActive = false;
        }


        float processSample(float pitch, float decay, float drive, float velocity, float sampleRate) {
            if (!mActive || velocity < 0.01f) return 0.0f;

            // 1. Envelope
            float safeDecay = std::max(0.005f, decay);
            mEnvelope *= std::exp(-10.0f / (safeDecay * sampleRate));

            // 2. Xorshift (Only works if mNoiseState != 0)
            mNoiseState ^= (mNoiseState << 13);
            mNoiseState ^= (mNoiseState >> 17);
            mNoiseState ^= (mNoiseState << 5);

            // Use unsigned cast for proper 32-bit mapping
            float noise = (static_cast<float>(mNoiseState) * (1.0f / 2147483647.0f)) - 1.0f;

            // 3. High-pass (Alpha check)
            // For 12kHz @ 44.1kHz sampleRate, alpha is ~0.27
            // open hihat :
            float dynamicPitch = pitch * (1.0f + mEnvelope * 0.05f); // Pitch sinkt leicht mit dem Decay
            float alpha = std::clamp(dynamicPitch / sampleRate, 0.01f, 0.99f);

            // float alpha = std::clamp(pitch / sampleRate, 0.01f, 0.99f);
            mFilterState += alpha * (noise - mFilterState);
            float hihatSignal = noise - mFilterState;

            // 4. Saturation
            float saturated = std::tanh(hihatSignal * drive * (0.5f + 0.5f * velocity));

            // 5. Kill-Switch
            if (mEnvelope < 0.001f) {
                mActive = false;
                mEnvelope = 0.0f;
                mFilterState = 0.0f;
            }

            return saturated * mEnvelope * velocity;
        }

    private:
        float mEnvelope = 0.0f;
        uint32_t mNoiseState = 0xACE12345; // Use uint32_t and non-zero seed
        float mFilterState = 0.0f;
        bool mActive = false;
    };



    //-----------------------------------------------------------------------------
    // Tom Drum
    // pitch:  80 - 150 Hz
    // decay: 0.2 - 0.6 s
    // drive: 3.0 (gives it that "punchy" floor tom feel)
    //-----------------------------------------------------------------------------
    class TomSynth {
    public:
        void trigger() {
            mPhase = 0.0f;
            mEnvelope = 1.0f;
            mPitchEnv = 1.0f;
            mActive = true;
        }
        // hold - no chance !
        void stop() {
        }


        float processSample(float pitch, float decay, float drive, float velocity, float sampleRate) {
            if (!mActive || velocity < 0.01f) return 0.0f;

            // 1. Envelopes
            float safeDecay = std::max(0.01f, decay);
            // Pitch drops slower than a kick for that "boing"
            mPitchEnv *= std::exp(-7.0f / (safeDecay * sampleRate));
            mEnvelope *= std::exp(-1.5f / (safeDecay * sampleRate));

            // 2. Oscillator with Pitch Sweep
            // Toms have a very characteristic pitch drop
            float currentFreq = pitch + (mPitchEnv * pitch * 1.5f);
            mPhase += currentFreq / sampleRate;
            if (mPhase >= 1.0f) mPhase -= 1.0f;

            float body = DSP::FastMath::fastSin(mPhase);

            // 3. Saturation & Velocity
            // Drive makes it sound like a real drum head being hit hard
            float saturated = std::tanh(body * drive * (0.7f + 0.3f * velocity));

            // 4. Kill-Switch
            if (mEnvelope < 0.001f) {
                mActive = false; mEnvelope = 0.0f; mPhase = 0.0f;
            }

            return saturated * mEnvelope * velocity;
        }


    private:
        float mPhase = 0.0f;
        float mEnvelope = 0.0f;
        float mPitchEnv = 0.0f;
        bool mActive = false;
    };

    //-----------------------------------------------------------------------------
    // Cymbals
    // Pitch 300 - 600 Hz (450.f)
    // Decay 0.8 - 2.5 s  (1.f)
    // Drive 1.5          (1.5f)
    //-----------------------------------------------------------------------------
    class CymbalsSynth {
    public:

        CymbalsSynth(): mNoiseState(0xACE12345) {};

        void trigger() {
            mEnvelope = 1.0f;
            mActive = true;
        }

        // hold
        void stop() {
            mEnvelope = 0.0f;
            for ( int i = 0; i < 6; ++i) mPhases[i] = 0.0f;
            mActive = false;
        }


        float processSample(float pitch, float decay, float drive, float velocity, float sampleRate) {
            if (!mActive || velocity < 0.01f) return 0.0f;

            // 1. Envelope (Exponentiell ist gut)
            mEnvelope *= std::exp(-1.0f / (decay * sampleRate));

            // 2. METALLIC CORE (The "808-on-Steroids" Approach)
            // Diese Ratios sind bewusst krumm (Primzahlen-nah), um Schwebungen zu vermeiden
            const float ratios[] = { 1.0f, 1.483f, 1.931f, 2.541f, 3.321f, 4.111f };
            float osc[6];

            for(int i = 0; i < 6; ++i) {
                mPhases[i] += (pitch * ratios[i]) / sampleRate;
                if (mPhases[i] >= 1.0f) mPhases[i] -= 1.0f;
                // Rechteck-Oszillator
                osc[i] = (mPhases[i] > 0.5f) ? 1.0f : -1.0f;
            }

            // --- RINGMODULATION & XOR-LOGIK ---
            // Wir multiplizieren die Oszillatoren paarweise.
            // Das erzeugt massive Inharmonik (Summen- und Differenztöne).
            float metalA = osc[0] * osc[1];
            float metalB = osc[2] * osc[3];
            float metalC = osc[4] * osc[5];

            // Alles zusammenmischen und durch schnelles "Schneiden" (XOR-artig) verschmutzen
            float metallic = (metalA + metalB + metalC) * 0.5f;
            if (metalA > 0.0f) metallic *= -1.0f; // Harte Phasen-Invertierung für mehr Sizzle

            // 3. NOISE GENERATION (Xorshift)
            mNoiseState ^= (mNoiseState << 13);
            mNoiseState ^= (mNoiseState >> 17);
            mNoiseState ^= (mNoiseState << 5);
            float whiteNoise = (static_cast<int32_t>(mNoiseState) * (1.0f / 2147483647.0f));

            // 4. MIX & FILTER (High-Pass ist Pflicht!)
            // Becken-Körper (Metallic) + Becken-Rauschen (White Noise)
            float mixedSource = (metallic * 0.4f) + (whiteNoise * 0.6f);

            // Aggressiver High-Pass: Cymbals brauchen Platz untenrum
            // Wir setzen den Cutoff deutlich höher an (z.B. 6-8 kHz)
            float hpCutoff = std::clamp(pitch * 8.0f, 6000.0f, 18000.0f);
            float alpha = hpCutoff / (hpCutoff + sampleRate / (2.0f * 3.14159f));

            mFilterState = alpha * (mFilterState + mixedSource - mLastInput);
            float cymbalSignal = mFilterState;
            mLastInput = mixedSource;

            // 5. SATURATION (Verklebt die Oszillatoren zu einem Teppich)
            float saturated = std::tanh(cymbalSignal * drive * (1.0f + velocity));

            if (mEnvelope < 0.0001f) { mActive = false; mEnvelope = 0.0f; }

            return saturated * mEnvelope * velocity * 0.5f;
        }

    private:
        float mEnvelope = 0.0f;
        float mFilterState = 0.f;
        bool mActive = false;
        float mPhases[6] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
        uint32_t mNoiseState = 0xACE12345;
        float mLastInput = 0.f;
    };
    //-----------------------------------------------------------------------------
    //-----------------------------------------------------------------------------
    // High Bell
    // Pitch 300 - 600 Hz
    // Decay 0.8 - 2.5 s
    // Drive 1.5
    //-----------------------------------------------------------------------------
    class HighBellSynth {
    public:
        void trigger() {
            mEnvelope = 1.0f;
            mActive = true;
        }

        // hold
        void stop() {
            mEnvelope = 0.0f;
            for ( int i = 0; i < 6; ++i) mPhases[i] = 0.0f;
            mActive = false;
        }


        float processSample(float pitch, float decay, float drive, float velocity, float sampleRate) {
            if (!mActive || velocity < 0.01f) return 0.0f;

            // 1. Long Decay for Cymbals
            float safeDecay = std::max(0.1f, decay);
            mEnvelope *= std::exp(-0.8f / (safeDecay * sampleRate));

            // 2. Metallic Noise (6 FM Oscillators)
            // Classic TR-808 style: 6 square waves with weird ratios
            float frequencies[] = { 1.1f, 1.45f, 1.91f, 2.3f, 2.73f, 3.14f };
            float metallicNoise = 0.0f;

            for(int i = 0; i < 6; ++i) {
                mPhases[i] += (pitch * frequencies[i]) / sampleRate;
                if (mPhases[i] >= 1.0f) mPhases[i] -= 1.0f;
                metallicNoise += (mPhases[i] > 0.5f) ? 1.0f : -1.0f; // Square waves
            }

            // 3. High-Pass Filter (Crucial for Cymbals)
            float alpha = std::clamp((pitch * 0.5f) / sampleRate, 0.01f, 0.95f);
            mFilterState += alpha * (metallicNoise - mFilterState);
            float cymbalSignal = metallicNoise - mFilterState;

            // 4. Drive & Velocity
            // Drive "smears" the frequencies into a wash
            float saturated = std::tanh(cymbalSignal * drive * (0.4f + 0.6f * velocity));

            // 5. Kill-Switch
            if (mEnvelope < 0.0005f) {
                mActive = false; mEnvelope = 0.0f;
            }

            return saturated * mEnvelope * velocity * 0.3f; // Reduced base gain
        }

    private:
        float mEnvelope = 0.0f;
        float mFilterState = 0.f;
        bool mActive = false;
        float mPhases[6] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };

        // float mLastOut = 0.0f;
        // float mLastIn = 0.0f;

    };

};}; //namespaces

