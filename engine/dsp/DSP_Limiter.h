//-----------------------------------------------------------------------------
// Copyright (c) 2026 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Digital Sound Processing : Limiter with integrated VU-Meter Data
//-----------------------------------------------------------------------------
#pragma once

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


    struct LimiterSettings {
        float Threshold ;  // Limit just before 1.f
        float Attack;      // How fast it turns down
        float Release;     // How slow it turns back up

        static const uint8_t CURRENT_VERSION = 1;
        void getBinary(std::ostream& os) const {
            uint8_t ver = CURRENT_VERSION;
            os.write(reinterpret_cast<const char*>(&ver), sizeof(ver));
            os.write(reinterpret_cast<const char*>(this), sizeof(LimiterSettings));
        }

        bool  setBinary(std::istream& is) {
            uint8_t fileVersion = 0;
            is.read(reinterpret_cast<char*>(&fileVersion), sizeof(fileVersion));
            if (fileVersion != CURRENT_VERSION) //Something is wrong !
                return false;
            is.read(reinterpret_cast<char*>(this), sizeof(LimiterSettings));
            return  is.good();
        }

        auto operator<=>(const LimiterSettings&) const = default; //C++20 lazy way


    };


    constexpr LimiterSettings LIMITER_DEFAULT = { 0.95f,  0.05f,  0.0000005f };
    constexpr LimiterSettings LIMITER_EIGHTY  = { 0.80f,  0.05f,  0.0000005f };
    constexpr LimiterSettings LIMITER_FIFTY   = { 0.50f,  0.05f,  0.0000005f };
    constexpr LimiterSettings LIMITER_LOWVOL  = { 0.25f,  0.05f,  0.0000005f };
    constexpr LimiterSettings LIMITER_EXTREM  = { 0.05f,  0.50f,  0.0000005f };

    constexpr LimiterSettings LIMITER_CUSTOM  = LIMITER_DEFAULT; //<< DUMMY

    static const std::array<DSP::LimiterSettings, 6> LIMITER_PRESETS = {
        DSP::LIMITER_CUSTOM,  // Index 0:
        DSP::LIMITER_DEFAULT, // Index 1
        DSP::LIMITER_EIGHTY,  // Index 2
        DSP::LIMITER_FIFTY,   // Index 3
        DSP::LIMITER_LOWVOL,  // Index 4
        DSP::LIMITER_EXTREM   // Index 5
    };


    class Limiter : public Effect {
    private:
        float mCurrentGain = 1.0f;
        LimiterSettings mSettings;


    public:

        Limiter(bool switchOn = false) :
            Effect(switchOn),
            mSettings(LIMITER_DEFAULT)
            {}

        DSP::EffectType getType() const override { return DSP::EffectType::Limiter; }
        //----------------------------------------------------------------------
        void setSettings(const LimiterSettings& s) {
                resetGain(); //also reset current gain.
                mSettings = s;
        }
        //----------------------------------------------------------------------
        void resetGain() { mCurrentGain = 1.f; }
        //----------------------------------------------------------------------
        LimiterSettings& getSettings() { return mSettings; }
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
        // fetch current Gain
        // 1.0 = open , 0.5 = -6dB
        float getGainReduction() const { return mCurrentGain; }
        //----------------------------------------------------------------------
        virtual void process(float* buffer, int numSamples) override {
            if (!isEnabled()) return;
            // Process in steps of 2 for Stereo Interleaved data
            for (int i = 0; i < numSamples; i += 2) {
                // 1. Get both channels
                float inputL = buffer[i];
                float inputR = buffer[i + 1];

                // Stereo-Link: Find the max absolute peak of BOTH channels
                float absL = std::abs(inputL);
                float absR = std::abs(inputR);
                float maxAbsInput = std::max(absL, absR);

                // Calculate Target Gain based on the loudest channel
                float targetGain = 1.0f;
                if (maxAbsInput > mSettings.Threshold) {
                    targetGain = mSettings.Threshold / (maxAbsInput + 1e-9f);
                }

                // Smooth the Gain (Shared for both L and R)
                if (targetGain < mCurrentGain) {
                    mCurrentGain += (targetGain - mCurrentGain) * mSettings.Attack;
                } else {
                    mCurrentGain += (targetGain - mCurrentGain) * mSettings.Release;
                }

                // Apply SAME Gain to both (Preserves stereo image)
                buffer[i]     = inputL * mCurrentGain;
                buffer[i + 1] = inputR * mCurrentGain;
            }
        }

    //----------------------------------------------------------------------
    #ifdef FLUX_ENGINE
        void renderUI() {
            ImGui::PushID("Limiter_Effect_Row");


            ImGui::BeginGroup();

            auto* lim = this;
            bool isEnabled = lim->isEnabled();


            if (ImFlux::LEDCheckBox("LIMITER", &isEnabled, ImVec4(1.0f, 0.4f, 0.4f, 1.0f))) {
                lim->setEnabled(isEnabled);
            }


            if (lim->isEnabled()) {
                const char* presetNames[] = { "CUSTOM", "DEFAULT", "EIGHTY", "FIFTY", "LOWVOL", "EXTREM" };
                bool changed = false;
                DSP::LimiterSettings& currentSettings = lim->getSettings();

                int currentIdx = 0; // Standard: "Custom"

                for (int i = 1; i < DSP::LIMITER_PRESETS.size(); ++i) {
                    if (currentSettings == DSP::LIMITER_PRESETS[i]) {
                        currentIdx = i;
                        break;
                    }
                }
                int displayIdx = currentIdx;  //<< keep currentIdx clean

                if (ImGui::BeginChild("EQ_Box", ImVec2(0, 75.f),  ImGuiChildFlags_Borders)) {

                    ImGui::BeginGroup();
                    ImGui::SetNextItemWidth(150);

                    if (ImFlux::ValueStepper("##Preset", &displayIdx, presetNames, IM_ARRAYSIZE(presetNames))) {
                        if (displayIdx > 0 && displayIdx < DSP::LIMITER_PRESETS.size()) {
                                    lim->setSettings(DSP::LIMITER_PRESETS[displayIdx]);
                        }
                    }

                    // Quick Reset Button (Now using the FLAT_EQ preset)
                    ImGui::SameLine(ImGui::GetWindowWidth() - 60);

                    // if (ImGui::SmallButton("Reset")) {
                    if (ImFlux::FaderButton("Reset", ImVec2(40.f, 20.f)))  {
                        lim->setSettings(DSP::LIMITER_DEFAULT);
                    }
                    // ImGui::Separator();
                    // changed |= ImFlux::FaderHWithText("Threshold", &currentSettings.Threshold, 0.01f, 1.f, "%.3f");
                    // changed |= ImFlux::FaderHWithText("Depth", &currentSettings.Attack, 0.01f, 1.f, "%.4f");
                    // changed |= ImFlux::FaderHWithText("Release", &currentSettings.Release, 0.0000005f, 0.0001f, "%.8f");
                    //
                    //
                    // if (changed) {
                    //     selectedPresetIdx = 0;
                    //     lim->setSettings(currentSettings);
                    // }

                    ImGui::Separator();
                    float reduction = 1.0f - getGainReduction();
                    ImGui::TextDisabled("Reduction: %3.3f", reduction);
                    ImFlux::PeakMeter(reduction,ImVec2(150.f, 7.f));
                    ImGui::EndGroup();

                } //box
                ImGui::EndChild();
            } //enabled

            ImGui::EndGroup();
            ImGui::PopID();
            ImGui::Spacing();
        }

    #endif //FLUX_ENGINE


    };

} // namespace DSP
