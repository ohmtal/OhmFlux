//-----------------------------------------------------------------------------
// Copyright (c) 2026 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Digital Sound Processing : Chorus
//-----------------------------------------------------------------------------
#pragma once

#include <cmath>
#include <vector>

#ifdef FLUX_ENGINE
#include <imgui.h>
#include <imgui_internal.h>
#include <gui/ImFlux.h>
#endif


#include "DSP_Effect.h"


namespace DSP {

    struct ChorusSettings {
        float rate;      // Speed of wiggle (0.1 - 2.0 Hz)
        float depth;     // Intensity (0.001 - 0.005)
        float delayBase; // Offset (0.01 - 0.03)
        float wet;       // Mix (0.0 - 0.5)
        float phaseOffset; // NEW: 0.0 to 1.0 (Phase shift between ears)

        static const uint8_t CURRENT_VERSION = 1;
        void getBinary(std::ostream& os) const {
            uint8_t ver = CURRENT_VERSION;
            os.write(reinterpret_cast<const char*>(&ver), sizeof(ver));
            os.write(reinterpret_cast<const char*>(this), sizeof(ChorusSettings));
        }

        bool  setBinary(std::istream& is) {
            uint8_t fileVersion = 0;
            is.read(reinterpret_cast<char*>(&fileVersion), sizeof(fileVersion));
            if (fileVersion != CURRENT_VERSION) //Something is wrong !
                return false;
            is.read(reinterpret_cast<char*>(this), sizeof(ChorusSettings));
            return  is.good();
        }
        auto operator<=>(const ChorusSettings&) const = default; //C++20 lazy way

    };


    // OFF
    constexpr ChorusSettings CUSTOM_CHORUS           = { 0.0f,  0.0f,   0.0f,   0.0f,  0.0f };

    // Lush 80s: Standard wide stereo chorus
    constexpr ChorusSettings LUSH80s_CHORUS       = { 0.4f,  0.003f, 0.025f, 0.4f,  0.25f };

    // Deep Ensemble: Very slow, very wide, thickens pads
    constexpr ChorusSettings DEEPENSEMPLE_CHORUS  = { 0.1f,  0.005f, 0.040f, 0.5f,  0.50f };

    // Fast Leslie: Simulates a rotating speaker
    constexpr ChorusSettings FASTLESLIE_CHORUS    = { 2.5f,  0.002f, 0.010f, 0.3f,  0.15f };

    // Juno-60 Style: Famous thick BBD chorus (Fast rate, high depth)
    constexpr ChorusSettings JUNO60_CHORUS        = { 0.9f,  0.004f, 0.015f, 0.5f,  0.20f };

    // Vibrato: 100% Wet so you only hear the pitch wiggle
    constexpr ChorusSettings VIBRATO_CHORUS       = { 1.5f,  0.002f, 0.010f, 1.0f,  0.00f };

    // Flanger: Very short delay creates the "jet plane" comb filter effect
    constexpr ChorusSettings FLANGER_CHORUS       = { 0.2f,  0.001f, 0.003f, 0.5f,  0.10f };


    static const char* CHORUS_PRESET_NAMES[] = {
        "Custom", "Lush 80s", "Deep Ensemble", "Fast Leslie",
        "Juno-60 Style", "Vibrato", "Flanger" };

    static const std::array<DSP::ChorusSettings, 7> CHORUS_PRESETS = {
        CUSTOM_CHORUS,
        LUSH80s_CHORUS,
        DEEPENSEMPLE_CHORUS,
        FASTLESLIE_CHORUS,
        JUNO60_CHORUS,
        VIBRATO_CHORUS,
        FLANGER_CHORUS
    };



    class Chorus : public Effect {
    private:
        // Two separate buffers for true stereo de-correlation
        std::vector<float> mDelayBufL;
        std::vector<float> mDelayBufR;

        int mWritePos = 0;
        float mLfoPhase = 0.0f;
        const float mSampleRate = getSampleRateF();
        const int mMaxBufferSize = static_cast<int>(mSampleRate / 10); // 100ms at 44.1kHz

        ChorusSettings mSettings;

