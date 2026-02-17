//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Digital Sound Processing : Warmth
// Warmth - A simple One-Pole Low-Pass Filter mimics the "warm" analog
//          output of 90s
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
    struct WarmthData {
        float cutoff;   // 0.0 to 1.0 (Filter intensity, lower is muddier/warmer)
        float drive;    // 1.0 to 2.0 (Analog saturation/harmonic thickness)
        float wet;      // 0.0 to 1.0
    };

    // changed |= ImFlux::FaderHWithText("Cutoff", &currentSettings.cutoff, 0.0f, 1.0f, "%.2f (Filter)");
    // changed |= ImFlux::FaderHWithText("Drive", &currentSettings.drive, 1.0f, 2.0f, "%.2f (Gain)");
    // changed |= ImFlux::FaderHWithText("Mix", &currentSettings.wet, 0.0f, 1.0f, "Wet %.2f");

    struct WarmthSettings: public ISettings {
        AudioParam<float> cutoff    { "Cutoff", 0.85f,0.0f,  1.0f, "%.2f (Filter)" };
        AudioParam<float> drive     { "Drive", 1.10f, 1.0f,  2.0f, "%.2f (Gain)" };
        AudioParam<float> wet       { "Mix",   0.50f, 0.0f, 1.0f, "Wet %.2f"};

        WarmthSettings() = default;
        REGISTER_SETTINGS(WarmthSettings, &cutoff, &drive, &wet)

        WarmthData getData() const {
            return { cutoff.get(), drive.get(), wet.get()};
        }
        void setData(const WarmthData& data) {
            cutoff.set(data.cutoff);
            drive.set(data.drive);
            wet.set(data.wet);
        }
        std::vector<std::shared_ptr<IPreset>> getPresets() const override {
            return {
                std::make_shared<Preset<WarmthSettings, WarmthData>>
                    ("Custom", WarmthData{ 0.85f, 1.1f, 0.5f }),

                std::make_shared<Preset<WarmthSettings, WarmthData>>
                    ("Gentle Warmth", WarmthData{ 0.85f, 1.1f, 0.5f }),

                std::make_shared<Preset<WarmthSettings, WarmthData>>
                    ("Analog Desk", WarmthData{ 0.70f, 1.3f, 0.8f }),

                std::make_shared<Preset<WarmthSettings, WarmthData>>
                    ("Tube Amp", WarmthData{ 0.50f, 1.6f, 1.0f }),


            };
        }
    };

    constexpr WarmthData DEFAULT_WARMTH_DATA         = { 0.85f, 1.1f, 0.5f };


    class Warmth : public Effect {
    private:
        WarmthSettings mSettings;
        std::vector<std::array<float, 4>> mChannelPoles;

    public:
        IMPLEMENT_EFF_CLONE(Warmth)

        Warmth(bool switchOn = false) :
            Effect(DSP::EffectType::Warmth, switchOn),
            mSettings()
        {}
        //----------------------------------------------------------------------
        virtual std::string getName() const override { return "ANALOG WARMTH / SATURATION";}
        //----------------------------------------------------------------------
        WarmthSettings& getSettings() { return mSettings; }
        //----------------------------------------------------------------------
        void setSettings(const WarmthSettings& s) {
            mSettings = s;
        }
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
        // Process :D
        //----------------------------------------------------------------------
        virtual void process(float* buffer, int numSamples, int numChannels) override {
            float wet = mSettings.wet.get();
            if (!isEnabled() || wet <= 0.001f) return;

            float cutoff = mSettings.cutoff.get();
            float drive  = mSettings.drive.get();

            //  Init poles
            if (mChannelPoles.size() != static_cast<size_t>(numChannels)) {
                mChannelPoles.resize(numChannels, {0.0f, 0.0f, 0.0f, 0.0f});
            }

            // Alpha represents the cutoff frequency (0.01 to 0.99)
            float alpha = DSP::clamp(cutoff, 0.01f, 0.99f);

            int channel = 0;
            for (int i = 0; i < numSamples; i++) {
                float dry = buffer[i];

                // Access the 4-pole state for the current channel
                std::array<float, 4>& p = mChannelPoles[channel];

                // 4-POLE CASCADE (-24dB/octave)
                // Each stage smooths the output of the previous stage
                p[0] = (dry  * alpha) + (p[0] * (1.0f - alpha));
                p[1] = (p[0] * alpha) + (p[1] * (1.0f - alpha));
                p[2] = (p[1] * alpha) + (p[2] * (1.0f - alpha));
                p[3] = (p[2] * alpha) + (p[3] * (1.0f - alpha));

                float filtered = p[3];

                // Analog Saturation (Soft Clipping)
                float x = filtered * drive;

                // Polynomial approximation of Tanh for "warm" analog saturation
                // Note: This approximation is valid for x between -1 and 1.
                // For higher drive, we clamp x before saturation or use a more robust function.
                float x_limited = std::clamp(x, -1.0f, 1.0f);
                float saturated = x_limited * (1.5f - (0.5f * x_limited * x_limited));

                // Final Mix and Clamp
                float mixed = (dry * (1.0f - wet)) + (saturated * wet);
                buffer[i] = std::clamp(mixed, -1.0f, 1.0f);

                if (++channel >= numChannels) channel = 0;
            }
        }
        //----------------------------------------------------------------------
        #ifdef FLUX_ENGINE
        virtual ImVec4 getColor() const  override { return  ImVec4(1.0f, 0.6f, 0.77f, 1.0f);}

        virtual void renderPaddle() override {
            DSP::WarmthSettings currentSettings = this->getSettings();
            currentSettings.wet.setKnobSettings(ImFlux::ksBlue); // NOTE only works here !
            if (currentSettings.DrawPaddle(this)) {
                this->setSettings(currentSettings);
            }
        }

        virtual void renderUIWide() override {
            DSP::WarmthSettings currentSettings = this->getSettings();
            if (currentSettings.DrawUIWide(this)) {
                this->setSettings(currentSettings);
            }
        }
        virtual void renderUI() override {
            DSP::WarmthSettings currentSettings = this->getSettings();
            if (currentSettings.DrawUI(this, 115.f, true)) {
                this->setSettings(currentSettings);
            }
        }

        // //----------------------------------------------------------------------
        // virtual void renderUIWide() override {
        //     ImGui::PushID("Warmth_Effect_Row_WIDE");
        //     if (ImGui::BeginChild("WARMTH_W_BOX", ImVec2(-FLT_MIN,65.f) )) {
        //         DSP::WarmthSettings currentSettings = this->getSettings();
        //         int currentIdx = 0; // Standard: "Custom"
        //         bool changed = false;
        //         ImFlux::GradientBox(ImVec2(-FLT_MIN, -FLT_MIN),0.f);
        //         ImGui::Dummy(ImVec2(2,0)); ImGui::SameLine();
        //         ImGui::BeginGroup();
        //         bool isEnabled = this->isEnabled();
        //         if (ImFlux::LEDCheckBox(getName(), &isEnabled, getColor())){
        //             this->setEnabled(isEnabled);
        //         }
        //         if (!isEnabled) ImGui::BeginDisabled();
        //         ImGui::SameLine();
        //         // -------- stepper >>>>
        //         for (int i = 1; i < DSP::WARMTH_PRESETS.size(); ++i) {
        //             if (currentSettings == DSP::WARMTH_PRESETS[i]) {
        //                 currentIdx = i;
        //                 break;
        //             }
        //         }
        //         int displayIdx = currentIdx;  //<< keep currentIdx clean
        //         ImGui::SameLine(ImGui::GetWindowWidth() - 260.f); // Right-align reset button
        //
        //         if (ImFlux::ValueStepper("##Preset", &displayIdx, WARMTH_PRESET_NAMES
        //             , IM_ARRAYSIZE(WARMTH_PRESET_NAMES)))
        //         {
        //             if (displayIdx > 0 && displayIdx < DSP::WARMTH_PRESETS.size()) {
        //                 currentSettings =  DSP::WARMTH_PRESETS[displayIdx];
        //                 changed = true;
        //             }
        //         }
        //         ImGui::SameLine();
        //         // if (ImFlux::FaderButton("Reset", ImVec2(40.f, 20.f)))  {
        //         if (ImFlux::ButtonFancy("RESET", ImFlux::SLATEDARK_BUTTON.WithSize(ImVec2(40.f, 20.f)) ))  {
        //             currentSettings = DSP::GENTLE_WARMTH; //DEFAULT
        //             // this->reset();
        //             changed = true;
        //         }
        //
        //         ImGui::Separator();
        //         changed |= ImFlux::MiniKnobF("Cutoff", &currentSettings.cutoff, 0.0f, 1.0f); ImGui::SameLine();
        //         changed |= ImFlux::MiniKnobF("Drive", &currentSettings.drive, 1.0f, 2.0f); ImGui::SameLine();
        //         changed |= ImFlux::MiniKnobF("Mix", &currentSettings.wet, 0.0f, 1.0f); ImGui::SameLine();
        //
        //         // Engine Update
        //         if (changed) {
        //             if (isEnabled) {
        //                 this->setSettings(currentSettings);
        //             }
        //         }
        //
        //         if (!isEnabled) ImGui::EndDisabled();
        //
        //         ImGui::EndGroup();
        //     }
        //     ImGui::EndChild();
        //     ImGui::PopID();
        // }
        // //----------------------------------------------------------------------
        // virtual void renderUI() override {
        //     ImGui::PushID("Warmth_Effect_Row");
        //     ImGui::BeginGroup();
        //
        //     bool isEnabled = this->isEnabled();
        //     if (ImFlux::LEDCheckBox("ANALOG WARMTH / SATURATION", &isEnabled, ImVec4(1.0f, 0.6f, 0.4f, 1.0f)))
        //         this->setEnabled(isEnabled);
        //
        //     if (isEnabled)
        //     {
        //         if (ImGui::BeginChild("Warmth_Box", ImVec2(0, 115), ImGuiChildFlags_Borders)) {
        //             DSP::WarmthSettings currentSettings = this->getSettings();
        //             bool changed = false;
        //             int currentIdx = 0; // Standard: "Custom"
        //             for (int i = 1; i < DSP::WARMTH_PRESETS.size(); ++i) {
        //                 if (currentSettings == DSP::WARMTH_PRESETS[i]) {
        //                     currentIdx = i;
        //                     break;
        //                 }
        //             }
        //             int displayIdx = currentIdx;  //<< keep currentIdx clean
        //             ImGui::SetNextItemWidth(150);
        //             if (ImFlux::ValueStepper("##Preset", &displayIdx, WARMTH_PRESET_NAMES, IM_ARRAYSIZE(WARMTH_PRESET_NAMES))) {
        //                 if (displayIdx > 0 && displayIdx < DSP::WARMTH_PRESETS.size()) {
        //                     currentSettings =  DSP::WARMTH_PRESETS[displayIdx];
        //                     changed = true;
        //                 }
        //             }
        //             ImGui::SameLine(ImGui::GetWindowWidth() - 60);
        //             if (ImFlux::FaderButton("Reset", ImVec2(40.f, 20.f)))  {
        //                 currentSettings = DSP::GENTLE_WARMTH;
        //                 changed = true;
        //             }
        //             ImGui::Separator();
        //             changed |= ImFlux::FaderHWithText("Cutoff", &currentSettings.cutoff, 0.0f, 1.0f, "%.2f (Filter)");
        //             changed |= ImFlux::FaderHWithText("Drive", &currentSettings.drive, 1.0f, 2.0f, "%.2f (Gain)");
        //             changed |= ImFlux::FaderHWithText("Mix", &currentSettings.wet, 0.0f, 1.0f, "Wet %.2f");
        //             if (changed) {
        //                 if (isEnabled) {
        //                     this->setSettings(currentSettings);
        //                 }
        //             }
        //         }
        //         ImGui::EndChild();
        //     } else {
        //         ImGui::Separator();
        //     }
        //     ImGui::EndGroup();
        //     ImGui::PopID();
        //     ImGui::Spacing();
        // }
#endif


    };


} // namespace DSP
