//-----------------------------------------------------------------------------
// Copyright (c) 2026 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Digital Sound Processing : Limiter with integrated VU-Meter Data
//-----------------------------------------------------------------------------
#pragma once

#include <vector>
#include <cstdint>
#include <algorithm>
#include <cstring>

#ifdef FLUX_ENGINE
#include <imgui.h>
#include <imgui_internal.h>
#include <gui/ImFlux.h>
#endif


#include "DSP_Effect.h"

namespace DSP {


    struct LimiterSettings {
        float Threshold ;  // Limit just before 1.f
        float Attack;      // How fast it turns down
        float Release;     // How slow it turns back up

        static const uint8_t CURRENT_VERSION = 1;
        void getBinary(std::ostream& os) const {
            uint8_t ver = CURRENT_VERSION;
            os.write(reinterpret_cast<const char*>(&ver), sizeof(ver));
            os.write(reinterpret_cast<const char*>(this), sizeof(LimiterSettings));
        }

        bool  setBinary(std::istream& is) {
            uint8_t fileVersion = 0;
            is.read(reinterpret_cast<char*>(&fileVersion), sizeof(fileVersion));
            if (fileVersion != CURRENT_VERSION) //Something is wrong !
                return false;
            is.read(reinterpret_cast<char*>(this), sizeof(LimiterSettings));
            return  is.good();
        }

        auto operator<=>(const LimiterSettings&) const = default; //C++20 lazy way

        // bool operator==(const LimiterSettings& other) const {
        //     return Threshold == other.Threshold && Attack == other.Attack && Release == other.Release;
        // }

    };


    constexpr LimiterSettings LIMITER_DEFAULT = { 0.95f,  0.05f,  0.0000005f };
    constexpr LimiterSettings LIMITER_EIGHTY  = { 0.80f,  0.05f,  0.0000005f };
    constexpr LimiterSettings LIMITER_FIFTY   = { 0.50f,  0.05f,  0.0000005f };
    constexpr LimiterSettings LIMITER_LOWVOL  = { 0.25f,  0.05f,  0.0000005f };
    constexpr LimiterSettings LIMITER_EXTREM  = { 0.05f,  0.50f,  0.0000005f };

    constexpr LimiterSettings LIMITER_CUSTOM  = LIMITER_DEFAULT; //<< DUMMY

    static const std::array<DSP::LimiterSettings, 6> LIMITER_PRESETS = {
        DSP::LIMITER_CUSTOM,  // Index 0:
        DSP::LIMITER_DEFAULT, // Index 1
        DSP::LIMITER_EIGHTY,  // Index 2
        DSP::LIMITER_FIFTY,   // Index 3
        DSP::LIMITER_LOWVOL,  // Index 4
        DSP::LIMITER_EXTREM   // Index 5
    };


    class Limiter : public Effect {
    private:
        float mCurrentGain = 1.0f;
        LimiterSettings mSettings;

        // i integrate left right values for a VU-Meter
        float mLeftLevel = 0.0f;
        float mRightLevel = 0.0f;

        float mRmsL = 0.0f;
        float mRmsR = 0.0f;


    public:

        Limiter(bool switchOn = false) :
            Effect(switchOn),
            mSettings(LIMITER_DEFAULT)
            {}

