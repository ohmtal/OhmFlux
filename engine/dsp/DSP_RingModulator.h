//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Digital Sound Processing : RingModulator
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
#include "DSP_Math.h"

namespace DSP {

struct RingModData {
    float frequency; // The carrier frequency (Hz). e.g., 200Hz - 2000Hz
    float wet;       // Dry/Wet mix (0.0 - 1.0)
};


struct RingModSettings: public ISettings {

    AudioParam<float> frequency  { "Frequency", 400.f,  200.0f, 2000.0f, "Stereo %.2f Hz"};
    AudioParam<float> wet        { "Mix",   0.50f ,  0.0f, 1.0f, "Wet %.2f"};


    RingModSettings() = default;
    REGISTER_SETTINGS(RingModSettings, &frequency, &wet)

    RingModData getData() const {
        return { frequency.get(), wet.get()};
    }

    void setData(const RingModData& data) {
        frequency.set(data.frequency);
        wet.set(data.wet);
    }
};

class RingModulator : public DSP::Effect {
private:
    RingModSettings mSettings;
    float mPhaseL = 0.0f; // Oscillator phase for left channel

public:
    IMPLEMENT_EFF_CLONE(RingModulator)

    RingModulator(bool switchOn = false) :
        DSP::Effect(DSP::EffectType::RingModulator, switchOn)
        ,mSettings()
    {
        mEffectName = "Ring Modulator";
        reset();
    }
    //----------------------------------------------------------------------
    // virtual std::string getName() const override { return "Ring Modulator"; }
    //----------------------------------------------------------------------
    void setSettings(const RingModSettings& s) { mSettings = s; }
    //----------------------------------------------------------------------
    RingModSettings getSettings() const { return mSettings; }
    //----------------------------------------------------------------------
    virtual void reset() override {
        mPhaseL = 0.0f;
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
        const float wet = mSettings.wet.get();
        if (!isEnabled() || wet <= 0.001f) return;

        const float phaseIncrement = mSettings.frequency.get() / mSampleRate;
        const float dryGain = 1.0f - wet;

        int channel = 0;
        for (int i = 0; i < numSamples; i++) {
            float dry = buffer[i];

            float carrier = DSP::FastMath::fastSin(mPhaseL);
            float modulated = dry * carrier;

            buffer[i] = (dry * dryGain) + (modulated * wet);

            if (++channel >= numChannels) {
                channel = 0;
                mPhaseL += phaseIncrement;
                if (mPhaseL >= 1.0f) mPhaseL -= 1.0f;
            }
        }
    }

    //--------------------------------------------------------------------------
#ifdef FLUX_ENGINE
    virtual ImVec4 getDefaultColor() const override { return ImVec4(0.1f, 0.2f, 0.7f, 1.0f); } // blueish

    virtual void renderPaddle() override {
        DSP::RingModSettings currentSettings = this->getSettings();
        currentSettings.wet.setKnobSettings(ImFlux::ksBlue); // NOTE only works here !
        if (currentSettings.DrawPaddle(this)) {
            this->setSettings(currentSettings);
        }
    }

    virtual void renderUIWide() override {
        DSP::RingModSettings currentSettings = this->getSettings();
        if (currentSettings.DrawUIWide(this)) {
            this->setSettings(currentSettings);
        }
    }
    virtual void renderUI() override {
        DSP::RingModSettings currentSettings = this->getSettings();
        if (currentSettings.DrawUI(this, 160.f, true)) {
            this->setSettings(currentSettings);
        }
    }


#endif
}; //CLASS
}; //namespace
