//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Digital Sound Processing : Bitcrusher - "Lo-Fi" Filter
//-----------------------------------------------------------------------------
// * using ISettings
//-----------------------------------------------------------------------------

#pragma once

#include <vector>
#include <cstdint>
#include <algorithm>
#include <cmath>


#ifdef FLUX_ENGINE
#include <imgui.h>
#include <imgui_internal.h>
#include <gui/ImFlux.h>
#endif

#include "DSP_Effect.h"


namespace DSP {

    // need the for the presets
    struct BitcrusherData {
        float bits;
        float sampleRate;
        float wet;
    };


    struct BitcrusherSettings : public ISettings {
        AudioParam<float> bits       { "Resolution", 8.f, 1.0f, 16.0f, "%.1f" };
        AudioParam<float> sampleRate { "Downsampling", 22050.f, 1000.0f, 44100.0f, "%.0f Hz" };
        AudioParam<float> wet        { "Mix", 1.f, 0.0f, 1.0f, "%.2f" };


        BitcrusherSettings() = default;
        REGISTER_SETTINGS(BitcrusherSettings, &bits, &sampleRate, &wet)


        BitcrusherData getData() const {
            return { bits.get(), sampleRate.get(), wet.get() };
        }

        void setData(const BitcrusherData& data) {
            bits.set(data.bits);
            sampleRate.set(data.sampleRate);
            wet.set(data.wet);
        }

        // 4. Presets (Die Daten-Struktur ist ja schon schmal)
        std::vector<std::shared_ptr<IPreset>> getPresets() const override {
            return {
                std::make_shared<Preset<BitcrusherSettings, BitcrusherData>>("Custom", BitcrusherData{8.f, 22050.f, 0.f})
               ,std::make_shared<Preset<BitcrusherSettings, BitcrusherData>>("Amiga (8-bit)", BitcrusherData{8.0f, 22050.0f, 1.0f })
               ,std::make_shared<Preset<BitcrusherSettings, BitcrusherData>>("NES (4-bit)", BitcrusherData{  4.0f, 11025.0f, 1.0f })
               ,std::make_shared<Preset<BitcrusherSettings, BitcrusherData>>("Phone (Lo-Fi)", BitcrusherData{ 12.0f, 4000.0f, 1.0f })
               ,std::make_shared<Preset<BitcrusherSettings, BitcrusherData>>("Extreme", BitcrusherData{ 2.0f, 4000.0f, 1.0f })

            };
        }
    };

    constexpr BitcrusherData DEFAULT_BITCRUSHER_DATA = { 16.0f, 44100.0f, 1.0f };


    class Bitcrusher : public Effect {
    private:
        BitcrusherSettings mSettings;
         std::vector<float> mSteps;

         float mSampleCount = 1000.0f;
         bool mTriggerHold = false;

    public:
        IMPLEMENT_EFF_CLONE(Bitcrusher)

        Bitcrusher(bool switchOn = false) :
            Effect(DSP::EffectType::Bitcrusher, switchOn)
            ,mSettings()

            {
                std::vector<float> mSteps{0.0f, 0.0f}; //default 2 channel

            }
        //----------------------------------------------------------------------
        virtual std::string getName() const override { return "BITCRUSHER";}
        //----------------------------------------------------------------------
        //NOTE  for porting  remove CONST !!!!!!!!
        BitcrusherSettings& getSettings() { return mSettings; }
        //----------------------------------------------------------------------
        void setSettings(const BitcrusherSettings& s) {
            mSettings = s;
            mSampleCount = 999999.0f;
        }
        //----------------------------------------------------------------------
        virtual void setSampleRate(float sampleRate) override {
            mSampleRate = sampleRate;
            mSettings.sampleRate.setMax(sampleRate);
        }
        //----------------------------------------------------------------------
        void reset() override { mSampleCount = 999999.0f; mTriggerHold = false;}
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

