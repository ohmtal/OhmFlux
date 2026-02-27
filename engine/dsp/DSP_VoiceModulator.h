//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Digital Sound Processing : VoiceModulator
//-----------------------------------------------------------------------------
// * using ISettings
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

struct VoiceData {
    float pitch; // Range: 0.5 (octave down) to 2.0 (octave up). Vader is ~0.8
    float grit; // 0.0 (clean) bis 1.0 (dirty)
    float wet;   // Mix 0.0 to 1.0
};


struct VoiceSettings : public ISettings {
    AudioParam<float> pitch  { "Pitch", 0.78f ,  0.5f, 2.0f, "%.2f"};
    AudioParam<float> grit   { "Grit",  0.85f ,  0.0f, 1.0f, "%.2f"};
    AudioParam<float> wet    { "Mix",   0.30f ,  0.0f, 1.0f, "Wet %.2f"};


    VoiceSettings() = default;
    REGISTER_SETTINGS(VoiceSettings, &pitch, &grit, &wet)

    VoiceData getData() const {
        return { pitch.get(), grit.get(), wet.get()};
    }

    void setData(const VoiceData& data) {
        pitch.set(data.pitch);
        grit.set(data.grit);
        wet.set(data.wet);
    }
    std::vector<std::shared_ptr<IPreset>> getPresets() const override {
        return {
            std::make_shared<Preset<VoiceSettings, VoiceData>>
                ("Custom", VoiceData{ 0.6f, 0.8f, 0.4f }),

            // Deep & menacing with a metallic edge
            std::make_shared<Preset<VoiceSettings, VoiceData>>
                ("Dark Lord", VoiceData{ 0.78f, 0.85f, 0.30f }),

            // Extremely deep and distorted
            std::make_shared<Preset<VoiceSettings, VoiceData>>
                ("Monster", VoiceData{ 0.55f, 0.95f, 0.50f }),

            // High pitched and clear
            std::make_shared<Preset<VoiceSettings, VoiceData>>
                ("Mouse", VoiceData{ 1.60f, 0.75f, 0.05f }),

            // Normal pitch but heavy digital grit
            std::make_shared<Preset<VoiceSettings, VoiceData>>
                ("Robot", VoiceData{ 1.00f, 0.60f, 0.70f }),

        };
    }
};


class VoiceModulator : public DSP::Effect {
private:
    VoiceSettings mSettings;
    const uint32_t mBufSize = 8192; //4096; // Short buffer for pitch shifting

    std::vector<std::vector<float>> mBuffers;
    std::vector<float> mReadPositions;
    uint32_t mWritePos = 0;

public:
    IMPLEMENT_EFF_CLONE(VoiceModulator)

