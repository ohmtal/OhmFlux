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
#include "Processors/Looper.h"


namespace DSP {

    struct DrumKitData {
        float vol;
        uint16_t bpm;
        uint16_t kickPat;
        uint16_t snarePat;
        uint16_t hiHatClosedPat;
        uint16_t hiHatOpenPat;
        uint16_t tomPat;
        uint16_t cymbalsPat;
    };

    // DrumKitData{ 125, 0x8282, 0x2222, 0xEEEE, 0x0101, 0x0000 }),
    struct DrumKitSettings : public ISettings {
        AudioParam<float> vol           { "Volume", 0.5f, 0.1f, 1.f, "%.2f" };
        AudioParam<uint16_t> bpm        { "Beats per Minute", 125, 15, 360, "%.0f bpm" };
        AudioParam<uint16_t> kickPat    { "Kick Drum pattern",    0x8282, 0, UINT16_MAX, "%.0f" };
        AudioParam<uint16_t> snarePat   { "Snare Drum pattern",   0x2222, 0, UINT16_MAX, "%.0f" };
        AudioParam<uint16_t> hiHatClosedPat   { "HiHat Closed Drum pattern",   0xEEEE, 0, UINT16_MAX, "%.0f" };
        AudioParam<uint16_t> hiHatOpenPat   { "HiHat Open Drum pattern",   0x0000, 0, UINT16_MAX, "%.0f" };
        AudioParam<uint16_t> tomPat     { "Tom Drum pattern",     0x0101, 0, UINT16_MAX, "%.0f" };
        AudioParam<uint16_t> cymbalsPat { "Cymbals Drum pattern", 0x0000, 0, UINT16_MAX, "%.0f" };


        DrumKitSettings() = default;
        //NOTE: IMPORTANT !!
        REGISTER_SETTINGS(DrumKitSettings, &vol, &bpm, &kickPat, &snarePat, &hiHatClosedPat, &hiHatOpenPat, &tomPat, &cymbalsPat)

        DrumKitData getData() const {
            return {
                vol.get(),
                bpm.get(),
                kickPat.get(),
                snarePat.get(),
                hiHatClosedPat.get(),
                hiHatOpenPat.get(),
                tomPat.get(),
                cymbalsPat.get(),
            };
        }

        void setData(const DrumKitData& data) {
            vol.set(data.vol);
            bpm.set(data.bpm);
            kickPat.set(data.kickPat);
            snarePat.set(data.snarePat);
            hiHatClosedPat.set(data.hiHatClosedPat);
            hiHatOpenPat.set(data.hiHatOpenPat);
            tomPat.set(data.tomPat);
            cymbalsPat.set(data.cymbalsPat);

        }


        DrumKitData customData;
        // with customdata !!!
        std::vector<std::shared_ptr<IPreset>> getPresets() const override {
            std::vector<std::shared_ptr<IPreset>> list;

               list.push_back(std::make_shared<Preset<DrumKitSettings, DrumKitData>>("Custom", DrumKitData{}));

                list.push_back(std::make_shared<Preset<DrumKitSettings, DrumKitData>>("Standard Rock",
                    DrumKitData{ 0.5f, 110, 0x8888, 0x2222, 0xAAAA,0x0000, 0x0000, 0x8080 }));

                list.push_back(std::make_shared<Preset<DrumKitSettings, DrumKitData>>("Driving Rock",
                    DrumKitData{ 0.5f,125, 0x8282, 0x2222, 0xEEEE,0x0000,  0x0101, 0x0000 }));

                list.push_back(std::make_shared<Preset<DrumKitSettings, DrumKitData>>("Heavy Half-Time",
                    DrumKitData{ 0.5f,80, 0x8080, 0x0202,0xAAAA, 0x0000, 0x0000, 0x8888 }));

                list.push_back(std::make_shared<Preset<DrumKitSettings, DrumKitData>>("Punk",
                    DrumKitData{ 0.5f,125, 34952, 0, 0x0000, 8738, 0x0000, 0x0000 }));

                list.push_back(std::make_shared<Preset<DrumKitSettings, DrumKitData>>("Simple Rock",
                    DrumKitData{ 0.5f,125, 34952, 0,12850, 0x0000,  0x0000, 128 }));

                list.push_back(std::make_shared<Preset<DrumKitSettings, DrumKitData>>("Metronome",
                    DrumKitData{ 0.5f,125, 0, 0, 34952,0x0000,  0x0000, 0 }));
            return list;
        }


        virtual uint8_t getDataVersion() const  override { return 2; };


    };

    //========================== CLASS ============================

