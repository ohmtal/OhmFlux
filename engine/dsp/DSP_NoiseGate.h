//-----------------------------------------------------------------------------
// Copyright (c) 2026 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Digital Sound Processing : Noise Gate + HPF/LPF Filter
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
        float Release;     // How slow it turns back up  in ms!

        // Low pass filter
        float lpfAlpha = 1.f;     // 1.0 = bypass, < 1.0 = filtering
        // High pass filter
        float hpfAlpha = 1.f;     // 1.0 = bypass



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


    constexpr NoiseGateSettings NOISEGATE_DEFAULT = { 0.03f,  10.f, 1.f, 1.f };

    class NoiseGate : public Effect {
    private:
        std::vector<float> mCurrentGains;
        std::vector<float> mLpfLastOut;
        std::vector<float> mHpfLastIn;
        std::vector<float> mHpfLastOut;

        NoiseGateSettings mSettings;
        float mSampleRate;
        float mReleaseSamples = 1.f;
    public:

        NoiseGate(bool switchOn = false) :
            Effect(switchOn),
            mSettings(NOISEGATE_DEFAULT)
            {
                mSampleRate = getSampleRateF();
                initVectors(2);
            }
        void updateReleaseSamples() {
            mReleaseSamples = ( mSettings.Release / 1000.0f) * mSampleRate;
            if (mReleaseSamples < 1.0f) mReleaseSamples = 1.0f;
        }
        virtual void setSampleRate(float sampleRate) override {
            mSampleRate = sampleRate;
            updateReleaseSamples();
        }

        DSP::EffectType getType() const override { return DSP::EffectType::NoiseGate; }
        //----------------------------------------------------------------------
        void setSettings(const NoiseGateSettings& s) {
                mSettings = s;
                updateReleaseSamples();
        }
        //----------------------------------------------------------------------
        void initVectors(int channel) {
            mCurrentGains.assign(channel, 0.0f);
            mLpfLastOut.assign(channel, 0.0f);
            mHpfLastIn.assign(channel, 0.0f);
            mHpfLastOut.assign(channel, 0.0f);

        }
        //----------------------------------------------------------------------
        void reset() override {
            initVectors(2);
        }
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
        int getChannelCount() { return mCurrentGains.size();}
        //----------------------------------------------------------------------
        virtual void process(float* buffer, int numSamples, int numChannels) override {
            if (!isEnabled()) return;

            float lThresHold =  mSettings.Threshold * 0.01f;

            if (mCurrentGains.size() != (size_t)numChannels) {
                initVectors(numChannels);
            }

            for (int i = 0; i < numSamples; i++) {
                int ch = i % numChannels;

                float inputAbs = std::abs(buffer[i]);

                if (inputAbs >= lThresHold) {
                         mCurrentGains[ch] = 1.0f;
                } else {
                         mCurrentGains[ch] -= 1.0f / mReleaseSamples;
                         if (mCurrentGains[ch] < 0.0f) mCurrentGains[ch] = 0.0f;
                }

                //HPF Filter
                float input = buffer[i];
                if ( mSettings.hpfAlpha < 1.f )
                {
                    input = buffer[i];
                    // float out = alpha * (last_out + input - last_input);
                    buffer[i] = mSettings.hpfAlpha * (mHpfLastOut[ch] + input - mHpfLastIn[ch]);
                    mHpfLastIn[ch] = input;
                    mHpfLastOut[ch] = buffer[i];
                }

                //LPF Filter
                if ( mSettings.lpfAlpha < 1.f )
                {
                    input = buffer[i];
                    // float out = last_out + alpha * (input - last_out);
                    buffer[i] = mLpfLastOut[ch] + mSettings.lpfAlpha * ( input - mLpfLastOut[ch] ) ;

                    mLpfLastOut[ch] = buffer[i];
                }



                buffer[i] *= mCurrentGains[ch];
            }
        }

    // virtual void process(float* buffer, int numSamples, int numChannels) override {
    //     if (!isEnabled()) return;
    //
    //     // 1. Memory Safety: Ensure states are correctly sized
    //     if (mEnvelope.size() != (size_t)numChannels) {
    //         mEnvelope.assign(numChannels, 0.0f);
    //         mTargetGains.assign(numChannels, 0.0f);
    //     }
    //
    //     // 2. Pre-calculate increments (Safeguard against 0 ms to avoid Division by Zero)
    //     float attackInMs = std::max(0.1f, mSettings.Attack);
    //     float releaseInMs = std::max(1.0f, mSettings.Release);
    //
    //     // How much the gain moves per sample (e.g. 0.0 to 1.0)
    //     float attackStep = 1.0f / (attackInMs * mSampleRate * 0.001f);
    //     float releaseStep = 1.0f / (releaseInMs * mSampleRate * 0.0001f); //  * 0.001f);
    //
    //     for (int i = 0; i < numSamples; i++) {
    //         int ch = i % numChannels;
    //         float inputAbs = std::abs(buffer[i]);
    //
    //         // 3. Simple Hysteresis: Open at Threshold, Close at 70% of Threshold
    //         if (inputAbs > mSettings.Threshold) {
    //             mTargetGains[ch] = 1.0f;
    //         } else if (inputAbs < mSettings.Threshold * 0.7f) {
    //             mTargetGains[ch] = 0.0f;
    //         }
    //
    //         // 4. Linear Ramping with strict Clamping
    //         if (mTargetGains[ch] > mEnvelope[ch]) {
    //             mEnvelope[ch] += attackStep;
    //             if (mEnvelope[ch] > 1.0f) mEnvelope[ch] = 1.0f; // HARD LIMIT
    //         } else if (mTargetGains[ch] < mEnvelope[ch]) {
    //             mEnvelope[ch] -= releaseStep;
    //             if (mEnvelope[ch] < 0.0f) mEnvelope[ch] = 0.0f; // HARD LIMIT
    //         }
    //
    //         // 5. Apply Gain and check for NaN (Safety first)
    //         float g = mEnvelope[ch];
    //         if (!std::isfinite(g)) { mEnvelope[ch] = 0.0f; g = 0.0f; }
    //
    //         buffer[i] *= g;
    //     }
    // }
    //----------------------------------------------------------------------
    virtual std::string getName() const override { return "NOISE GATE";}
#ifdef FLUX_ENGINE
    virtual ImVec4 getColor() const  override { return ImVec4(0.6f, 0.4f, 0.6f, 1.0f);}
    virtual void renderUIWide() override {
        ImGui::PushID("NoiseGate_Effect_Row_WIDE");
        if (ImGui::BeginChild("NOISEGATE_BOX", ImVec2(-FLT_MIN,65.f) )) {

            DSP::NoiseGateSettings currentSettings = this->getSettings();
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
            if (ImFlux::ButtonFancy("RESET", ImFlux::SLATEDARK_BUTTON.WithSize(ImVec2(40.f, 20.f)) ))  {
                currentSettings = DSP::NOISEGATE_DEFAULT; //DEFAULT
                this->reset();
                changed = true;
            }

            ImGui::Separator();
            // ImFlux::MiniKnobF(label, &value, min_v, max_v);
            changed |= ImFlux::MiniKnobF("Threshold", &currentSettings.Threshold, 0.01f, 1.f); ImGui::SameLine();
            changed |= ImFlux::MiniKnobF("Release (ms)", &currentSettings.Release, 10.f, 100.f); ImGui::SameLine();
            // only slider can go backwards!
            changed |= ImFlux::MiniKnobF("Low pass filter", &currentSettings.lpfAlpha, 0.9f, 1.f); //, "%.2f");
            ImGui::SameLine();
            changed |= ImFlux::MiniKnobF("High pass filter", &currentSettings.hpfAlpha, 0.9f, 1.f); //, "%.2f");
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
                if (ImGui::BeginChild("LIM_Box", ImVec2(0, 100.f),  ImGuiChildFlags_Borders)) {

                    ImGui::BeginGroup();
                    changed |= ImFlux::FaderHWithText("Threshold", &currentSettings.Threshold, 0.01f, 1.f, "%.3f");
                    changed |= ImFlux::FaderHWithText("Release", &currentSettings.Release, 10.f, 100.f, "%4.f");
                    changed |= ImFlux::FaderHWithText("Low pass filter", &currentSettings.lpfAlpha, 0.9f, 1.f, "%.2f");
                    changed |= ImFlux::FaderHWithText("High pass filter", &currentSettings.hpfAlpha, 0.9f, 1.f, "%.2f");
                    if (changed) {
                         lim->setSettings(currentSettings);
                    }
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