    public:
        Chorus(bool switchOn = false) :
            Effect(switchOn)
        {
            mDelayBufL.assign(mMaxBufferSize, 0.0f);
            mDelayBufR.assign(mMaxBufferSize, 0.0f);
            mSettings = LUSH80s_CHORUS;
        }




        const ChorusSettings& getSettings() { return mSettings; }

        void setSettings(const ChorusSettings& s) {
            mSettings = s;
        }

        DSP::EffectType getType() const override { return DSP::EffectType::Chorus; }
        void save(std::ostream& os) const override {
            Effect::save(os);              // Save mEnabled
            mSettings.getBinary(os);       // Save Settings
        }

        bool load(std::istream& is) override {
            if (!Effect::load(is)) return false; // Load mEnabled
            return mSettings.setBinary(is);      // Load Settings
        }

        virtual void process(float* buffer, int numSamples, int numChannels) override {
            if (numChannels !=  2) { return;  }  //FIXME REWRITE from stereo TO variable CHANNELS

            if (!isEnabled()) return;

            // Skip if wet mix is effectively zero
            if (mSettings.wet <= 0.001f) return;

            const float TWO_PI = 2.0f * M_PI;

            for (int i = 0; i < numSamples; i++) {
                // 1. Dry signal is already float (-1.0 to 1.0)
                float dry = buffer[i];

                // 2. Determine LFO Phase for this specific channel
                float channelPhase = mLfoPhase;
                if (i % 2 != 0) { // Right Channel
                    channelPhase += (mSettings.phaseOffset * TWO_PI);
                    // Wrap phase
                    if (channelPhase >= TWO_PI) channelPhase -= TWO_PI;
                }

                // 3. Calculate modulated delay time
                float lfo = std::sin(channelPhase);
                float currentDelaySec = mSettings.delayBase + (lfo * mSettings.depth);
                float delaySamples = currentDelaySec * mSampleRate;

                // 4. Linear Interpolation
                float readPos = static_cast<float>(mWritePos) - delaySamples;
                while (readPos < 0) readPos += static_cast<float>(mMaxBufferSize);

                int idx1 = static_cast<int>(readPos) % mMaxBufferSize;
                int idx2 = (idx1 + 1) % mMaxBufferSize;
                float frac = readPos - static_cast<float>(idx1);

                float wetSample = 0.0f;
                if (i % 2 == 0) {
                    // LEFT CHANNEL (Ensure mDelayBufL is float*)
                    wetSample = mDelayBufL[idx1] * (1.0f - frac) + mDelayBufL[idx2] * frac;
                    mDelayBufL[mWritePos] = dry;
                } else {
                    // RIGHT CHANNEL (Ensure mDelayBufR is float*)
                    wetSample = mDelayBufR[idx1] * (1.0f - frac) + mDelayBufR[idx2] * frac;
                    mDelayBufR[mWritePos] = dry;
                }

                // 5. Mix Dry + (Wet * Mix)
                // No clamping needed. Final output remains in float range.
                buffer[i] = dry + (wetSample * mSettings.wet);

                // 6. Update LFO and Write Position (after the Right channel/Stereo frame)
                if (i % 2 != 0) {
                    mLfoPhase += (TWO_PI * mSettings.rate) / mSampleRate;
                    if (mLfoPhase >= TWO_PI) mLfoPhase -= TWO_PI;

                    mWritePos = (mWritePos + 1) % mMaxBufferSize;
                }
            }
        }
        //----------------------------------------------------------------------
        virtual std::string getName() const override { return "CHORUS / ENSEMBLE";}
#ifdef FLUX_ENGINE
        virtual ImVec4 getColor() const  override { return ImVec4(0.6f, 0.4f, 1.0f, 1.0f);}