    class DrumKit: public Effect {
    private:
        DrumKitData mLooperStartTicks = DrumKitData{ 0.5f,125, 0, 0, 34952,0x0000,  0x0000, 0 };
        DrumKitSettings mLooperSavCurrent;
        Processors::LooperMode mLooperMode = Processors::LooperMode::Off;
        float mLooperSeconds = 30.f;
        bool mLooperInitDone = false;
        int mLooperInitSteps = -1;
        Processors::Looper mLooper;

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
        // NOTE WHY DID I DO THAT ?? i can save the customdata in save!!!
        // bool saveToFile( std::string filePath )
        // {
        //     try {
        //         std::ofstream ofs(filePath, std::ios::binary);
        //         DSP_STREAM_TOOLS::write_binary(ofs, DSP_STREAM_TOOLS::MakeMagic("DRUM"));
        //         mSettings.save(ofs);
        //         ofs.close();
        //         return true;
        //     } catch (const std::exception& e) {
        //          std::cerr << e.what() << std::endl;
        //         return false;
        //     }
        // }
        // bool loadFromFile(std::string filePath) {
        //     if (!std::filesystem::exists(filePath)) {
        //         return false;
        //     }
        //     std::ifstream ifs;
        //     ifs.exceptions(std::ifstream::badbit | std::ifstream::failbit);
        //     try {
        //         ifs.open(filePath, std::ios::binary);
        //         uint32_t magic = 0;
        //         DSP::DSP_STREAM_TOOLS::read_binary(ifs,magic);
        //         if (magic != DSP_STREAM_TOOLS::MakeMagic("DRUM")) return false;
        //         if (!mSettings.load(ifs)) return false;
        //
        //         // customdata hackfest
        //         // i dont check if it's not custom better than empty!
        //         // i dont save the customdata as a clone ... bsss
        //         mSettings.customData = mSettings.getData();
        //
        //         return true;
        //     } catch (const std::ios_base::failure& e) {
        //         std::cerr << e.what() << std::endl;
        //         return false;
        //     } catch (const std::exception& e) {
        //         std::cerr << e.what() << std::endl;
        //         return false;
        //     }
        // }
     //----------------------------------------------------------------------

        void save(std::ostream& os) const override {
            Effect::save(os);              // Save mEnabled
            mSettings.save(os);       // Save Settings

            //... VERSION 2
            mLooper.save(os);

        }
        //----------------------------------------------------------------------
        bool load(std::istream& is) override {
            if (!Effect::load(is)) return false; // Load mEnabled
            if (! mSettings.load(is) ) return false;

            if ( mSettings.mReadVersion >=2 )
            {
                if (!mLooper.load(is)) return false;
            }

            return true;
        }

