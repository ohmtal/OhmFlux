//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Digital Sound Processing : Reverb
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

    struct ReverbData {
        float decay;
        int sizeL;
        int sizeR;
        float wet;
    };

    struct ReverbSettings : public ISettings {
        AudioParam<float> decay        { "Decay", 0.40f ,  0.1f, 0.98f, "%.2f"};
        AudioParam<int>   sizeL        { "Size L", 4000,  500, 35000, "%.0f smp"};
        AudioParam<int>   sizeR        { "Size R", 3950,  500, 35000, "%.0f smp"};
        AudioParam<float> wet          { "Mix",   0.25f ,  0.0f, 1.0f, "Wet %.2f"};



        ReverbSettings() = default;
        REGISTER_SETTINGS(ReverbSettings, &decay,&sizeL, &sizeR, &wet)

        ReverbData getData() const {
            return { decay.get(), sizeL.get(), sizeR.get(), wet.get()};
        }

        void setData(const ReverbData& data) {
            decay.set(data.decay);
            sizeL.set(data.sizeL);
            sizeR.set(data.sizeR);
            wet.set(data.wet);
        }
        std::vector<std::shared_ptr<IPreset>> getPresets() const override {
            return {
                std::make_shared<Preset<ReverbSettings, ReverbData>>
                    ("Custom", ReverbData{0.0f, 0, 0, 0.0f }),

                // Large, lush reflections Concert Hall
                std::make_shared<Preset<ReverbSettings, ReverbData>>
                    ("Concert Hall", ReverbData{ 0.82f, 17640, 17201,  0.45f }),

                // Massive, long decay
                std::make_shared<Preset<ReverbSettings, ReverbData>>
                    ("Massive Cave", ReverbData{ 0.90f, 30000, 29800,  0.60f }),

                // Short, tight reflections
                std::make_shared<Preset<ReverbSettings, ReverbData>>
                    ("Small Room", ReverbData{ 0.40f,  4000,  3950,  0.25f }),

                // Haunted Corridor
                std::make_shared<Preset<ReverbSettings, ReverbData>>
                    ("Haunted Corridor", ReverbData{ 0.88f, 22050, 21500,  0.60f }),
            };
        }
    };


    constexpr ReverbData DEFAULT_REVERB_DATA = { 0.40f,  4000,  3950,  0.25f };

class Reverb : public DSP::Effect {
private:
    std::vector<std::vector<float>> mBuffers;
    std::vector<uint32_t> mPositions;
    std::vector<uint32_t> mSizes; // Stores unique sizes per channel
    uint32_t mMaxBufSize;
    ReverbSettings mSettings;

    void updateSizes( int numChannels)
    {
        int sizeL = mSettings.sizeL.get();
        int sizeR = mSettings.sizeR.get();

        mSizes.resize(numChannels);
        for (int c = 0; c < numChannels; ++c) {
            float ratio = (numChannels > 1) ? (float)c / (numChannels - 1) : 0.0f;
            mSizes[c] = static_cast<uint32_t>(sizeL + ratio * (sizeR - sizeL));
            if (mSizes[c] > mMaxBufSize) mSizes[c] = mMaxBufSize;
        }
    }

public:
    IMPLEMENT_EFF_CLONE(Reverb)

