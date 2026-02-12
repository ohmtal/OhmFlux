//-----------------------------------------------------------------------------
// Copyright (c) 2026 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Digital Sound Processing : Distortion - very basic
//-----------------------------------------------------------------------------
#pragma once

#include <cstdint>
#include <cstring>
#include <atomic>
#include <mutex>
#include <string>

#include <algorithm>
#include <cmath>
#include <vector>
#include <array>

#ifdef FLUX_ENGINE
#include <imgui.h>
#include <imgui_internal.h>
#include <gui/ImFlux.h>
#endif


#include "DSP_Effect.h"
namespace DSP {

    struct DistortionBasicSettings {
        float gain;  // Input gain (10 - 50) // extreme would be 500 but destroy the sound
        float level;    //  (0.0 - 1.0)

        static const uint8_t CURRENT_VERSION = 1;
        void getBinary(std::ostream& os) const {
            uint8_t ver = CURRENT_VERSION;
            DSP_STREAM_TOOLS::write_binary(os, ver);

            DSP_STREAM_TOOLS::write_binary(os, gain);
            DSP_STREAM_TOOLS::write_binary(os, level);
        }

        bool  setBinary(std::istream& is) {
            uint8_t fileVersion = 0;
            DSP_STREAM_TOOLS::read_binary(is, fileVersion);
            if (fileVersion != CURRENT_VERSION) return false;

            DSP_STREAM_TOOLS::read_binary(is, gain);
            DSP_STREAM_TOOLS::read_binary(is, level);

            return  is.good();
        }
        auto operator<=>(const DistortionBasicSettings&) const = default; //C++20 lazy way
    };

    class DistortionBasic : public DSP::Effect {
    private:
        DistortionBasicSettings mSettings;
    public:
        IMPLEMENT_EFF_CLONE(DistortionBasic)

        DistortionBasic(bool switchOn = false) : DSP::Effect(switchOn) {
            mSettings.gain = 20.f;
            mSettings.level = 0.5;
        }

        void setSettings(const DistortionBasicSettings& s) {mSettings = s;}
        const DistortionBasicSettings& getSettings() { return mSettings; }

        virtual DSP::EffectType getType() const override { return DSP::EffectType::DistortionBasic; }
        virtual std::string getName() const override { return "Distortion"; }


        // process per single float value
        virtual float processFloat(float input) override {
            if (!isEnabled() || mSettings.level <= 0.001f) return input;
            float out = input;
            out =  DSP::fast_tanh(out * mSettings.gain) * mSettings.level;
            return out;
        }


        virtual void process(float* buffer, int numSamples, int numChannels) override {
            if (!isEnabled() || mSettings.level <= 0.001f) return;

            // no channel handling needed here
            for (int i = 0; i < numSamples; i++) {
                buffer[i] = processFloat(buffer[i]);
            }
        }

        #ifdef FLUX_ENGINE
        virtual ImVec4 getColor() const  override { return  ImVec4(0.6f, 0.6f, 0.0f, 1.0f);}

        virtual void renderPaddle() override {
            ImGui::PushID("Basic_Distortion_Effect_PADDLE");
            paddleHeader(getName().c_str(), ImGui::ColorConvertFloat4ToU32(getColor()), mEnabled);
            DistortionBasicSettings currentSettings = this->getSettings();
            bool changed = false;
            changed |= rackKnob("GAIN", &currentSettings.gain, {1.0f, 50.0f}, ksRed);
            ImGui::SameLine();
            changed |= rackKnob("LEVEL", &currentSettings.level, {0.0f, 1.f}, ksBlack);

            if (changed) this->setSettings(currentSettings);
            ImGui::PopID();
        }

        virtual void renderUIWide() override {
            ImGui::PushID("Basic_Distortion_Effect_Row_WIDE");
            if (ImGui::BeginChild("Basic_Distortion_W_BOX", ImVec2(-FLT_MIN,65.f) )) {

                ImFlux::GradientBox(ImVec2(-FLT_MIN, -FLT_MIN),0.f);
                ImGui::Dummy(ImVec2(2,0)); ImGui::SameLine();
                ImGui::BeginGroup();
                bool isEnabled = this->isEnabled();
                if (ImFlux::LEDCheckBox(getName(), &isEnabled, getColor())){
                    this->setEnabled(isEnabled);
                }
                ImGui::Separator();
                bool changed = false;
                DistortionBasicSettings currentSettings = this->getSettings();
                if (!isEnabled) ImGui::BeginDisabled();

                changed |= ImFlux::MiniKnobF("Gain", &currentSettings.gain, 1.0f, 50.0f); ImGui::SameLine();
                changed |= ImFlux::MiniKnobF("Level", &currentSettings.level, 0.0f, 1.0f); ImGui::SameLine();

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

        virtual void renderUI() override {
            ImGui::PushID("Basic_Distortion_Effect");
            DistortionBasicSettings currentSettings = this->getSettings();
            bool changed = false;
            bool enabled = this->isEnabled();

            if (ImFlux::LEDCheckBox(getName(), &enabled, getColor())) setEnabled(enabled);

            if (enabled) {
                if (ImGui::BeginChild("DIST_BOX", ImVec2(-FLT_MIN, 75.f), ImGuiChildFlags_Borders)) {
                    changed |= ImFlux::FaderHWithText("Gain", &currentSettings.gain, 1.0f, 50.0f, "%.1f");
                    changed |= ImFlux::FaderHWithText("Level", &currentSettings.level, 0.0f, 1.0f, "%.2f");
                    if (changed) setSettings(currentSettings);
                }
                ImGui::EndChild();
            } else {
                ImGui::Separator();
            }
            ImGui::PopID();
        }
        #endif
    };

} // namespace
