//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Digital Sound Processing : Bitcrusher - "Lo-Fi" Filter
//-----------------------------------------------------------------------------
// NOTE: not to me when updating all effects:
// FIXME GUI should now be moved to ISettings
// WARNING: gui must be called here ... to keep it variable. keyword 9Band..
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

         float mSampleRate;
         float mSampleCount = 1000.0f;

    public:
        IMPLEMENT_EFF_CLONE(Bitcrusher)

        Bitcrusher(bool switchOn = false) :
            Effect(switchOn)
            ,mSettings()

            {
                std::vector<float> mSteps{0.0f, 0.0f}; //default 2 channel
                mSampleRate = getSampleRateF();
                // FIXME set default ?! maybe set by init variables... mSettings(AMIGA_BITCRUSHER)
            }
        //----------------------------------------------------------------------
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
        void reset() override { mSampleCount = 999999.0f; }
        //----------------------------------------------------------------------
        DSP::EffectType getType() const override { return DSP::EffectType::Bitcrusher; }
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
        // Ensure mSteps is resized to numChannels in a 'prepare' or 'reset' method
        // std::vector<float> mSteps;
        virtual void process(float* buffer, int numSamples, int numChannels) override {

            const float currentWet  = mSettings.wet.get();

            if (!isEnabled() || currentWet  <= 0.001f) return;

            // NOTE when you update all effects
            //      do not forget to pre define the variables!! (keyword: atomic)
            const float currentBits = mSettings.bits.get();
            const float currentSR   = mSettings.sampleRate.get();


            // Resize state buffer if channel count changes dynamically
            if (mSteps.size() != (size_t)numChannels) {
                mSteps.assign(numChannels, 0.0f);
            }

            float samplesToHold = mSampleRate / std::max(1.0f, currentSR);
            float levels = std::pow(2.0f, std::clamp(currentBits, 1.0f, 16.0f));


            for (int i = 0; i < numSamples; i++) {
                int channel = i % numChannels;
                float dry = buffer[i];

                // Update sample-and-hold values at the start of a new multi-channel frame
                if (channel == 0) {
                    mSampleCount++;
                    if (mSampleCount >= samplesToHold) {
                        mSampleCount = 0;
                        // Capture the current dry value for all channels in this frame
                        // Note: This assumes interleaved data [C1, C2, ..., Cn, C1, C2...]
                        for (int c = 0; c < numChannels; ++c) {
                            mSteps[c] = buffer[i + c];
                        }
                    }
                }

                float held = mSteps[channel];

                // Bit Crushing
                float shifted = (held + 1.0f) * 0.5f;
                float quantized = std::round(shifted * (levels - 1.0f)) / (levels - 1.0f);
                float crushed = (quantized * 2.0f) - 1.0f;

                // Mix
                buffer[i] = (dry * (1.0f - currentWet )) + (crushed * currentWet );

            }

        }

        //----------------------------------------------------------------------
        virtual std::string getName() const override { return "BITCRUSHER";}
#ifdef FLUX_ENGINE
    virtual ImVec4 getColor() const  override { return ImVec4(0.8f, 0.4f, 0.5f, 1.0f);}


    virtual void renderUIWide() override {
        ImGui::PushID("BitCrusher_Effect_Row_WIDE");
        if (ImGui::BeginChild("DELAY_BOX", ImVec2(-FLT_MIN,65.f) )) {

            DSP::BitcrusherSettings currentSettings = this->getSettings();
            int currentIdx = 0; // Standard: "Custom"
            bool changed = false;
            ImFlux::GradientBox(ImVec2(-FLT_MIN, -FLT_MIN),0.f);
            ImGui::Dummy(ImVec2(2,0)); ImGui::SameLine();
            ImGui::BeginGroup();
            bool isEnabled = this->isEnabled();
            if (ImFlux::LEDCheckBox(getName(), &isEnabled, getColor())){
                this->setEnabled(isEnabled);
            }
            if (!isEnabled) ImGui::BeginDisabled();

            ImGui::SameLine();
            // -------- stepper >>>>
            changed |= mSettings.drawStepper(currentSettings);

            ImGui::SameLine();
            // if (ImFlux::FaderButton("Reset", ImVec2(40.f, 20.f)))  {
            if (ImFlux::ButtonFancy("RESET", ImFlux::SLATEDARK_BUTTON.WithSize(ImVec2(40.f, 20.f)) ))  {
                currentSettings.resetToDefaults();
                changed = true;
            }

            ImGui::Separator();
            for (auto* param :currentSettings.getAll() ) {
                changed |= param->MiniKnobF();
                ImGui::SameLine();
            }
            // Engine Update
            if (changed) {
                if (isEnabled) {
                    this->setSettings(currentSettings);
                }
            }
            if (!isEnabled) ImGui::EndDisabled();
            ImGui::EndGroup();
        }
        ImGui::EndChild();
        ImGui::PopID();

    }
    //----------------------------------------------------------------------
    virtual void renderUI() override {
        ImGui::PushID("BitCrusher_Effect_Row");
        ImGui::BeginGroup();
        bool isEnabled = this->isEnabled();
        if (ImFlux::LEDCheckBox(getName(), &isEnabled, getColor())){
                this->setEnabled(isEnabled);
        }
        if (isEnabled)
        {
            if (ImGui::BeginChild("BC_Box", ImVec2(0, 110), ImGuiChildFlags_Borders)) {
                ImGui::BeginGroup();
                DSP::BitcrusherSettings currentSettings = this->getSettings();
                bool changed = false;

                int currentIdx = 0; // Standard: "Custom"

                changed |= mSettings.drawStepper(currentSettings);
                ImGui::SameLine(ImGui::GetWindowWidth() - 60); // Right-align reset button

                if (ImFlux::FaderButton("Reset", ImVec2(40.f, 20.f)))  {
                    currentSettings.resetToDefaults();
                    this->reset();
                    changed = true;
                }
                ImGui::Separator();

                // Control Sliders
                for (auto* param :currentSettings.getAll() ) {
                    changed |= param->FaderHWithText();
                }
                // Engine Update
                if (changed) {
                    if (isEnabled) {
                        this->setSettings(currentSettings);
                    }
                }
                ImGui::EndGroup();
            }
            ImGui::EndChild();
        } else {
            ImGui::Separator();
        }

        ImGui::EndGroup();
        ImGui::PopID();
        ImGui::Spacing(); // Add visual gap before the next effect
    }
#endif

    }; //CLASS
} // namespace DSP
