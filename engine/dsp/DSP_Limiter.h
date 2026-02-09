//-----------------------------------------------------------------------------
// Copyright (c) 2026 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Digital Sound Processing : Limiter with integrated VU-Meter Data
//-----------------------------------------------------------------------------
#pragma once

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


    struct LimiterSettings {
        float Threshold ;  // Limit just before 1.f
        float Attack;      // How fast it turns down
        float Release;     // How slow it turns back up

        static const uint8_t CURRENT_VERSION = 1;
        void getBinary(std::ostream& os) const {
            uint8_t ver = CURRENT_VERSION;
            os.write(reinterpret_cast<const char*>(&ver), sizeof(ver));
            os.write(reinterpret_cast<const char*>(this), sizeof(LimiterSettings));
        }

        bool  setBinary(std::istream& is) {
            uint8_t fileVersion = 0;
            is.read(reinterpret_cast<char*>(&fileVersion), sizeof(fileVersion));
            if (fileVersion != CURRENT_VERSION) //Something is wrong !
                return false;
            is.read(reinterpret_cast<char*>(this), sizeof(LimiterSettings));
            return  is.good();
        }

        auto operator<=>(const LimiterSettings&) const = default; //C++20 lazy way


    };


    constexpr LimiterSettings LIMITER_DEFAULT = { 0.95f,  0.05f,  0.150f };
    constexpr LimiterSettings LIMITER_EIGHTY  = { 0.80f,  0.05f,  0.150f };
    constexpr LimiterSettings LIMITER_FIFTY   = { 0.50f,  0.05f,  0.150f };
    constexpr LimiterSettings LIMITER_LOWVOL  = { 0.25f,  0.05f,  0.150f };
    constexpr LimiterSettings LIMITER_EXTREM  = { 0.05f,  0.50f,  0.150f };

    constexpr LimiterSettings LIMITER_CUSTOM  = LIMITER_DEFAULT; //<< DUMMY

    static const char* LIMITER_PRESET_NAMES[] = {
        "CUSTOM", "DEFAULT", "EIGHTY"
        , "FIFTY", "LOWVOL", "EXTREM" };


    static const std::array<DSP::LimiterSettings, 6> LIMITER_PRESETS = {
        DSP::LIMITER_CUSTOM,  // Index 0:
        DSP::LIMITER_DEFAULT, // Index 1
        DSP::LIMITER_EIGHTY,  // Index 2
        DSP::LIMITER_FIFTY,   // Index 3
        DSP::LIMITER_LOWVOL,  // Index 4
        DSP::LIMITER_EXTREM   // Index 5
    };


    class Limiter : public Effect {
    private:
        float mCurrentGain = 1.0f;
        LimiterSettings mSettings;


    public:

        Limiter(bool switchOn = true) :
            Effect(switchOn),
            mSettings(LIMITER_DEFAULT)
            {}

        DSP::EffectType getType() const override { return DSP::EffectType::Limiter; }
        //----------------------------------------------------------------------
        void setSettings(const LimiterSettings& s) {
                // we call reset extra reset(); //also reset current gain.
                mSettings = s;
        }
        //----------------------------------------------------------------------
        void reset() override { mCurrentGain = 1.f; }
        //----------------------------------------------------------------------
        LimiterSettings& getSettings() { return mSettings; }
        //----------------------------------------------------------------------
        void save(std::ostream& os) const override {
            Effect::save(os);              // Save mEnabled
            mSettings.getBinary(os);       // Save Settings
        }
        //----------------------------------------------------------------------
        bool load(std::istream& is) override {
            if (!Effect::load(is)) return false; // Load mEnabled
            return mSettings.setBinary(is);      // Load Settings
        }
        //----------------------------------------------------------------------
        // fetch current Gain
        // 1.0 = open , 0.5 = -6dB
        float getGain() const { return mCurrentGain; }
        float getGainReduction() const { return 1.f - mCurrentGain; }
        //----------------------------------------------------------------------
        virtual void process(float* buffer, int numSamples, int numChannels) override {
            // Basic safety check: exit if disabled or no channels present
            if (!isEnabled() || numChannels <= 0) return;

            // Calculate how many timeframes (multi-channel bundles) are in the buffer
            int numFrames = numSamples / numChannels;

            for (int f = 0; f < numFrames; f++) {
                // Calculate the starting index for this timeframe
                int frameIndex = f * numChannels;

                // 1. Peak Detection: Find the maximum absolute peak across ALL channels in this frame
                // This ensures a "Stereo Link" (equal gain reduction for all channels)
                float maxAbsInput = 0.0f;
                for (int c = 0; c < numChannels; c++) {
                    float absVal = std::abs(buffer[frameIndex + c]);
                    if (absVal > maxAbsInput) {
                        maxAbsInput = absVal;
                    }
                }

                // 2. Gain Calculation: Determine target gain based on the threshold
                float targetGain = 1.0f;
                if (maxAbsInput > mSettings.Threshold) {
                    // Prevent division by zero using a small epsilon (1e-9f)
                    targetGain = mSettings.Threshold / (maxAbsInput + 1e-9f);
                }

                // 3. Envelope Smoothing: Apply Attack or Release depending on gain direction
                // Note: Attack and Release should be coefficients based on the sample rate
                if (targetGain < mCurrentGain) {
                    // Attack phase (gain reduction is getting stronger)
                    mCurrentGain += (targetGain - mCurrentGain) * mSettings.Attack;
                } else {
                    // Release phase (gain is returning to unity)
                    mCurrentGain += (targetGain - mCurrentGain) * mSettings.Release * 0.0001f;
                }

                // 4. Application: Apply the calculated gain to every channel in this frame
                // This preserves the spatial balance (stereo image) of the audio
                for (int c = 0; c < numChannels; c++) {
                    buffer[frameIndex + c] *= mCurrentGain;
                }
            }
        }


    //----------------------------------------------------------------------
    virtual std::string getName() const override { return "LIMITER";}
#ifdef FLUX_ENGINE
    virtual ImVec4 getColor() const  override { return ImVec4(0.6f, 0.4f, 0.6f, 1.0f);}
    virtual void renderUIWide() override {
        ImGui::PushID("Limiter_Effect_Row_WIDE");
        if (ImGui::BeginChild("LIMITER_BOX", ImVec2(-FLT_MIN,65.f) )) {

            DSP::LimiterSettings currentSettings = this->getSettings();
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
            for (int i = 1; i < DSP::LIMITER_PRESETS.size(); ++i) {
                if (currentSettings == DSP::LIMITER_PRESETS[i]) {
                    currentIdx = i;
                    break;
                }
            }
            int displayIdx = currentIdx;  //<< keep currentIdx clean
            ImGui::SameLine(ImGui::GetWindowWidth() - 260.f); // Right-align reset button

            if (ImFlux::ValueStepper("##Preset", &displayIdx, LIMITER_PRESET_NAMES
                , IM_ARRAYSIZE(LIMITER_PRESET_NAMES)))
            {
                if (displayIdx > 0 && displayIdx < DSP::LIMITER_PRESETS.size()) {
                    currentSettings =  DSP::LIMITER_PRESETS[displayIdx];
                    changed = true;
                }
            }
            ImGui::SameLine();
            // if (ImFlux::FaderButton("Reset", ImVec2(40.f, 20.f)))  {
            if (ImFlux::ButtonFancy("RESET", ImFlux::SLATEDARK_BUTTON.WithSize(ImVec2(40.f, 20.f)) ))  {
                currentSettings = DSP::LIMITER_DEFAULT; //DEFAULT
                this->reset();
                changed = true;
            }

            ImGui::Separator();
            // ImFlux::MiniKnobF(label, &value, min_v, max_v);
            changed |= ImFlux::MiniKnobF("Threshold", &currentSettings.Threshold, 0.01f, 1.f); ImGui::SameLine();
            changed |= ImFlux::MiniKnobF("Depth", &currentSettings.Attack, 0.0f, 1.f); ImGui::SameLine();
            changed |= ImFlux::MiniKnobF("Release", &currentSettings.Release, 0.05f, 1.f); ImGui::SameLine();



            ImGui::BeginGroup();
            float reduction = getGainReduction();
            ImGui::TextDisabled("Reduction: %4.1f%%", reduction * 100.f);
            ImFlux::PeakMeter(reduction,ImVec2(125.f, 8.f));
            ImGui::EndGroup();


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
            ImGui::PushID("Limiter_Effect_Row");


            ImGui::BeginGroup();

            auto* lim = this;
            bool isEnabled = lim->isEnabled();


            if (ImFlux::LEDCheckBox(getName(), &isEnabled, getColor())) {
                lim->setEnabled(isEnabled);
            }


            if (lim->isEnabled()) {
                bool changed = false;
                DSP::LimiterSettings& currentSettings = lim->getSettings();

                int currentIdx = 0; // Standard: "Custom"

                for (int i = 1; i < DSP::LIMITER_PRESETS.size(); ++i) {
                    if (currentSettings == DSP::LIMITER_PRESETS[i]) {
                        currentIdx = i;
                        break;
                    }
                }
                int displayIdx = currentIdx;  //<< keep currentIdx clean

                // height 150 with all sliders!

                if (ImGui::BeginChild("LIM_Box", ImVec2(0, 75.f),  ImGuiChildFlags_Borders)) {

                    ImGui::BeginGroup();
                    ImGui::SetNextItemWidth(150);

                    if (ImFlux::ValueStepper("##Preset", &displayIdx, LIMITER_PRESET_NAMES, IM_ARRAYSIZE(LIMITER_PRESET_NAMES))) {
                        if (displayIdx > 0 && displayIdx < DSP::LIMITER_PRESETS.size()) {
                                    lim->setSettings(DSP::LIMITER_PRESETS[displayIdx]);
                        }
                    }

                    // Quick Reset Button (Now using the FLAT_EQ preset)
                    ImGui::SameLine(ImGui::GetWindowWidth() - 60);

                    // if (ImGui::SmallButton("Reset")) {
                    if (ImFlux::FaderButton("Reset", ImVec2(40.f, 20.f)))  {
                        lim->setSettings(DSP::LIMITER_DEFAULT);
                        lim->reset();
                    }
                    // ImGui::Separator();
                    // changed |= ImFlux::FaderHWithText("Threshold", &currentSettings.Threshold, 0.01f, 1.f, "%.3f");
                    // changed |= ImFlux::FaderHWithText("Depth", &currentSettings.Attack, 0.f, 1.f, "%.2f ms");
                    // changed |= ImFlux::FaderHWithText("Release", &currentSettings.Release, 0.05f, 1.f, "%.5f");
                    //
                    //
                    if (changed) {
                         lim->setSettings(currentSettings);
                    }

                    ImGui::Separator();
                    float reduction = getGainReduction();
                    ImGui::TextDisabled("Reduction: %3.3f", reduction);
                    ImFlux::PeakMeter(reduction,ImVec2(150.f, 7.f));
                    ImGui::EndGroup();

                } //box
                ImGui::EndChild();
            } else {
                ImGui::Separator();
            }

            ImGui::EndGroup();
            ImGui::PopID();
            ImGui::Spacing();

        }

    #endif //FLUX_ENGINE


    };

} // namespace DSP
