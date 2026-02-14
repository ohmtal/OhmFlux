//-----------------------------------------------------------------------------
// Copyright (c) 2026 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Digital Sound Processing : ToneControl
// TODO:
// [ ] finish class
// [~] NOT! Test FastMath biquad
// [ ] port BiQuad ... do i really need it 3times ?!
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
#include "MonoProcessors/ToneControl.h"

#ifdef FLUX_ENGINE
#include <imgui.h>
#include <imgui_internal.h>
#include <gui/ImFlux.h>
#endif


namespace DSP {

struct ToneControlData {
    float volume;
    float bass;
    float treble;
    float presence;
};

struct ToneControlSettings : public ISettings {
    AudioParam<float> volume      { "Volume"   , 1.f, 0.0f,  2.0f, "%.2f" };
    AudioParam<float> bass        { "Bass"     , 0.f,-15.0f,15.0f, "%.2f db" };
    AudioParam<float> treble      { "Treble"   , 0.f,-15.0f,15.0f, "%.2f db" };
    AudioParam<float> presence    { "Presence" , 0.f,-15.0f,15.0f, "%.2f db" };



    ToneControlSettings() = default;
    REGISTER_SETTINGS(ToneControlSettings, &volume, &bass, &treble, &presence)

    ToneControlData getData() const {
        return { volume.get(), bass.get(), treble.get(), presence.get()};
    }

    void setData(const ToneControlData& data) {
        volume.set(data.volume);
        bass.set(data.bass);
        treble.set(data.treble);
        presence.set(data.presence);
    }
    std::vector<std::shared_ptr<IPreset>> getPresets() const override {
        return {
            std::make_shared<Preset<ToneControlSettings, ToneControlData>>("Default", ToneControlData{ 1.f, 0.0f, 0.0f, 0.0f})
        };
    }
};
class ToneControl: public Effect {
public:
    IMPLEMENT_EFF_CLONE(ToneControl)
    ToneControl(bool switchOn = false) :
        Effect(DSP::EffectType::ToneControl, switchOn)
        , mSettings()
        {}

    //----------------------------------------------------------------------
    virtual std::string getName() const override { return "Volume and Tone ";}

    // //----------------------------------------------------------------------
    ToneControlSettings& getSettings() { return mSettings; }
    // //----------------------------------------------------------------------
    void setSettings(const ToneControlSettings& s) {
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
    virtual void process(float* buffer, int numSamples, int numChannels) override {
        const float volume = mSettings.volume.get();

        if (!isEnabled() || volume <= 0.001f) return;

        const float bass = mSettings.bass.get();
        const float treble = mSettings.treble.get();
        const float presence = mSettings.presence.get();

        // no channel handling needed here ...
        for (int i = 0; i < numSamples; i++) {
            buffer[i] = mToneControl.process(buffer[i], volume,  bass, treble, presence, mSampleRate);
        }
    }

private:
    ToneControlSettings mSettings;
    MonoProcessors::ToneControl mToneControl;

    #ifdef FLUX_ENGINE
public:
    virtual ImVec4 getColor() const  override { return ImVec4(0.1f, 0.4f, 0.5f, 1.0f);} //FIXME check color

    // i'am so happy with this, before it was hell to add the gui's :D
    virtual void renderPaddle() override {
        DSP::ToneControlSettings currentSettings = this->getSettings();
        if (currentSettings.DrawPaddle(this)) {
            this->setSettings(currentSettings);
        }
    }

    virtual void renderUIWide() override {
        DSP::ToneControlSettings currentSettings = this->getSettings();
        if (currentSettings.DrawUIWide(this)) {
            this->setSettings(currentSettings);
        }
    }
    virtual void renderUI() override {
        DSP::ToneControlSettings currentSettings = this->getSettings();
        if (currentSettings.DrawUI(this)) {
            this->setSettings(currentSettings);
        }
    }

    #endif

}; //CLASS*/

}; //namespace
