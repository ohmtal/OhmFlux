//-----------------------------------------------------------------------------
// Copyright (c) 2026 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Digital Sound Processing : Bitcrusher - "Lo-Fi" Filter
//-----------------------------------------------------------------------------
#pragma once

#include <vector>
#include <cstdint>
#include <algorithm>
#include <cmath>

#include "DSP_Effect.h"

namespace DSP {

    struct BitcrusherSettings {
        float bits;        // 1.0 to 16.0 (e.g., 8.0 for 8-bit sound)
        float sampleRate;  // 1000.0 to 44100.0 (e.g., 8000.0 for lo-fi)
        float wet;         // 0.0 to 1.0

        static const uint8_t CURRENT_VERSION = 1;
        void getBinary(std::ostream& os) const {
            uint8_t ver = CURRENT_VERSION;
            os.write(reinterpret_cast<const char*>(&ver), sizeof(ver));
            os.write(reinterpret_cast<const char*>(this), sizeof(BitcrusherSettings));
        }

        bool  setBinary(std::istream& is) {
            uint8_t fileVersion = 0;
            is.read(reinterpret_cast<char*>(&fileVersion), sizeof(fileVersion));
            if (fileVersion != CURRENT_VERSION) //Something is wrong !
                return false;
            is.read(reinterpret_cast<char*>(this), sizeof(BitcrusherSettings));
            return  is.good();
        }
    };

    // OFF
    constexpr BitcrusherSettings OFF_BITCRUSHER = { 16.0f, 44100.0f, 0.0f };

    // Classic "Lo-Fi" presets
    constexpr BitcrusherSettings AMIGA_BITCRUSHER  = {  8.0f, 22050.0f, 1.0f };
    constexpr BitcrusherSettings NES_BITCRUSHER    = {  4.0f, 11025.0f, 1.0f };
    constexpr BitcrusherSettings PHONE_BITCRUSHER  = { 12.0f, 4000.0f, 1.0f };

    constexpr BitcrusherSettings EXTREME_BITCRUSHER  = { 2.0f, 4000.0f, 1.0f };




    class Bitcrusher : public Effect {
    private:
        BitcrusherSettings mSettings;
        float mStepL = 0.0f;
        float mStepR = 0.0f;
        float mSampleCount = 1000.0f;

    public:
        Bitcrusher(bool switchOn = false) :
            Effect(switchOn),
            mSettings(AMIGA_BITCRUSHER)
            {}


        const BitcrusherSettings& getSettings() { return mSettings; }

        void setSettings(const BitcrusherSettings& s) {
            mSettings = s;
            mSampleCount = 999999.0f;
        }

        DSP::EffectType getType() const override { return DSP::EffectType::Bitcrusher; }

        void save(std::ostream& os) const override {
            Effect::save(os);              // Save mEnabled
            mSettings.getBinary(os);       // Save Settings
        }

        bool load(std::istream& is) override {
            if (!Effect::load(is)) return false; // Load mEnabled
            return mSettings.setBinary(is);      // Load Settings
        }


        virtual void process(float* buffer, int numSamples) override
        {
            if (!isEnabled()) return;
            if (mSettings.wet <= 0.001f) return;

            float samplesToHold = 44100.0f / std::max(1.0f, mSettings.sampleRate);
            float levels = std::pow(2.0f, std::clamp(mSettings.bits, 1.0f, 16.0f));

            for (int i = 0; i < numSamples; i++) {
                float dry = buffer[i];

                // --- FIXED SAMPLE & HOLD LOGIC ---
                bool isLeft = (i % 2 == 0);

                if (isLeft) {
                    mSampleCount++; // Increment only once per stereo pair
                }

                if (mSampleCount >= samplesToHold) {
                    // Update the specific channel for this iteration
                    if (isLeft) mStepL = dry;
                    else mStepR = dry;

                    // ONLY reset after the Right channel has had a chance to update
                    if (!isLeft) {
                        mSampleCount = 0;
                    }
                }

                float held = isLeft ? mStepL : mStepR;
                // ----------------------------------

                // 3. Bit Crushing
                float shifted = (held + 1.0f) * 0.5f;
                float quantized = std::round(shifted * (levels - 1.0f)) / (levels - 1.0f);
                float crushed = (quantized * 2.0f) - 1.0f;

                // 4. Mix
                buffer[i] = (dry * (1.0f - mSettings.wet)) + (crushed * mSettings.wet);
            }
        }

    };
} // namespace DSP
