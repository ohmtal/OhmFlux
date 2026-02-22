//-----------------------------------------------------------------------------
// Copyright (c) 2026 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Digital Sound Processing : Tremolo
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
#include "MonoProcessors/Tremolo.h"

#ifdef FLUX_ENGINE
#include <imgui.h>
#include <imgui_internal.h>
#include <gui/ImFlux.h>
#endif

namespace DSP {

struct TremoloData {
    float rate;
    float depth;
};

struct TremoloSettings : public ISettings {
    AudioParam<float> rate      { "Rate"   , 5.0f, 0.0f, 20.0f, "%.2f Hz" };
    AudioParam<float> depth     { "Depth"  , 0.5f, 0.0f,  1.0f, "%.1f" };



    TremoloSettings() = default;
    REGISTER_SETTINGS(TremoloSettings, &rate, &depth)

    TremoloData getData() const {
        return { rate.get(), depth.get()};
    }

    void setData(const TremoloData& data) {
        rate.set(data.rate);
        depth.set(data.depth);
    }
    //FIXME
    std::vector<std::shared_ptr<IPreset>> getPresets() const override {
        return {
            // std::make_shared<Preset<TremoloSettings, TremoloData>>("Default", ToneControlData{ 1.f, 0.0f, 0.0f, 0.0f})
        };
    }
};
class Tremolo: public Effect {
public:
    IMPLEMENT_EFF_CLONE(Tremolo)
    Tremolo(bool switchOn = false) :
        Effect(DSP::EffectType::Tremolo, switchOn)
        , mSettings()
        { }

    //----------------------------------------------------------------------
    virtual std::string getName() const override { return "Tremolo";}

    // //----------------------------------------------------------------------
    TremoloSettings& getSettings() { return mSettings; }
    // //----------------------------------------------------------------------
    void setSettings(const TremoloSettings& s) {
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
        const float depth = mSettings.depth.get();
        if (!isEnabled() || depth <= 0.001f) return;

        if (mTremolos.size() != (size_t) numChannels ) {
            mTremolos.resize(numChannels, DSP::MonoProcessors::Tremolo());
        }


        const float rate = mSettings.rate.get();

        for (int i = 0; i < numSamples; i++) {
            for ( int ch = 0; ch < numChannels; ch++) {
                buffer[i] = mTremolos[ch].process(buffer[i], rate,depth, mSampleRate);
            }
        }
    }

private:
    TremoloSettings mSettings;
    std::vector<MonoProcessors::Tremolo> mTremolos;

    #ifdef FLUX_ENGINE
public:
    virtual ImVec4 getColor() const  override { return ImVec4(0.4f, 0.4f, 0.1f, 1.0f);}

    // i'am so happy with this, before it was hell to add the gui's :D
    virtual void renderPaddle() override {
        DSP::TremoloSettings currentSettings = this->getSettings();
        // currentSettings.volume.setKnobSettings(ImFlux::ksBlue); // NOTE only works here !
        if (currentSettings.DrawPaddle(this)) {
            this->setSettings(currentSettings);
        }
    }

    virtual void renderUIWide() override {
        DSP::TremoloSettings currentSettings = this->getSettings();
        if (currentSettings.DrawUIWide(this)) {
            this->setSettings(currentSettings);
        }
    }
    virtual void renderUI() override {
        DSP::TremoloSettings currentSettings = this->getSettings();
        if (currentSettings.DrawUI(this, 150.f)) {
            this->setSettings(currentSettings);
        }
    }

    #endif

}; //CLASS*/

}; //namespace