    Reverb(bool switchOn = false) :
        Effect(DSP::EffectType::Reverb, switchOn)
        , mSettings()
    {
        updateBufferSize(); //should be all do what we need
    }
    //----------------------------------------------------------------------
    virtual std::string getName() const override { return "REVERB / SPACE";}
    //----------------------------------------------------------------------
    ReverbSettings& getSettings() { return mSettings; }
    //----------------------------------------------------------------------
    void setSettings(const ReverbSettings& s) {
        mSettings = s;
        int curChannels = (int)mBuffers.size();
        mPositions.assign(curChannels, 0);
        updateSizes(curChannels);
    }
    //----------------------------------------------------------------------
    virtual void setSampleRate(float sampleRate) override {
        mSampleRate = sampleRate;
        updateBufferSize();

    }
    //----------------------------------------------------------------------
    void updateBufferSize()
    {
        mMaxBufSize = static_cast<uint32_t>(mSampleRate);
        int curChannels = (int)mBuffers.size();
        if (curChannels == 0) curChannels = 2; //default stereo
        mBuffers.resize(curChannels,  std::vector<float>(mMaxBufSize, 0.0f ));
        mPositions.resize(curChannels, 0);
        updateSizes(curChannels);
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
        return mSettings.decay.get() * 1.5f; // Add a small safety margin
    }
    //----------------------------------------------------------------------
    // process
    //----------------------------------------------------------------------
    virtual void process(float* buffer, int numSamples, int numChannels) override {
        float wet = mSettings.wet.get();
        if (!isEnabled() || wet <= 0.001f) return;

        float decay = mSettings.decay.get();
        const float dryGain = 1.0f - wet;

        if (mBuffers.size() != (size_t)numChannels) {
            updateBufferSize();
        }

        // prefetch channel pointers
        float* channelData[8];
        for (int ch = 0; ch < numChannels; ++ch) channelData[ch] = mBuffers[ch].data();


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
    //----------------------------------------------------------------------


#ifdef FLUX_ENGINE
    virtual ImVec4 getColor() const  override { return ImVec4(0.2f, 0.9f, 0.5f, 1.0f);}

    virtual void renderPaddle() override {
        DSP::ReverbSettings currentSettings = this->getSettings();
        currentSettings.wet.setKnobSettings(ImFlux::ksBlue); // NOTE only works here !
        if (currentSettings.DrawPaddle(this)) {
            this->setSettings(currentSettings);
        }
    }

    virtual void renderUIWide() override {
        DSP::ReverbSettings currentSettings = this->getSettings();
        if (currentSettings.DrawUIWide(this)) {
            this->setSettings(currentSettings);
        }
    }
    virtual void renderUI() override {
        DSP::ReverbSettings currentSettings = this->getSettings();
        if (currentSettings.DrawUI(this, 140.f, true)) {
            this->setSettings(currentSettings);
        }
    }


    // virtual void renderUIWide() override {
    //     ImGui::PushID("Reverb_Effect_Row_WIDE");
    //     if (ImGui::BeginChild("REVERB_W_BOX", ImVec2(-FLT_MIN,65.f) )) {
    //         DSP::ReverbSettings currentSettings = this->getSettings();
    //         int currentIdx = 0; // Standard: "Custom"
    //         bool changed = false;
    //
    //         ImFlux::GradientBox(ImVec2(-FLT_MIN, -FLT_MIN),0.f);
    //         ImGui::Dummy(ImVec2(2,0)); ImGui::SameLine();
    //         ImGui::BeginGroup();
    //         bool isEnabled = this->isEnabled();
    //         if (ImFlux::LEDCheckBox(getName(), &isEnabled, getColor())){
    //             this->setEnabled(isEnabled);
    //         }
    //         if (!isEnabled) ImGui::BeginDisabled();
    //         ImGui::SameLine();
    //         // -------- stepper >>>>
    //         for (int i = 1; i < DSP::REVERB_PRESETS.size(); ++i) {
    //             if (currentSettings == DSP::REVERB_PRESETS[i]) {
    //                 currentIdx = i;
    //                 break;
    //             }
    //         }
    //         int displayIdx = currentIdx;  //<< keep currentIdx clean
    //         ImGui::SameLine(ImGui::GetWindowWidth() - 260.f); // Right-align reset button
    //
    //         if (ImFlux::ValueStepper("##Preset", &displayIdx, REVERB_PRESET_NAMES
    //             , IM_ARRAYSIZE(REVERB_PRESET_NAMES)))
    //         {
    //             if (displayIdx > 0 && displayIdx < DSP::REVERB_PRESETS.size()) {
    //                 currentSettings =  DSP::REVERB_PRESETS[displayIdx];
    //                 changed = true;
    //             }
    //         }
    //         ImGui::SameLine();
    //         // if (ImFlux::FaderButton("Reset", ImVec2(40.f, 20.f)))  {
    //         if (ImFlux::ButtonFancy("RESET", ImFlux::SLATEDARK_BUTTON.WithSize(ImVec2(40.f, 20.f)) ))  {
    //             currentSettings = DSP::ROOM_REVERB; //DEFAULT
    //             // this->reset();
    //             changed = true;
    //         }
    //
    //         ImGui::Separator();
    //         // ImFlux::MiniKnobF(label, &value, min_v, max_v);
    //         changed |= ImFlux::MiniKnobF("Decay", &currentSettings.decay, 0.1f, 0.98f ); ImGui::SameLine();
    //         float sizeL = (float) currentSettings.sizeL;
    //         float sizeR = (float) currentSettings.sizeR;
    //         changed |= ImFlux::MiniKnobF("Size L", &sizeL, 500, 35000); ImGui::SameLine();
    //         changed |= ImFlux::MiniKnobF("Size R", &sizeR, 500, 35000); ImGui::SameLine();
    //         currentSettings.sizeL = (int) sizeL;
    //         currentSettings.sizeR = (int) sizeR;
    //         changed |= ImFlux::MiniKnobF("Mix", &currentSettings.wet, 0.0f, 1.0f); ImGui::SameLine();
    //         // Engine Update
    //         if (changed) {
    //             if (isEnabled) {
    //                 this->setSettings(currentSettings);
    //             }
    //         }
    //
    //         if (!isEnabled) ImGui::EndDisabled();
    //
    //         ImGui::EndGroup();
    //     }
    //     ImGui::EndChild();
    //     ImGui::PopID();
    //
    // }
    // //--------------------------------------------------------------------------
    //
    // virtual void renderUI() override {
    //     ImGui::PushID("Reverb_Effect_Row");
    //     ImGui::BeginGroup();
    //     bool isEnabled = this->isEnabled();
    //     if (ImFlux::LEDCheckBox(getName(), &isEnabled, getColor())) this->setEnabled(isEnabled);
    //     if (isEnabled)
    //     {
    //         if (ImGui::BeginChild("Reverb_Box", ImVec2(0, 140), ImGuiChildFlags_Borders)) {
    //             DSP::ReverbSettings currentSettings = this->getSettings();
    //             bool changed = false;
    //             int currentIdx = 0; // Standard: "Custom"
    //             for (int i = 1; i < DSP::REVERB_PRESETS.size(); ++i) {
    //                 if (currentSettings == DSP::REVERB_PRESETS[i]) {
    //                     currentIdx = i;
    //                     break;
    //                 }
    //             }
    //             int displayIdx = currentIdx;  //<< keep currentIdx clean
    //             ImGui::SetNextItemWidth(150);
    //             if (ImFlux::ValueStepper("##Preset", &displayIdx, DSP::REVERB_PRESET_NAMES, IM_ARRAYSIZE(DSP::REVERB_PRESET_NAMES))) {
    //                 if (displayIdx > 0 && displayIdx < DSP::REVERB_PRESETS.size()) {
    //                     currentSettings =  DSP::REVERB_PRESETS[displayIdx];
    //                     changed = true;
    //                 }
    //             }
    //             ImGui::SameLine(ImGui::GetWindowWidth() - 60);
    //             if (ImFlux::FaderButton("Reset", ImVec2(40.f, 20.f)))  {
    //                 currentSettings = DSP::ROOM_REVERB;
    //                 changed = true;
    //             }
    //             ImGui::Separator();
    //             changed |= ImFlux::FaderHWithText("Decay", &currentSettings.decay, 0.1f, 0.98f, "%.2f");
    //             float sizeL = (float) currentSettings.sizeL;
    //             float sizeR = (float) currentSettings.sizeR;
    //             changed |= ImFlux::FaderHWithText("Size L", &sizeL, 500, 35000, "%5.0f smp");
    //             changed |= ImFlux::FaderHWithText("Size R", &sizeR, 500, 35000, "%5.0f smp");
    //             currentSettings.sizeL = (int) sizeL;
    //             currentSettings.sizeR = (int) sizeR;
    //             changed |= ImFlux::FaderHWithText("Mix", &currentSettings.wet, 0.0f, 1.0f, "Wet %.2f");
    //
    //             if (changed) {
    //                 if (isEnabled) {
    //                     this->setSettings(currentSettings);
    //                 }
    //             }
    //         }
    //         ImGui::EndChild();
    //     } else {
    //         ImGui::Separator();
    //     }
    //     ImGui::EndGroup();
    //     ImGui::PopID();
    //     ImGui::Spacing();
    // }
#endif
}; //CLASS
}; //namespace
