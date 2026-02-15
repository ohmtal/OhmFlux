//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Digital Sound Processing : Reverb
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

struct ReverbSettings {
    float decay;
    int sizeL;
    int sizeR;
    float wet;

    static const uint8_t CURRENT_VERSION = 1;
    void getBinary(std::ostream& os) const {
        uint8_t ver = CURRENT_VERSION;
        DSP_STREAM_TOOLS::write_binary(os, ver);

        DSP_STREAM_TOOLS::write_binary(os, decay);
        DSP_STREAM_TOOLS::write_binary(os, sizeL);
        DSP_STREAM_TOOLS::write_binary(os, sizeR);
        DSP_STREAM_TOOLS::write_binary(os, wet);

    }

    bool  setBinary(std::istream& is) {
        uint8_t fileVersion = 0;
        DSP_STREAM_TOOLS::read_binary(is, fileVersion);
        if (fileVersion != CURRENT_VERSION) return false;
        DSP_STREAM_TOOLS::read_binary(is, decay);
        DSP_STREAM_TOOLS::read_binary(is, sizeL);
        DSP_STREAM_TOOLS::read_binary(is, sizeR);
        DSP_STREAM_TOOLS::read_binary(is, wet);

        return  is.good();
    }

    auto operator<=>(const ReverbSettings&) const = default; //C++20 lazy way

};


//-----
constexpr ReverbSettings CUSTOM_REVERB        = { 0.00f,    0,     0,   0.00f }; // No effect << dummy
//-----
constexpr ReverbSettings HALL_REVERB      = { 0.82f, 17640, 17201,  0.45f }; // Large, lush reflections Concert Hall
constexpr ReverbSettings CAVE_REVERB      = { 0.90f, 30000, 29800,  0.60f }; // Massive, long decay
constexpr ReverbSettings ROOM_REVERB      = { 0.40f,  4000,  3950,  0.25f }; // Short, tight reflections
constexpr ReverbSettings HAUNTED_REVERB   = { 0.88f, 22050, 21500,  0.60f }; // Haunted Corridor


static const char* REVERB_PRESET_NAMES[] = {
    "Custom", "Concert Hall", "Massive Cave", "Small Room", "Haunted Corridor" };

static const std::array<DSP::ReverbSettings, 5> REVERB_PRESETS = {
    CUSTOM_REVERB,
    HALL_REVERB,
    CAVE_REVERB,
    ROOM_REVERB,
    HAUNTED_REVERB

};

class Reverb : public DSP::Effect {
private:

    std::vector<std::vector<float>> mBuffers;
    std::vector<uint32_t> mPositions;
    std::vector<uint32_t> mSizes; // Stores unique sizes per channel

    uint32_t mMaxBufSize;

    ReverbSettings mSettings;

    void updateSizes( int numChannels)
    {
        mSizes.resize(numChannels);
        for (int c = 0; c < numChannels; ++c) {
            float ratio = (numChannels > 1) ? (float)c / (numChannels - 1) : 0.0f;
            mSizes[c] = static_cast<uint32_t>(mSettings.sizeL + ratio * (mSettings.sizeR - mSettings.sizeL));
            if (mSizes[c] > mMaxBufSize) mSizes[c] = mMaxBufSize;
        }
    }


public:
    IMPLEMENT_EFF_CLONE(Reverb)

    Reverb(bool switchOn = false) :
    Effect(DSP::EffectType::Reverb, switchOn)
    {

        mSettings = ROOM_REVERB;

        // mMaxBufSize = SAMPLE_RATE_I;
        // //default stereo
        // mBuffers.assign(2,  std::vector<float>(mMaxBufSize, 0.0f ));
        // mPositions = { 0, 0 };
        // updateSizes(2);

        updateBufferSize(); //should be all do what we need

    }

    const ReverbSettings& getSettings() { return mSettings; }


    virtual void setSampleRate(float sampleRate) override {
        mSampleRate = sampleRate;
        updateBufferSize();

    }

    void updateBufferSize()
    {
        mMaxBufSize = static_cast<uint32_t>(mSampleRate);
        int curChannels = (int)mBuffers.size();
        if (curChannels == 0) curChannels = 2; //default stereo
        mBuffers.assign(curChannels,  std::vector<float>(mMaxBufSize, 0.0f ));
        mPositions.assign(curChannels, 0);
        updateSizes(curChannels);
    }

    void setSettings(const ReverbSettings& s) {
        mSettings = s;
        int curChannels = (int)mBuffers.size();
        mPositions.assign(curChannels, 0);
        updateSizes(curChannels);
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
        return mSettings.decay * 1.5f; // Add a small safety margin
    }


