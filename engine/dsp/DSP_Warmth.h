//-----------------------------------------------------------------------------
// Copyright (c) 2026 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Digital Sound Processing : Warmth
// Warmth - A simple One-Pole Low-Pass Filter mimics the "warm" analog
//          output of 90s
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

    struct WarmthSettings {
        float cutoff;   // 0.0 to 1.0 (Filter intensity, lower is muddier/warmer)
        float drive;    // 1.0 to 2.0 (Analog saturation/harmonic thickness)
        float wet;      // 0.0 to 1.0

        static const uint8_t CURRENT_VERSION = 1;
        void getBinary(std::ostream& os) const {
            uint8_t ver = CURRENT_VERSION;
            os.write(reinterpret_cast<const char*>(&ver), sizeof(ver));
            os.write(reinterpret_cast<const char*>(this), sizeof(WarmthSettings));
        }

        bool  setBinary(std::istream& is) {
            uint8_t fileVersion = 0;
            is.read(reinterpret_cast<char*>(&fileVersion), sizeof(fileVersion));
            if (fileVersion != CURRENT_VERSION) //Something is wrong !
                return false;
            is.read(reinterpret_cast<char*>(this), sizeof(WarmthSettings));
            return  is.good();
        }
        auto operator<=>(const WarmthSettings&) const = default; //C++20 lazy way

    };

    constexpr WarmthSettings GENTLE_WARMTH         = { 0.85f, 1.1f, 0.5f };
    constexpr WarmthSettings ANALOGDESK_WARMTH     = { 0.70f, 1.3f, 0.8f };
    constexpr WarmthSettings TUBEAMP_WARMTH        = { 0.50f, 1.6f, 1.0f };
    constexpr WarmthSettings EXTREME_WARMTH        = { 0.10f, 2.0f, 1.0f };
    constexpr WarmthSettings CUSTOM__WARMTH =  GENTLE_WARMTH; //<<dummy


    static const char* WARMTH_PRESET_NAMES[] = {
        "Custom", "Gentle Warmth", "Analog Desk", "Tube Amp", "Extreme" };
    static const std::array<DSP::WarmthSettings, 5> WARMTH_PRESETS = {
        CUSTOM__WARMTH,
        GENTLE_WARMTH,
        ANALOGDESK_WARMTH,
        TUBEAMP_WARMTH,
        EXTREME_WARMTH
    };



    class Warmth : public Effect {
    private:
        WarmthSettings mSettings;
        // float mPolesL[4];
        // float mPolesR[4];
        std::vector<std::array<float, 4>> mChannelPoles;

    public:
        IMPLEMENT_EFF_CLONE(Warmth)

        Warmth(bool switchOn = false) :
            Effect(switchOn),
            mSettings(GENTLE_WARMTH)
        {
            // std::memset(mPolesL, 0, sizeof(mPolesL));
            // std::memset(mPolesR, 0, sizeof(mPolesR));
        }

        DSP::EffectType getType() const override { return DSP::EffectType::Warmth; }

        const WarmthSettings& getSettings() { return mSettings; }

        void setSettings(const WarmthSettings& s) {
            mSettings = s;
            // std::memset(mPolesL, 0, sizeof(mPolesL));
            // std::memset(mPolesR, 0, sizeof(mPolesR));
        }

        void save(std::ostream& os) const override {
            Effect::save(os);              // Save mEnabled
            mSettings.getBinary(os);       // Save Settings
        }

        bool load(std::istream& is) override {
            if (!Effect::load(is)) return false; // Load mEnabled
            return mSettings.setBinary(is);      // Load Settings
        }


        virtual void process(float* buffer, int numSamples, int numChannels) override {
            if (!isEnabled() || mSettings.wet <= 0.001f) return;

            // 1. Initialize poles for all channels if count changed
            if (mChannelPoles.size() != static_cast<size_t>(numChannels)) {
                mChannelPoles.assign(numChannels, {0.0f, 0.0f, 0.0f, 0.0f});
            }

            // Alpha represents the cutoff frequency (0.01 to 0.99)
            float alpha = std::clamp(mSettings.cutoff, 0.01f, 0.99f);

            for (int i = 0; i < numSamples; i++) {
                int channel = i % numChannels;
                float dry = buffer[i];

                // 2. Access the 4-pole state for the current channel
                std::array<float, 4>& p = mChannelPoles[channel];

                // 3. 4-POLE CASCADE (-24dB/octave)
                // Each stage smooths the output of the previous stage
                p[0] = (dry  * alpha) + (p[0] * (1.0f - alpha));
                p[1] = (p[0] * alpha) + (p[1] * (1.0f - alpha));
                p[2] = (p[1] * alpha) + (p[2] * (1.0f - alpha));
                p[3] = (p[2] * alpha) + (p[3] * (1.0f - alpha));

                float filtered = p[3];

                // 4. Analog Saturation (Soft Clipping)
                float x = filtered * mSettings.drive;

                // Polynomial approximation of Tanh for "warm" analog saturation
                // Note: This approximation is valid for x between -1 and 1.
                // For higher drive, we clamp x before saturation or use a more robust function.
                float x_limited = std::clamp(x, -1.0f, 1.0f);
                float saturated = x_limited * (1.5f - (0.5f * x_limited * x_limited));

                // 5. Final Mix and Clamp
                float mixed = (dry * (1.0f - mSettings.wet)) + (saturated * mSettings.wet);
                buffer[i] = std::clamp(mixed, -1.0f, 1.0f);
            }
        }


        // virtual void process(float* buffer, int numSamples, int numChannels) override {
        //     if (numChannels !=  2) { return;  }  //FIXME REWRITE from stereo TO variable CHANNELS
        //     if (!isEnabled()) return;
        //     if (mSettings.wet <= 0.001f) return;
        //
        //     // alpha represents the cutoff frequency (0.01 to 0.99)
        //     float alpha = std::clamp(mSettings.cutoff, 0.01f, 0.99f);
        //
        //     for (int i = 0; i < numSamples; i++) {
        //         // 1. Input is already float (-1.0 to 1.0)
        //         float dry = buffer[i];
        //
        //         // 2. Select poles for Left or Right channel
        //         // Ensure mPolesL/R are float[4] initialized to 0.0f
        //         float* p = (i % 2 == 0) ? mPolesL : mPolesR;
        //
        //         // 3. 4-POLE CASCADE (-24dB/octave)
        //         p[0] = (dry  * alpha) + (p[0] * (1.0f - alpha));
        //         p[1] = (p[0] * alpha) + (p[1] * (1.0f - alpha));
        //         p[2] = (p[1] * alpha) + (p[2] * (1.0f - alpha));
        //         p[3] = (p[2] * alpha) + (p[3] * (1.0f - alpha));
        //
        //         float filtered = p[3];
        //
        //         // 4. Analog Saturation (Soft Clipping)
        //         // In the float pipeline, 'x' is already normalized.
        //         float x = filtered * mSettings.drive;
        //
        //         // Polynomial approximation of Tanh for "warm" analog saturation
        //         float saturated = x * (1.5f - (0.5f * x * x));
        //
        //         // 5. Final Mix and Clamp
        //         // We clamp here because saturation/drive can push values outside [-1, 1]
        //         float mixed = (dry * (1.0f - mSettings.wet)) + (saturated * mSettings.wet);
        //         buffer[i] = std::clamp(mixed, -1.0f, 1.0f);
        //     }
        // }
        //----------------------------------------------------------------------
        virtual std::string getName() const override { return "ANALOG WARMTH / SATURATION";}
        #ifdef FLUX_ENGINE
        virtual ImVec4 getColor() const  override { return  ImVec4(1.0f, 0.6f, 0.77f, 1.0f);}
        //----------------------------------------------------------------------
        virtual void renderUIWide() override {
            ImGui::PushID("Warmth_Effect_Row_WIDE");
            if (ImGui::BeginChild("WARMTH_W_BOX", ImVec2(-FLT_MIN,65.f) )) {
                DSP::WarmthSettings currentSettings = this->getSettings();
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
                for (int i = 1; i < DSP::WARMTH_PRESETS.size(); ++i) {
                    if (currentSettings == DSP::WARMTH_PRESETS[i]) {
                        currentIdx = i;
                        break;
                    }
                }
                int displayIdx = currentIdx;  //<< keep currentIdx clean
                ImGui::SameLine(ImGui::GetWindowWidth() - 260.f); // Right-align reset button

                if (ImFlux::ValueStepper("##Preset", &displayIdx, WARMTH_PRESET_NAMES
                    , IM_ARRAYSIZE(WARMTH_PRESET_NAMES)))
                {
                    if (displayIdx > 0 && displayIdx < DSP::WARMTH_PRESETS.size()) {
                        currentSettings =  DSP::WARMTH_PRESETS[displayIdx];
                        changed = true;
                    }
                }
                ImGui::SameLine();
                // if (ImFlux::FaderButton("Reset", ImVec2(40.f, 20.f)))  {
                if (ImFlux::ButtonFancy("RESET", ImFlux::SLATEDARK_BUTTON.WithSize(ImVec2(40.f, 20.f)) ))  {
                    currentSettings = DSP::GENTLE_WARMTH; //DEFAULT
                    // this->reset();
                    changed = true;
                }

                ImGui::Separator();
                changed |= ImFlux::MiniKnobF("Cutoff", &currentSettings.cutoff, 0.0f, 1.0f); ImGui::SameLine();
                changed |= ImFlux::MiniKnobF("Drive", &currentSettings.drive, 1.0f, 2.0f); ImGui::SameLine();
                changed |= ImFlux::MiniKnobF("Mix", &currentSettings.wet, 0.0f, 1.0f); ImGui::SameLine();

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
            ImGui::PushID("Warmth_Effect_Row");
            ImGui::BeginGroup();

            bool isEnabled = this->isEnabled();
            if (ImFlux::LEDCheckBox("ANALOG WARMTH / SATURATION", &isEnabled, ImVec4(1.0f, 0.6f, 0.4f, 1.0f)))
                this->setEnabled(isEnabled);

            if (isEnabled)
            {
                if (ImGui::BeginChild("Warmth_Box", ImVec2(0, 115), ImGuiChildFlags_Borders)) {
                    DSP::WarmthSettings currentSettings = this->getSettings();
                    bool changed = false;
                    int currentIdx = 0; // Standard: "Custom"
                    for (int i = 1; i < DSP::WARMTH_PRESETS.size(); ++i) {
                        if (currentSettings == DSP::WARMTH_PRESETS[i]) {
                            currentIdx = i;
                            break;
                        }
                    }
                    int displayIdx = currentIdx;  //<< keep currentIdx clean
                    ImGui::SetNextItemWidth(150);
                    if (ImFlux::ValueStepper("##Preset", &displayIdx, WARMTH_PRESET_NAMES, IM_ARRAYSIZE(WARMTH_PRESET_NAMES))) {
                        if (displayIdx > 0 && displayIdx < DSP::WARMTH_PRESETS.size()) {
                            currentSettings =  DSP::WARMTH_PRESETS[displayIdx];
                            changed = true;
                        }
                    }
                    ImGui::SameLine(ImGui::GetWindowWidth() - 60);
                    if (ImFlux::FaderButton("Reset", ImVec2(40.f, 20.f)))  {
                        currentSettings = DSP::GENTLE_WARMTH;
                        changed = true;
                    }
                    ImGui::Separator();
                    changed |= ImFlux::FaderHWithText("Cutoff", &currentSettings.cutoff, 0.0f, 1.0f, "%.2f (Filter)");
                    changed |= ImFlux::FaderHWithText("Drive", &currentSettings.drive, 1.0f, 2.0f, "%.2f (Gain)");
                    changed |= ImFlux::FaderHWithText("Mix", &currentSettings.wet, 0.0f, 1.0f, "Wet %.2f");
                    if (changed) {
                        if (isEnabled) {
                            this->setSettings(currentSettings);
                        }
                    }
                }
                ImGui::EndChild();
            } else {
                ImGui::Separator();
            }
            ImGui::EndGroup();
            ImGui::PopID();
            ImGui::Spacing();
        }
#endif


    };


} // namespace DSP
