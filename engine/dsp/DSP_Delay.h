//-----------------------------------------------------------------------------
// Copyright (c) 2026 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Digital Sound Processing : Delay
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


struct DelaySettings {
    float time;      //  10 - 2000ms max 2 sek
    float feedback;  //   0 - 0.95f
    float wet;       // 0.1 - 1.f

    static const uint8_t CURRENT_VERSION = 1;
    void getBinary(std::ostream& os) const {
        uint8_t ver = CURRENT_VERSION;
        os.write(reinterpret_cast<const char*>(&ver), sizeof(ver));
        os.write(reinterpret_cast<const char*>(this), sizeof(DelaySettings));
    }

    bool  setBinary(std::istream& is) {
        uint8_t fileVersion = 0;
        is.read(reinterpret_cast<char*>(&fileVersion), sizeof(fileVersion));
        if (fileVersion != CURRENT_VERSION) //Something is wrong !
            return false;
        is.read(reinterpret_cast<char*>(this), sizeof(DelaySettings));
        return  is.good();
    }

    auto operator<=>(const DelaySettings&) const = default; //C++20 lazy way

};


//-----
constexpr DelaySettings SLAPBACK_DELAY  = { 50.f,  0.3f, 0.2f }; // Slapback style
constexpr DelaySettings MEDIUM_DELAY = { 300.f, 0.4f, 0.3f }; // Standard echo
constexpr DelaySettings SPACEY_DELAY   = { 800.f, 0.5f, 0.4f }; // Atmospheric/Spacey


constexpr DelaySettings  CUSTOM_DELAY     = MEDIUM_DELAY;
//-----



static const char* DELAY_PRESET_NAMES[] = { "Custom","Standard", "Slapback", "Spacey" };

static const std::array<DSP::DelaySettings, 4> DELAY_PRESETS = {
    CUSTOM_DELAY,
    MEDIUM_DELAY,
    SLAPBACK_DELAY,
    SPACEY_DELAY
};

class Delay : public DSP::Effect {
private:
    std::vector<float> mBufL;
    std::vector<float> mBufR;
    uint32_t mPosL = 0;
    uint32_t mPosR = 0;
    DelaySettings mSettings;

    float mSampleRate = getSampleRateF();
    uint32_t mMaxBufSize;

    float mSmoothedDelaySamples = 0.f;


public:
    Delay(bool switchOn = false) :
    Effect(switchOn)
    {
        // Allocate 1 second of buffer for 44.1kHz as float
        mSampleRate = getSampleRateF();
        mMaxBufSize = static_cast<uint32_t>(mSampleRate * 2.f); //2 sec

        mBufL.assign(mMaxBufSize, 0.0f);
        mBufR.assign(mMaxBufSize, 0.0f);

        mSettings = MEDIUM_DELAY;

        mPosL = 0;
        mPosR = 0;

        mSmoothedDelaySamples = 0.f;

    }



    DSP::EffectType getType() const override { return DSP::EffectType::Delay; }

    const DelaySettings& getSettings() { return mSettings; }

    void setSettings(const DelaySettings& s) {
        mSettings = s;
    }

    void reset() override {

        std::fill(mBufL.begin(), mBufL.end(), 0.0f);
        std::fill(mBufR.begin(), mBufR.end(), 0.0f);
        mPosL = 0;
        mPosR = 0;
        mSmoothedDelaySamples = 0.f;
    }

    void save(std::ostream& os) const override {
        Effect::save(os);              // Save mEnabled
        mSettings.getBinary(os);       // Save Settings
    }

    bool load(std::istream& is) override {
        if (!Effect::load(is)) return false; // Load mEnabled
        return mSettings.setBinary(is);      // Load Settings
    }

    virtual float getTailLengthSeconds() const override {
        if (!isEnabled()) return 0.0f;

        float feedback = mSettings.feedback;
        float delayMs = mSettings.time;

        // If feedback is very low, tail is just one delay cycle
        if (feedback < 0.01f) return delayMs / 1000.0f;

        // Safety: Clamp feedback to avoid infinite tail or log(1) error
        if (feedback >= 0.99f) feedback = 0.99f;

        // n = log(threshold) / log(feedback)
        // -60dB threshold (0.001) is standard for "silence"
        float iterations = logf(0.001f) / logf(feedback);
        return (iterations * delayMs) / 1000.0f;
    }



    virtual void process(float* buffer, int numSamples) override {
        if (!isEnabled() || mSettings.wet <= 0.001f) return;

        // 1. Target delay in samples (as float for interpolation)
        float targetDelaySamples = (mSettings.time / 1000.0f) * mSampleRate;

        for (int i = 0; i < numSamples; i++) {
            // 2. Smooth the delay time to prevent clicks
            // If you don't want pitch shifting, use a very small factor like 0.0001f
            mSmoothedDelaySamples += 0.001f * (targetDelaySamples - mSmoothedDelaySamples);

            float dry = buffer[i];
            float delayed = 0.0f;

            // Determine which buffer and position to use
            float* activeBuf = (i % 2 == 0) ? mBufL.data() : mBufR.data();
            uint32_t& activePos = (i % 2 == 0) ? mPosL : mPosR;

            // 3. Safe Read Position Calculation
            // Subtract smoothed samples from current write position
            float readPos = static_cast<float>(activePos) - mSmoothedDelaySamples;

            // Wrap-around logic (Modulo replacement)
            while (readPos < 0.0f) readPos += static_cast<float>(mMaxBufSize);

            // 4. Linear Interpolation (Prevents the "staircase" clicking)
            int indexA = static_cast<int>(readPos);
            int indexB = (indexA + 1) % mMaxBufSize;
            float fraction = readPos - static_cast<float>(indexA);

            delayed = activeBuf[indexA] + fraction * (activeBuf[indexB] - activeBuf[indexA]);

            // 5. Update Buffer (Feedback)
            activeBuf[activePos] = dry + (delayed * mSettings.feedback);

            // 6. Final Mix and Advance
            buffer[i] = (dry * (1.0f - mSettings.wet)) + (delayed * mSettings.wet);

            activePos = (activePos + 1) % mMaxBufSize;
        }
    }


