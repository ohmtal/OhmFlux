//-----------------------------------------------------------------------------
// Copyright (c) 2026 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Digital Sound Processing : HiHat
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


struct HiHatData {
    float pitch;
    float decay;
    float drive;
    float velocity;
};

struct HiHatSettings : public ISettings {
    //default params of open :
    AudioParam<float> pitch      { "Pitch", 8000.0f, 2000.0f, 18000.0f, "%.f Hz" };
    AudioParam<float> decay      { "Decay", 0.5f, 0.01f, 1.f, "%.2f s" };
    AudioParam<float> drive      { "Drive", 3.0f, 1.0f, 15.0f, "%.2f" };
    //should be intern but can be set for some reason, can change in every triggerVelo!
    AudioParam<float> velocity   { "Velocity", 1.f, 0.1f, 1.0f, "%.2f" };


    HiHatSettings() = default;
    REGISTER_SETTINGS(HiHatSettings,  &pitch, &decay, &drive, &velocity)

    HiHatData getData() const {
        return { pitch.get(), decay.get(), drive.get(), velocity.get() };
    }

    void setData(const HiHatData& data) {
        pitch.set(data.pitch);
        decay.set(data.decay);
        drive.set(data.drive);
        velocity.set(data.velocity);
    }


    std::vector<std::shared_ptr<IPreset>> getPresets() const override {
        return {
            //NOTE: keep the first two names !! i used em in lookups
            std::make_shared<Preset<HiHatSettings, HiHatData>>("Open", HiHatData{ 8000.0f, 0.5f, 3.0f, 1.f}),
            std::make_shared<Preset<HiHatSettings, HiHatData>>("Closed", HiHatData{ 14000.0f, 0.05f, 5.0f, 1.f}),
        };
    }
};

class HiHat: public Effect {
private:
    HiHatSettings mSettings;
    DrumSynth::HiHatSynth mHiHatSynth;



public:
    IMPLEMENT_EFF_CLONE(HiHat)
    HiHat(bool switchOn = false) :
        Effect(DSP::EffectType::HiHat, switchOn)
        , mSettings()
        {
            mEffectName = "HiHat";
        }

    //----------------------------------------------------------------------
    virtual void triggerVelo(float velocity) override{
        mSettings.velocity.set(velocity);
        mHiHatSynth.trigger();
    }
    //----------------------------------------------------------------------
    virtual void trigger() override{
        mHiHatSynth.trigger();
    }
    //----------------------------------------------------------------------
    // virtual std::string getName() const override { return "HiHat";}

    // //----------------------------------------------------------------------
    HiHatSettings& getSettings() { return mSettings; }
    // //----------------------------------------------------------------------
    void setSettings(const HiHatSettings& s) {
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
        const float decay = mSettings.decay.get();
        const float velocity = mSettings.velocity.get();
        const float drive = mSettings.drive.get();
        const float samplerate = mSampleRate;


        for (int i = 0; i < numSamples; i += numChannels) {
            // generate once
            float monoOut = 0;
            monoOut = mHiHatSynth.processSample(pitch, decay, drive, velocity,  samplerate);

            // put into channels
            for (int ch = 0; ch < numChannels; ch++) {
                int idx = i + ch;
                if (idx < numSamples) {
                    buffer[idx] += monoOut;
                }
            }
        }
    }

public:
    #ifdef FLUX_ENGINE

    virtual ImVec4 getDefaultColor() const  override { return ImVec4(0.2f, 0.8f, 0.2f, 1.0f);}

    virtual void renderDrumPad(ImVec2 size = { 80.f, 80.f}) override {
        float velo = 0.f;
        if (ImFlux::VelocityPad(getName().c_str(), size, getColorU32(), &velo)) {
            this->triggerVelo(velo);
        }
    }


    virtual void renderCustomUI() override {
        ImGui::PushID(this);
        ImGui::SeparatorText(getName().c_str());
        DSP::HiHatSettings s = this->getSettings();
        bool changed = false;

        changed |= s.DrawFaderH();

        ImFlux::ButtonParams bp = ImFlux::SLATEDARK_BUTTON;
        bp.size=ImVec2(100.f, 80.f);
        ImFlux::SameLineCentered(bp.size.y + 20.f) ;
        ImGui::BeginGroup();
        renderDrumPad(bp.size);

        if (ImFlux::ButtonFancy("Open", ImFlux::YELLOW_BUTTON.WithSize(ImVec2(bp.size.x / 2.f,20.f)))) {
            if (auto p = this->getSettings().findPresetByName("Open"))
            {
                p->apply(s);
                changed = true;
            }
        }
        ImGui::SameLine();
        if (ImFlux::ButtonFancy("Closed", ImFlux::YELLOW_BUTTON.WithSize(ImVec2(bp.size.x / 2.f,20.f)))) {
            if (auto p = this->getSettings().findPresetByName("Closed"))
            {
                p->apply(s);
                changed = true;
            }
        }
        ImGui::EndGroup();

        ImGui::PopID();
        if (changed) this->setSettings(s);
    }


    virtual void renderPaddle() override {
        DSP::HiHatSettings s = this->getSettings();
        if (s.DrawPaddle(this)) {
            this->setSettings(s);
        }
    }

    virtual void renderUIWide( ) override {
        DSP::HiHatSettings currentSettings = this->getSettings();
        if (currentSettings.DrawUIWide(this)) {
            this->setSettings(currentSettings);
        }
    }
    virtual void renderUI() override {
        DSP::HiHatSettings currentSettings = this->getSettings();
        if (currentSettings.DrawUI(this)) {
            this->setSettings(currentSettings);
        }
    }

    #endif

}; //CLASS

}; //namespace
