//-----------------------------------------------------------------------------
// Copyright (c) 2026 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Digital Sound Processing : AutoWah
//-----------------------------------------------------------------------------
// * using ISettings
//-----------------------------------------------------------------------------
#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <cmath>
#include <atomic>
#include <algorithm>

#include "DSP_Effect.h"
#include "MonoProcessors/AutoWah.h"

#ifdef FLUX_ENGINE
#include <imgui.h>
#include <imgui_internal.h>
#include <gui/ImFlux.h>
#endif

namespace DSP {

struct AutoWahData {
    float sensitivity;
    float resonance;
    float range;
    float mix;
};

struct AutoWahSettings : public ISettings {
    AudioParam<float> sensitivity   { "Sensitivity" , 0.5f,0.0f,1.0f, "%.2f" };
    AudioParam<float> resonance     { "Resonance"   , 0.4f,0.0f,0.7f, "%.2f" };
    AudioParam<float> range         { "Range" , 0.85f,0.0f,1.0f, "%.2f" };
    AudioParam<float> mix           { "Mix"   , 0.5f,0.0f,1.0f, "%.2f" };


    AutoWahSettings() = default;
    REGISTER_SETTINGS(AutoWahSettings, &sensitivity, &resonance, &range, &mix)

    AutoWahData getData() const {
        return { sensitivity.get(), resonance.get(), range.get(), mix.get()};
    }

    void setData(const AutoWahData& data) {
        sensitivity.set(data.sensitivity);
        resonance.set(data.resonance);
        range.set(data.range);
        mix.set(data.mix);
    }
    // FIXME
    // std::vector<std::shared_ptr<IPreset>> getPresets() const override {
    //     return {
    //     };
    // }
};
class AutoWah: public Effect {
public:
    IMPLEMENT_EFF_CLONE(AutoWah)
    AutoWah(bool switchOn = false) :
        Effect(DSP::EffectType::AutoWah, switchOn)
        , mSettings()
        {}
    //----------------------------------------------------------------------
    virtual std::string getName() const override { return "AutoWah";}

    // //----------------------------------------------------------------------
    AutoWahSettings& getSettings() { return mSettings; }
    // //----------------------------------------------------------------------
    void setSettings(const AutoWahSettings& s) {
        mSettings = s;
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
        return true;
    }

    //----------------------------------------------------------------------
    virtual void reset() override {
    }
    //----------------------------------------------------------------------
    virtual void process(float* buffer, int numSamples, int numChannels) override {
        const float mix = mSettings.mix.get();

        if (!isEnabled() || mix <= 0.001f) return;

        if (mAutoWahs.size() != (size_t) numChannels ) {
            mAutoWahs.resize(numChannels, DSP::MonoProcessors::AutoWah());
        }


        const float sensitivity = mSettings.sensitivity.get();
        const float resonance = mSettings.resonance.get();
        const float range = mSettings.range.get();

        for (int i = 0; i < numSamples; i++) {
            for ( int ch = 0; ch < numChannels; ch++) {
                buffer[i] = mAutoWahs[ch].process(buffer[i], sensitivity, resonance, range, mix, mSampleRate);
            }
        }
    }

private:
    AutoWahSettings mSettings;
    std::vector<MonoProcessors::AutoWah> mAutoWahs;

    #ifdef FLUX_ENGINE
public:
    virtual ImVec4 getColor() const  override { return ImVec4(0.6f, 0.1f, 0.4f, 1.0f);}

    // i'am so happy with this, before it was hell to add the gui's :D
    virtual void renderPaddle() override {
        DSP::AutoWahSettings currentSettings = this->getSettings();
        // currentSettings.volume.setKnobSettings(ImFlux::ksBlue); // NOTE only works here !
        if (currentSettings.DrawPaddle(this)) {
            this->setSettings(currentSettings);
        }
    }

    virtual void renderUIWide() override {
        DSP::AutoWahSettings currentSettings = this->getSettings();
        if (currentSettings.DrawUIWide(this)) {
            this->setSettings(currentSettings);
        }
    }
    virtual void renderUI() override {
        DSP::AutoWahSettings currentSettings = this->getSettings();
        if (currentSettings.DrawUI(this, 150.f)) {
            this->setSettings(currentSettings);
        }
    }

    #endif

}; //CLASS*/

}; //namespace
