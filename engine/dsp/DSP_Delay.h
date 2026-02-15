//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Digital Sound Processing : Delay
//-----------------------------------------------------------------------------
// * using ISettings
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

struct DelayData  {
    float time;      //  10 - 2000ms max 2 sek
    float feedback;  //   0 - 0.95f
    float wet;       // 0.1 - 1.f
};

// default { 300.f, 0.4f, 0.3f }
struct DelaySettings : public ISettings {
    AudioParam<float> time     { "Time" ,  300.f, 10.f,  2000.f, "%.1f ms" };
    AudioParam<float> feedback { "Feedback", 0.4f, 0.1f,  0.95f, "%.2f" };
    AudioParam<float> wet      { "Mix",   0.3f ,  0.0f, 1.0f, "Wet %.2f"};

    DelaySettings() = default;
    REGISTER_SETTINGS(DelaySettings, &time, &feedback, &wet)

    DelayData getData() const {
        return { time.get(), feedback.get(), wet.get()};
    }

    void setData(const DelayData& data) {
        time.set(data.time);
        feedback.set(data.feedback);
        wet.set(data.wet);
    }
    std::vector<std::shared_ptr<IPreset>> getPresets() const override {
        return {
            std::make_shared<Preset<DelaySettings, DelayData>>
                ("Custom", DelayData{ 300.f, 0.4f, 0.3f }),

            std::make_shared<Preset<DelaySettings, DelayData>>
                ("Slapback", DelayData{ 50.f,  0.3f, 0.2f }),

            std::make_shared<Preset<DelaySettings, DelayData>>
                ("Standard", DelayData{ 300.f, 0.4f, 0.3f }),

            std::make_shared<Preset<DelaySettings, DelayData>>
                ("Spacey", DelayData{ 800.f, 0.5f, 0.4f })
        };
    }

};

//-----
constexpr DelayData DEFAULT_DELAY_DATA = { 300.f, 0.4f, 0.3f }; // Standard echo
//-----


class Delay : public DSP::Effect {
private:

    std::vector<std::vector<float>> mBuffers;
    std::vector<uint32_t> mPositions;
    float* mChannelPtrs[8];

    DelaySettings mSettings;
    uint32_t mMaxBufSize;

    float mSmoothedDelaySamples = 0.f;


public:
    IMPLEMENT_EFF_CLONE(Delay)

    Delay(bool switchOn = false) :
    Effect(DSP::EffectType::Delay, switchOn)
    ,mSettings()
    {
        setSampleRate(mSampleRate);
        mSmoothedDelaySamples = 0.f;
    }
    //----------------------------------------------------------------------
    virtual std::string getName() const override { return "DELAY";}
    //----------------------------------------------------------------------
    DelaySettings& getSettings() { return mSettings; }
    //----------------------------------------------------------------------
    void setSettings(const DelaySettings& s) {
        mSettings = s;
    }
    //----------------------------------------------------------------------
    void updateBuffers( int numChannels) {
        mBuffers.assign(numChannels, std::vector<float>(mMaxBufSize, 0.0f));
        mPositions.assign(numChannels, 0);

        for (int i = 0; i < numChannels; ++i) {
            mChannelPtrs[i] = mBuffers[i].data();
        }

    }
    //----------------------------------------------------------------------
    void init(int numChannels, float sampleRate) {
        mMaxBufSize = 32768;
        while (mMaxBufSize < (int)(sampleRate * 2.0f)) mMaxBufSize <<= 1;
        updateBuffers(numChannels);
    }
    //----------------------------------------------------------------------
    void setSampleRate(float sampleRate) override {

        mSampleRate = sampleRate;
        int curChannels = (int) mBuffers.size();
        init(curChannels, sampleRate);
    }
    //----------------------------------------------------------------------
    void reset() override {

        int curChannels = (int) mBuffers.size();
        updateBuffers(curChannels);

        mSmoothedDelaySamples = 0.f;
    }
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
    //----------------------------------------------------------------------
    // Process
    //----------------------------------------------------------------------
    virtual void process(float* buffer, int numSamples, int numChannels) override {
        if (!isEnabled() || mSettings.wet <= 0.001f) return;


        int curChannels = (int) mBuffers.size();
        if (numChannels != curChannels) updateBuffers(numChannels);


        // prepare ( mMaxBufSize must have power of 2 !!! )
        const uint32_t mask = mMaxBufSize - 1;
        const float targetDelaySamples = (mSettings.time / 1000.0f) * mSampleRate;
        const float feedback = mSettings.feedback;
        const float wet = mSettings.wet;
        const float dryGain = 1.0f - wet;

        // Pointer-Array
        for (int c = 0; c < numChannels; ++c) mChannelPtrs[c] = mBuffers[c].data();

        int channel = 0;
        for (int i = 0; i < numSamples; i++) {
            //  Smoothing (
            if (channel == 0) {
                mSmoothedDelaySamples += 0.001f * (targetDelaySamples - mSmoothedDelaySamples);
            }

            float dry = buffer[i];
            float* activeBuf = mChannelPtrs[channel];
            uint32_t& activePos = mPositions[channel];

            //  Read Position (no Modulo!)
            float readPos = (float)activePos - mSmoothedDelaySamples;
            while (readPos < 0.0f) readPos += (float)mMaxBufSize;

            uint32_t indexA = (uint32_t)readPos & mask;
            uint32_t indexB = (indexA + 1) & mask;
            float fraction = readPos - (float)((uint32_t)readPos);

            // Linear Interpolation
            float delayed = activeBuf[indexA] + fraction * (activeBuf[indexB] - activeBuf[indexA]);

            // Update Buffer & Mix
            activeBuf[activePos] = dry + (delayed * feedback);
            buffer[i] = (dry * dryGain) + (delayed * wet);

            // Increment (Bitmasking instead of %)
            activePos = (activePos + 1) & mask;

            if (++channel >= numChannels) channel = 0;
        }
    }


