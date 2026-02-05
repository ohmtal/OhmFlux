//-----------------------------------------------------------------------------
// Copyright (c) 2026 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Digital Sound Processing : VoiceModulator
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


struct VoiceSettings {
    float pitch; // Range: 0.5 (octave down) to 2.0 (octave up). Vader is ~0.8
    float wet;   // Mix 0.0 to 1.0
    float grit; // 0.0 (clean) bis 1.0 (dirty)

    static const uint8_t CURRENT_VERSION = 1;

    void getBinary(std::ostream& os) const {
        uint8_t ver = CURRENT_VERSION;
        os.write(reinterpret_cast<const char*>(&ver), sizeof(ver));
        os.write(reinterpret_cast<const char*>(this), sizeof(VoiceSettings));
    }

    bool  setBinary(std::istream& is) {
        uint8_t fileVersion = 0;
        is.read(reinterpret_cast<char*>(&fileVersion), sizeof(fileVersion));
        if (fileVersion != CURRENT_VERSION) //Something is wrong !
            return false;
        is.read(reinterpret_cast<char*>(this), sizeof(VoiceSettings));
        return  is.good();
    }

    auto operator<=>(const VoiceSettings&) const = default; //C++20 lazy way

};


// Pitch 1.0f = Normal, 0.5f = Octave Down, 2.0f = Octave Up
// Preset definitions { pitch, wet, grit }
constexpr VoiceSettings VADER_VOICE    = { 0.78f, 0.85f, 0.30f }; // Deep & menacing with a metallic edge
constexpr VoiceSettings MONSTER_VOICE  = { 0.55f, 0.95f, 0.50f }; // Extremely deep and distorted
constexpr VoiceSettings CHIPMUNK_VOICE = { 1.60f, 0.75f, 0.05f }; // High pitched and clear
constexpr VoiceSettings ROBOT_VOICE    = { 1.00f, 0.60f, 0.70f }; // Normal pitch but heavy digital grit
constexpr VoiceSettings CUSTOM_VOICE   = VADER_VOICE;

static const char* VOICE_PRESET_NAMES[] = { "Custom", "Vader", "Monster", "Chipmunk", "Robot" };

static const std::array<DSP::VoiceSettings, 5> VOICE_PRESETS = {
    CUSTOM_VOICE,
    VADER_VOICE,
    MONSTER_VOICE,
    CHIPMUNK_VOICE,
    ROBOT_VOICE
};




class VoiceModulator : public DSP::Effect {
private:
    VoiceSettings mSettings = VADER_VOICE;
    std::vector<float> mBufL, mBufR;
    float mReadPosL = 0.0f,
    mReadPosR = 0.0f;
    uint32_t mWritePos = 0;
    const uint32_t mBufSize = 4096; // Short buffer for pitch shifting

public:
    VoiceModulator(bool switchOn = false) :
        DSP::Effect(switchOn) {
        mBufL.resize(mBufSize, 0.0f);
        mBufR.resize(mBufSize, 0.0f);
        mSettings = VADER_VOICE;
    }



    virtual DSP::EffectType getType() const override { return DSP::EffectType::VoiceModulator; }
    virtual std::string getName() const override { return "Voice Modulator"; }

    const VoiceSettings& getSettings() { return mSettings; }
    void setSettings(const VoiceSettings& s) { mSettings = s; }

    virtual void reset() override {
        std::fill(mBufL.begin(), mBufL.end(), 0.0f);
        std::fill(mBufR.begin(), mBufR.end(), 0.0f);
        mWritePos = 0;
        mReadPosL = 0.0f;
        mReadPosR = 0.0f;
    }
    void save(std::ostream& os) const override {
        Effect::save(os);              // Save mEnabled
        mSettings.getBinary(os);       // Save Settings
    }

    bool load(std::istream& is) override {
        if (!Effect::load(is)) return false; // Load mEnabled
        return mSettings.setBinary(is);      // Load Settings
    }