        //----------------------------------------------------------------------
        void stopLooper() {

            // we stopped im init restore drum data
            if (!mLooperInitDone) {
               mSettings.setData(mLooperSavCurrent.getData());
            }

            mLooperMode = Processors::LooperMode::Off;
            mLooper.setMode(mLooperMode);
        }
        //----------------------------------------------------------------------
        void startLooperRecording(float seconds = 30.f, bool doOverDup = false) {
            setEnabled(false); //stop if we are playing

            // update vol + bpm of looper start ticks
            mLooperStartTicks.vol = mSettings.vol.get();
            mLooperStartTicks.bpm = mSettings.bpm.get();
            // save current settings
            mLooperSavCurrent.setData(mSettings.getData());
            // set ticks as new loop
            mSettings.setData(mLooperStartTicks);
            mLooperInitDone = false;
            mLooperInitSteps = -1;
            if ( doOverDup ) mLooperMode = Processors::LooperMode::RecordingOverDup;
            else mLooperMode = Processors::LooperMode::Recording;

            mLooperSeconds = seconds;

            // now we need to start the ticker, on the 5th tick we
            // restore the  mLooperSavCurrent and really start the looper
            // recording

            setEnabled(true);

        }
        //----------------------------------------------------------------------
        void startLooperPlaying() {
            setEnabled(false); //stop if we are playing
            mLooperInitDone = true;  // we have no init here
            mLooperMode = Processors::LooperMode::Playing;
            mLooper.setMode(mLooperMode);
            setEnabled(true);
        }
        //----------------------------------------------------------------------
        bool onLooperStep(int numChannels) {
            //Looper before drums are mixed in
            // handle looper init phase:
            // handle the steps
            if ( !mLooperInitDone  && (mLooperMode == Processors::LooperMode::Recording
                || mLooperMode == Processors::LooperMode::RecordingOverDup))
            {
                mLooperInitSteps++;
                if ( mLooperInitSteps == 16)
                {
                    mLooperInitDone = true;
                    // restore drums
                    mSettings.setData(mLooperSavCurrent.getData());
                    // init the looper ...
                    mLooper.initWithSec(mLooperSeconds, mSettings.bpm.get() , mSampleRate, numChannels, 4 );
                    // start the looper
                    mLooper.setMode(mLooperMode);
                    // pray ... :P
                    return true;
                }
            }
            return false;
        }
        //----------------------------------------------------------------------
        virtual void process(float* buffer, int numSamples, int numChannels) override {
            if (!isEnabled() )  return;


            float    vol = mSettings.vol.get();
            uint16_t bpm = mSettings.bpm.get();
            uint16_t kickPat = mSettings.kickPat.get();
            uint16_t snarePat = mSettings.snarePat.get();

            uint16_t hiHatClosedPat = mSettings.hiHatClosedPat.get();
            uint16_t hiHatOpenPat = mSettings.hiHatOpenPat.get();

            uint16_t tomPat = mSettings.tomPat.get();
            uint16_t cymPat = mSettings.cymbalsPat.get();
            float    sampleRate = mSampleRate;

            const float beatsPerBar = 4.f;
            double samplesPerStep = (mSampleRate * 60.0) / (bpm * beatsPerBar);

            uint16_t stepper = 0;

            //Looper >>>
            if (mLooperMode != Processors::LooperMode::Off && mLooperInitDone) {
                mLooper.process(buffer, numSamples, numChannels);
            }
            //<<<<<


            for (int i = 0; i < numSamples; i += numChannels) {
                float out = 0.f;
                // Phase-Accumulator
                mPhase += 1.0;
                int step = static_cast<int>(mPhase / samplesPerStep) % 16;
                if (step != mCurrentStep) {

                    //NOTE: looper => we call the init looper state check
                    if (onLooperStep(numChannels)) {
                        // we need to update here !!
                        kickPat = mSettings.kickPat.get();
                        snarePat = mSettings.snarePat.get();

                        hiHatClosedPat = mSettings.hiHatClosedPat.get();
                        hiHatOpenPat = mSettings.hiHatOpenPat.get();

                        tomPat = mSettings.tomPat.get();
                        cymPat = mSettings.cymbalsPat.get();
                    }
                    mCurrentStep = step;
                    stepper = (1 << (15 - step));

                    if (kickPat & stepper) { mKick.trigger(); }
                    if (snarePat & stepper) { mSnare.trigger(); }

                    if (hiHatOpenPat & stepper) {mHiHatOpen.trigger(); }
                    if (hiHatClosedPat & stepper) {mHiHatClosed.trigger(); mHiHatOpen.stop();}

                    if (tomPat & stepper) { mTomTom.trigger(); }
                    if (cymPat & stepper) { mCymbals.trigger(); }
                }

                //using  default values here ...
                out +=  mKick.processSample(50.f, 0.3f, 0.5f, 1.f, 1.f, sampleRate);
                out +=  mSnare.processSample(180.f, 0.2f, 0.7f, 1.5f, 1.f, sampleRate);
                out +=  mHiHatClosed.processSample(14000.f, 0.2f, 5.f, 1.f, sampleRate);
                out +=  mHiHatOpen.processSample(8000.f, 0.5f, 3.f, 0.5f, sampleRate);
                out +=  mTomTom.processSample(120.f,0.4f,0.50f,1.0f, sampleRate);
                out +=  mCymbals.processSample(450.f, 1.0f, 1.5f, 0.5f, sampleRate);
                out *= vol;
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
        DrumSynth::HiHatSynth mHiHatClosed;
        DrumSynth::HiHatSynth mHiHatOpen;
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

            ImGui::SetNextWindowSize(ImVec2(550, 400), ImGuiCond_FirstUseEver);
            ImGui::Begin("Drum Kit", showWindow ); //window start .........


            // LOOPER TEST , FIXME Gui!
            // handle mMode ...

            if ( mLooperMode == Processors::LooperMode::Off)
            {
                if (ImGui::Button(" START Recording 1sec")) {
                    startLooperRecording(1.f);
                }
                if (ImGui::Button(" START Recording 30sec")) {
                    startLooperRecording();
                }
                if (!mLooper.bufferFilled()) ImGui::BeginDisabled();
                if (ImGui::Button(" START Playing")) {
                    startLooperPlaying();
                }
                if (!mLooper.bufferFilled())ImGui::EndDisabled();

            } else {
                if (ImGui::Button(" STOP LOOPER")) {
                    stopLooper();
                }

            }
            ImGui::TextDisabled("LOOPER STATE = %d (initdone = %d, initsteps = %d)", mLooperMode,mLooperInitDone, mLooperInitSteps);
            bool isRecording = ( mLooper.getMode() == Processors::LooperMode::Recording || mLooper.getMode() == Processors::LooperMode::RecordingOverDup);
            ImFlux::DrawLED("Recording", isRecording, ImFlux::LED_RED_ALERT);
            ImGui::SameLine();
            ImFlux::PeakMeter(mLooper.getPosition(),ImVec2(150.f, 7.f));

            ImGui::SeparatorText("LIVE INFORMATION");
            Processors::LooperPositionInfo info = mLooper.getPositionInfo();
            // ImGui::TextDisabled("BAR/BEAT/ %03d/%02d/%d", info.step, info.beat, info.bar);
            ImFlux::LCDNumber(info.bar, 2, 0, 24.f, IM_COL32(200,20,20,255)); ImFlux::Hint("BAR (measure)"); ImGui::SameLine();
            ImFlux::LCDNumber(info.beat, 2, 0, 24.f, IM_COL32(20,200,20,255)); ImFlux::Hint("BEAT"); ImGui::SameLine();
            ImFlux::LCDNumber(info.step, 3, 0, 24.f, IM_COL32(200,20,200,255)); ImFlux::Hint("STEP"); ImGui::SameLine();

            ImFlux::LCDNumber(info.seconds, 4, 1, 24.f, IM_COL32(20,200,200,255)); ImFlux::Hint("Seconds");

            {
                ImGui::SeparatorText("BUFFER INFORMATION:");
                Processors::LooperPositionInfo info = mLooper.getInfo();
                // ImGui::TextDisabled("BAR/BEAT/ %03d/%02d/%d", info.step, info.beat, info.bar);
                ImFlux::LCDNumber(info.bar, 2, 0, 24.f, IM_COL32(100,20,20,255)); ImFlux::Hint("BAR (measure)"); ImGui::SameLine();
                ImFlux::LCDNumber(info.beat, 2, 0, 24.f, IM_COL32(20,100,20,255)); ImFlux::Hint("BEAT"); ImGui::SameLine();
                ImFlux::LCDNumber(info.step, 3, 0, 24.f, IM_COL32(100,20,100,255)); ImFlux::Hint("STEP"); ImGui::SameLine();

                ImFlux::LCDNumber(info.seconds, 4, 1, 24.f, IM_COL32(20,100,100,255)); ImFlux::Hint("Seconds");


            }





            // renderUIHeader();
            currentSettings.DrawPaddleHeader(this, 400);

            bool changed = false;
            bool presetChanged = false;


            // NOTE tricky access the AudioParams:
            using DrumParamPtr = AudioParam<uint16_t> DrumKitSettings::*;
            static const DrumParamPtr patterns[] = {
                &DrumKitSettings::kickPat,
                &DrumKitSettings::snarePat,
                &DrumKitSettings::hiHatClosedPat,
                &DrumKitSettings::hiHatOpenPat,
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
            // ImGui::SameLine();
            // changed |= currentSettings.bpm.RackKnob();


            ImGui::SameLine();
            ImGui::BeginGroup();
            auto bpmButton = [&](uint16_t bpm) {
                if (ImFlux::ButtonFancy(std::format("{} bpm", bpm))) {
                    currentSettings.bpm.set(bpm);
                    return true;
                }
                return false;
            };
            changed |= bpmButton(60); ImGui::SameLine(); changed |= bpmButton(72);
            changed |= bpmButton(118); ImGui::SameLine(); changed |= bpmButton(125);
            changed |= bpmButton(144); ImGui::SameLine(); changed |= bpmButton(160);
            ImGui::EndGroup();

            ImGui::SameLine();
            ImGui::BeginGroup();
            ImGui::SetNextItemWidth(120.f);
            int bpmInt = (int)currentSettings.bpm.get();
            if (ImGui::InputInt("Bmp##manual", &bpmInt,1,15)) {
                bpmInt = DSP::clamp((uint16_t)bpmInt, currentSettings.bpm.getMin(), currentSettings.bpm.getMax());
                currentSettings.bpm.set(bpmInt);
                changed = true;

            }

            // changed |= bpmButton(120); ImGui::SameLine() changed |= bpmButton(144);
            // ImGui::SameLine();
            ImFlux::LCDNumber(currentSettings.bpm.get(), 3, 0, 40.0f);
            ImGui::EndGroup();


            ImGui::Dummy(ImVec2(0.f, 10.f));


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
            //  renderUIFooter();
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
            currentSettings.DrawPaddleFooter();
            ImGui::End(); //window



        }

        #endif

    }; //class

}; //namespace
