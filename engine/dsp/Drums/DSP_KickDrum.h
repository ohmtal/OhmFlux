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
    float pitch;
    float decay;
    float click;
    float velocity;
    float drive;
};

struct KickSettings : public ISettings {
    AudioParam<float> pitch      { "Pitch", 50.0f, 30.0f, 100.0f, "%.1f Hz" };
    AudioParam<float> decay      { "Decay", 0.3f, 0.01f, 2.0f, "%.2f s" };
    AudioParam<float> click      { "Click", 0.5f, 0.0f, 1.0f, "%.2f" };
    //should be intern but can be set for some reason, can change in every triggerVelo!
    AudioParam<float> velocity   { "Velocity", 1.f, 0.1f, 1.0f, "%.2f" };
    AudioParam<float> drive      { "Drive", 1.5f, 1.0f, 5.0f, "%.1f" };

    KickSettings() = default;
    REGISTER_SETTINGS(KickSettings,  &pitch, &decay, &click, &velocity,  &drive)

    KickData getData() const {
        return { pitch.get(), decay.get(), click.get(), velocity.get(), drive.get() };
    }

    void setData(const KickData& data) {
        pitch.set(data.pitch);
        click.set(data.click);
        decay.set(data.decay);
        velocity.set(data.velocity);
        drive.set(data.drive);
    }
    std::vector<std::shared_ptr<IPreset>> getPresets() const override {
        return {
            std::make_shared<Preset<KickSettings, KickData>>("Default", KickData{ 50.0f, 0.3f, 0.5f, 1.f, 1.5f})
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
    virtual void triggerVelo(float velocity) override{
        mSettings.velocity.set(velocity);
        mKickSynth.trigger();
    }
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

        const float pitch = mSettings.pitch.get();
        const float click = mSettings.click.get();
        const float decay = mSettings.decay.get();
        const float velocity = mSettings.velocity.get();
        const float drive = mSettings.drive.get();
        const float samplerate = mSampleRate;


        for (int i = 0; i < numSamples; i += numChannels) {
            // generate once
            float monoOut = 0;
            monoOut = mKickSynth.processSample(pitch, decay, click, drive, velocity,  samplerate);

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
    bool mShowKnobs = true;
public:
    virtual ImVec4 getColor() const  override { return ImVec4(0.1f, 0.4f, 0.5f, 1.0f);} //FIXME check color

    virtual void renderCustomUI() override {
        ImFlux::ShadowText(getName().c_str());
        if (ImGui::CollapsingHeader("Settings", &mShowKnobs )){
            DSP::KickSettings s = this->getSettings();
            // if (s.DrawRackKnobs()) this->setSettings(s);
            if (s.DrawMiniKnobs()) this->setSettings(s);

        }


        ImFlux::ButtonParams bp = ImFlux::SLATEDARK_BUTTON;
        bp.size=ImVec2(100.f, 100.f);
        if (ImFlux::ButtonFancy("Trigger", bp))
            this->trigger();

    }


    virtual void renderPaddle() override {
        DSP::KickSettings s = this->getSettings();
        if (s.DrawPaddle(this)) {
            this->setSettings(s);
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
