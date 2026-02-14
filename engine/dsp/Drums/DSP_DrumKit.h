//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Digital Sound Processing : DrumKit - Metronome with spice
// FIXME do i need to add the params of each DrumSynth here ??
//       for now i use the default values .... this is a basic Metronome :P
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


namespace DSP {

    struct DrumKitData {
        float vol;
        uint16_t bpm;
        uint16_t kickPat;
        uint16_t snarePat;
        uint16_t hiHatPat;
        uint16_t tomPat;
        uint16_t cymbalsPat;
    };

    // DrumKitData{ 125, 0x8282, 0x2222, 0xEEEE, 0x0101, 0x0000 }),
    struct DrumKitSettings : public ISettings {
        AudioParam<float> vol           { "Volume", 1.f, 0.1f, 1.f, "%.2f" };
        AudioParam<uint16_t> bpm        { "Beats per Minute", 125, 15, 360, "%.0f bpm" };
        AudioParam<uint16_t> kickPat    { "Kick Drum pattern",    0x8282, 0, UINT16_MAX, "%.0f" };
        AudioParam<uint16_t> snarePat   { "Snare Drum pattern",   0x2222, 0, UINT16_MAX, "%.0f" };
        AudioParam<uint16_t> hiHatPat   { "HiHat Drum pattern",   0xEEEE, 0, UINT16_MAX, "%.0f" };
        AudioParam<uint16_t> tomPat     { "Tom Drum pattern",     0x0101, 0, UINT16_MAX, "%.0f" };
        AudioParam<uint16_t> cymbalsPat { "Cymbals Drum pattern", 0x0000, 0, UINT16_MAX, "%.0f" };


        DrumKitSettings() = default;
        //NOTE: IMPORTANT !!
        REGISTER_SETTINGS(DrumKitSettings, &vol, &bpm, &kickPat, &snarePat, &hiHatPat, &tomPat, &cymbalsPat)

        DrumKitData getData() const {
            return {
                vol.get(),
                bpm.get(),
                kickPat.get(),
                snarePat.get(),
                hiHatPat.get(),
                tomPat.get(),
                cymbalsPat.get(),
            };
        }

        void setData(const DrumKitData& data) {
            vol.set(data.vol);
            bpm.set(data.bpm);
            kickPat.set(data.kickPat);
            snarePat.set(data.snarePat);
            hiHatPat.set(data.hiHatPat);
            tomPat.set(data.tomPat);
            cymbalsPat.set(data.cymbalsPat);

        }

        std::vector<std::shared_ptr<IPreset>> getPresets() const override {
            return {
                std::make_shared<Preset<DrumKitSettings, DrumKitData>>("Custom", DrumKitData{}),
                std::make_shared<Preset<DrumKitSettings, DrumKitData>>("Standard Rock",
                    DrumKitData{ 1.f, 110, 0x8888, 0x2222, 0xAAAA, 0x0000, 0x8080 }),

                std::make_shared<Preset<DrumKitSettings, DrumKitData>>("Driving Rock",
                    DrumKitData{ 1.f,125, 0x8282, 0x2222, 0xEEEE, 0x0101, 0x0000 }),

                std::make_shared<Preset<DrumKitSettings, DrumKitData>>("Heavy Half-Time",
                    DrumKitData{ 1.f,80, 0x8080, 0x0202, 0xAAAA, 0x0000, 0x8888 })
            };
        }
    };

    //========================== CLASS ============================

    class DrumKit: public Effect {
    public:
        IMPLEMENT_EFF_CLONE(DrumKit)

        DrumKit(bool switchOn = false) :
            Effect(DSP::EffectType::DrumKit, switchOn)
            ,mSettings()
        {}

        virtual std::string getName() const override { return "Drum Kit / Metronome";}

        // //----------------------------------------------------------------------
        DrumKitSettings& getSettings() { return mSettings; }
        // //----------------------------------------------------------------------
        void setSettings(const DrumKitSettings& s) {
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

            float    vol = mSettings.vol.get();
            uint16_t bpm = mSettings.bpm.get();
            uint16_t kickPat = mSettings.kickPat.get();
            uint16_t snarePat = mSettings.snarePat.get();
            uint16_t hiPat = mSettings.hiHatPat.get();
            uint16_t tomPat = mSettings.tomPat.get();
            uint16_t cymPat = mSettings.cymbalsPat.get();


            double samplesPerStep = (mSampleRate * 60.0) / (bpm * 4.0);


            for (int i = 0; i < numSamples; i += numChannels) {
                // Phase-Accumulator
                mPhase += 1.0;
                int step = static_cast<int>(mPhase / samplesPerStep) % 16;

                if (step != mCurrentStep) {
                    mCurrentStep = step;
                    if (kickPat & (1 << (15 - step))) { mKick.trigger(); }
                    if (snarePat & (1 << (15 - step))) { mSnare.trigger(); }
                    if (hiPat & (1 << (15 - step))) { mHiHat.trigger(); }
                    if (tomPat & (1 << (15 - step))) { mTomTom.trigger(); }
                    if (cymPat & (1 << (15 - step))) { mCymbals.trigger(); }

                }

                float out = 0.f;

                //using  default values here ...
                out +=  mKick.processSample(50.f, 0.3f, 0.5f, 1.f, mSampleRate);
                out +=  mSnare.processSample(180.f, 0.2f, 0.7f, mSampleRate);
                out +=  mHiHat.processSample(0.05f, 3000.f, mSampleRate);
                out +=  mTomTom.processSample(100.f,0.3f,0.4f, mSampleRate);
                out +=  mCymbals.processSample(0.5f,4000.f, mSampleRate);

                out *= vol;

                // Und addieren es auf alle Kanäle
                for (int ch = 0; ch < numChannels; ch++) {
                    if ((i + ch) < numSamples) {
                        buffer[i + ch] += out;
                    }
                }
            }
        }
        //----------------------------------------------------------------------
        // void Trigger() {
        //         mTriggered = true;
        // }

    private:
        DrumKitSettings mSettings;
        DrumSynth::KickSynth mKick;
        DrumSynth::SnareSynth mSnare;
        DrumSynth::HiHatSynth mHiHat;
        DrumSynth::TomSynth mTomTom;
        DrumSynth::CymbalsSynth mCymbals;

        double mPhase = 0.0;    // position
        int mCurrentStep = -1;  // triggered step

        #ifdef FLUX_ENGINE
    public:
        virtual ImVec4 getColor() const  override { return ImVec4(0.1f, 0.4f, 0.8f, 1.0f);} //FIXME check color

        // i'am so happy with this, before it was hell to add the gui's :D
        virtual void renderPaddle() override {
            DSP::DrumKitSettings currentSettings = this->getSettings();
            if (currentSettings.DrawPaddle(this)) {
                this->setSettings(currentSettings);
            }
        }

        virtual void renderUIWide() override {
            DSP::DrumKitSettings currentSettings = this->getSettings();
            if (currentSettings.DrawUIWide(this)) {
                this->setSettings(currentSettings);
            }
        }
        virtual void renderUI() override {
            DSP::DrumKitSettings currentSettings = this->getSettings();
            if (currentSettings.DrawUI(this, 200.f)) {
                this->setSettings(currentSettings);
            }
        }

        #endif

    }; //class

}; //namespace
