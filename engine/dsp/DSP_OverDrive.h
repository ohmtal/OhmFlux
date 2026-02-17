//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Digital Sound Processing : Distortion
//-----------------------------------------------------------------------------
// * using ISettings
//-----------------------------------------------------------------------------

#pragma once

#include <mutex>
#include <string>
#include <vector>

#include "DSP_Math.h"

#ifdef FLUX_ENGINE
#include <imgui.h>
#include <imgui_internal.h>
#include <gui/ImFlux.h>
#endif


#include "DSP_Effect.h"
namespace DSP {

    struct OverDriveData {
        float drive;  // Input gain (1.0 - 20.0)
        float tone;   // Low-pass/High-pass tilt (0.0 - 1.0)
        float bassPreserve;  // Cross-over frequency for clean bass (0.0 - 1.0)
        float wet;    // Mix (0.0 - 1.0)
    };

    struct OverDriveSettings : public ISettings {
        AudioParam<float> drive         { "Drive" ,   5.0f, 1.0f,20.0f, "%.1f" };
        AudioParam<float> tone          { "Tone",     0.5f, 0.0f, 1.0f, "%.2f" };
        AudioParam<float> bassPreserve  { "Preserve", 0.3f, 0.0f, 1.0f, "%.2f" };
        AudioParam<float> wet           { "Mix",      1.0f, 0.0f, 1.0f, "%.2f"};


        OverDriveSettings() = default;
        REGISTER_SETTINGS(OverDriveSettings, &drive,&tone, &bassPreserve, &wet)

        OverDriveData getData() const {
            return { drive.get(), tone.get(), bassPreserve.get(), wet.get()};
        }

        void setData(const OverDriveData& data) {
            drive.set(data.drive);
            tone.set(data.tone);
            bassPreserve.set(data.bassPreserve);
            wet.set(data.wet);
        }

    };


    class OverDrive : public DSP::Effect {
    private:
        OverDriveSettings mSettings;
        std::vector<float> mToneStates;  // High-cut state per channel
        std::vector<float> mBassStates;  // Low-pass state for Bass Preserve per channel

    public:
        IMPLEMENT_EFF_CLONE(OverDrive)

        OverDrive(bool switchOn = false) : DSP::Effect(DSP::EffectType::OverDrive, switchOn)
            ,mSettings()
            { }
        //----------------------------------------------------------------------
        virtual std::string getName() const override { return "OverDrive"; }
        //----------------------------------------------------------------------
        void setSettings(const OverDriveSettings& s) { mSettings = s; }
        //----------------------------------------------------------------------
        OverDriveSettings getSettings() const { return mSettings; }
        //----------------------------------------------------------------------
        virtual void reset() override {
            std::fill(mToneStates.begin(), mToneStates.end(), 0.0f);
            std::fill(mBassStates.begin(), mBassStates.end(), 0.0f);
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
        virtual void process(float* buffer, int numSamples, int numChannels) override {
            float wet = mSettings.wet.get();
            if (!isEnabled() || wet <= 0.001f) return;

            if (mToneStates.size() != (size_t)numChannels) {
                mToneStates.resize(numChannels, 0.0f);
                mBassStates.resize(numChannels, 0.0f);
            }

            // Coefficients
            float toneAlpha = DSP::clamp(mSettings.tone.get(), 0.01f, 0.99f);
            // Bass Alpha (Low-pass crossover, usually around 100Hz - 400Hz)
            float bassAlpha = DSP::clamp(mSettings.bassPreserve.get() * 0.2f, 0.01f, 0.5f);

            float drive = mSettings.drive.get();


            int channel = 0;
            for (int i = 0; i < numSamples; i++) {
                float dry = buffer[i];

                // 1. Extract Clean Bass (Low-pass filter)
                mBassStates[channel] = (dry * bassAlpha) + (mBassStates[channel] * (1.0f - bassAlpha));
                float cleanBass = mBassStates[channel];

                // 2. Drive & Clip (processed signal)
                float x = (dry - cleanBass) * drive; // Distort mainly mids/highs

                // Cubic Soft-Clipper
                float clipped = DSP::clamp(x - (x * x * x * 0.15f), -0.8f, 0.8f);

                // 3. Tone Control (High-cut on the distorted part)
                mToneStates[channel] = (clipped * toneAlpha) + (mToneStates[channel] * (1.0f - toneAlpha));
                float distorted = mToneStates[channel];

                // 4. Re-combine: Distorted Signal + Clean Bass
                float combined = distorted + cleanBass;

                // 5. Final Mix
                float out = (dry * (1.0f - wet)) + (combined * wet);
                buffer[i] = DSP::clamp(out, -1.0f, 1.0f);

                if (++channel >= numChannels) channel = 0;
            }
        }

        //--------------------------------------------------------------------------
        #ifdef FLUX_ENGINE
        virtual ImVec4 getColor() const  override { return  ImVec4(0.827f, 0.521f, 0.329f, 1.0f);}

        virtual void renderPaddle() override {
            DSP::OverDriveSettings currentSettings = this->getSettings();
            currentSettings.drive.setKnobSettings(ImFlux::ksRed);
            // currentSettings.tone.setKnobSettings(ImFlux::ksBlue);
            currentSettings.bassPreserve.setKnobSettings(ImFlux::ksPurple);
            currentSettings.wet.setKnobSettings(ImFlux::ksBlue);
            if (currentSettings.DrawPaddle(this)) {
                this->setSettings(currentSettings);
            }
        }

        virtual void renderUIWide() override {
            DSP::OverDriveSettings currentSettings = this->getSettings();
            if (currentSettings.DrawUIWide(this)) {
                this->setSettings(currentSettings);
            }
        }
        virtual void renderUI() override {
            DSP::OverDriveSettings currentSettings = this->getSettings();
            if (currentSettings.DrawUI(this, 160.f, true)) {
                this->setSettings(currentSettings);
            }
        }


        //--------------------------------------------------------------------------
        // virtual void renderPaddle() override {
        //     ImGui::PushID("OVERDRIVE_Effect_PADDLE");
        //     paddleHeader(getName().c_str(), ImGui::ColorConvertFloat4ToU32(getColor()), mEnabled);
        //     OverDriveSettings currentSettings = this->getSettings();
        //     bool changed = false;
        //     changed |= rackKnob("DRIVE", &currentSettings.drive, {1.0f, 20.0f}, ImFlux::ksRed);ImGui::SameLine();
        //     changed |= rackKnob("TONE", &currentSettings.tone, {0.0f, 1.0f}, ImFlux::ksBlue);ImGui::SameLine();
        //     changed |= rackKnob("PRESERVE", &currentSettings.bassPreserve, {0.0f, 1.0f}, ImFlux::ksBlue);ImGui::SameLine();
        //     changed |= rackKnob("LEVEL", &currentSettings.wet, {0.0f, 1.f}, ImFlux::ksBlack);
        //     if (changed) this->setSettings(currentSettings);
        //     ImGui::PopID();
        // }

        #endif
    };

} // namespace
