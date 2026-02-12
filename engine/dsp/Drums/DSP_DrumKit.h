//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Digital Sound Processing : Kick Drum
//-----------------------------------------------------------------------------
// TODO build a trigger system
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
#include "KickSynh.h"


namespace DSP {

    struct DrumKitData {
        float bpm;
    };

    struct DrumKitSettings : public ISettings {
        AudioParam<float> bpm  { "Beats per Minute", 60.0f, 15.0f, 360.0f, "%3.f bpm" };

        DrumKitSettings() = default;
        REGISTER_SETTINGS(DrumKitSettings, &bpm)

        DrumKitData getData() const {
            return { bpm.get() };
        }

        void setData(const DrumKitData& data) {
            bpm.set(data.bpm);
        }

        std::vector<std::shared_ptr<IPreset>> getPresets() const override {
            return {
                std::make_shared<Preset<DrumKitSettings, DrumKitData>>("Default", DrumKitData{60.f})
            };
        }
    };




    class DrumKit: public Effect {
    public:
        IMPLEMENT_EFF_CLONE(DrumKit)

        DrumKit(bool switchOn = false) :
            Effect(switchOn)
            ,mSettings()
        {
            mSampleRate = getSampleRateF();
        }

        virtual std::string getName() const override { return "Drum Kit";}
        DSP::EffectType getType() const override { return DSP::EffectType::DrumKit;}

        // //----------------------------------------------------------------------
        DrumKitSettings& getSettings() { return mSettings; }
        // //----------------------------------------------------------------------
        void setSettings(const DrumKitSettings& s) {
            mSettings = s;
        }
        //----------------------------------------------------------------------
        virtual void setSampleRate(float sampleRate) override {
            mSampleRate = sampleRate;
        }
        //----------------------------------------------------------------------
        void save(std::ostream& os) const override {
            Effect::save(os);              // Save mEnabled
            // mSettings.save(os);       // Save Settings
        }
        //----------------------------------------------------------------------
        bool load(std::istream& is) override {
            if (!Effect::load(is)) return false; // Load mEnabled
            // return mSettings.load(is);      // Load Settings
            return true;
        }
        //----------------------------------------------------------------------
        virtual void process(float* buffer, int numSamples, int numChannels) override {
            if (!isEnabled() )  return;
            // const float pitch = mSettings.pitch.get();
            // const float click = mSettings.click.get();
            // const float decay = mSettings.decay.get();
            //FIXME ... should do it different !!!
        }
        //----------------------------------------------------------------------
        // void Trigger() {
        //         mTriggered = true;
        // }

    private:
        DSP::Drums::KickSynth mKick;
        DrumKitSettings mSettings;
        float mSampleRate;
        // bool mTriggered = false;

    }; //class

}; //namespace
