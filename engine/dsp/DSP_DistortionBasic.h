//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Digital Sound Processing : Distortion - very basic
//-----------------------------------------------------------------------------
// * using ISettings
//-----------------------------------------------------------------------------

#pragma once

#include <cstdint>
#include <cstring>
#include <atomic>
#include <mutex>
#include <string>

#include <algorithm>
#include <cmath>
#include <vector>
#include <array>

#ifdef FLUX_ENGINE
#include <imgui.h>
#include <imgui_internal.h>
#include <gui/ImFlux.h>
#endif


#include "DSP_Effect.h"
namespace DSP {

    struct DistortionBasicData {
        float gain;  // Input gain (1 - 50) // extreme would be 500 but destroy the sound
        float level;    //  (0.0 - 1.0)
    };
    struct DistortionBasicSettings: public ISettings {
        AudioParam<float> gain      { "Gain"  , 20.f  ,   1.f,  50.f, "%.1f" };
        AudioParam<float> level     { "Level" , 0.5f  ,   0.f,  1.f, "%.2f" };

        DistortionBasicSettings() = default;
        REGISTER_SETTINGS(DistortionBasicSettings, &gain, &level)

        DistortionBasicData getData() const {
            return { gain.get(), level.get()};
        }

        void setData(const DistortionBasicData& data) {
            gain.set(data.gain);
            level.set(data.level);
        }

    };

    class DistortionBasic : public DSP::Effect {
    private:
        DistortionBasicSettings mSettings;
    public:
        IMPLEMENT_EFF_CLONE(DistortionBasic)

        DistortionBasic(bool switchOn = false) :
            DSP::Effect(DSP::EffectType::DistortionBasic, switchOn)
            , mSettings()
            {
                mEffectName = "Distortion";
            }

        //----------------------------------------------------------------------
        // virtual std::string getName() const override { return "Distortion"; }
        //----------------------------------------------------------------------
        void setSettings(const DistortionBasicSettings& s) {mSettings = s;}
        //----------------------------------------------------------------------
        DistortionBasicSettings& getSettings() { return mSettings; }
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
            float level = mSettings.level.get();
            float gain  = mSettings.gain.get();

            if (!isEnabled() || level <= 0.001f) return;

            // no channel handling needed here
            for (int i = 0; i < numSamples; i++) {
                buffer[i] = DSP::softClip(DSP::fast_tanh(buffer[i] * gain) * level);
            }
        }

        //----------------------------------------------------------------------
        #ifdef FLUX_ENGINE
        virtual ImVec4 getDefaultColor() const  override { return  ImVec4(0.6f, 0.6f, 0.0f, 1.0f);}

        virtual void renderPaddle() override {
            DSP::DistortionBasicSettings currentSettings = this->getSettings();
            currentSettings.gain.setKnobSettings(ImFlux::ksRed); // NOTE only works here !
            if (currentSettings.DrawPaddle(this)) {
                this->setSettings(currentSettings);
            }
        }

        virtual void renderUIWide() override {
            DSP::DistortionBasicSettings currentSettings = this->getSettings();
            if (currentSettings.DrawUIWide(this)) {
                this->setSettings(currentSettings);
            }
        }
        virtual void renderUI() override {
            DSP::DistortionBasicSettings currentSettings = this->getSettings();
            if (currentSettings.DrawUI(this, 85.f)) {
                this->setSettings(currentSettings);
            }
        }

        #endif
    };

} // namespace