    //--------------------------------------------------------------------------
    virtual void process(float* buffer, int numSamples) override {
        if (!isEnabled()) return;

        for (int i = 0; i < numSamples; i += 2) {
            float dryL = buffer[i];
            float dryR = buffer[i + 1];

            // 1. Store input in ring buffer
            mBufL[mWritePos] = dryL;
            mBufR[mWritePos] = dryR;

            // 2. Dual-Tap Pitch Shifter with crossfading to prevent clicks
            auto processChannel = [&](std::vector<float>& buf, float& readPos) {
                int p1 = static_cast<int>(readPos);
                int p2 = (p1 + mBufSize / 2) % mBufSize;

                // Linear crossfade window
                float fade = std::abs((readPos / static_cast<float>(mBufSize)) * 2.0f - 1.0f);
                float sample = buf[p1] * fade + buf[p2] * (1.0f - fade);

                // Increment read pointer by pitch factor
                readPos += mSettings.pitch;
                while (readPos >= mBufSize) readPos -= mBufSize;

                return sample;
            };

            float outL = processChannel(mBufL, mReadPosL);
            float outR = processChannel(mBufR, mReadPosR);

            // 3. Apply Grit (Bitcrushing)
            if (mSettings.grit > 0.01f) {
                // Reduce bit depth based on grit setting (16-bit down to approx 2-bit)
                float steps = std::pow(2.0f, 16.0f * (1.0f - mSettings.grit * 0.8f));
                if (steps < 2.0f) steps = 2.0f;

                outL = std::floor(outL * steps) / steps;
                outR = std::floor(outR * steps) / steps;

                // Slight volume compensation for grit harshness
                outL *= (1.0f + mSettings.grit * 0.1f);
                outR *= (1.0f + mSettings.grit * 0.1f);
            }

            // 4. Final Dry/Wet Mix
            buffer[i]     = (dryL * (1.0f - mSettings.wet)) + (outL * mSettings.wet);
            buffer[i + 1] = (dryR * (1.0f - mSettings.wet)) + (outR * mSettings.wet);

            // Advance write pointer
            mWritePos = (mWritePos + 1) % mBufSize;
        }
    }

    // virtual void process(float* buffer, int numSamples) override {
    //     if (!isEnabled()) return;
    //
    //     for (int i = 0; i < numSamples; i += 2) {
    //         float dryL = buffer[i];
    //         float dryR = buffer[i+1];
    //
    //         // 1. Write to ring buffer
    //         mBufL[mWritePos] = dryL;
    //         mBufR[mWritePos] = dryR;
    //
    //         // 2. Dual-Tap Pitch Shifting Logic (to prevent clicks)
    //         // We read at a different speed (mSettings.pitch)
    //         auto processChannel = [&](std::vector<float>& buf, float& readPos) {
    //             int p1 = (int)readPos;
    //             int p2 = (p1 + mBufSize / 2) % mBufSize;
    //
    //             // Triangular crossfade window
    //             float fade = abs((readPos / (float)mBufSize) * 2.0f - 1.0f);
    //
    //             float sample = buf[p1] * fade + buf[p2] * (1.0f - fade);
    //
    //             readPos += mSettings.pitch;
    //             while (readPos >= mBufSize) readPos -= mBufSize;
    //             return sample;
    //         };
    //
    //         float outL = processChannel(mBufL, mReadPosL);
    //         float outR = processChannel(mBufR, mReadPosR);
    //
    //         if (mSettings.grit > 0.01f) {
    //             float steps = powf(2.0f, 16.0f * (1.0f - mSettings.grit * 0.7f));
    //             if (steps < 2.0f) steps = 2.0f;
    //
    //             outL = std::floor(outL * steps) / steps;
    //             outR = std::floor(outR * steps) / steps;
    //
    //             outL *= (1.0f + mSettings.grit * 0.2f);
    //             outR *= (1.0f + mSettings.grit * 0.2f);
    //         }
    //
    //         // 3. Final Mix
    //         buffer[i]   = (dryL * (1.0f - mSettings.wet)) + (outL * mSettings.wet);
    //         buffer[i+1] = (dryR * (1.0f - mSettings.wet)) + (outR * mSettings.wet);
    //
    //         mWritePos = (mWritePos + 1) % mBufSize;
    //     }
    // }

