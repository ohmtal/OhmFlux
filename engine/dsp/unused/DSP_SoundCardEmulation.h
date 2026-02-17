//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Digital Sound Processing : SoundCardEmulation (maybe obsolte since warmth)
//-----------------------------------------------------------------------------
#pragma once
#define _USE_MATH_DEFINES // Required for M_PI on some systems (like Windows/MSVC)
#include <cmath>
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

    static const char* SC_RENDER_MODE_NAMES[] = {
        "Blended (Smooth)", "Modern LPF (Warm)",
        "Sound Blaster Pro", "Sound Blaster", "AdLib Gold", "Sound Blaster Clone"
    };

    enum class RenderMode : uint32_t { // Explicitly set size to 4 bytes
        BLENDED, MODERN_LPF, SBPRO, SB_ORIGINAL, ADLIB_GOLD, CLONE_CARD
    };

    struct SoundCardEmulationSettings {
        RenderMode renderMode = RenderMode::BLENDED; // Default initialization

        static const uint8_t CURRENT_VERSION = 1;

        void getBinary(std::ostream& os) const {
            uint8_t ver = CURRENT_VERSION;
            DSP_STREAM_TOOLS::write_binary(os, ver);

            DSP_STREAM_TOOLS::write_binary(os, renderMode);
        }

        bool setBinary(std::istream& is) {
            uint8_t fileVersion = 0;
            DSP_STREAM_TOOLS::read_binary(is, fileVersion);
            if (fileVersion != CURRENT_VERSION) return false;
            DSP_STREAM_TOOLS::read_binary(is, renderMode);

            // Safety check: Ensure the loaded enum value is valid
            if (renderMode > RenderMode::CLONE_CARD) {
                renderMode = RenderMode::BLENDED;
                return false; //?!
            }

            return is.good();
        }


        auto operator<=>(const SoundCardEmulationSettings&) const = default; //C++20 lazy way
    };


    constexpr SoundCardEmulationSettings BLENDED_MODE  = {  RenderMode::BLENDED };


class SoundCardEmulation : public DSP::Effect {
    struct FilterChannelState {
        float prevInput = 0.0f;
        float filterState = 0.0f;
    };

private:
    float mAlpha = 1.0f;
    float mGain = 1.0f;
    bool mUseBlending = true;

    SoundCardEmulationSettings mSettings;

    // State for filtering and blending
    std::vector<FilterChannelState> mStates;

public:
    IMPLEMENT_EFF_CLONE(SoundCardEmulation)

    SoundCardEmulation(bool switchOn = false) : DSP::Effect(DSP::EffectType::SoundCardEmulation, switchOn) {
        mSettings.renderMode = RenderMode::BLENDED;
    }

    virtual std::string getName() const override { return "SOUND RENDERING";}

    SoundCardEmulationSettings& getSettings() { return mSettings; }

    void setSettings(const SoundCardEmulationSettings& s) {
        mSettings = s;
        setMode(s.renderMode);
    }

    RenderMode getMode() {
        return mSettings.renderMode;
    }

private:
    void setMode(RenderMode mode) {
        float cutoff = 20000.0f;
        mUseBlending = true;
        mGain = 1.0f;

        switch (mode) {
            case RenderMode::BLENDED:     cutoff = 20000.0f; break;
            case RenderMode::SBPRO:       cutoff = 3200.0f;  mGain = 1.2f; break;
            case RenderMode::SB_ORIGINAL: cutoff = 2800.0f;  mGain = 1.3f; break;
            case RenderMode::ADLIB_GOLD:  cutoff = 16000.0f; mGain = 1.05f; break;
            case RenderMode::MODERN_LPF:  cutoff = 12000.0f; break;
            case RenderMode::CLONE_CARD:  cutoff = 8000.0f;  mUseBlending = false; mGain = 0.9f; break;
        }

        // Standard Alpha calculation for 1-pole LPF
        float dt = 1.0f / mSampleRate;
        float rc = 1.0f / (2.0f * M_PI * cutoff);
        mAlpha = (cutoff >= 20000.0f) ? 1.0f : (dt / (rc + dt));

        mStates = {{0.f,0.f} , {0.f, 0.f}};

        // mFilterStateL = 0.0f;
        // mFilterStateR = 0.0f;
        // mPrevInputL = 0.0f;
        // mPrevInputR = 0.0f;
    }
public:

    void save(std::ostream& os) const override {
        Effect::save(os);              // Save mEnabled
        mSettings.getBinary(os);       // Save Settings
    }