        virtual void process(float* buffer, int numSamples, int numChannels) override {

            const float currentWet  = mSettings.wet.get();

            if (!isEnabled() || currentWet  <= 0.001f) return;

            const float currentBits = mSettings.bits.get();
            const float currentSR   = mSettings.sampleRate.get();


            // Resize state buffer if channel count changes dynamically
            if (mSteps.size() != (size_t)numChannels) {
                mSteps.assign(numChannels, 0.0f);
            }

            float samplesToHold = mSampleRate / std::max(1.0f, currentSR);
            float levels = std::pow(2.0f, std::clamp(currentBits, 1.0f, 16.0f));


            int channel = 0;
            for (int i = 0; i < numSamples; i++) {
                float dry = buffer[i];

                if (channel == 0) {
                    mSampleCount++;
                    if (mSampleCount >= samplesToHold) {
                        mSampleCount = 0;
                        mTriggerHold = true;
                    } else {
                        mTriggerHold = false;
                    }
                }

                // Sample-and-Hold
                if (mTriggerHold) {
                    mSteps[channel] = dry;
                }

                float held = mSteps[channel];

                // fast Bit-Crushing (w/o std::round)
                float shifted = (held + 1.0f) * 0.5f;
                // (int)(x + 0.5f)  round() replacemnet
                float quantized = static_cast<int>(shifted * (levels - 1.0f) + 0.5f) / (levels - 1.0f);
                float crushed = (quantized * 2.0f) - 1.0f;

                // 4. Mix
                buffer[i] = (dry * (1.0f - currentWet)) + (crushed * currentWet);

                if (++channel >= numChannels) channel = 0;
            } //for

        }


        // virtual void process(float* buffer, int numSamples, int numChannels) override {
        //
        //     const float currentWet  = mSettings.wet.get();
        //
        //     if (!isEnabled() || currentWet  <= 0.001f) return;
        //
        //     // NOTE when you update all effects
        //     //      do not forget to pre define the variables!! (keyword: atomic)
        //     const float currentBits = mSettings.bits.get();
        //     const float currentSR   = mSettings.sampleRate.get();
        //
        //
        //     // Resize state buffer if channel count changes dynamically
        //     if (mSteps.size() != (size_t)numChannels) {
        //         mSteps.assign(numChannels, 0.0f);
        //     }
        //
        //     float samplesToHold = mSampleRate / std::max(1.0f, currentSR);
        //     float levels = std::pow(2.0f, std::clamp(currentBits, 1.0f, 16.0f));
        //
        //
        //     for (int i = 0; i < numSamples; i++) {
        //         int channel = i % numChannels;
        //         float dry = buffer[i];
        //
        //         // Update sample-and-hold values at the start of a new multi-channel frame
        //         if (channel == 0) {
        //             mSampleCount++;
        //             if (mSampleCount >= samplesToHold) {
        //                 mSampleCount = 0;
        //                 // Capture the current dry value for all channels in this frame
        //                 // Note: This assumes interleaved data [C1, C2, ..., Cn, C1, C2...]
        //                 for (int c = 0; c < numChannels; ++c) {
        //                     mSteps[c] = buffer[i + c];
        //                 }
        //             }
        //         }
        //
        //         float held = mSteps[channel];
        //
        //         // Bit Crushing
        //         float shifted = (held + 1.0f) * 0.5f;
        //         float quantized = std::round(shifted * (levels - 1.0f)) / (levels - 1.0f);
        //         float crushed = (quantized * 2.0f) - 1.0f;
        //
        //         // Mix
        //         buffer[i] = (dry * (1.0f - currentWet )) + (crushed * currentWet );
        //
        //     }
        //
        // }

        //----------------------------------------------------------------------
#ifdef FLUX_ENGINE
    virtual ImVec4 getColor() const  override { return ImVec4(0.8f, 0.4f, 0.5f, 1.0f);}


    virtual void renderPaddle( ) override {
        DSP::BitcrusherSettings currentSettings = this->getSettings();
        currentSettings.wet.setKnobSettings(ImFlux::ksPurple); // NOTE only works here !
        if (currentSettings.DrawPaddle(this)) {
            this->setSettings(currentSettings);
        }
    }
    virtual void renderUIWide( ) override {
        DSP::BitcrusherSettings currentSettings = this->getSettings();
        if (currentSettings.DrawUIWide(this)) {
            this->setSettings(currentSettings);
        }
    }
    //----------------------------------------------------------------------
    // Thats it after Template is ready :D
    // how cool is this ?! ...
    virtual void renderUI() override {
        DSP::BitcrusherSettings currentSettings = this->getSettings();
        if (currentSettings.DrawUI(this)) {
            this->setSettings(currentSettings);
        }
    }

#endif

    }; //CLASS
} // namespace DSP