        DSP::EffectType getType() const override { return DSP::EffectType::Limiter; }
        //----------------------------------------------------------------------
        void setSettings(const LimiterSettings& s) {
                mSettings = s;
        }
        //----------------------------------------------------------------------
        LimiterSettings& getSettings() { return mSettings; }
        //----------------------------------------------------------------------
        void save(std::ostream& os) const override {
            Effect::save(os);              // Save mEnabled
            mSettings.getBinary(os);       // Save Settings
        }
        //----------------------------------------------------------------------
        bool load(std::istream& is) override {
            if (!Effect::load(is)) return false; // Load mEnabled
            return mSettings.setBinary(is);      // Load Settings
        }
        //----------------------------------------------------------------------
        // fetch current Gain
        // 1.0 = open , 0.5 = -6dB
        float getGainReduction() const { return mCurrentGain; }
        //----------------------------------------------------------------------
        void getLevels(float& outL, float& outR) { //VU-Meter
            outL = mLeftLevel * 2.f;
            outR = mRightLevel * 2.f;
        }
        //----------------------------------------------------------------------
        void getDecible (float& outL, float& outR) { //VU-Meter
            outL = 20.0f * std::log10(mLeftLevel + 1e-9f);
            outR = 20.0f * std::log10(mRightLevel + 1e-9f);
        }
        //----------------------------------------------------------------------
        void getRMS(float& outL, float& outR) { //VU-Meter
            outL = mRmsL;
            outR = mRmsR;
        }
        //----------------------------------------------------------------------
        virtual void process(float* buffer, int numSamples) override {
            if (!isEnabled()) return;

            float sumL = 0.0f, sumR = 0.0f;

            // Process in steps of 2 for Stereo Interleaved data
            for (int i = 0; i < numSamples; i += 2) {
                // 1. Get both channels
                float inputL = buffer[i];
                float inputR = buffer[i + 1];

                // 2. Stereo-Link: Find the max absolute peak of BOTH channels
                float absL = std::abs(inputL);
                float absR = std::abs(inputR);
                float maxAbsInput = std::max(absL, absR);

                // 3. Calculate Target Gain based on the loudest channel
                float targetGain = 1.0f;
                if (maxAbsInput > mSettings.Threshold) {
                    targetGain = mSettings.Threshold / (maxAbsInput + 1e-9f);
                }

                // 4. Smooth the Gain (Shared for both L and R)
                if (targetGain < mCurrentGain) {
                    mCurrentGain += (targetGain - mCurrentGain) * mSettings.Attack;
                } else {
                    mCurrentGain += (targetGain - mCurrentGain) * mSettings.Release;
                }

                // 5. Apply SAME Gain to both (Preserves stereo image)
                buffer[i]     = inputL * mCurrentGain;
                buffer[i + 1] = inputR * mCurrentGain;

                // 6. Harvest Data for VU (using values AFTER gain/limiting)
                sumL += (buffer[i] * buffer[i]);
                sumR += (buffer[i + 1] * buffer[i + 1]);
            }

            // --- RMS & Smoothing Calculation ---
            int numFrames = numSamples / 2;
            if (numFrames > 0) {
                mRmsL = std::sqrt(sumL / (float)numFrames);
                mRmsR = std::sqrt(sumR / (float)numFrames);

                // Smooth the levels for the GUI (Ballistics)
                mLeftLevel  = (mLeftLevel  * 0.8f) + (mRmsL * 0.2f);
                mRightLevel = (mRightLevel * 0.8f) + (mRmsR * 0.2f);
            }
        }

    //----------------------------------------------------------------------
    #ifdef FLUX_ENGINE
    protected:
        ImFlux::VUMeterState mVUMeterStateLeft   = ImFlux::VUMETER_DEFAULT;
        ImFlux::VUMeterState mVUMeterStateRight  = ImFlux::VUMETER_DEFAULT;
    public:

    void renderVU(ImVec2 size, int century = 80)
    {
        // Calculate half width for stereo pairs
        float spacing = ImGui::GetStyle().ItemSpacing.x;
        ImVec2 halfSize = ImVec2((size.x - spacing) * 0.5f, size.y);

        if (century >= 80) { // 80s and 90s (LED Styles)
            float levL, levR;
            this->getLevels(levL, levR);
            // this->getRMS(levL, levR);

            // 90s Style: Slim vertical-ish bars with dots
            if (century >= 90) {
                ImFlux::VUMeter90th(levL, this->mVUMeterStateLeft);
                ImFlux::VUMeter90th(levR, this->mVUMeterStateRight);
            }
            // 80s Style: Chunky horizontal blocks
            else {
                ImFlux::VUMeter80th(levL, 8, ImVec2(22.f, 8.f));
                ImFlux::VUMeter80th(levR, 8, ImVec2(22.f, 8.f));
            }
        }
        else { // 70s Style (Analog Needle)
            float dbL, dbR;
            this->getDecible(dbL, dbR);

            auto mapDB = [](float db) {
                float minDB = -60.0f;
                return (db < minDB) ? 0.0f : (db - minDB) / (0.0f - minDB);
            };

            ImFlux::VUMeter70th(halfSize, mapDB(dbL));
            ImGui::SameLine();
            ImFlux::VUMeter70th(halfSize, mapDB(dbR));
        }
    }