    // virtual void process(float* buffer, int numSamples, int numChannels) override {
    //     if (!isEnabled() || mSettings.wet <= 0.001f) return;
    //
    //     // Initialize/Resize buffers and positions if channel count changes
    //     if (mBuffers.size() != static_cast<size_t>(numChannels)) {
    //         mBuffers.assign(numChannels, std::vector<float>(mMaxBufSize, 0.0f));
    //         mPositions.assign(numChannels, 0);
    //     }
    //
    //     // Target delay in samples
    //     float targetDelaySamples = (mSettings.time / 1000.0f) * mSampleRate;
    //
    //     for (int i = 0; i < numSamples; i++) {
    //         int channel = i % numChannels;
    //         float dry = buffer[i];
    //
    //         // 1. Smooth the delay time (Only once per frame to maintain pitch consistency)
    //         if (channel == 0) {
    //             mSmoothedDelaySamples += 0.001f * (targetDelaySamples - mSmoothedDelaySamples);
    //         }
    //
    //         // 2. Identify active resources for current channel
    //         std::vector<float>& activeBuf = mBuffers[channel];
    //         uint32_t& activePos = mPositions[channel];
    //
    //         // 3. Safe Read Position Calculation
    //         float readPos = static_cast<float>(activePos) - mSmoothedDelaySamples;
    //
    //         // Wrap-around logic
    //         while (readPos < 0.0f) readPos += static_cast<float>(mMaxBufSize);
    //
    //         // 4. Linear Interpolation
    //         int indexA = static_cast<int>(readPos) % mMaxBufSize;
    //         int indexB = (indexA + 1) % mMaxBufSize;
    //         float fraction = readPos - static_cast<float>(indexA);
    //
    //         float delayed = activeBuf[indexA] + fraction * (activeBuf[indexB] - activeBuf[indexA]);
    //
    //         // 5. Update Buffer (Feedback)
    //         activeBuf[activePos] = dry + (delayed * mSettings.feedback);
    //
    //         // 6. Final Mix
    //         buffer[i] = (dry * (1.0f - mSettings.wet)) + (delayed * mSettings.wet);
    //
    //         // 7. Advance write position for this specific channel
    //         activePos = (activePos + 1) % mMaxBufSize;
    //     }
    // }


    // virtual void process(float* buffer, int numSamples, int numChannels) override {
    //     if (numChannels !=  2) { return;  }  //FIXME REWRITE from stereo TO variable CHANNELS
    //     if (!isEnabled() || mSettings.wet <= 0.001f) return;
    //
    //     // 1. Target delay in samples (as float for interpolation)
    //     float targetDelaySamples = (mSettings.time / 1000.0f) * mSampleRate;
    //
    //     for (int i = 0; i < numSamples; i++) {
    //         // 2. Smooth the delay time to prevent clicks
    //         // If you don't want pitch shifting, use a very small factor like 0.0001f
    //         mSmoothedDelaySamples += 0.001f * (targetDelaySamples - mSmoothedDelaySamples);
    //
    //         float dry = buffer[i];
    //         float delayed = 0.0f;
    //
    //         // Determine which buffer and position to use
    //         float* activeBuf = (i % 2 == 0) ? mBufL.data() : mBufR.data();
    //         uint32_t& activePos = (i % 2 == 0) ? mPosL : mPosR;
    //
    //         // 3. Safe Read Position Calculation
    //         // Subtract smoothed samples from current write position
    //         float readPos = static_cast<float>(activePos) - mSmoothedDelaySamples;
    //
    //         // Wrap-around logic (Modulo replacement)
    //         while (readPos < 0.0f) readPos += static_cast<float>(mMaxBufSize);
    //
    //         // 4. Linear Interpolation (Prevents the "staircase" clicking)
    //         int indexA = static_cast<int>(readPos);
    //         int indexB = (indexA + 1) % mMaxBufSize;
    //         float fraction = readPos - static_cast<float>(indexA);
    //
    //         delayed = activeBuf[indexA] + fraction * (activeBuf[indexB] - activeBuf[indexA]);
    //
    //         // 5. Update Buffer (Feedback)
    //         activeBuf[activePos] = dry + (delayed * mSettings.feedback);
    //
    //         // 6. Final Mix and Advance
    //         buffer[i] = (dry * (1.0f - mSettings.wet)) + (delayed * mSettings.wet);
    //
    //         activePos = (activePos + 1) % mMaxBufSize;
    //     }
    // }


#ifdef FLUX_ENGINE
    virtual ImVec4 getColor() const  override { return ImVec4(0.3f, 0.8f, 0.6f, 1.0f);}

    virtual void renderPaddle() override {
        DSP::DelaySettings currentSettings = this->getSettings();
        currentSettings.wet.setKnobSettings(ImFlux::ksBlue); // NOTE only works here !
        if (currentSettings.DrawPaddle(this)) {
            this->setSettings(currentSettings);
        }
    }

    virtual void renderUIWide() override {
        DSP::DelaySettings currentSettings = this->getSettings();
        if (currentSettings.DrawUIWide(this)) {
            this->setSettings(currentSettings);
        }
    }
    virtual void renderUI() override {
        DSP::DelaySettings currentSettings = this->getSettings();
        if (currentSettings.DrawUI(this, 110.f, true)) {
            this->setSettings(currentSettings);
        }
    }
/*


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
    }*/
    #endif

}; //CLASS
}; //namespace
