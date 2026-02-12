//-----------------------------------------------------------------------------
// Copyright (c) 2026 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Digital Sound Processing : Distortion
//-----------------------------------------------------------------------------
#pragma once

#include <mutex>
#include <string>
#include <vector>

#include "DSP_Math.h"

#ifdef FLUX_ENGINE
#include <imgui.h>
#include <imgui_internal.h>
#include <gui/ImFlux.h>
#endif


#include "DSP_Effect.h"
namespace DSP {

    struct OverDriveSettings {
        float drive;  // Input gain (1.0 - 20.0)
        float tone;   // Low-pass/High-pass tilt (0.0 - 1.0)
        float bassPreserve;  // Cross-over frequency for clean bass (0.0 - 1.0)
        float wet;    // Mix (0.0 - 1.0)

        static const uint8_t CURRENT_VERSION = 1;
        void getBinary(std::ostream& os) const {
            uint8_t ver = CURRENT_VERSION;
            DSP_STREAM_TOOLS::write_binary(os, ver);
            DSP_STREAM_TOOLS::write_binary(os, drive);
            DSP_STREAM_TOOLS::write_binary(os, tone);
            DSP_STREAM_TOOLS::write_binary(os, bassPreserve);
            DSP_STREAM_TOOLS::write_binary(os, wet);
        }

        bool  setBinary(std::istream& is) {
            uint8_t fileVersion = 0;
            DSP_STREAM_TOOLS::read_binary(is, fileVersion);
            if (fileVersion != CURRENT_VERSION) return false;
            DSP_STREAM_TOOLS::read_binary(is, drive);
            DSP_STREAM_TOOLS::read_binary(is, tone);
            DSP_STREAM_TOOLS::read_binary(is, bassPreserve);
            DSP_STREAM_TOOLS::read_binary(is, wet);

            return  is.good();
        }
    };

    class OverDrive : public DSP::Effect {
    private:
        OverDriveSettings mSettings;
        std::vector<float> mToneStates;  // High-cut state per channel
        std::vector<float> mBassStates;  // Low-pass state for Bass Preserve per channel

    public:
        IMPLEMENT_EFF_CLONE(OverDrive)

        OverDrive(bool switchOn = false) : DSP::Effect(switchOn) {
            mSettings.drive = 5.0f;
            mSettings.tone = 0.5f;
            mSettings.bassPreserve = 0.3f;
            mSettings.wet = 1.0f;
        }


        void setSettings(const OverDriveSettings& s) { mSettings = s; }
        OverDriveSettings getSettings() const { return mSettings; }

        virtual DSP::EffectType getType() const override { return DSP::EffectType::OverDrive; }
        virtual std::string getName() const override { return "OverDrive"; }

        virtual void reset() override {
            std::fill(mToneStates.begin(), mToneStates.end(), 0.0f);
            std::fill(mBassStates.begin(), mBassStates.end(), 0.0f);
        }

        virtual void process(float* buffer, int numSamples, int numChannels) override {
            if (!isEnabled() || mSettings.wet <= 0.001f) return;

            if (mToneStates.size() != (size_t)numChannels) {
                mToneStates.assign(numChannels, 0.0f);
                mBassStates.assign(numChannels, 0.0f);
            }

            // Coefficients
            float toneAlpha = DSP::clamp(mSettings.tone, 0.01f, 0.99f);
            // Bass Alpha (Low-pass crossover, usually around 100Hz - 400Hz)
            float bassAlpha = DSP::clamp(mSettings.bassPreserve * 0.2f, 0.01f, 0.5f);

            for (int i = 0; i < numSamples; i++) {
                int channel = i % numChannels;
                float dry = buffer[i];

                // 1. Extract Clean Bass (Low-pass filter)
                mBassStates[channel] = (dry * bassAlpha) + (mBassStates[channel] * (1.0f - bassAlpha));
                float cleanBass = mBassStates[channel];

                // 2. Drive & Clip (processed signal)
                float x = (dry - cleanBass) * mSettings.drive; // Distort mainly mids/highs

                // Cubic Soft-Clipper
                float clipped = DSP::clamp(x - (x * x * x * 0.15f), -0.8f, 0.8f);

                // 3. Tone Control (High-cut on the distorted part)
                mToneStates[channel] = (clipped * toneAlpha) + (mToneStates[channel] * (1.0f - toneAlpha));
                float distorted = mToneStates[channel];

                // 4. Re-combine: Distorted Signal + Clean Bass
                float combined = distorted + cleanBass;

                // 5. Final Mix
                float out = (dry * (1.0f - mSettings.wet)) + (combined * mSettings.wet);
                buffer[i] = DSP::clamp(out, -1.0f, 1.0f);
            }
        }