    void renderPeakTest(bool withBackGround = true)
    {
        if (ImGui::BeginChild("RENDER_PEAK_TEST", ImVec2(0.f, 400.f))) {
            if (withBackGround) {
                ImFlux::GradientBox(ImVec2(-FLT_MIN, -FLT_MIN), 0.f);
                // ImGui::Dummy(ImVec2(2, 0)); ImGui::SameLine();
            }

            ImFlux::ShadowText("LIMITER / VU METERS");

            // GAIN REDUCTION (How much is being cut)
            // Usually, 0.0 means no reduction. 1.0 means total silence.
            float reduction = 1.0f - getGainReduction();
            ImGui::TextDisabled("Reduction: %7.3f", reduction);
            ImFlux::PeakMeter(reduction);

            // RMS (The raw power)
            float rmsL, rmsR;
            this->getRMS(rmsL, rmsR);
            ImGui::TextDisabled("RMS L:%7.3f R:%7.3f", rmsL, rmsR);
            ImFlux::PeakMeter(rmsL); ImGui::SameLine(); ImFlux::PeakMeter(rmsR);

            // Levels
            float levL, levR;
            this->getLevels(levL, levR);
            ImGui::TextDisabled("levels L:%7.3f R:%7.3f", levL, levR);
            ImFlux::PeakMeter(levL); ImGui::SameLine(); ImFlux::PeakMeter(levR);

            // 3. DECIBELS (The professional way)
            float dbL, dbR;
            this->getDecible(dbL, dbR);
            ImGui::TextDisabled("dB  L:%7.1f R:%7.1f", dbL, dbR);

            // FIX for dB Meter: Map -60dB...0dB to 0.0...1.0 linear for the PeakMeter
            // We call this "Normalized dB"
            auto mapDB = [](float db) {
                float minDB = -60.0f; // Silence floor
                if (db < minDB) return 0.0f;
                return (db - minDB) / (0.0f - minDB); // Map to 0.0 -> 1.0 range
            };
            ImFlux::PeakMeter(mapDB(dbL)); ImGui::SameLine(); ImFlux::PeakMeter(mapDB(dbR));


            this->renderVU(ImVec2(240.f, 75.f), 70);
            this->renderVU(ImVec2(240.f, 75.f), 80);
            this->renderVU(ImVec2(240.f, 75.f), 90);
        }
        ImGui::EndChild();
    }
    //----------------------------------------------------------------------
        void renderUI(bool withBackGround = true) {
            ImGui::PushID("Limiter_Effect_Row");
            ImGui::BeginGroup();

            auto* lim = this;
            bool isEnabled = lim->isEnabled();

            if (ImFlux::LEDCheckBox("LIMITER", &isEnabled, ImVec4(1.0f, 0.4f, 0.4f, 1.0f))) {
                lim->setEnabled(isEnabled);
            }


            if (lim->isEnabled()) {
                const char* presetNames[] = { "CUSTOM", "DEFAULT", "EIGHTY", "FIFTY", "LOWVOL", "EXTREM" };
                bool changed = false;
                DSP::LimiterSettings& currentSettings = lim->getSettings();

                int currentIdx = 0; // Standard: "Custom"

                for (int i = 1; i < DSP::LIMITER_PRESETS.size(); ++i) {
                    if (currentSettings == DSP::LIMITER_PRESETS[i]) {
                        currentIdx = i;
                        break;
                    }
                }
                int displayIdx = currentIdx;  //<< keep currentIdx clean

                if (ImGui::BeginChild("EQ_Box", ImVec2(0, 38), ImGuiChildFlags_Borders)) {

                    if ( withBackGround ) {
                        ImFlux::GradientBox(ImVec2(-FLT_MIN, -FLT_MIN),0.f);
                        ImGui::Dummy(ImVec2(2,0)); ImGui::SameLine();
                    }
                    ImGui::SetNextItemWidth(150);

                    if (ImFlux::ValueStepper("##Preset", &displayIdx, presetNames, IM_ARRAYSIZE(presetNames))) {
                        if (displayIdx > 0 && displayIdx < DSP::LIMITER_PRESETS.size()) {
                                    lim->setSettings(DSP::LIMITER_PRESETS[displayIdx]);
                        }
                    }

                    // Quick Reset Button (Now using the FLAT_EQ preset)
                    ImGui::SameLine(ImGui::GetWindowWidth() - 60);

                    // if (ImGui::SmallButton("Reset")) {
                    if (ImFlux::FaderButton("Reset", ImVec2(40.f, 20.f)))  {
                        lim->setSettings(DSP::LIMITER_DEFAULT);
                    }
                    // ImGui::Separator();
                    // changed |= ImFlux::FaderHWithText("Threshold", &currentSettings.Threshold, 0.01f, 1.f, "%.3f");
                    // changed |= ImFlux::FaderHWithText("Depth", &currentSettings.Attack, 0.01f, 1.f, "%.4f");
                    // changed |= ImFlux::FaderHWithText("Release", &currentSettings.Release, 0.0000005f, 0.0001f, "%.8f");
                    //
                    //
                    // if (changed) {
                    //     selectedPresetIdx = 0;
                    //     lim->setSettings(currentSettings);
                    // }

                } //box
                ImGui::EndChild();
            } //enabled

            ImGui::EndGroup();
            ImGui::PopID();
            ImGui::Spacing();
        }

    #endif //FLUX_ENGINE


    };

} // namespace DSP
