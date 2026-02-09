//-----------------------------------------------------------------------------
// Copyright (c) 2026 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Digital Sound Processing : Noise Gate
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



    struct NoiseGateSettings {
        float Threshold ;  // Limit just before 1.f
        float Attack;      // How fast it turns down    in ms!
        float Release;     // How slow it turns back up  in ms!

        static const uint8_t CURRENT_VERSION = 1;
        void getBinary(std::ostream& os) const {
            uint8_t ver = CURRENT_VERSION;
            os.write(reinterpret_cast<const char*>(&ver), sizeof(ver));
            os.write(reinterpret_cast<const char*>(this), sizeof(NoiseGateSettings));
        }

        bool  setBinary(std::istream& is) {
            uint8_t fileVersion = 0;
            is.read(reinterpret_cast<char*>(&fileVersion), sizeof(fileVersion));
            if (fileVersion != CURRENT_VERSION) //Something is wrong !
                return false;
            is.read(reinterpret_cast<char*>(this), sizeof(NoiseGateSettings));
            return  is.good();
        }

        auto operator<=>(const NoiseGateSettings&) const = default; //C++20 lazy way


    };


    constexpr NoiseGateSettings NOISEGATE_DEFAULT = { 0.03f,  2.0f,  0.150f };

    class NoiseGate : public Effect {
    private:
        std::vector<float> mEnvelope; // Current attenuation level per channel
        std::vector<float> mTargetGains;
        NoiseGateSettings mSettings;
        float mSampleRate;
    public:

        NoiseGate(bool switchOn = false) :
            Effect(switchOn),
            mSettings(NOISEGATE_DEFAULT)
            {
                mSampleRate = getSampleRateF();
            }

        virtual void setSampleRate(float sampleRate) override {
            mSampleRate = sampleRate;
        }

        DSP::EffectType getType() const override { return DSP::EffectType::NoiseGate; }
        //----------------------------------------------------------------------
        void setSettings(const NoiseGateSettings& s) {
                // we call reset extra reset(); //also reset current gain.
                mSettings = s;
        }
        //----------------------------------------------------------------------
        void reset() override {  }
        //----------------------------------------------------------------------
        NoiseGateSettings& getSettings() { return mSettings; }
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
        int getChannelCount() { return mEnvelope.size();}
        float getEnvelop(int channel) const { return mEnvelope[channel]; }
        // float getGainReduction() const { return 1.f - mCurrentGain; }
        //----------------------------------------------------------------------
        virtual void process(float* buffer, int numSamples, int numChannels) override {
            if (!isEnabled()) return;

            // 1. Memory Safety: Ensure states are correctly sized
            if (mEnvelope.size() != (size_t)numChannels) {
                mEnvelope.assign(numChannels, 0.0f);
                mTargetGains.assign(numChannels, 0.0f);
            }




            // 2. Pre-calculate increments (Safeguard against 0 ms to avoid Division by Zero)
            float attackInMs = std::max(0.1f, mSettings.Attack);
            float releaseInMs = std::max(1.0f, mSettings.Release);

            // How much the gain moves per sample (e.g. 0.0 to 1.0)
            float attackStep = 1.0f / (attackInMs * mSampleRate * 0.001f);
            float releaseStep = 1.0f / (releaseInMs * mSampleRate * 0.0001f); //  * 0.001f);

            for (int i = 0; i < numSamples; i++) {
                int ch = i % numChannels;
                float inputAbs = std::abs(buffer[i]);

                // 3. Simple Hysteresis: Open at Threshold, Close at 70% of Threshold
                if (inputAbs > mSettings.Threshold) {
                    mTargetGains[ch] = 1.0f;
                } else if (inputAbs < mSettings.Threshold * 0.7f) {
                    mTargetGains[ch] = 0.0f;
                }

                // 4. Linear Ramping with strict Clamping
                if (mTargetGains[ch] > mEnvelope[ch]) {
                    mEnvelope[ch] += attackStep;
                    if (mEnvelope[ch] > 1.0f) mEnvelope[ch] = 1.0f; // HARD LIMIT
                } else if (mTargetGains[ch] < mEnvelope[ch]) {
                    mEnvelope[ch] -= releaseStep;
                    if (mEnvelope[ch] < 0.0f) mEnvelope[ch] = 0.0f; // HARD LIMIT
                }

                // 5. Apply Gain and check for NaN (Safety first)
                float g = mEnvelope[ch];
                if (!std::isfinite(g)) { mEnvelope[ch] = 0.0f; g = 0.0f; }

                buffer[i] *= g;
            }
        }
    //----------------------------------------------------------------------
    virtual std::string getName() const override { return "NOISE GATE";}
#ifdef FLUX_ENGINE
    virtual ImVec4 getColor() const  override { return ImVec4(0.6f, 0.4f, 0.6f, 1.0f);}
    virtual void renderUIWide() override {
        ImGui::PushID("NoiseGate_Effect_Row_WIDE");
        if (ImGui::BeginChild("NOISEGATE_BOX", ImVec2(-FLT_MIN,65.f) )) {

            DSP::NoiseGateSettings currentSettings = this->getSettings();
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
            // for (int i = 1; i < DSP::LIMITER_PRESETS.size(); ++i) {
            //     if (currentSettings == DSP::LIMITER_PRESETS[i]) {
            //         currentIdx = i;
            //         break;
            //     }
            // }
            // int displayIdx = currentIdx;  //<< keep currentIdx clean
            // ImGui::SameLine(ImGui::GetWindowWidth() - 260.f); // Right-align reset button
            //
            // if (ImFlux::ValueStepper("##Preset", &displayIdx, LIMITER_PRESET_NAMES
            //     , IM_ARRAYSIZE(LIMITER_PRESET_NAMES)))
            // {
            //     if (displayIdx > 0 && displayIdx < DSP::LIMITER_PRESETS.size()) {
            //         currentSettings =  DSP::LIMITER_PRESETS[displayIdx];
            //         changed = true;
            //     }
            // }
            // ImGui::SameLine();
            // if (ImFlux::FaderButton("Reset", ImVec2(40.f, 20.f)))  {
            if (ImFlux::ButtonFancy("RESET", ImFlux::SLATEDARK_BUTTON.WithSize(ImVec2(40.f, 20.f)) ))  {
                currentSettings = DSP::NOISEGATE_DEFAULT; //DEFAULT
                this->reset();
                changed = true;
            }

            ImGui::Separator();
            // ImFlux::MiniKnobF(label, &value, min_v, max_v);
            changed |= ImFlux::MiniKnobF("Threshold", &currentSettings.Threshold, 0.01f, 1.f); ImGui::SameLine();
            changed |= ImFlux::MiniKnobF("Depth (ms)", &currentSettings.Attack, 10.f, 1000.f); ImGui::SameLine();
            changed |= ImFlux::MiniKnobF("Release (ms)", &currentSettings.Release, 0.15f, 1.f); ImGui::SameLine();

            ImGui::BeginGroup();
            for (int ch = 0; ch < getChannelCount(); ch++) {
                ImGui::TextDisabled("Envelope [%d]: %3.3f", ch, getEnvelop(ch));
            }

            // float reduction = getGainReduction();
            // ImGui::TextDisabled("Reduction: %4.1f%%", reduction * 100.f);
            // ImFlux::PeakMeter(reduction,ImVec2(125.f, 8.f));
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
            ImGui::PushID("NOISEGATE_Effect_Row");


            ImGui::BeginGroup();

            auto* lim = this;
            bool isEnabled = lim->isEnabled();


            if (ImFlux::LEDCheckBox(getName(), &isEnabled, getColor())) {
                lim->setEnabled(isEnabled);
            }


            if (lim->isEnabled()) {
                bool changed = false;
                DSP::NoiseGateSettings& currentSettings = lim->getSettings();

                int currentIdx = 0; // Standard: "Custom"

                // for (int i = 1; i < DSP::LIMITER_PRESETS.size(); ++i) {
                //     if (currentSettings == DSP::LIMITER_PRESETS[i]) {
                //         currentIdx = i;
                //         break;
                //     }
                // }
                // int displayIdx = currentIdx;  //<< keep currentIdx clean

                // 150.f with sliders and Stepper --- if any
                if (ImGui::BeginChild("LIM_Box", ImVec2(0, 75.f),  ImGuiChildFlags_Borders)) {

                    ImGui::BeginGroup();
                    // ImGui::SetNextItemWidth(150);
                    //
                    // if (ImFlux::ValueStepper("##Preset", &displayIdx, LIMITER_PRESET_NAMES, IM_ARRAYSIZE(LIMITER_PRESET_NAMES))) {
                    //     if (displayIdx > 0 && displayIdx < DSP::LIMITER_PRESETS.size()) {
                    //                 lim->setSettings(DSP::LIMITER_PRESETS[displayIdx]);
                    //     }
                    // }
                    //
                    // Quick Reset Button (Now using the FLAT_EQ preset)
                    // ImGui::SameLine(ImGui::GetWindowWidth() - 60);
                    // if (ImFlux::FaderButton("Reset", ImVec2(40.f, 20.f)))  {
                    //     lim->setSettings(DSP::NOISEGATE_DEFAULT);
                    //     lim->reset();
                    // }
                    // ImGui::Separator();
                    changed |= ImFlux::FaderHWithText("Threshold", &currentSettings.Threshold, 0.01f, 1.f, "%.3f");
                    // changed |= ImFlux::FaderHWithText("Depth", &currentSettings.Attack, 10.f, 1000.f, "%4.f ms");
                    // changed |= ImFlux::FaderHWithText("Release", &currentSettings.Release, 0.15f, 1.f, "%.5f");
                    if (changed) {
                         lim->setSettings(currentSettings);
                    }

                    ImGui::Separator();

                    // one is ok :P
                    if ( getChannelCount() > 0) ImGui::TextDisabled("Envelope: %3.3f", getEnvelop(0));
                    // for (int ch = 0; ch < getChannelCount(); ch++) {
                    //     ImGui::TextDisabled("Envelope [%d]: %3.3f", ch, getEnvelop(ch));
                    // }


                    // float reduction = getGainReduction();
                    // ImGui::TextDisabled("Reduction: %3.3f", reduction);
                    // ImFlux::PeakMeter(reduction,ImVec2(150.f, 7.f));
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
