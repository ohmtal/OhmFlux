//-----------------------------------------------------------------------------
// Copyright (c) 2026 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Digital Sound Processing : Cymbals
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


struct CymbalsData {
    float pitch;
    float decay;
    float drive;
    float velocity;
};

struct CymbalsSettings : public ISettings {
    AudioParam<float> pitch      { "Pitch", 450.0f, 300.0f, 600.0f, "%.1f Hz" };
    AudioParam<float> decay      { "Decay", 1.f, 0.1f, 2.5f, "%.2f s" };
    AudioParam<float> drive      { "Drive", 1.5f, 1.0f, 5.0f, "%.1f" };
    //should be intern but can be set for some reason, can change in every triggerVelo!
    AudioParam<float> velocity   { "Velocity", 1.f, 0.1f, 1.0f, "%.2f" };


    CymbalsSettings() = default;
    REGISTER_SETTINGS(CymbalsSettings,  &pitch, &decay, &drive, &velocity)

    CymbalsData getData() const {
        return { pitch.get(), decay.get(), drive.get(), velocity.get() };
    }

    void setData(const CymbalsData& data) {
        pitch.set(data.pitch);
        decay.set(data.decay);
        drive.set(data.drive);
        velocity.set(data.velocity);
    }


    // std::vector<std::shared_ptr<IPreset>> getPresets() const override {
    //     return {
    //         std::make_shared<Preset<CymbalsSettings, CymbalsData>>("Default", CymbalsData{ 450.0f, 1.0f, 1.5f, 1.f})
    //     };
    // }
};

class Cymbals: public Effect {
private:
    CymbalsSettings mSettings;
    DrumSynth::CymbalsSynth mCymbalsSynth;



public:
    IMPLEMENT_EFF_CLONE(Cymbals)
    Cymbals(bool switchOn = false) :
        Effect(DSP::EffectType::Cymbals, switchOn)
        , mSettings()
        {
            mEffectName = "Cymbals";

        }

    //----------------------------------------------------------------------
    virtual void triggerVelo(float velocity) override{
        mSettings.velocity.set(velocity);
        mCymbalsSynth.trigger();
    }
    //----------------------------------------------------------------------
    virtual void trigger() override{
        mCymbalsSynth.trigger();
    }
    //----------------------------------------------------------------------
    // virtual std::string getName() const override { return "Cymbals";}

    // //----------------------------------------------------------------------
    CymbalsSettings& getSettings() { return mSettings; }
    // //----------------------------------------------------------------------
    void setSettings(const CymbalsSettings& s) {
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
            monoOut = mCymbalsSynth.processSample(pitch, decay, drive, velocity,  samplerate);

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

    virtual ImVec4 getDefaultColor() const  override { return ImVec4(0.8f, 0.8f, 0.2f, 1.0f);}

    virtual void renderDrumPad(ImVec2 size = { 80.f, 80.f}) override {
        float velo = 0.f;
        if (ImFlux::VelocityPad(getName().c_str(), size, getColorU32(), &velo)) {
            this->triggerVelo(velo);
        }
    }


    virtual void renderCustomUI() override {
        ImGui::PushID(this);
        ImGui::SeparatorText(getName().c_str());
        DSP::CymbalsSettings s = this->getSettings();
        if (s.DrawFaderH() ) this->setSettings(s);

        ImFlux::ButtonParams bp = ImFlux::SLATEDARK_BUTTON;
        bp.size=ImVec2(100.f, 80.f);
        ImFlux::SameLineCentered(bp.size.y + 20.f) ;
        ImGui::BeginGroup();
        renderDrumPad(bp.size);


        if (ImFlux::ButtonFancy("Reset", ImFlux::YELLOW_BUTTON.WithSize(ImVec2(bp.size.x,20.f)))) {
            this->setSettings(DSP::CymbalsSettings());
        }
        ImGui::EndGroup();

        ImGui::PopID();
    }


    virtual void renderPaddle() override {
        DSP::CymbalsSettings s = this->getSettings();
        if (s.DrawPaddle(this)) {
            this->setSettings(s);
        }
    }

    virtual void renderUIWide( ) override {
        DSP::CymbalsSettings currentSettings = this->getSettings();
        if (currentSettings.DrawUIWide(this)) {
            this->setSettings(currentSettings);
        }
    }
    virtual void renderUI() override {
        DSP::CymbalsSettings currentSettings = this->getSettings();
        if (currentSettings.DrawUI(this)) {
            this->setSettings(currentSettings);
        }
    }

    #endif

}; //CLASS

}; //namespace
