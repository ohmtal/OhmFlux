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
        DSP_STREAM_TOOLS::write_binary(os, ver);

        DSP_STREAM_TOOLS::write_binary(os, pitch);
        DSP_STREAM_TOOLS::write_binary(os, wet);
        DSP_STREAM_TOOLS::write_binary(os, grit);
    }

    bool  setBinary(std::istream& is) {
        uint8_t fileVersion = 0;
        DSP_STREAM_TOOLS::read_binary(is, fileVersion);
        if (fileVersion != CURRENT_VERSION) return false;
        DSP_STREAM_TOOLS::read_binary(is, pitch);
        DSP_STREAM_TOOLS::read_binary(is, wet);
        DSP_STREAM_TOOLS::read_binary(is, grit);

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
    // std::vector<float> mBufL, mBufR;
    // float mReadPosL = 0.0f,
    // mReadPosR = 0.0f;
    // uint32_t mWritePos = 0;

    const uint32_t mBufSize = 4096; // Short buffer for pitch shifting

    std::vector<std::vector<float>> mBuffers;
    std::vector<float> mReadPositions;
    uint32_t mWritePos = 0;


public:
    IMPLEMENT_EFF_CLONE(VoiceModulator)

    VoiceModulator(bool switchOn = false) :
        DSP::Effect(switchOn) {
        // mBufL.resize(mBufSize, 0.0f);
        // mBufR.resize(mBufSize, 0.0f);
        mSettings = VADER_VOICE;
    }



    virtual DSP::EffectType getType() const override { return DSP::EffectType::VoiceModulator; }
    virtual std::string getName() const override { return "Voice Modulator"; }

    const VoiceSettings& getSettings() { return mSettings; }
    void setSettings(const VoiceSettings& s) { mSettings = s; }

    virtual void reset() override {
        // std::fill(mBufL.begin(), mBufL.end(), 0.0f);
        // std::fill(mBufR.begin(), mBufR.end(), 0.0f);
        // mWritePos = 0;
        // mReadPosL = 0.0f;
        // mReadPosR = 0.0f;
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
    virtual void process(float* buffer, int numSamples, int numChannels) override {
        if (!isEnabled()) return;

        if (mBuffers.size() != static_cast<size_t>(numChannels)) {
            mBuffers.assign(numChannels, std::vector<float>(mBufSize, 0.0f));
            mReadPositions.assign(numChannels, 0.0f);
        }

        float grit = std::clamp(mSettings.grit, 0.0f, 1.0f);
        float levels = std::pow(2.0f, std::max(1.0f, 16.0f * (1.0f - grit * 0.9f)));

        for (int i = 0; i < numSamples; i++) {
            int channel = i % numChannels;
            float dry = buffer[i];

            // 1. Write to buffer
            mBuffers[channel][mWritePos] = dry;

            // 2. Pitch Shifting mit Crossfading
            float& rp = mReadPositions[channel];
            std::vector<float>& buf = mBuffers[channel];

            // Zwei Taps, die 180 Grad phasenverschoben sind
            float p1 = rp;
            float p2 = rp + (mBufSize * 0.5f);
            if (p2 >= mBufSize) p2 -= mBufSize;

            // Hann-Window-ähnlicher Crossfade (sinusoidal ist weicher als linear)
            // Verhindert das "Knacken" an den Loop-Punkten
            float phase = rp / static_cast<float>(mBufSize);
            float fade = 0.5f * (1.0f - std::cos(2.0f * M_PI * phase));

            auto getInterpolated = [&](float pos) {
                int i1 = static_cast<int>(pos);
                int i2 = (i1 + 1) % mBufSize;
                float frac = pos - i1;
                return buf[i1] * (1.0f - frac) + buf[i2] * frac;
            };

            // Crossfade zwischen Tap 1 und Tap 2
            float wet = getInterpolated(p1) * (1.0f - fade) + getInterpolated(p2) * fade;

            // 3. Grit Logic (Bitcrushing)
            if (grit > 0.01f) {
                float shifted = (wet + 1.0f) * 0.5f;
                float quantized = std::round(shifted * (levels - 1.0f)) / (levels - 1.0f);
                wet = (quantized * 2.0f) - 1.0f;
                wet *= (1.0f + grit * 0.2f);
            }

            // 4. Final Mix
            buffer[i] = (dry * (1.0f - mSettings.wet)) + (wet * mSettings.wet);

            // 5. Sync Update: Alle Kanäle im Frame müssen denselben Fortschritt haben
            if (channel == numChannels - 1) {
                mWritePos = (mWritePos + 1) % mBufSize;
                for (int c = 0; c < numChannels; ++c) {
                    mReadPositions[c] += mSettings.pitch;
                    if (mReadPositions[c] >= mBufSize) mReadPositions[c] -= mBufSize;
                    if (mReadPositions[c] < 0) mReadPositions[c] += mBufSize;
                }
            }
        }
    }

    // virtual void processOLD(float* buffer, int numSamples, int numChannels) override {
    //     if (numChannels !=  2) { return;  }  //FIXME REWRITE from stereo TO variable CHANNELS
    //     if (!isEnabled()) return;
    //
    //     for (int i = 0; i < numSamples; i += 2) {
    //         float dryL = buffer[i];
    //         float dryR = buffer[i + 1];
    //
    //         // 1. Store input in ring buffer
    //         mBufL[mWritePos] = dryL;
    //         mBufR[mWritePos] = dryR;
    //
    //         // 2. Dual-Tap Pitch Shifter with crossfading to prevent clicks
    //         auto processChannel = [&](std::vector<float>& buf, float& readPos) {
    //             int p1 = static_cast<int>(readPos);
    //             int p2 = (p1 + mBufSize / 2) % mBufSize;
    //
    //             // Linear crossfade window
    //             float fade = std::abs((readPos / static_cast<float>(mBufSize)) * 2.0f - 1.0f);
    //             float sample = buf[p1] * fade + buf[p2] * (1.0f - fade);
    //
    //             // Increment read pointer by pitch factor
    //             readPos += mSettings.pitch;
    //             while (readPos >= mBufSize) readPos -= mBufSize;
    //
    //             return sample;
    //         };
    //
    //         float outL = processChannel(mBufL, mReadPosL);
    //         float outR = processChannel(mBufR, mReadPosR);
    //
    //         // 3. Apply Grit (Bitcrushing)
    //         if (mSettings.grit > 0.01f) {
    //             // Reduce bit depth based on grit setting (16-bit down to approx 2-bit)
    //             float steps = std::pow(2.0f, 16.0f * (1.0f - mSettings.grit * 0.8f));
    //             if (steps < 2.0f) steps = 2.0f;
    //
    //             outL = std::floor(outL * steps) / steps;
    //             outR = std::floor(outR * steps) / steps;
    //
    //             // Slight volume compensation for grit harshness
    //             outL *= (1.0f + mSettings.grit * 0.1f);
    //             outR *= (1.0f + mSettings.grit * 0.1f);
    //         }
    //
    //         // 4. Final Dry/Wet Mix
    //         buffer[i]     = (dryL * (1.0f - mSettings.wet)) + (outL * mSettings.wet);
    //         buffer[i + 1] = (dryR * (1.0f - mSettings.wet)) + (outR * mSettings.wet);
    //
    //         // Advance write pointer
    //         mWritePos = (mWritePos + 1) % mBufSize;
    //     }
    // }
    //
    //--------------------------------------------------------------------------
    #ifdef FLUX_ENGINE
    virtual ImVec4 getColor() const override { return ImVec4(0.8f, 0.4f, 0.2f, 1.0f); } // Darth Vader Orange/Red

    virtual void renderUIWide() override {
        ImGui::PushID("VoiceMod_Effect_Row_WIDE");
        if (ImGui::BeginChild("VOICEMOD_W_BOX", ImVec2(-FLT_MIN, 65.f))) {

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

    virtual void renderUI() override {
        ImGui::PushID("VoiceMod_Effect_Row");

        ImGui::BeginGroup();

        bool isEnabled = this->isEnabled();
        if (ImFlux::LEDCheckBox(getName(), &isEnabled, getColor())){
            this->setEnabled(isEnabled);
        }
        if (isEnabled)
        {
            if (ImGui::BeginChild("VoiceModulator_BOX", ImVec2(0, 110), ImGuiChildFlags_Borders)) {
                ImGui::BeginGroup();
                DSP::VoiceSettings currentSettings = this->getSettings();
                bool changed = false;
                int currentIdx = 0;
                for (int i = 1; i < DSP::VOICE_PRESETS.size(); ++i) {
                    if (currentSettings == DSP::VOICE_PRESETS[i]) {
                        currentIdx = i;
                        break;
                    }
                }
                int displayIdx = currentIdx;  //<< keep currentIdx clean

                ImGui::SetNextItemWidth(150);
                if (ImFlux::ValueStepper("##Preset", &displayIdx, VOICE_PRESET_NAMES,
                        IM_ARRAYSIZE(VOICE_PRESET_NAMES)))
                {
                    if (displayIdx > 0 && displayIdx < DSP::VOICE_PRESETS.size()) {
                        currentSettings =  DSP::VOICE_PRESETS[displayIdx];
                        changed = true;
                    }
                }
                ImGui::SameLine(ImGui::GetWindowWidth() - 60); // Right-align reset button
                if (ImFlux::FaderButton("Reset", ImVec2(40.f, 20.f)))  {
                    currentSettings = DSP::VADER_VOICE; //DEFAULT
                    changed = true;
                }
                ImGui::Separator();

                // Pitch conversion for UI: Factor -> Semitones
                // 1.0 -> 0 semitones, 0.5 -> -12 semitones, 2.0 -> +12 semitones
                float semitones = log2f(currentSettings.pitch) * 12.0f;
                if (ImFlux::FaderHWithText("Pitch", &semitones, -12.0f, 12.0f, "%.1f")) {
                    // Convert back: Semitones -> Factor
                    currentSettings.pitch = powf(2.0f, semitones / 12.0f);
                    changed = true;
                }
                changed |= ImFlux::FaderHWithText("Grit", &currentSettings.grit, 0.0f, 1.0f , "%.2f");
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
