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
        os.write(reinterpret_cast<const char*>(&ver), sizeof(ver));
        os.write(reinterpret_cast<const char*>(this), sizeof(RingModSettings));
    }

    bool  setBinary(std::istream& is) {
        uint8_t fileVersion = 0;
        is.read(reinterpret_cast<char*>(&fileVersion), sizeof(fileVersion));
        if (fileVersion != CURRENT_VERSION) //Something is wrong !
            return false;
        is.read(reinterpret_cast<char*>(this), sizeof(RingModSettings));
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
        if (numChannels !=  2) { return;  }  //FIXME REWRITE from stereo TO variable CHANNELS

        if (!isEnabled()) return;
        if (mSettings.wet <= 0.001f) return;

        // Frequency increment per sample step (LFO speed)
        const float phaseIncrement = (2.0f * M_PI * mSettings.frequency) / mSampleRate;

        for (int i = 0; i < numSamples; i += 2) {
            float dryL = buffer[i];
            float dryR = buffer[i+1]; // Keep stereo image but use mono carrier

            // Calculate the carrier wave (sinusoidal LFO)
            // We run one phase accumulator (mLPhase) for simplicity
            float carrier = sinf(mPhaseL);

            // Ring Modulation: Simple multiplication of dry signal by carrier wave
            float modulatedL = dryL * carrier;
            float modulatedR = dryR * carrier;

            // Mix: Dry (Original) + Wet (Modulated)
            buffer[i]     = (dryL * (1.0f - mSettings.wet)) + (modulatedL * mSettings.wet);
            buffer[i + 1] = (dryR * (1.0f - mSettings.wet)) + (modulatedR * mSettings.wet);

            // Advance phase
            mPhaseL += phaseIncrement;
            while (mPhaseL >= 2.0f * M_PI) mPhaseL -= 2.0f * M_PI;
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
            } //enabled
        ImGui::PopID();
    }

#endif
}; //CLASS
}; //namespace