    //----------------------------------------

    virtual void process(float* buffer, int numSamples, int numChannels) override {
        if (!isEnabled() || mSettings.wet <= 0.001f) return;

        if (mBuffers.size() != (size_t)numChannels) {
            updateBufferSize();
        }

        // prefetch channel pointers
        float* channelData[8];
        for (int ch = 0; ch < numChannels; ++ch) channelData[ch] = mBuffers[ch].data();

        const float decay = mSettings.decay;
        const float wet = mSettings.wet;
        const float dryGain = 1.0f - wet;

        //  Loop opimized keyword: int channel = i % numChannels;
        int ch = 0;
        for (int i = 0; i < numSamples; i++) {
            float dry = buffer[i];

            float* activeBuf = channelData[ch];
            uint32_t& activePos = mPositions[ch];
            uint32_t activeSize = mSizes[ch];

            float delayed = activeBuf[activePos];

            activeBuf[activePos] = dry + (delayed * decay);
            buffer[i] = (dry * dryGain) + (delayed * wet);

            if (++activePos >= activeSize) activePos = 0;
            if (++ch >= numChannels) ch = 0;
        }
    }



    // good but there is always a better way ;)
    // virtual void process(float* buffer, int numSamples, int numChannels) override {
    //     if (!isEnabled() || mSettings.wet <= 0.001f) return;
    //
    //     // 1. Initialize resources if channel count changed
    //     if (mBuffers.size() != static_cast<size_t>(numChannels)) {
    //         mBuffers.assign(numChannels, std::vector<float>(mMaxBufSize, 0.0f));
    //         mPositions.assign(numChannels, 0);
    //         updateSizes(numChannels);
    //     }
    //
    //     for (int i = 0; i < numSamples; i++) {
    //         int channel = i % numChannels;
    //         float dry = buffer[i];
    //
    //         // 3. Access resources for the current channel
    //         std::vector<float>& activeBuf = mBuffers[channel];
    //         uint32_t& activePos = mPositions[channel];
    //         uint32_t activeSize = mSizes[channel];
    //
    //         // 4. Reverb logic
    //         float delayed = activeBuf[activePos];
    //
    //         // Feedback loop
    //         activeBuf[activePos] = dry + (delayed * mSettings.decay);
    //
    //         // 5. Wrap position based on this channel's specific size
    //         activePos = (activePos + 1);
    //         if (activePos >= activeSize) {
    //             activePos = 0;
    //         }
    //
    //         // 6. Mix Dry + Wet
    //         buffer[i] = (dry * (1.0f - mSettings.wet)) + (delayed * mSettings.wet);
    //     }
    // }

    // virtual void process(float* buffer, int numSamples, int numChannels) override {
    //     if (numChannels !=  2) { return;  }  //FIXME REWRITE from stereo TO variable CHANNELS
    //
    //     if (!isEnabled()) return;
    //     if (mSettings.wet <= 0.001f) return;
    //
    //     for (int i = 0; i < numSamples; i++) {
    //         float dry = buffer[i];
    //         float delayed;
    //
    //         if (i % 2 == 0) { // Left Channel
    //             delayed = mBufL[mPosL];
    //             // FEEDBACK: High precision float math
    //             mBufL[mPosL] = dry + (delayed * mSettings.decay);
    //             // Wrap position based on current room size
    //             mPosL = (mPosL + 1) % mSettings.sizeL;
    //         } else { // Right Channel
    //             delayed = mBufR[mPosR];
    //             mBufR[mPosR] = dry + (delayed * mSettings.decay);
    //             mPosR = (mPosR + 1) % mSettings.sizeR;
    //         }
    //
    //         // MIX: Linear interpolation between dry and wet
    //         buffer[i] = (dry * (1.0f - mSettings.wet)) + (delayed * mSettings.wet);
    //     }
    // }


    virtual std::string getName() const override { return "REVERB / SPACE";}
#ifdef FLUX_ENGINE
    virtual ImVec4 getColor() const  override { return ImVec4(0.2f, 0.9f, 0.5f, 1.0f);}

