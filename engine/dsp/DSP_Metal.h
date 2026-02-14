//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Digital Sound Processing : Metal Distortion
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

    struct MetalSettings {
        float gain;   // Gain (1.0 - 500.0)
        float tight;  // Pre-HPF (0.0 - 1.0)
        float level;  // Output Level (0.0 - 1.0)

        static const uint8_t CURRENT_VERSION = 1;
        void getBinary(std::ostream& os) const {
            uint8_t ver = CURRENT_VERSION;
            DSP_STREAM_TOOLS::write_binary(os, ver);
            DSP_STREAM_TOOLS::write_binary(os, gain);
            DSP_STREAM_TOOLS::write_binary(os, tight);
            DSP_STREAM_TOOLS::write_binary(os, level);
        }

        bool setBinary(std::istream& is) {
            uint8_t fileVersion = 0;
            DSP_STREAM_TOOLS::read_binary(is, fileVersion);
            if (fileVersion != CURRENT_VERSION) return false;
            DSP_STREAM_TOOLS::read_binary(is, gain);
            DSP_STREAM_TOOLS::read_binary(is, tight);
            DSP_STREAM_TOOLS::read_binary(is, level);
            return is.good();
        }
        auto operator<=>(const MetalSettings&) const = default;
    };

    class Metal : public DSP::Effect {
    private:
        MetalSettings mSettings;

        // States for filtering
        struct FilterState {
            float last_in_hpf = 0.0f;
            float last_out_hpf = 0.0f;
            float last_out_lpf = 0.0f;
        };
        std::vector<FilterState> mStates;


    public:
        IMPLEMENT_EFF_CLONE(Metal)

        Metal(bool switchOn = false) : DSP::Effect(DSP::EffectType::Metal, switchOn) {
            mSettings.gain = 50.0f;
            mSettings.tight = 0.5f;
            mSettings.level = 0.5f;
            mStates.assign(2, FilterState()); // Default to stereo
        }

        void setSettings(const MetalSettings& s) { mSettings = s; }
        const MetalSettings& getSettings() { return mSettings; }

        virtual void setSampleRate(float sampleRate) override {
            mSampleRate = sampleRate;
        }


        virtual std::string getName() const override { return "Metal Distortion"; }

        virtual void process(float* buffer, int numSamples, int numChannels) override {
            if (!isEnabled()) return;

            if (mStates.size() != static_cast<size_t>(numChannels)) {
                mStates.assign(numChannels, FilterState());
            }


            float dt = 1.0f / mSampleRate;

            // Pre-calculate filter parameters
            float hpf_freq = 80.0f + mSettings.tight * 800.0f;
            float rc_hpf = 1.0f / (2.0f * (float)M_PI * hpf_freq);
            float alpha_hpf = rc_hpf / (rc_hpf + dt);

            float lpf_freq = 4500.0f;
            float rc_lpf = 1.0f / (2.0f * (float)M_PI * lpf_freq);
            float alpha_lpf = dt / (rc_lpf + dt);

            for (int i = 0; i < numSamples; i++) {
                int ch = i % numChannels;
                FilterState& s = mStates[ch];
                float input = buffer[i];

                // 1. Pre-filtering (Tightness)
                float hpf_out = alpha_hpf * (s.last_out_hpf + input - s.last_in_hpf);
                s.last_in_hpf = input;
                s.last_out_hpf = hpf_out;

                // 2. High Gain Distortion
                float drive = hpf_out * mSettings.gain;
                float distorted = std::tanh(drive);
                // Soft clipping / refinement
                if (distorted > 0.9f) distorted = 0.9f + (distorted - 0.9f) * 0.1f;
                if (distorted < -0.9f) distorted = -0.9f + (distorted + 0.9f) * 0.1f;

                // 3. Post-filtering (Cabinet-ish)
                float out = s.last_out_lpf + alpha_lpf * (distorted - s.last_out_lpf);
                s.last_out_lpf = out;

                buffer[i] = out * mSettings.level;
            }
        }

        #ifdef FLUX_ENGINE
        virtual ImVec4 getColor() const override { return ImVec4(0.7f, 0.0f, 0.0f, 1.0f); }

        virtual void renderPaddle() override {
            ImGui::PushID("Metal_Distortion_Effect_PADDLE");
            paddleHeader(getName().c_str(), ImGui::ColorConvertFloat4ToU32(getColor()), mEnabled);
            MetalSettings currentSettings = this->getSettings();
            bool changed = false;
            changed |= rackKnob("GAIN", &currentSettings.gain, {1.0f, 500.0f}, ksRed);
            ImGui::SameLine();
            changed |= rackKnob("TIGHT", &currentSettings.tight, {0.0f, 1.0f}, ksBlack);
            ImGui::SameLine();
            changed |= rackKnob("LEVEL", &currentSettings.level, {0.0f, 1.0f}, ksBlack);

            if (changed) this->setSettings(currentSettings);
            ImGui::PopID();
        }

        virtual void renderUIWide() override {
            ImGui::PushID("Metal_Distortion_Effect_Row_WIDE");
            if (ImGui::BeginChild("Metal_Distortion_W_BOX", ImVec2(-FLT_MIN, 65.f))) {
                ImFlux::GradientBox(ImVec2(-FLT_MIN, -FLT_MIN), 0.f);
                ImGui::Dummy(ImVec2(2, 0)); ImGui::SameLine();
                ImGui::BeginGroup();
                bool isEnabled = this->isEnabled();
                if (ImFlux::LEDCheckBox(getName(), &isEnabled, getColor())) {
                    this->setEnabled(isEnabled);
                }
                ImGui::Separator();
                bool changed = false;
                MetalSettings currentSettings = this->getSettings();
                if (!isEnabled) ImGui::BeginDisabled();

                changed |= ImFlux::MiniKnobF("Gain", &currentSettings.gain, 1.0f, 500.0f); ImGui::SameLine();
                changed |= ImFlux::MiniKnobF("Tight", &currentSettings.tight, 0.0f, 1.0f); ImGui::SameLine();
                changed |= ImFlux::MiniKnobF("Level", &currentSettings.level, 0.0f, 1.0f); ImGui::SameLine();

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
            ImGui::PushID("Metal_Distortion_Effect");
            MetalSettings currentSettings = this->getSettings();
            bool changed = false;
            bool enabled = this->isEnabled();

            if (ImFlux::LEDCheckBox(getName(), &enabled, getColor())) setEnabled(enabled);

            if (enabled) {
                if (ImGui::BeginChild("METAL_DIST_BOX", ImVec2(-FLT_MIN, 100.f), ImGuiChildFlags_Borders)) {
                    changed |= ImFlux::FaderHWithText("Gain", &currentSettings.gain, 1.0f, 500.0f, "%.1f");
                    changed |= ImFlux::FaderHWithText("Tight", &currentSettings.tight, 0.0f, 1.0f, "%.2f");
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

} // namespace DSP