    bool load(std::istream& is) override {
        if (!Effect::load(is)) return false; // Load mEnabled
        return mSettings.setBinary(is);      // Load Settings
    }


    virtual void process(float* buffer, int numSamples, int numChannels) override {
        if (!mEnabled) return;

        // Ensure we have a state object for every channel
        if (mStates.size() != static_cast<size_t>(numChannels)) {
            mStates.resize(numChannels, FilterChannelState());
        }

        int channel = 0;
        for (int i = 0; i < numSamples; i++) {
            float input = buffer[i];

            // Access the state for the current channel
            FilterChannelState& s = mStates[channel];

            // 1. Blending (Linear Interpolation)
            // Average of current input and previous input
            float blended = mUseBlending ? (input + s.prevInput) * 0.5f : input;

            // Save raw input for the next sample's blending
            s.prevInput = input;

            // 2. Low-Pass Filter (One-Pole IIR)
            // Simple smoothing formula: y[n] = y[n-1] + alpha * (x[n] - y[n-1])
            s.filterState = s.filterState + mAlpha * (blended - s.filterState);

            // 3. Apply Gain and Clamp
            float out = DSP::clamp(s.filterState * mGain, -1.0f, 1.0f);

            // Safety: Check for NaN and reset if necessary
            if (std::isnan(out)) {
                s.filterState = 0.0f;
                out = 0.0f;
            }

            buffer[i] = out;

            if (++channel >= numChannels) channel = 0;
        }
    }

    #ifdef FLUX_ENGINE
    virtual ImVec4 getColor() const  override { return ImVec4(0.62f, 0.42f, 0.5f, 1.0f);}

    virtual void renderUIWide() override {
        ImGui::PushID("RenderSoundCardEmu_Effect_Row_WIDE");
        if (ImGui::BeginChild("SCEMU_BOX", ImVec2(-FLT_MIN,30.f) )) {
            auto* lEmu = this;
            ImFlux::GradientBox(ImVec2(-FLT_MIN, -FLT_MIN),0.f);
            ImGui::Dummy(ImVec2(2,0)); ImGui::SameLine();
            ImGui::BeginGroup();
            bool isEnabled = this->isEnabled();
            if (ImFlux::LEDCheckBox(getName(), &isEnabled, getColor())){
                this->setEnabled(isEnabled);
            }
            if (!isEnabled) ImGui::BeginDisabled();

            // -------- stepper >>>>
             ImGui::SameLine(ImGui::GetWindowWidth() - 260.f); // Right-align reset button
            DSP::RenderMode currentMode = lEmu->getMode();
            int lRenderModeInt  = (int)currentMode;
            if (ImFlux::ValueStepper("##Preset", &lRenderModeInt, SC_RENDER_MODE_NAMES, IM_ARRAYSIZE(SC_RENDER_MODE_NAMES))) {
                lEmu->setSettings({ (DSP::RenderMode)lRenderModeInt });
            }
            ImGui::SameLine();
            if (ImFlux::ButtonFancy("RESET", ImFlux::SLATEDARK_BUTTON.WithSize(ImVec2(40.f, 20.f)) ))  {
                lEmu->setSettings( BLENDED_MODE );
            }

            if (!isEnabled) ImGui::EndDisabled();

            ImGui::EndGroup();
        }
        ImGui::EndChild();
        ImGui::PopID();

    }

    virtual void renderUI() override {
        ImGui::PushID("RenderSoundCardEmu_Effect_Row");
        ImGui::BeginGroup();

        auto* lEmu = this;
        bool isEnabled = lEmu->isEnabled();
        if (ImFlux::LEDCheckBox(getName(), &isEnabled, getColor()))
            lEmu->setEnabled(isEnabled);

        if (isEnabled) {
            if (ImGui::BeginChild("SC_Box", ImVec2(0, 35), ImGuiChildFlags_Borders)) {
                DSP::RenderMode currentMode = lEmu->getMode();
                int lRenderModeInt  = (int)currentMode;
                if (ImFlux::ValueStepper("##Preset", &lRenderModeInt, SC_RENDER_MODE_NAMES, IM_ARRAYSIZE(SC_RENDER_MODE_NAMES))) {
                    lEmu->setSettings({ (DSP::RenderMode)lRenderModeInt });
                }
            }
            ImGui::EndChild();
        } else {
            ImGui::Separator();
        }

        ImGui::EndGroup();
        ImGui::PopID();
        ImGui::Spacing(); // Add visual gap before the next effect
    }
#endif


}; //class
}; //namespace