    virtual void renderUIWide() override {
        ImGui::PushID("Reverb_Effect_Row_WIDE");
        if (ImGui::BeginChild("REVERB_W_BOX", ImVec2(-FLT_MIN,65.f) )) {
            DSP::ReverbSettings currentSettings = this->getSettings();
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
            for (int i = 1; i < DSP::REVERB_PRESETS.size(); ++i) {
                if (currentSettings == DSP::REVERB_PRESETS[i]) {
                    currentIdx = i;
                    break;
                }
            }
            int displayIdx = currentIdx;  //<< keep currentIdx clean
            ImGui::SameLine(ImGui::GetWindowWidth() - 260.f); // Right-align reset button

            if (ImFlux::ValueStepper("##Preset", &displayIdx, REVERB_PRESET_NAMES
                , IM_ARRAYSIZE(REVERB_PRESET_NAMES)))
            {
                if (displayIdx > 0 && displayIdx < DSP::REVERB_PRESETS.size()) {
                    currentSettings =  DSP::REVERB_PRESETS[displayIdx];
                    changed = true;
                }
            }
            ImGui::SameLine();
            // if (ImFlux::FaderButton("Reset", ImVec2(40.f, 20.f)))  {
            if (ImFlux::ButtonFancy("RESET", ImFlux::SLATEDARK_BUTTON.WithSize(ImVec2(40.f, 20.f)) ))  {
                currentSettings = DSP::ROOM_REVERB; //DEFAULT
                // this->reset();
                changed = true;
            }

            ImGui::Separator();
            // ImFlux::MiniKnobF(label, &value, min_v, max_v);
            changed |= ImFlux::MiniKnobF("Decay", &currentSettings.decay, 0.1f, 0.98f ); ImGui::SameLine();
            float sizeL = (float) currentSettings.sizeL;
            float sizeR = (float) currentSettings.sizeR;
            changed |= ImFlux::MiniKnobF("Size L", &sizeL, 500, 35000); ImGui::SameLine();
            changed |= ImFlux::MiniKnobF("Size R", &sizeR, 500, 35000); ImGui::SameLine();
            currentSettings.sizeL = (int) sizeL;
            currentSettings.sizeR = (int) sizeR;
            changed |= ImFlux::MiniKnobF("Mix", &currentSettings.wet, 0.0f, 1.0f); ImGui::SameLine();
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
    //--------------------------------------------------------------------------

    virtual void renderUI() override {
        ImGui::PushID("Reverb_Effect_Row");
        ImGui::BeginGroup();
        bool isEnabled = this->isEnabled();
        if (ImFlux::LEDCheckBox(getName(), &isEnabled, getColor())) this->setEnabled(isEnabled);
        if (isEnabled)
        {
            if (ImGui::BeginChild("Reverb_Box", ImVec2(0, 140), ImGuiChildFlags_Borders)) {
                DSP::ReverbSettings currentSettings = this->getSettings();
                bool changed = false;
                int currentIdx = 0; // Standard: "Custom"
                for (int i = 1; i < DSP::REVERB_PRESETS.size(); ++i) {
                    if (currentSettings == DSP::REVERB_PRESETS[i]) {
                        currentIdx = i;
                        break;
                    }
                }
                int displayIdx = currentIdx;  //<< keep currentIdx clean
                ImGui::SetNextItemWidth(150);
                if (ImFlux::ValueStepper("##Preset", &displayIdx, DSP::REVERB_PRESET_NAMES, IM_ARRAYSIZE(DSP::REVERB_PRESET_NAMES))) {
                    if (displayIdx > 0 && displayIdx < DSP::REVERB_PRESETS.size()) {
                        currentSettings =  DSP::REVERB_PRESETS[displayIdx];
                        changed = true;
                    }
                }
                ImGui::SameLine(ImGui::GetWindowWidth() - 60);
                if (ImFlux::FaderButton("Reset", ImVec2(40.f, 20.f)))  {
                    currentSettings = DSP::ROOM_REVERB;
                    changed = true;
                }
                ImGui::Separator();
                changed |= ImFlux::FaderHWithText("Decay", &currentSettings.decay, 0.1f, 0.98f, "%.2f");
                float sizeL = (float) currentSettings.sizeL;
                float sizeR = (float) currentSettings.sizeR;
                changed |= ImFlux::FaderHWithText("Size L", &sizeL, 500, 35000, "%5.0f smp");
                changed |= ImFlux::FaderHWithText("Size R", &sizeR, 500, 35000, "%5.0f smp");
                currentSettings.sizeL = (int) sizeL;
                currentSettings.sizeR = (int) sizeR;
                changed |= ImFlux::FaderHWithText("Mix", &currentSettings.wet, 0.0f, 1.0f, "Wet %.2f");

                if (changed) {
                    if (isEnabled) {
                        this->setSettings(currentSettings);
                    }
                }
            }
            ImGui::EndChild();
        } else {
            ImGui::Separator();
        }
        ImGui::EndGroup();
        ImGui::PopID();
        ImGui::Spacing();
    }
#endif
}; //CLASS
}; //namespace