    virtual std::string getName() const override { return "DELAY";}
#ifdef FLUX_ENGINE
    virtual ImVec4 getColor() const  override { return ImVec4(0.3f, 0.8f, 0.6f, 1.0f);}

    virtual void renderUIWide() override {
        ImGui::PushID("Delay_Effect_Row_WIDE");
        if (ImGui::BeginChild("DELAY_BOX", ImVec2(-FLT_MIN,65.f) )) {

            DSP::DelaySettings currentSettings = this->getSettings();
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
            for (int i = 1; i < DSP::DELAY_PRESETS.size(); ++i) {
                if (currentSettings == DSP::DELAY_PRESETS[i]) {
                    currentIdx = i;
                    break;
                }
            }
            int displayIdx = currentIdx;  //<< keep currentIdx clean
            ImGui::SameLine(ImGui::GetWindowWidth() - 260.f); // Right-align reset button

            if (ImFlux::ValueStepper("##Preset", &displayIdx, DELAY_PRESET_NAMES
                , IM_ARRAYSIZE(DELAY_PRESET_NAMES)))
            {
                if (displayIdx > 0 && displayIdx < DSP::DELAY_PRESETS.size()) {
                    currentSettings =  DSP::DELAY_PRESETS[displayIdx];
                    changed = true;
                }
            }
            ImGui::SameLine();
            // if (ImFlux::FaderButton("Reset", ImVec2(40.f, 20.f)))  {
            if (ImFlux::ButtonFancy("RESET", ImFlux::SLATEDARK_BUTTON.WithSize(ImVec2(40.f, 20.f)) ))  {
                currentSettings = DSP::MEDIUM_DELAY; //DEFAULT
                this->reset();
                changed = true;
            }

            ImGui::Separator();
            // ImFlux::MiniKnobF(label, &value, min_v, max_v);
            changed |= ImFlux::MiniKnobF("Time", &currentSettings.time, 10.0f, 2000.0f); ImGui::SameLine();
            changed |= ImFlux::MiniKnobF("Feedback", &currentSettings.feedback, 0.1f, 0.95f); ImGui::SameLine();
            changed |= ImFlux::MiniKnobF("Mix", &currentSettings.wet, 0.01f, 1.0f); ImGui::SameLine();

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
        ImGui::PushID("Delay_Effect_Row");

        ImGui::BeginGroup();

        bool isEnabled = this->isEnabled();
        if (ImFlux::LEDCheckBox(getName(), &isEnabled, getColor())){
                this->setEnabled(isEnabled);
        }
            if (isEnabled)
            {
                if (ImGui::BeginChild("BC_Box", ImVec2(0, 110), ImGuiChildFlags_Borders)) {


                    ImGui::BeginGroup();


                    DSP::DelaySettings currentSettings = this->getSettings();
                    bool changed = false;

                    int currentIdx = 0; // Standard: "Custom"
                    for (int i = 1; i < DSP::DELAY_PRESETS.size(); ++i) {
                        if (currentSettings == DSP::DELAY_PRESETS[i]) {
                            currentIdx = i;
                            break;
                        }
                    }
                    int displayIdx = currentIdx;  //<< keep currentIdx clean

                    ImGui::SetNextItemWidth(150);
                    if (ImFlux::ValueStepper("##Preset", &displayIdx, DELAY_PRESET_NAMES, IM_ARRAYSIZE(DELAY_PRESET_NAMES))) {
                        if (displayIdx > 0 && displayIdx < DSP::DELAY_PRESETS.size()) {
                            currentSettings =  DSP::DELAY_PRESETS[displayIdx];
                            changed = true;
                        }
                    }
                    ImGui::SameLine(ImGui::GetWindowWidth() - 60); // Right-align reset button

                    if (ImFlux::FaderButton("Reset", ImVec2(40.f, 20.f)))  {
                        currentSettings = DSP::MEDIUM_DELAY; //DEFAULT
                        this->reset();
                        changed = true;
                    }
                    ImGui::Separator();

//
                    // Control Sliders
                    changed |= ImFlux::FaderHWithText("Time", &currentSettings.time, 10.0f, 2000.0f, "%.1f ms");
                    changed |= ImFlux::FaderHWithText("Feedback", &currentSettings.feedback, 0.1f, 0.95f, "%.2f");
                    changed |= ImFlux::FaderHWithText("Mix", &currentSettings.wet, 0.01f, 1.0f, "%.2f wet");

                    // Engine Update
                    if (changed) {
                        if (isEnabled) {
                            this->setSettings(currentSettings);
                        }
                    }
                    ImGui::EndGroup();
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

}; //CLASS
}; //namespace
