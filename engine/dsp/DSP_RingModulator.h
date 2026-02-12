//-----------------------------------------------------------------------------
// Copyright (c) 2026 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Digital Sound Processing : RingModulator
//-----------------------------------------------------------------------------
#pragma once

#include <vector>
#include <cstdint>
#include <algorithm>
#include <cstring>
#include <atomic>
#include <mutex>

#ifdef FLUX_ENGINE
#include <imgui.h>
#include <imgui_internal.h>
#include <gui/ImFlux.h>
#endif


#include "DSP_Effect.h"

namespace DSP {


struct RingModSettings {
    float frequency; // The carrier frequency (Hz). e.g., 200Hz - 2000Hz
    float wet;       // Dry/Wet mix (0.0 - 1.0)

    static const uint8_t CURRENT_VERSION = 1;

    void getBinary(std::ostream& os) const {
        uint8_t ver = CURRENT_VERSION;
        DSP_STREAM_TOOLS::write_binary(os, ver);

        DSP_STREAM_TOOLS::write_binary(os, frequency);
        DSP_STREAM_TOOLS::write_binary(os, wet);

    }

    bool  setBinary(std::istream& is) {
        uint8_t fileVersion = 0;
        DSP_STREAM_TOOLS::read_binary(is, fileVersion);
        if (fileVersion != CURRENT_VERSION) return false;

        DSP_STREAM_TOOLS::read_binary(is, frequency);
        DSP_STREAM_TOOLS::read_binary(is, wet);

        return  is.good();
    }

    auto operator<=>(const RingModSettings&) const = default; //C++20 lazy way

};

class RingModulator : public DSP::Effect {
private:
    RingModSettings mSettings;
    float mPhaseL = 0.0f; // Oscillator phase for left channel
    float mSampleRate = getSampleRateF(); // Assumed fixed SR, needs updating in prepareToPlay

public:
    IMPLEMENT_EFF_CLONE(RingModulator)

    RingModulator(bool switchOn = false) :
        DSP::Effect(switchOn)
    {
        mSettings.frequency = 400.0f; // Default carrier freq
        mSettings.wet = 0.5f;         // Usually 100% wet for this effect
        reset();
    }

    virtual DSP::EffectType getType() const override { return DSP::EffectType::RingModulator; }
    virtual std::string getName() const override { return "Ring Modulator"; }

    void setSettings(const RingModSettings& s) { mSettings = s; }
    RingModSettings getSettings() const { return mSettings; }

    virtual void reset() override {

        mPhaseL = 0.0f;
        mSampleRate = getSampleRateF();

    }

    virtual void process(float* buffer, int numSamples, int numChannels) override {
        if (!isEnabled() || mSettings.wet <= 0.001f) return;

        const float TWO_PI = 2.0f * (float)M_PI;
        // Frequency increment per sample frame
        const float phaseIncrement = (TWO_PI * mSettings.frequency) / mSampleRate;

        for (int i = 0; i < numSamples; i++) {
            int channel = i % numChannels;
            float dry = buffer[i];

            // 1. Calculate the carrier wave (sinusoidal LFO)
            // We use the same phase for all channels in a frame to preserve phase alignment
            float carrier = std::sin(mPhaseL);

            // 2. Ring Modulation: Multiply dry signal by carrier
            float modulated = dry * carrier;

            // 3. Mix: Dry + Wet
            buffer[i] = (dry * (1.0f - mSettings.wet)) + (modulated * mSettings.wet);

            // 4. Advance phase only after all channels of the current frame are processed
            if (channel == numChannels - 1) {
                mPhaseL += phaseIncrement;

                // Wrap phase to keep it within [0, 2*PI]
                if (mPhaseL >= TWO_PI) {
                    mPhaseL -= TWO_PI;
                }
            }
        }
    }

    //--------------------------------------------------------------------------
#ifdef FLUX_ENGINE
    virtual ImVec4 getColor() const override { return ImVec4(0.1f, 0.2f, 0.7f, 1.0f); } // blueish

    virtual void renderUIWide() override {
        ImGui::PushID("RingMod_Effect_Row_WIDE");
        if (ImGui::BeginChild("RINGMOD_BOX", ImVec2(-FLT_MIN, 65.f))) {

            DSP::RingModSettings currentSettings = this->getSettings();
            bool changed = false;

            ImFlux::GradientBox(ImVec2(-FLT_MIN, -FLT_MIN), 0.f);
            ImGui::Dummy(ImVec2(2, 0)); ImGui::SameLine();
            ImGui::BeginGroup();

            bool isEnabled = this->isEnabled();
            if (ImFlux::LEDCheckBox(getName(), &isEnabled, getColor())) {
                this->setEnabled(isEnabled);
            }
            if (!isEnabled) ImGui::BeginDisabled();
            ImGui::SameLine(ImGui::GetWindowWidth() - 65.f);
            if (ImFlux::ButtonFancy("RESET", ImFlux::SLATEDARK_BUTTON.WithSize(ImVec2(40.f, 20.f)))) {
                this->reset();
            }
            ImGui::Separator();
            changed |= ImFlux::MiniKnobF("Frequency", &currentSettings.frequency, 200.0f, 2000.0f);ImGui::SameLine();
            changed |= ImFlux::MiniKnobF("Mix", &currentSettings.wet, 0.0f, 1.0f);ImGui::SameLine();

            if (changed) this->setSettings(currentSettings);
            if (!isEnabled) ImGui::EndDisabled();
            ImGui::EndGroup();
        }
        ImGui::EndChild();
        ImGui::PopID();
    }

    virtual void renderUI() override {
        ImGui::PushID("RingMod_Effect_Row");

            DSP::RingModSettings currentSettings = this->getSettings();
            bool changed = false;

            bool isEnabled = this->isEnabled();
            if (ImFlux::LEDCheckBox(getName(), &isEnabled, getColor())) {
                this->setEnabled(isEnabled);
            }
            if (isEnabled)
            {
                if (ImGui::BeginChild("RINGMOD_BOX", ImVec2(-FLT_MIN, 65.f),ImGuiChildFlags_Borders)) {

                    ImGui::BeginGroup();
                    changed |= ImFlux::FaderHWithText("Frequency", &currentSettings.frequency, 200.0f, 2000.0f, "%5.2f Hz");
                    changed |= ImFlux::FaderHWithText("Mix", &currentSettings.wet, 0.0f, 1.0f, "%.2f wet");
                    ImGui::EndGroup();
                    ImGui::SameLine(ImGui::GetWindowWidth() - 65.f);
                    if (ImFlux::ButtonFancy("RESET", ImFlux::SLATEDARK_BUTTON.WithSize(ImVec2(40.f, 20.f)))) {
                        this->reset();
                    }

                    if (changed) this->setSettings(currentSettings);
                }
                ImGui::EndChild();
            } else {
                ImGui::Separator();
            }

        ImGui::PopID();
        ImGui::Spacing(); // Add visual gap before the next effect
    }

#endif
}; //CLASS
}; //namespace
