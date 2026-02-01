//-----------------------------------------------------------------------------
// Copyright (c) 2026 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Digital Sound Processing : Chorus
//-----------------------------------------------------------------------------
#pragma once

#include <cmath>
#include <vector>

namespace DSP {

    struct ChorusSettings {
        float rate;      // Speed of wiggle (0.1 - 2.0 Hz)
        float depth;     // Intensity (0.001 - 0.005)
        float delayBase; // Offset (0.01 - 0.03)
        float wet;       // Mix (0.0 - 0.5)
        float phaseOffset; // NEW: 0.0 to 1.0 (Phase shift between ears)

        static const uint8_t CURRENT_VERSION = 1;
        void getBinary(std::ostream& os) const {
            uint8_t ver = CURRENT_VERSION;
            os.write(reinterpret_cast<const char*>(&ver), sizeof(ver));
            os.write(reinterpret_cast<const char*>(this), sizeof(ChorusSettings));
        }

        bool  setBinary(std::istream& is) {
            uint8_t fileVersion = 0;
            is.read(reinterpret_cast<char*>(&fileVersion), sizeof(fileVersion));
            if (fileVersion != CURRENT_VERSION) //Something is wrong !
                return false;
            is.read(reinterpret_cast<char*>(this), sizeof(ChorusSettings));
            return  is.good();
        }
        auto operator<=>(const ChorusSettings&) const = default; //C++20 lazy way

    };


    // OFF
    constexpr ChorusSettings OFF_CHORUS           = { 0.0f,  0.0f,   0.0f,   0.0f,  0.0f };

    // Lush 80s: Standard wide stereo chorus
    constexpr ChorusSettings LUSH80s_CHORUS       = { 0.4f,  0.003f, 0.025f, 0.4f,  0.25f };

    // Deep Ensemble: Very slow, very wide, thickens pads
    constexpr ChorusSettings DEEPENSEMPLE_CHORUS  = { 0.1f,  0.005f, 0.040f, 0.5f,  0.50f };

    // Fast Leslie: Simulates a rotating speaker
    constexpr ChorusSettings FASTLESLIE_CHORUS    = { 2.5f,  0.002f, 0.010f, 0.3f,  0.15f };

    // Juno-60 Style: Famous thick BBD chorus (Fast rate, high depth)
    constexpr ChorusSettings JUNO60_CHORUS        = { 0.9f,  0.004f, 0.015f, 0.5f,  0.20f };

    // Vibrato: 100% Wet so you only hear the pitch wiggle
    constexpr ChorusSettings VIBRATO_CHORUS       = { 1.5f,  0.002f, 0.010f, 1.0f,  0.00f };

    // Flanger: Very short delay creates the "jet plane" comb filter effect
    constexpr ChorusSettings FLANGER_CHORUS       = { 0.2f,  0.001f, 0.003f, 0.5f,  0.10f };



    class Chorus : public Effect {
    private:
        // Two separate buffers for true stereo de-correlation
        std::vector<float> mDelayBufL;
        std::vector<float> mDelayBufR;

        int mWritePos = 0;
        float mLfoPhase = 0.0f;
        const float mSampleRate = 44100.0f;
        const int mMaxBufferSize = 4410; // 100ms at 44.1kHz

        ChorusSettings mSettings;

    public:
        Chorus(bool switchOn = false) :
            Effect(switchOn)
        {
            mDelayBufL.assign(mMaxBufferSize, 0.0f);
            mDelayBufR.assign(mMaxBufferSize, 0.0f);
            mSettings = LUSH80s_CHORUS;
        }

        const ChorusSettings& getSettings() { return mSettings; }

        void setSettings(const ChorusSettings& s) {
            mSettings = s;
        }

        DSP::EffectType getType() const override { return DSP::EffectType::Chorus; }
        void save(std::ostream& os) const override {
            Effect::save(os);              // Save mEnabled
            mSettings.getBinary(os);       // Save Settings
        }

        bool load(std::istream& is) override {
            if (!Effect::load(is)) return false; // Load mEnabled
            return mSettings.setBinary(is);      // Load Settings
        }

        virtual void process(float* buffer, int numSamples) override {
            if (!isEnabled()) return;

            // Skip if wet mix is effectively zero
            if (mSettings.wet <= 0.001f) return;

            const float TWO_PI = 2.0f * M_PI;

            for (int i = 0; i < numSamples; i++) {
                // 1. Dry signal is already float (-1.0 to 1.0)
                float dry = buffer[i];

                // 2. Determine LFO Phase for this specific channel
                float channelPhase = mLfoPhase;
                if (i % 2 != 0) { // Right Channel
                    channelPhase += (mSettings.phaseOffset * TWO_PI);
                    // Wrap phase
                    if (channelPhase >= TWO_PI) channelPhase -= TWO_PI;
                }

                // 3. Calculate modulated delay time
                float lfo = std::sin(channelPhase);
                float currentDelaySec = mSettings.delayBase + (lfo * mSettings.depth);
                float delaySamples = currentDelaySec * mSampleRate;

                // 4. Linear Interpolation
                float readPos = static_cast<float>(mWritePos) - delaySamples;
                while (readPos < 0) readPos += static_cast<float>(mMaxBufferSize);

                int idx1 = static_cast<int>(readPos) % mMaxBufferSize;
                int idx2 = (idx1 + 1) % mMaxBufferSize;
                float frac = readPos - static_cast<float>(idx1);

                float wetSample = 0.0f;
                if (i % 2 == 0) {
                    // LEFT CHANNEL (Ensure mDelayBufL is float*)
                    wetSample = mDelayBufL[idx1] * (1.0f - frac) + mDelayBufL[idx2] * frac;
                    mDelayBufL[mWritePos] = dry;
                } else {
                    // RIGHT CHANNEL (Ensure mDelayBufR is float*)
                    wetSample = mDelayBufR[idx1] * (1.0f - frac) + mDelayBufR[idx2] * frac;
                    mDelayBufR[mWritePos] = dry;
                }

                // 5. Mix Dry + (Wet * Mix)
                // No clamping needed. Final output remains in float range.
                buffer[i] = dry + (wetSample * mSettings.wet);

                // 6. Update LFO and Write Position (after the Right channel/Stereo frame)
                if (i % 2 != 0) {
                    mLfoPhase += (TWO_PI * mSettings.rate) / mSampleRate;
                    if (mLfoPhase >= TWO_PI) mLfoPhase -= TWO_PI;

                    mWritePos = (mWritePos + 1) % mMaxBufferSize;
                }
            }
        }
    }; //class

} // namespace DSP