    //--------------------------------------------------------------------------
    #ifdef FLUX_ENGINE
    virtual ImVec4 getColor() const override { return ImVec4(0.8f, 0.4f, 0.2f, 1.0f); } // Darth Vader Orange/Red

    virtual void renderUIWide() override {
        ImGui::PushID("VoiceMod_Effect_Row_WIDE");
        if (ImGui::BeginChild("VOICEMOD_BOX", ImVec2(-FLT_MIN, 65.f))) {

            DSP::VoiceSettings currentSettings = this->getSettings();
            int currentIdx = 0; // "Custom"
            bool changed = false;

            ImFlux::GradientBox(ImVec2(-FLT_MIN, -FLT_MIN), 0.f);
            ImGui::Dummy(ImVec2(2, 0)); ImGui::SameLine();
            ImGui::BeginGroup();

            bool isEnabled = this->isEnabled();
            if (ImFlux::LEDCheckBox(getName(), &isEnabled, getColor())) {
                this->setEnabled(isEnabled);
            }

            if (!isEnabled) ImGui::BeginDisabled();

            ImGui::SameLine();

            // Preset Detection
            // for (int i = 1; i < DSP::VOICE_PRESETS.size(); ++i) {
            //     // Approximate check for floats
            //     if (std::abs(currentSettings.pitch - DSP::VOICE_PRESETS[i].pitch) < 0.01f) {
            //         currentIdx = i;
            //         break;
            //     }
            // }
            for (int i = 1; i < DSP::VOICE_PRESETS.size(); ++i) {
                if (currentSettings == DSP::VOICE_PRESETS[i]) {
                    currentIdx = i;
                    break;
                }
            }


            int displayIdx = currentIdx;
            ImGui::SameLine(ImGui::GetWindowWidth() - 260.f);

            if (ImFlux::ValueStepper("##VoicePreset", &displayIdx, VOICE_PRESET_NAMES,
                IM_ARRAYSIZE(VOICE_PRESET_NAMES)))
            {
                if (displayIdx > 0 && displayIdx < DSP::VOICE_PRESETS.size()) {
                    currentSettings = DSP::VOICE_PRESETS[displayIdx];
                    changed = true;
                }
            }

            ImGui::SameLine();
            if (ImFlux::ButtonFancy("RESET", ImFlux::SLATEDARK_BUTTON.WithSize(ImVec2(40.f, 20.f)))) {
                currentSettings = DSP::VADER_VOICE; // Default to Vader!
                changed = true;
            }

            ImGui::Separator();

            // Pitch conversion for UI: Factor -> Semitones
            // 1.0 -> 0 semitones, 0.5 -> -12 semitones, 2.0 -> +12 semitones
            float semitones = log2f(currentSettings.pitch) * 12.0f;

            if (ImFlux::MiniKnobF("Pitch", &semitones, -12.0f, 12.0f)) {
                // Convert back: Semitones -> Factor
                currentSettings.pitch = powf(2.0f, semitones / 12.0f);
                changed = true;
            }

            ImGui::SameLine();
            changed |= ImFlux::MiniKnobF("Grit", &currentSettings.grit, 0.0f, 1.0f);ImGui::SameLine();
            changed |= ImFlux::MiniKnobF("Mix", &currentSettings.wet, 0.0f, 1.0f);ImGui::SameLine();

            if (changed) {

                this->setSettings(currentSettings);
            }

            if (!isEnabled) ImGui::EndDisabled();

            ImGui::EndGroup();
        }
        ImGui::EndChild();
        ImGui::PopID();
    }
    #endif
}; //CLASS
}; //namespace