        //--------------------------------------------------------------------------
        #ifdef FLUX_ENGINE
        virtual ImVec4 getColor() const  override { return  ImVec4(0.827f, 0.521f, 0.329f, 1.0f);}

        //--------------------------------------------------------------------------
        virtual void renderPaddle() override {
            ImGui::PushID("OVERDRIVE_Effect_PADDLE");
            paddleHeader(getName().c_str(), ImGui::ColorConvertFloat4ToU32(getColor()), mEnabled);
            OverDriveSettings currentSettings = this->getSettings();
            bool changed = false;
            changed |= rackKnob("DRIVE", &currentSettings.drive, {1.0f, 20.0f}, ksRed);ImGui::SameLine();
            changed |= rackKnob("TONE", &currentSettings.tone, {0.0f, 1.0f}, ksBlue);ImGui::SameLine();
            changed |= rackKnob("PRESERVE", &currentSettings.bassPreserve, {0.0f, 1.0f}, ksBlue);ImGui::SameLine();
            changed |= rackKnob("LEVEL", &currentSettings.wet, {0.0f, 1.f}, ksBlack);
            if (changed) this->setSettings(currentSettings);
            ImGui::PopID();
        }

        virtual void renderUIWide() override {
            ImGui::PushID("OverDrive_Effect_WIDE");
            if (ImGui::BeginChild("OVERDRIVE_BOX", ImVec2(-FLT_MIN, 65.f))) {

                DSP::OverDriveSettings currentSettings = this->getSettings();
                bool changed = false;

                ImFlux::GradientBox(ImVec2(-FLT_MIN, -FLT_MIN), 0.f);
                ImGui::Dummy(ImVec2(2, 0)); ImGui::SameLine();
                ImGui::BeginGroup();

                bool isEnabled = this->isEnabled();
                if (ImFlux::LEDCheckBox(getName(), &isEnabled, getColor())) {
                    this->setEnabled(isEnabled);
                }
                if (!isEnabled) ImGui::BeginDisabled();
                // ImGui::SameLine(ImGui::GetWindowWidth() - 65.f);
                // if (ImFlux::ButtonFancy("RESET", ImFlux::SLATEDARK_BUTTON.WithSize(ImVec2(40.f, 20.f)))) {
                //     this->reset();
                // }
                ImGui::Separator();
                changed |= ImFlux::MiniKnobF("Drive", &currentSettings.drive, 1.0f, 20.0f); //, "%.1fx");
                ImGui::SameLine();
                changed |= ImFlux::MiniKnobF("Tone", &currentSettings.tone, 0.0f, 1.0f); //, "%.2f");
                ImGui::SameLine();
                changed |= ImFlux::MiniKnobF("Preserve", &currentSettings.bassPreserve, 0.0f, 1.0f); //, "%.2f");
                ImGui::SameLine();
                changed |= ImFlux::MiniKnobF("Mix", &currentSettings.wet, 0.0f, 1.0f); //, "%.2f");

                if (changed) setSettings(currentSettings);

                if (!isEnabled) ImGui::EndDisabled();
                ImGui::EndGroup();
            }
            ImGui::EndChild();
            ImGui::PopID();
        }

        //--------------------------------------------------------------------------
        virtual void renderUI() override {
            ImGui::PushID("OverDrive_Effect");
            OverDriveSettings currentSettings = this->getSettings();
            bool changed = false;
            bool enabled = this->isEnabled();

            if (ImFlux::LEDCheckBox(getName(), &enabled, getColor())) setEnabled(enabled);

            if (enabled) {
                if (ImGui::BeginChild("DIST_BOX", ImVec2(-FLT_MIN, 75.f), ImGuiChildFlags_Borders)) {
                    changed |= ImFlux::FaderHWithText("Drive", &currentSettings.drive, 1.0f, 20.0f, "%.1fx");
                    changed |= ImFlux::FaderHWithText("Tone", &currentSettings.tone, 0.0f, 1.0f, "%.2f");
                    changed |= ImFlux::FaderHWithText("Preserve", &currentSettings.bassPreserve, 0.0f, 1.0f, "%.2f");
                    changed |= ImFlux::FaderHWithText("Mix", &currentSettings.wet, 0.0f, 1.0f, "%.2f");
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