        virtual void renderUIWide() override {
            ImGui::PushID("Chorus_Effect_Row_WIDE");
            if (ImGui::BeginChild("CHORUS_BOX", ImVec2(-FLT_MIN,65.f) )) {

                DSP::ChorusSettings currentSettings = this->getSettings();
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
                for (int i = 1; i < DSP::CHORUS_PRESETS.size(); ++i) {
                    if (currentSettings == DSP::CHORUS_PRESETS[i]) {
                        currentIdx = i;
                        break;
                    }
                }
                int displayIdx = currentIdx;  //<< keep currentIdx clean
                ImGui::SameLine(ImGui::GetWindowWidth() - 260.f); // Right-align reset button

                if (ImFlux::ValueStepper("##Preset", &displayIdx, CHORUS_PRESET_NAMES
                    , IM_ARRAYSIZE(CHORUS_PRESET_NAMES)))
                {
                    if (displayIdx > 0 && displayIdx < DSP::CHORUS_PRESETS.size()) {
                        currentSettings =  DSP::CHORUS_PRESETS[displayIdx];
                        changed = true;
                    }
                }
                ImGui::SameLine();
                // if (ImFlux::FaderButton("Reset", ImVec2(40.f, 20.f)))  {
                if (ImFlux::ButtonFancy("RESET", ImFlux::SLATEDARK_BUTTON.WithSize(ImVec2(40.f, 20.f)) ))  {
                    currentSettings = DSP::LUSH80s_CHORUS;
                    this->reset();
                    changed = true;
                }

                ImGui::Separator();
                // ImFlux::MiniKnobF(label, &value, min_v, max_v);
                changed |= ImFlux::MiniKnobF("Rate",  &currentSettings.rate, 0.1f, 2.5f); ImGui::SameLine();
                changed |= ImFlux::MiniKnobF("Depth", &currentSettings.depth, 0.001f, 0.010f); ImGui::SameLine();
                changed |= ImFlux::MiniKnobF("Delay", &currentSettings.delayBase, 0.001f, 0.040f); ImGui::SameLine();
                changed |= ImFlux::MiniKnobF("Phase", &currentSettings.phaseOffset, 0.0f, 1.0f); ImGui::SameLine();
                changed |= ImFlux::MiniKnobF("Mix",   &currentSettings.wet, 0.0f, 1.0f); ImGui::SameLine();

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
            ImGui::PushID("Chorus_Effect_Row");
            ImGui::BeginGroup();

            bool isEnabled = this->isEnabled();

            if (ImFlux::LEDCheckBox(getName(), &isEnabled, getColor()))
                this->setEnabled(isEnabled);


            if (isEnabled)
            {
                if (ImGui::BeginChild("Chorus_Box", ImVec2(0, 160), ImGuiChildFlags_Borders)) {

                    DSP::ChorusSettings currentSettings = this->getSettings();
                    bool changed = false;


                    int currentIdx = 0; // Standard: "Custom"

                    for (int i = 1; i < DSP::CHORUS_PRESETS.size(); ++i) {
                        if (currentSettings == DSP::CHORUS_PRESETS[i]) {
                            currentIdx = i;
                            break;
                        }
                    }
                    int displayIdx = currentIdx;  //<< keep currentIdx clean

                    ImGui::SetNextItemWidth(150);
                    if (ImFlux::ValueStepper("##Preset", &displayIdx, CHORUS_PRESET_NAMES, IM_ARRAYSIZE(CHORUS_PRESET_NAMES))) {
                        if (displayIdx > 0 && displayIdx < DSP::CHORUS_PRESETS.size()) {
                            currentSettings =  DSP::CHORUS_PRESETS[displayIdx];
                            changed = true;
                        }
                    }
                    ImGui::SameLine(ImGui::GetWindowWidth() - 60);

                    if (ImFlux::FaderButton("Reset", ImVec2(40.f, 20.f)))  {
                        currentSettings = DSP::LUSH80s_CHORUS;
                        this->reset();
                        changed = true;
                    }


                    ImGui::Separator();

                    // Parameter Sliders
                    changed |= ImFlux::FaderHWithText("Rate",  &currentSettings.rate, 0.1f, 2.5f, "%.2f Hz");
                    changed |= ImFlux::FaderHWithText("Depth", &currentSettings.depth, 0.001f, 0.010f, "%.4f");
                    changed |= ImFlux::FaderHWithText("Delay", &currentSettings.delayBase, 0.001f, 0.040f, "%.3f s");
                    changed |= ImFlux::FaderHWithText("Phase", &currentSettings.phaseOffset, 0.0f, 1.0f, "Stereo %.2f");
                    changed |= ImFlux::FaderHWithText("Mix",   &currentSettings.wet, 0.0f, 1.0f, "Wet %.2f");

                    // Engine Update logic
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


    }; //class

} // namespace DSP
