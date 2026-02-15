//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Digital Sound Processing : Distortion - very basic
//-----------------------------------------------------------------------------
// * using ISettings
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

    struct DistortionBasicData {
        float gain;  // Input gain (1 - 50) // extreme would be 500 but destroy the sound
        float level;    //  (0.0 - 1.0)
    };
    struct DistortionBasicSettings: public ISettings {
        AudioParam<float> gain      { "Gain"  , 20.f  ,   1.f,  50.f, "%.1f" };
        AudioParam<float> level     { "Level" , 0.5f  ,   0.f,  1.f, "%.2f" };

        DistortionBasicSettings() = default;
        REGISTER_SETTINGS(DistortionBasicSettings, &gain, &level)

        DistortionBasicData getData() const {
            return { gain.get(), level.get()};
        }

        void setData(const DistortionBasicData& data) {
            gain.set(data.gain);
            level.set(data.level);
        }

    };

    class DistortionBasic : public DSP::Effect {
    private:
        DistortionBasicSettings mSettings;
    public:
        IMPLEMENT_EFF_CLONE(DistortionBasic)

        DistortionBasic(bool switchOn = false) :
            DSP::Effect(DSP::EffectType::DistortionBasic, switchOn)
            , mSettings()
            {}

        //----------------------------------------------------------------------
        virtual std::string getName() const override { return "Distortion"; }
        //----------------------------------------------------------------------
        void setSettings(const DistortionBasicSettings& s) {mSettings = s;}
        //----------------------------------------------------------------------
        DistortionBasicSettings& getSettings() { return mSettings; }
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

        //----------------------------------------------------------------------
        #ifdef FLUX_ENGINE
        virtual ImVec4 getColor() const  override { return  ImVec4(0.6f, 0.6f, 0.0f, 1.0f);}

        virtual void renderPaddle() override {
            DSP::DistortionBasicSettings currentSettings = this->getSettings();
            currentSettings.gain.setKnobSettings(ImFlux::ksRed); // NOTE only works here !
            if (currentSettings.DrawPaddle(this)) {
                this->setSettings(currentSettings);
            }
        }

        virtual void renderUIWide() override {
            DSP::DistortionBasicSettings currentSettings = this->getSettings();
            if (currentSettings.DrawUIWide(this)) {
                this->setSettings(currentSettings);
            }
        }
        virtual void renderUI() override {
            DSP::DistortionBasicSettings currentSettings = this->getSettings();
            if (currentSettings.DrawUI(this, 85.f)) {
                this->setSettings(currentSettings);
            }
        }

/*
        virtual void renderPaddle() override {
            ImGui::PushID("Basic_Distortion_Effect_PADDLE");
            paddleHeader(getName().c_str(), ImGui::ColorConvertFloat4ToU32(getColor()), mEnabled);
            DistortionBasicSettings currentSettings = this->getSettings();
            bool changed = false;
            changed |= rackKnob("GAIN", &currentSettings.gain, {1.0f, 50.0f}, ImFlux::ksRed);
            ImGui::SameLine();
            changed |= rackKnob("LEVEL", &currentSettings.level, {0.0f, 1.f}, ImFlux::ksBlack);

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
        }*/
        #endif
    };

} // namespace
