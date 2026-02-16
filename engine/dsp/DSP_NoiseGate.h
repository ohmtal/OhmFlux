//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Digital Sound Processing : Noise Gate + HPF/LPF Filter
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

    struct NoiseGateData {
        float Threshold ;  // Limit just before 1.f
        float Release;     // How slow it turns back up  in ms!

        // High pass filter
        float hpfAlpha = 1.f;     // 1.0 = bypass
        // Low pass filter
        float lpfAlpha = 1.f;     // 1.0 = bypass, < 1.0 = filtering
    };

    struct NoiseGateSettings : public ISettings {
        AudioParam<float> Threshold { "Threshold" , 0.03f ,   0.01f,  1.f, "%.3f" };
        AudioParam<float> Release   { "Release" ,    10.f,   10.f,  100.f, "%.1f" };

        AudioParam<float> hpfAlpha  { "High pass filter" , 1.f,  0.9f,  1.f, "%.2f" };
        AudioParam<float> lpfAlpha  { "Low pass filter" , 1.f,   0.0f,  1.f, "%.2f" };

        NoiseGateSettings () = default;
        REGISTER_SETTINGS(NoiseGateSettings , &Threshold, &Release, &hpfAlpha, &lpfAlpha )

        NoiseGateData getData() const {
            return { Threshold.get(), Release.get(), hpfAlpha.get(), lpfAlpha.get()};
        }

        void setData(const NoiseGateData& data) {
            Threshold.set(data.Threshold);
            Release.set(data.Release);
            hpfAlpha.set(data.hpfAlpha);
            lpfAlpha.set(data.lpfAlpha);
        }

        // examples from limiter:
        // std::vector<std::shared_ptr<IPreset>> getPresets() const override {
        //     return {
        //         std::make_shared<Preset<LimiterSettings, LimiterData>>
        //         ("Custom", LimiterData{ 0.90f,  0.05f,  0.150f }),
        //         std::make_shared<Preset<LimiterSettings, LimiterData>>
        //         ("DEFAULT", LimiterData{ 0.95f,  0.05f,  0.150f }),
        //         std::make_shared<Preset<LimiterSettings, LimiterData>>
        //         ("EIGHTY", LimiterData{ 0.80f,  0.05f,  0.150f }),
        //         std::make_shared<Preset<LimiterSettings, LimiterData>>
        //         ("FIFTY", LimiterData{ 0.50f,  0.05f,  0.150f }),
        //         std::make_shared<Preset<LimiterSettings, LimiterData>>
        //         ("LOWVOL", LimiterData{ 0.25f,  0.05f,  0.150f }),
        //         std::make_shared<Preset<LimiterSettings, LimiterData>>
        //         ("EXTREM", LimiterData{ 0.05f,  0.05f,  0.150f }),
        //     };
        // }
    };


    constexpr NoiseGateData DEFAULT_NOISEGATE_DATA = { 0.03f,  10.f, 1.f, 1.f };

    class NoiseGate : public Effect {
    private:
        std::vector<float> mCurrentGains;
        std::vector<float> mLpfLastOut;
        std::vector<float> mHpfLastIn;
        std::vector<float> mHpfLastOut;

        NoiseGateSettings mSettings;
        float mReleaseSamples = 1.f;
    public:
        IMPLEMENT_EFF_CLONE(NoiseGate)

        NoiseGate(bool switchOn = false) :
            Effect(DSP::EffectType::NoiseGate, switchOn),
            mSettings()
            {
                initVectors(2);
            }
        //----------------------------------------------------------------------
        virtual std::string getName() const override { return "NOISE GATE";}
        //----------------------------------------------------------------------
        NoiseGateSettings& getSettings() { return mSettings; }
        //----------------------------------------------------------------------
        void setSettings(const NoiseGateSettings& s) {
            mSettings = s;
            updateReleaseSamples();
        }
        //----------------------------------------------------------------------
        void updateReleaseSamples() {
            mReleaseSamples = ( mSettings.Release.get() / 1000.0f) * mSampleRate;
            if (mReleaseSamples < 1.0f) mReleaseSamples = 1.0f;
        }
        //----------------------------------------------------------------------
        virtual void setSampleRate(float sampleRate) override {
            mSampleRate = sampleRate;
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
        int getChannelCount() { return mCurrentGains.size();}
        //----------------------------------------------------------------------
        virtual void process(float* buffer, int numSamples, int numChannels) override {
            if (!isEnabled()) return;

            float lThresHold =  mSettings.Threshold.get() * 0.01f;
            float hpf = mSettings.hpfAlpha.get();
            float lpf = mSettings.lpfAlpha.get();

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
                if ( hpf < 1.f )
                {
                    input = buffer[i];
                    // float out = alpha * (last_out + input - last_input);
                    buffer[i] = hpf * (mHpfLastOut[ch] + input - mHpfLastIn[ch]);
                    mHpfLastIn[ch] = input;
                    mHpfLastOut[ch] = buffer[i];
                }

                //LPF Filter
                if ( lpf < 1.f )
                {
                    input = buffer[i];
                    // float out = last_out + alpha * (input - last_out);
                    buffer[i] = mLpfLastOut[ch] + lpf * ( input - mLpfLastOut[ch] ) ;

                    mLpfLastOut[ch] = buffer[i];
                }



                buffer[i] *= mCurrentGains[ch];
            }
        }

#ifdef FLUX_ENGINE
    virtual ImVec4 getColor() const  override { return ImVec4(0.6f, 0.4f, 0.6f, 1.0f);}


    virtual void renderPaddle() override {
        DSP::NoiseGateSettings currentSettings = this->getSettings();
        if (currentSettings.DrawPaddle(this)) {
            this->setSettings(currentSettings);
        }
    }

    virtual void renderUIWide() override {
        DSP::NoiseGateSettings currentSettings = this->getSettings();
        if (currentSettings.DrawUIWide(this)) {
            this->setSettings(currentSettings);
        }
    }
    virtual void renderUI() override {
        DSP::NoiseGateSettings currentSettings = this->getSettings();
        if (currentSettings.DrawUI(this, 160.f, true)) {
            this->setSettings(currentSettings);
        }
    }

    // virtual void renderUIWide() override {
    //     ImGui::PushID("NoiseGate_Effect_Row_WIDE");
    //     if (ImGui::BeginChild("NOISEGATE_BOX", ImVec2(-FLT_MIN,65.f) )) {
    //
    //         DSP::NoiseGateSettings currentSettings = this->getSettings();
    //         bool changed = false;
    //
    //         ImFlux::GradientBox(ImVec2(-FLT_MIN, -FLT_MIN),0.f);
    //         ImGui::Dummy(ImVec2(2,0)); ImGui::SameLine();
    //         ImGui::BeginGroup();
    //         bool isEnabled = this->isEnabled();
    //         if (ImFlux::LEDCheckBox(getName(), &isEnabled, getColor())){
    //             this->setEnabled(isEnabled);
    //         }
    //
    //         if (!isEnabled) ImGui::BeginDisabled();
    //
    //         ImGui::SameLine();
    //         if (ImFlux::ButtonFancy("RESET", ImFlux::SLATEDARK_BUTTON.WithSize(ImVec2(40.f, 20.f)) ))  {
    //             currentSettings = DSP::NOISEGATE_DEFAULT; //DEFAULT
    //             this->reset();
    //             changed = true;
    //         }
    //
    //         ImGui::Separator();
    //         // ImFlux::MiniKnobF(label, &value, min_v, max_v);
    //         changed |= ImFlux::MiniKnobF("Threshold", &currentSettings.Threshold, 0.01f, 1.f); ImGui::SameLine();
    //         changed |= ImFlux::MiniKnobF("Release (ms)", &currentSettings.Release, 10.f, 100.f); ImGui::SameLine();
    //         // only slider can go backwards!
    //         changed |= ImFlux::MiniKnobF("Low pass filter", &currentSettings.lpfAlpha, 0.9f, 1.f); //, "%.2f");
    //         ImGui::SameLine();
    //         changed |= ImFlux::MiniKnobF("High pass filter", &currentSettings.hpfAlpha, 0.9f, 1.f); //, "%.2f");
    //         if (changed) {
    //             if (isEnabled) {
    //                 this->setSettings(currentSettings);
    //             }
    //         }
    //         if (!isEnabled) ImGui::EndDisabled();
    //         ImGui::EndGroup();
    //     }
    //     ImGui::EndChild();
    //     ImGui::PopID();
    //
    // }
    //
    // //--------------------------------------------------------------------------
    // virtual void renderUI() override {
    //         ImGui::PushID("NOISEGATE_Effect_Row");
    //         ImGui::BeginGroup();
    //         auto* lim = this;
    //         bool isEnabled = lim->isEnabled();
    //         if (ImFlux::LEDCheckBox(getName(), &isEnabled, getColor())) {
    //             lim->setEnabled(isEnabled);
    //         }
    //         if (lim->isEnabled()) {
    //             bool changed = false;
    //             DSP::NoiseGateSettings& currentSettings = lim->getSettings();
    //             if (ImGui::BeginChild("LIM_Box", ImVec2(0, 100.f),  ImGuiChildFlags_Borders)) {
    //
    //                 ImGui::BeginGroup();
    //                 changed |= ImFlux::FaderHWithText("Threshold", &currentSettings.Threshold, 0.01f, 1.f, "%.3f");
    //                 changed |= ImFlux::FaderHWithText("Release", &currentSettings.Release, 10.f, 100.f, "%4.f");
    //                 changed |= ImFlux::FaderHWithText("Low pass filter", &currentSettings.lpfAlpha, 0.9f, 1.f, "%.2f");
    //                 changed |= ImFlux::FaderHWithText("High pass filter", &currentSettings.hpfAlpha, 0.9f, 1.f, "%.2f");
    //                 if (changed) {
    //                      lim->setSettings(currentSettings);
    //                 }
    //                 ImGui::EndGroup();
    //
    //             } //box
    //             ImGui::EndChild();
    //         } else {
    //             ImGui::Separator();
    //         }
    //
    //         ImGui::EndGroup();
    //         ImGui::PopID();
    //         ImGui::Spacing();
    //     }

    #endif //FLUX_ENGINE


    };

} // namespace DSP