    VoiceModulator(bool switchOn = false) :
        DSP::Effect(DSP::EffectType::VoiceModulator, switchOn)
        , mSettings() {
            mEffectName = "Voice Modulator";
        }
    //----------------------------------------------------------------------
    // virtual std::string getName() const override { return "Voice Modulator"; }
    //----------------------------------------------------------------------
    VoiceSettings& getSettings() { return mSettings; }
    void setSettings(const VoiceSettings& s) { mSettings = s; }
    //----------------------------------------------------------------------
    virtual void reset() override {
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
    //--------------------------------------------------------------------------
    // Process :D
    //--------------------------------------------------------------------------
    virtual void process(float* buffer, int numSamples, int numChannels) override {
        const float wet = mSettings.wet.get();
        if (!isEnabled() || wet <= 0.001f) return;

        if (mBuffers.size() != static_cast<size_t>(numChannels)) {
            mBuffers.resize(numChannels, std::vector<float>(mBufSize, 0.0f));
            mReadPositions.resize(numChannels, 0.0f);
        }


        const uint32_t mask = mBufSize - 1;
        const float halfBuf = mBufSize * 0.5f;
        const float invBuf = 1.0f / (float)mBufSize;

        const float pitch = mSettings.pitch.get();
        const float grit = std::clamp(mSettings.grit.get(), 0.0f, 1.0f);
        const float levels = std::pow(2.0f, std::max(1.0f, 16.0f * (1.0f - grit * 0.9f)));
        const float invLevelsMinus1 = 1.0f / (levels - 1.0f);

        float* channelPtrs[8];
        for (int c = 0; c < numChannels; ++c) channelPtrs[c] = mBuffers[c].data();

        int channel = 0;
        for (int i = 0; i < numSamples; i++) {
            float dry = buffer[i];
            float* buf = channelPtrs[channel];
            float& rp = mReadPositions[channel];

            float p1 = rp;
            float p2 = rp + halfBuf;
            if (p2 >= (float)mBufSize) p2 -= (float)mBufSize;

            float phase01 = p1 * invBuf;
            float fade = 0.5f * (1.0f - DSP::FastMath::fastCos(phase01));

            auto sampleAt = [&](float pos) {
                uint32_t i1 = (uint32_t)pos & mask;
                uint32_t i2 = (i1 + 1) & mask;
                float f = pos - (float)((uint32_t)pos);
                return buf[i1] + f * (buf[i2] - buf[i1]);
            };

            float wet = sampleAt(p1) * (1.0f - fade) + sampleAt(p2) * fade;

            if (grit > 0.01f) {
                float shifted = (wet + 1.0f) * 0.5f;
                wet = (std::floor(shifted * (levels - 1.0f) + 0.5f) * invLevelsMinus1) * 2.0f - 1.0f;
                wet *= (1.0f + grit * 0.2f);
            }

            buffer[i] = (dry * (1.0f - wet)) + (wet * wet);
            buf[mWritePos] = dry;

            if (++channel >= numChannels) {
                channel = 0;
                mWritePos = (mWritePos + 1) & mask;
                for (int c = 0; c < numChannels; ++c) {
                    mReadPositions[c] += pitch;
                    if (mReadPositions[c] >= (float)mBufSize) mReadPositions[c] -= (float)mBufSize;
                    else if (mReadPositions[c] < 0) mReadPositions[c] += (float)mBufSize;
                }
            }
        }
    }
    //--------------------------------------------------------------------------
    #ifdef FLUX_ENGINE
    virtual ImVec4 getDefaultColor() const override { return ImVec4(0.8f, 0.4f, 0.2f, 1.0f); } // Darth Vader Orange/Red

    virtual void renderPaddle() override {
        DSP::VoiceSettings currentSettings = this->getSettings();
        currentSettings.wet.setKnobSettings(ImFlux::ksBlue); // NOTE only works here !
        if (currentSettings.DrawPaddle(this)) {
            this->setSettings(currentSettings);
        }
    }

    virtual void renderUIWide() override {
        DSP::VoiceSettings currentSettings = this->getSettings();
        if (currentSettings.DrawUIWide(this)) {
            this->setSettings(currentSettings);
        }
    }
    virtual void renderUI() override {
        DSP::VoiceSettings currentSettings = this->getSettings();
        if (currentSettings.DrawUI(this, 160.f, true)) {
            this->setSettings(currentSettings);
        }
    }


    // virtual void renderUIWide() override {
    //     ImGui::PushID("VoiceMod_Effect_Row_WIDE");
    //     if (ImGui::BeginChild("VOICEMOD_W_BOX", ImVec2(-FLT_MIN, 65.f))) {
    //
    //         DSP::VoiceSettings currentSettings = this->getSettings();
    //         int currentIdx = 0; // "Custom"
    //         bool changed = false;
    //
    //         ImFlux::GradientBox(ImVec2(-FLT_MIN, -FLT_MIN), 0.f);
    //         ImGui::Dummy(ImVec2(2, 0)); ImGui::SameLine();
    //         ImGui::BeginGroup();
    //
    //         bool isEnabled = this->isEnabled();
    //         if (ImFlux::LEDCheckBox(getName(), &isEnabled, getColor())) {
    //             this->setEnabled(isEnabled);
    //         }
    //
    //         if (!isEnabled) ImGui::BeginDisabled();
    //
    //         ImGui::SameLine();
    //         for (int i = 1; i < DSP::VOICE_PRESETS.size(); ++i) {
    //             if (currentSettings == DSP::VOICE_PRESETS[i]) {
    //                 currentIdx = i;
    //                 break;
    //             }
    //         }
    //
    //
    //         int displayIdx = currentIdx;
    //         ImGui::SameLine(ImGui::GetWindowWidth() - 260.f);
    //
    //         if (ImFlux::ValueStepper("##VoicePreset", &displayIdx, VOICE_PRESET_NAMES,
    //             IM_ARRAYSIZE(VOICE_PRESET_NAMES)))
    //         {
    //             if (displayIdx > 0 && displayIdx < DSP::VOICE_PRESETS.size()) {
    //                 currentSettings = DSP::VOICE_PRESETS[displayIdx];
    //                 changed = true;
    //             }
    //         }
    //
    //         ImGui::SameLine();
    //         if (ImFlux::ButtonFancy("RESET", ImFlux::SLATEDARK_BUTTON.WithSize(ImVec2(40.f, 20.f)))) {
    //             currentSettings = DSP::VADER_VOICE; // Default to Vader!
    //             changed = true;
    //         }
    //
    //         ImGui::Separator();
    //
    //         // Pitch conversion for UI: Factor -> Semitones
    //         // 1.0 -> 0 semitones, 0.5 -> -12 semitones, 2.0 -> +12 semitones
    //         float semitones = log2f(currentSettings.pitch) * 12.0f;
    //
    //         if (ImFlux::MiniKnobF("Pitch", &semitones, -12.0f, 12.0f)) {
    //             // Convert back: Semitones -> Factor
    //             currentSettings.pitch = powf(2.0f, semitones / 12.0f);
    //             changed = true;
    //         }
    //
    //         ImGui::SameLine();
    //         changed |= ImFlux::MiniKnobF("Grit", &currentSettings.grit, 0.0f, 1.0f);ImGui::SameLine();
    //         changed |= ImFlux::MiniKnobF("Mix", &currentSettings.wet, 0.0f, 1.0f);ImGui::SameLine();
    //
    //         if (changed) {
    //
    //             this->setSettings(currentSettings);
    //         }
    //
    //         if (!isEnabled) ImGui::EndDisabled();
    //
    //         ImGui::EndGroup();
    //     }
    //     ImGui::EndChild();
    //     ImGui::PopID();
    // }
    //
    // virtual void renderUI() override {
    //     ImGui::PushID("VoiceMod_Effect_Row");
    //
    //     ImGui::BeginGroup();
    //
    //     bool isEnabled = this->isEnabled();
    //     if (ImFlux::LEDCheckBox(getName(), &isEnabled, getColor())){
    //         this->setEnabled(isEnabled);
    //     }
    //     if (isEnabled)
    //     {
    //         if (ImGui::BeginChild("VoiceModulator_BOX", ImVec2(0, 110), ImGuiChildFlags_Borders)) {
    //             ImGui::BeginGroup();
    //             DSP::VoiceSettings currentSettings = this->getSettings();
    //             bool changed = false;
    //             int currentIdx = 0;
    //             for (int i = 1; i < DSP::VOICE_PRESETS.size(); ++i) {
    //                 if (currentSettings == DSP::VOICE_PRESETS[i]) {
    //                     currentIdx = i;
    //                     break;
    //                 }
    //             }
    //             int displayIdx = currentIdx;  //<< keep currentIdx clean
    //
    //             ImGui::SetNextItemWidth(150);
    //             if (ImFlux::ValueStepper("##Preset", &displayIdx, VOICE_PRESET_NAMES,
    //                     IM_ARRAYSIZE(VOICE_PRESET_NAMES)))
    //             {
    //                 if (displayIdx > 0 && displayIdx < DSP::VOICE_PRESETS.size()) {
    //                     currentSettings =  DSP::VOICE_PRESETS[displayIdx];
    //                     changed = true;
    //                 }
    //             }
    //             ImGui::SameLine(ImGui::GetWindowWidth() - 60); // Right-align reset button
    //             if (ImFlux::FaderButton("Reset", ImVec2(40.f, 20.f)))  {
    //                 currentSettings = DSP::VADER_VOICE; //DEFAULT
    //                 changed = true;
    //             }
    //             ImGui::Separator();
    //
    //             // Pitch conversion for UI: Factor -> Semitones
    //             // 1.0 -> 0 semitones, 0.5 -> -12 semitones, 2.0 -> +12 semitones
    //             float semitones = log2f(currentSettings.pitch) * 12.0f;
    //             if (ImFlux::FaderHWithText("Pitch", &semitones, -12.0f, 12.0f, "%.1f")) {
    //                 // Convert back: Semitones -> Factor
    //                 currentSettings.pitch = powf(2.0f, semitones / 12.0f);
    //                 changed = true;
    //             }
    //             changed |= ImFlux::FaderHWithText("Grit", &currentSettings.grit, 0.0f, 1.0f , "%.2f");
    //             changed |= ImFlux::FaderHWithText("Mix", &currentSettings.wet, 0.01f, 1.0f, "%.2f wet");
    //             // Engine Update
    //             if (changed) {
    //                 if (isEnabled) {
    //                     this->setSettings(currentSettings);
    //                 }
    //             }
    //             ImGui::EndGroup();
    //         }
    //         ImGui::EndChild();
    //     } else {
    //         ImGui::Separator();
    //     }
    //
    //     ImGui::EndGroup();
    //     ImGui::PopID();
    //     ImGui::Spacing(); // Add visual gap before the next effect
    // }

    #endif
}; //CLASS
}; //namespace
