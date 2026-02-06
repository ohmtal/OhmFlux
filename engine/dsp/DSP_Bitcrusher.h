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

#ifdef FLUX_ENGINE
#include <imgui.h>
#include <imgui_internal.h>
#include <gui/ImFlux.h>
#endif

#include "DSP_Effect.h"

namespace DSP {

    struct BitcrusherSettings {
        float bits;        // 1.0 to 16.0 (e.g., 8.0 for 8-bit sound)
        float sampleRate;  // 1000.0 to SAMPLE_RATE (e.g., 8000.0 for lo-fi)
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

         auto operator<=>(const BitcrusherSettings&) const = default; //C++20 lazy way
    };


    // Classic "Lo-Fi" presets
    constexpr BitcrusherSettings AMIGA_BITCRUSHER  = {  8.0f, 22050.0f, 1.0f };
    constexpr BitcrusherSettings NES_BITCRUSHER    = {  4.0f, 11025.0f, 1.0f };
    constexpr BitcrusherSettings PHONE_BITCRUSHER  = { 12.0f, 4000.0f, 1.0f };
    constexpr BitcrusherSettings EXTREME_BITCRUSHER  = { 2.0f, 4000.0f, 1.0f };

    constexpr BitcrusherSettings CUSTOM_BITCRUSHER  = {  8.0f, 22050.0f, 0.0f }; //DUMMY!

    static const char* BITCRUSHER_PRESETS_NAMES[] = {
        "Custom", "Amiga (8-bit)", "NES (4-bit)", "Phone (Lo-Fi)", "Extreme"
    };

    static const std::array<DSP::BitcrusherSettings, 5> BITCRUSHER_PRESETS = {
        DSP::CUSTOM_BITCRUSHER,
        DSP::AMIGA_BITCRUSHER,
        DSP::NES_BITCRUSHER,
        DSP::PHONE_BITCRUSHER,
        DSP::EXTREME_BITCRUSHER
    };


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




        //----------------------------------------------------------------------
        const BitcrusherSettings& getSettings() { return mSettings; }
        //----------------------------------------------------------------------
        void setSettings(const BitcrusherSettings& s) {
            mSettings = s;
            mSampleCount = 999999.0f;
        }
        //----------------------------------------------------------------------
        void reset() override { mSampleCount = 999999.0f; }
        //----------------------------------------------------------------------
        DSP::EffectType getType() const override { return DSP::EffectType::Bitcrusher; }
        //----------------------------------------------------------------------
        void save(std::ostream& os) const override {
            Effect::save(os);              // Save mEnabled
            mSettings.getBinary(os);       // Save Settings
        }
        //----------------------------------------------------------------------
        bool load(std::istream& is) override {
            if (!Effect::load(is)) return false; // Load mEnabled
            return mSettings.setBinary(is);      // Load Settings
        }
        //----------------------------------------------------------------------
        virtual void process(float* buffer, int numSamples) override
        {
            if (!isEnabled()) return;
            if (mSettings.wet <= 0.001f) return;

            float samplesToHold = getSampleRateF() / std::max(1.0f, mSettings.sampleRate);
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

        //----------------------------------------------------------------------
        virtual std::string getName() const override { return "BITCRUSHER";}
    #ifdef FLUX_ENGINE
    virtual ImVec4 getColor() const  override { return ImVec4(0.8f, 0.4f, 0.5f, 1.0f);}

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

                    for (int i = 1; i < DSP::BITCRUSHER_PRESETS.size(); ++i) {
                        if (currentSettings == DSP::BITCRUSHER_PRESETS[i]) {
                            currentIdx = i;
                            break;
                        }
                    }
                    int displayIdx = currentIdx;  //<< keep currentIdx clean

                    ImGui::SetNextItemWidth(150);
                    if (ImFlux::ValueStepper("##Preset", &displayIdx, BITCRUSHER_PRESETS_NAMES, IM_ARRAYSIZE(BITCRUSHER_PRESETS_NAMES))) {
                        if (displayIdx > 0 && displayIdx < DSP::BITCRUSHER_PRESETS.size()) {
                            currentSettings =  DSP::BITCRUSHER_PRESETS[displayIdx];
                            changed = true;
                        }
                    }
                    ImGui::SameLine(ImGui::GetWindowWidth() - 60); // Right-align reset button

                    if (ImFlux::FaderButton("Reset", ImVec2(40.f, 20.f)))  {
                        currentSettings = DSP::AMIGA_BITCRUSHER; //DEFAULT
                        this->reset();
                        changed = true;
                    }
                    ImGui::Separator();


                    // Control Sliders
                    changed |= ImFlux::FaderHWithText("Bits", &currentSettings.bits, 1.0f, 16.0f, "%.1f");
                    changed |= ImFlux::FaderHWithText("Rate", &currentSettings.sampleRate, 1000.0f, getSampleRateF(), "%.0f Hz");
                    changed |= ImFlux::FaderHWithText("Mix", &currentSettings.wet, 0.0f, 1.0f, "%.2f");

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
