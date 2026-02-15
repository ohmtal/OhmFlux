//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Digital Sound Processing : DrumKit - Metronome with spice
//-----------------------------------------------------------------------------
#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <cmath>
#include <atomic>
#include <algorithm>
#include <filesystem>

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


        DrumKitData customData;
        // with customdata !!!
        std::vector<std::shared_ptr<IPreset>> getPresets() const override {
            std::vector<std::shared_ptr<IPreset>> list;

               list.push_back(std::make_shared<Preset<DrumKitSettings, DrumKitData>>("Custom", DrumKitData{}));

                list.push_back(std::make_shared<Preset<DrumKitSettings, DrumKitData>>("Standard Rock",
                    DrumKitData{ 0.8f, 110, 0x8888, 0x2222, 0xAAAA, 0x0000, 0x8080 }));

                list.push_back(std::make_shared<Preset<DrumKitSettings, DrumKitData>>("Driving Rock",
                    DrumKitData{ 0.8f,125, 0x8282, 0x2222, 0xEEEE, 0x0101, 0x0000 }));

                list.push_back(std::make_shared<Preset<DrumKitSettings, DrumKitData>>("Heavy Half-Time",
                    DrumKitData{ 0.8f,80, 0x8080, 0x0202, 0xAAAA, 0x0000, 0x8888 }));

                list.push_back(std::make_shared<Preset<DrumKitSettings, DrumKitData>>("Punk",
                    DrumKitData{ 0.8f,125, 34952, 0, 8738, 0x0000, 0x0000 }));

                list.push_back(std::make_shared<Preset<DrumKitSettings, DrumKitData>>("Simple Rock",
                    DrumKitData{ 0.8f,125, 34952, 0, 12850, 0x0000, 128 }));

                list.push_back(std::make_shared<Preset<DrumKitSettings, DrumKitData>>("Metronome",
                    DrumKitData{ 0.8f,125, 0, 0, 34952, 0x0000, 0 }));
            return list;
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
        // FIXME DEFINE A "MAGIC" SYSTEM and move this to settings!!
        bool saveToFile( std::string filePath )
        {
            try {
                std::ofstream ofs(filePath, std::ios::binary);
                DSP_STREAM_TOOLS::write_binary(ofs, DSP_STREAM_TOOLS::MakeMagic("DRUM"));
                mSettings.save(ofs);
                ofs.close();
                return true;
            } catch (const std::exception& e) {
                 std::cerr << e.what() << std::endl;
                return false;
            }
        }
        bool loadFromFile(std::string filePath) {
            if (!std::filesystem::exists(filePath)) {
                return false;
            }
            std::ifstream ifs;
            ifs.exceptions(std::ifstream::badbit | std::ifstream::failbit);
            try {
                ifs.open(filePath, std::ios::binary);
                uint32_t magic = 0;
                DSP::DSP_STREAM_TOOLS::read_binary(ifs,magic);
                if (magic != DSP_STREAM_TOOLS::MakeMagic("DRUM")) return false;
                if (!mSettings.load(ifs)) return false;

                // customdata hackfest
                // i dont check if it's not custom better than empty!
                // i dont save the customdata as a clone ... bsss
                mSettings.customData = mSettings.getData();

                return true;
            } catch (const std::ios_base::failure& e) {
                std::cerr << e.what() << std::endl;
                return false;
            } catch (const std::exception& e) {
                std::cerr << e.what() << std::endl;
                return false;
            }
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
        virtual void reset() override {
            mPhase = 0.0;
            mCurrentStep = -1;
        }

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
        virtual void renderPaddle( ) override {
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
        virtual void renderUI() override {  }

        //---------------- Special Window !!! ------------------
        virtual void renderSequencerWindow(bool* showWindow)  {
            if (!*showWindow) return;

            DSP::DrumKitSettings currentSettings = this->getSettings();

            ImGui::SetNextWindowSize(ImVec2(280, 400), ImGuiCond_FirstUseEver);
            ImGui::Begin("Drum Kit", showWindow ); //window start .........
            renderUIHeader();
            bool changed = false;
            bool presetChanged = false;


            // NOTE tricky access the AudioParams:
            using DrumParamPtr = AudioParam<uint16_t> DrumKitSettings::*;
            static const DrumParamPtr patterns[] = {
                &DrumKitSettings::kickPat,
                &DrumKitSettings::snarePat,
                &DrumKitSettings::hiHatPat,
                &DrumKitSettings::tomPat,
                &DrumKitSettings::cymbalsPat
            };

            ImGui::PushID(this); ImGui::PushID("UI_WIDE");
            // bool isEnabled = this->isEnabled();
            ImFlux::GradientBox(ImVec2(-FLT_MIN, -FLT_MIN),0.f);
            ImGui::Dummy(ImVec2(2,0)); ImGui::SameLine();

            ImGui::SameLine();
            // -------- stepper >>>>
            presetChanged |= mSettings.drawStepper(currentSettings, 260.f);
            changed |= presetChanged;

            ImGui::SameLine();
            if (ImFlux::ButtonFancy("RESET", ImFlux::SLATEDARK_BUTTON.WithSize(ImVec2(40.f, 20.f)) ))  {
                mSettings.resetToDefaults();
                this->reset();
                presetChanged = true;
                changed = true;
            }
            ImGui::Separator();

            changed |= currentSettings.vol.RackKnob();
            ImGui::SameLine();
            changed |= currentSettings.bpm.RackKnob();


            ImGui::SameLine();
            ImGui::BeginGroup();
            auto bpmButton = [&](uint16_t bpm) {
                if (ImFlux::ButtonFancy(std::format("{} bpm", bpm))) {
                    currentSettings.bpm.set(bpm);
                    return true;
                }
                return false;
            };
            changed |= bpmButton(60); ImGui::SameLine(); changed |= bpmButton(90);
            changed |= bpmButton(120); ImGui::SameLine(); changed |= bpmButton(144);
            ImGui::SetNextItemWidth(80.f);
            int bpmInt = (int)currentSettings.bpm.get();
            if (ImGui::InputInt("Bmp##manual", &bpmInt,1,15)) {
                bpmInt = DSP::clamp((uint16_t)bpmInt, currentSettings.bpm.getMin(), currentSettings.bpm.getMax());
                currentSettings.bpm.set(bpmInt);
                changed = true;

            }

            // changed |= bpmButton(120); ImGui::SameLine() changed |= bpmButton(144);
            ImGui::EndGroup();
            ImGui::SameLine();
            ImFlux::LCDNumber(currentSettings.bpm.get(), 3, 0, 24.0f);


            ImGui::Dummy(ImVec2(0.f, 30.f));


            ImGui::BeginGroup();

            for (auto ptr : patterns) {
                auto& param = currentSettings.*ptr;

                uint16_t bits = param.get();

                if (ImFlux::PatternEditor16Bit(param.getName().c_str(), &bits, mCurrentStep)) {
                    // dLog("Bits: %d", bits); //FIXME REMOVE THIS AGAIN
                    param.set(bits);
                    changed = true;
                }
            }
            ImGui::EndGroup();
            ImGui::NewLine();


            ImGui::PopID();ImGui::PopID();
            renderUIFooter();
            if (changed)  {
                //hackfest!!
                if ( !presetChanged ) {
                    mSettings.customData = currentSettings.getData();
                } else {
                    if (currentSettings.isMatchingPreset(currentSettings.getPresets()[0].get())) {
                        currentSettings.setData(mSettings.customData);
                    }

                }
                this->setSettings(currentSettings);

            }

            ImGui::End(); //window
        }

        #endif

    }; //class

}; //namespace
