//-----------------------------------------------------------------------------
// Copyright (c) 2026 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Digital Sound Processing : Distortion (WiP)
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

    struct OverDriveSettings {
        float drive;  // Input gain (1.0 - 20.0)
        float tone;   // Low-pass/High-pass tilt (0.0 - 1.0)
        float bassPreserve;  // Cross-over frequency for clean bass (0.0 - 1.0)
        float wet;    // Mix (0.0 - 1.0)

        static const uint8_t CURRENT_VERSION = 1;
        //FIXME Load Save
    };

    class OverDrive : public DSP::Effect {
    private:
        DistortionSettings mSettings;
        std::vector<float> mToneStates;  // High-cut state per channel
        std::vector<float> mBassStates;  // Low-pass state for Bass Preserve per channel

    public:
        OverDrive(bool switchOn = false) : DSP::Effect(switchOn) {
            mSettings.drive = 5.0f;
            mSettings.tone = 0.5f;
            mSettings.bassPreserve = 0.3f;
            mSettings.wet = 1.0f;
        }

        virtual DSP::EffectType getType() const override { return DSP::EffectType::Distortion; }
        virtual std::string getName() const override { return "Distortion"; }

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
            float toneAlpha = std::clamp(mSettings.tone, 0.01f, 0.99f);
            // Bass Alpha (Low-pass crossover, usually around 100Hz - 400Hz)
            float bassAlpha = std::clamp(mSettings.bassPreserve * 0.2f, 0.01f, 0.5f);

            for (int i = 0; i < numSamples; i++) {
                int channel = i % numChannels;
                float dry = buffer[i];

                // 1. Extract Clean Bass (Low-pass filter)
                mBassStates[channel] = (dry * bassAlpha) + (mBassStates[channel] * (1.0f - bassAlpha));
                float cleanBass = mBassStates[channel];

                // 2. Drive & Clip (processed signal)
                float x = (dry - cleanBass) * mSettings.drive; // Distort mainly mids/highs

                // Cubic Soft-Clipper
                float clipped = std::clamp(x - (x * x * x * 0.15f), -0.8f, 0.8f);

                // 3. Tone Control (High-cut on the distorted part)
                mToneStates[channel] = (clipped * toneAlpha) + (mToneStates[channel] * (1.0f - toneAlpha));
                float distorted = mToneStates[channel];

                // 4. Re-combine: Distorted Signal + Clean Bass
                float combined = distorted + cleanBass;

                // 5. Final Mix
                float out = (dry * (1.0f - mSettings.wet)) + (combined * mSettings.wet);
                buffer[i] = std::clamp(out, -1.0f, 1.0f);
            }
        }

        #ifdef FLUX_ENGINE
        //FIXME renderUIWide
        virtual void renderUI() override {
            ImGui::PushID("Distortion_Effect");
            DistortionSettings currentSettings = this->getSettings();
            bool changed = false;
            bool enabled = this->isEnabled();

            if (ImFlux::LEDCheckBox(getName(), &enabled, ImVec4(0.8f, 0.1f, 0.1f, 1.0f))) setEnabled(enabled);

            if (enabled) {
                if (ImGui::BeginChild("DIST_BOX", ImVec2(-FLT_MIN, 75.f), ImGuiChildFlags_Borders)) {
                    changed |= ImFlux::FaderHWithText("Drive", &currentSettings.drive, 1.0f, 20.0f, "%.1fx");
                    changed |= ImFlux::FaderHWithText("Tone", &currentSettings.tone, 0.0f, 1.0f, "%.2f");
                    changed |= ImFlux::FaderHWithText("Bass", &currentSettings.bassPreserve, 0.0f, 1.0f, "%.2f");
                    changed |= ImFlux::FaderHWithText("Mix", &currentSettings.wet, 0.0f, 1.0f, "%.2f");
                    if (changed) setSettings(currentSettings);
                }
                ImGui::EndChild();
            }
            ImGui::PopID();
        }
        #endif
    };

} // namespace
