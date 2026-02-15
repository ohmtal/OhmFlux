//-----------------------------------------------------------------------------
// Copyright (c) 2026 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Digital Sound Processing : Kick-Drum
//-----------------------------------------------------------------------------
#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <cmath>
#include <atomic>
#include <algorithm>

#include "../DSP_Effect.h"
#include "DrumSynth.h"

#ifdef FLUX_ENGINE
#include <imgui.h>
#include <imgui_internal.h>
#include <gui/ImFlux.h>
#endif


namespace DSP {

struct KickData {
    uint8_t kicktype;
    float pitch;
    float decay;
    float click;
    float velocity;
    float drive;
};

struct KickSettings : public ISettings {
    //types: normal, techno (with drive), normal varian
    AudioParam<uint8_t> kicktype  { "Type" , 0, 0, 2, "%d" }; // kick or Techno or variant (like 0)
    AudioParam<float> pitch      { "Pitch", 50.0f, 30.0f, 100.0f, "%.1f Hz" };
    AudioParam<float> decay      { "Decay", 0.3f, 0.01f, 2.0f, "%.2f s" };
    AudioParam<float> click      { "Click", 0.5f, 0.0f, 1.0f, "%.2f" };
    AudioParam<float> velocity   { "Velocity", 1.f, 0.1f, 1.0f, "%.2f" };
    AudioParam<float> drive      { "Drive", 1.5f, 1.0f, 5.0f, "%.1f" };

    KickSettings() = default;
    REGISTER_SETTINGS(KickSettings, &kicktype, &pitch, &decay, &click, &velocity,  &drive)

    KickData getData() const {
        return { kicktype.get(), pitch.get(), decay.get(), click.get(), velocity.get(), drive.get() };
    }

    void setData(const KickData& data) {
        kicktype.set(data.kicktype);
        pitch.set(data.pitch);
        click.set(data.click);
        decay.set(data.decay);
        velocity.set(data.velocity);
        drive.set(data.drive);
    }
    std::vector<std::shared_ptr<IPreset>> getPresets() const override {
        return {
            std::make_shared<Preset<KickSettings, KickData>>("Default", KickData{ 0, 50.0f, 0.3f, 0.5f, 1.f, 1.5f})
        };
    }
};

class KickDrum: public Effect {
public:
    IMPLEMENT_EFF_CLONE(KickDrum)
    KickDrum(bool switchOn = false) :
        Effect(DSP::EffectType::KickDrum, switchOn)
        , mSettings()
        {}

    //----------------------------------------------------------------------
    virtual void trigger() override{
        mKickSynth.trigger();
    }
    //----------------------------------------------------------------------
    virtual std::string getName() const override { return "Kick Drum";}

    // //----------------------------------------------------------------------
    KickSettings& getSettings() { return mSettings; }
    // //----------------------------------------------------------------------
    void setSettings(const KickSettings& s) {
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
        if (!isEnabled() )  return;

        const uint8_t kicktype = mSettings.kicktype.get();
        const float pitch = mSettings.pitch.get();
        const float click = mSettings.click.get();
        const float decay = mSettings.decay.get();
        const float velocity = mSettings.velocity.get();
        const float drive = mSettings.drive.get();
        const float sampleRate = mSampleRate;


        //NOTE special handling. we add stuff here. => i += numChannels
        for (int i = 0; i < numSamples; i += numChannels) {

            // generate once
            float monoOut = 0;
            switch (kicktype) {
                case 1: monoOut = mKickSynth.processSampleDrive(pitch, decay, click, drive,velocity,  sampleRate); break;
                case 2: monoOut = mKickSynth.processSample_variant(pitch, decay, click, velocity, sampleRate); break;
                default: monoOut = mKickSynth.processSample(pitch, decay, click, velocity, sampleRate); break;
            }

            // put into channels
            for (int ch = 0; ch < numChannels; ch++) {
                int idx = i + ch;
                if (idx < numSamples) {
                    buffer[idx] += monoOut;
                }
            }
        }
    }

private:
    KickSettings mSettings;
    DrumSynth::KickSynth mKickSynth;

    #ifdef FLUX_ENGINE
public:
    virtual ImVec4 getColor() const  override { return ImVec4(0.1f, 0.4f, 0.5f, 1.0f);} //FIXME check color

    // i'am so happy with this, before it was hell to add the gui's :D
    virtual void renderPaddle() override {
        DSP::KickSettings currentSettings = this->getSettings();
        if (currentSettings.DrawPaddle(this)) {
            this->setSettings(currentSettings);
        }
    }

    virtual void renderUIWide( ) override {
        DSP::KickSettings currentSettings = this->getSettings();
        if (currentSettings.DrawUIWide(this)) {
            this->setSettings(currentSettings);
        }
    }
    virtual void renderUI() override {
        DSP::KickSettings currentSettings = this->getSettings();
        if (currentSettings.DrawUI(this)) {
            this->setSettings(currentSettings);
        }
    }

    #endif

}; //CLASS

}; //namespace
