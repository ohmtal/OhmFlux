//-----------------------------------------------------------------------------
// Copyright (c) 2026 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Digital Sound Processing : 9 Band Equalizer
// ISO Standard Frequencies for the 9 bands (63Hz to 16kHz).
//-----------------------------------------------------------------------------
#pragma once
#define _USE_MATH_DEFINES // Required for M_PI on some systems (like Windows/MSVC)
#include <cmath>          // Provides pow, sin, cos, and math constants

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
// #include "DSP_Equilizer.h"

namespace DSP {

    struct Equalizer9BandSettings {
        std::array<float, 9> gains; // Gain for each band in dB

        static const uint8_t CURRENT_VERSION = 1;
        void getBinary(std::ostream& os) const {
            uint8_t ver = CURRENT_VERSION;
            DSP_STREAM_TOOLS::write_binary(os, ver);

            DSP_STREAM_TOOLS::write_binary(os, gains);
        }

        bool  setBinary(std::istream& is) {
            uint8_t fileVersion = 0;
            DSP_STREAM_TOOLS::read_binary(is, fileVersion);
            if (fileVersion != CURRENT_VERSION) return false;

            DSP_STREAM_TOOLS::read_binary(is, gains);

            return  is.good();
        }

        auto operator<=>(const Equalizer9BandSettings&) const = default; //C++20 lazy way

    };

    // Preset: Flat (No change)
    constexpr Equalizer9BandSettings FLAT_EQ = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
    // Preset: Bass Boost (Classic "V" shape but focused on low end)
    constexpr Equalizer9BandSettings BASS_BOOST_EQ =  { 5.5f, 4.0f, 2.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
    // Preset: "Loudness" / Smile (Boosted lows and highs, dipped mids)
    constexpr Equalizer9BandSettings SMILE_EQ = {4.5f, 3.0f, 0.0f, -2.0f, -3.5f, -2.0f, 0.0f, 3.0f, 4.5f };
    // Preset: Radio / Telephone (Cuts lows and highs, boosts mids)
    constexpr Equalizer9BandSettings RADIO_EQ = {-12.0f, -12.0f, -6.0f, 3.0f, 6.0f, 3.0f, -6.0f, -12.0f, -12.0f };
    // Preset: Air / Clarity (Subtle low cut, boost in the presence and high frequencies)
    constexpr Equalizer9BandSettings CLARITY_EQ = {-2.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.5f, 3.0f, 4.5f, 6.0f };

    constexpr Equalizer9BandSettings CUSTOM_EQ = FLAT_EQ; //DUMMY


    static const char* EQ9BAND_PRESET_NAMES[] = {
        "Custom", "Flat", "Bass Boost", "Loudness", "Radio", "Clarity" };

    static const char* EQ9BAND_LABELS[] = { "63", "125", "250", "500", "1k", "2k", "4k", "8k", "16k" };

    static const std::array<DSP::Equalizer9BandSettings, 6> EQ9BAND_PRESETS = {
        CUSTOM_EQ,
        FLAT_EQ,
        BASS_BOOST_EQ,
        SMILE_EQ,
        RADIO_EQ,
        CLARITY_EQ
    };






    class Equalizer9Band : public DSP::Effect {
        struct FilterState {
            float x1 = 0, x2 = 0, y1 = 0, y2 = 0;
        };

    private:
        Equalizer9BandSettings mSettings;
        static constexpr int NUM_BANDS = 9;
        // Standard ISO 9-band center frequencies
        const float mFrequencies[NUM_BANDS] = { 63.0f, 125.0f, 250.0f, 500.0f, 1000.0f, 2000.0f, 4000.0f, 8000.0f, 16000.0f };

        float mSampleRate = getSampleRateF();

        BiquadCoeffs mCoeffs[NUM_BANDS];

        std::vector<std::vector<FilterState>> mChannelStates;

        void calculateBand(int band) {
            float A = pow(10.0f, mSettings.gains[band] / 40.0f);
            float omega = 2.0f * M_PI * mFrequencies[band] / mSampleRate;
            float sn = sin(omega);
            float cs = cos(omega);
            float Q = 1.414f; // Steepness tuned for 9-band spacing
            float alpha = sn / (2.0f * Q);

            float a0 = 1.0f + alpha / A;
            mCoeffs[band].b0 = (1.0f + alpha * A) / a0;
            mCoeffs[band].b1 = (-2.0f * cs) / a0;
            mCoeffs[band].b2 = (1.0f - alpha * A) / a0;
            mCoeffs[band].a1 = (-2.0f * cs) / a0;
            mCoeffs[band].a2 = (1.0f - alpha / A) / a0;
        }

        void updateAllBands() {
            for (int i = 0; i < 9; i++) calculateBand(i);
        }

    public:
        IMPLEMENT_EFF_CLONE(Equalizer9Band)

        Equalizer9Band(bool switchOn = false, float sampleRate = DSP::SAMPLE_RATE)
        : Effect(switchOn)
        , mSampleRate(sampleRate)
        {
            setSettings(FLAT_EQ);
            updateAllBands();
        }

        DSP::EffectType getType() const override { return DSP::EffectType::Equalizer9Band; }

        float getSampleRate() const { return mSampleRate; }

        void setSampleRate(float newRate) override {
            if (newRate <= 0) return;
            mSampleRate = newRate;
            updateAllBands();
        }


        Equalizer9BandSettings getSettings() const { return mSettings; }

        // Setter for loading
        void setSettings(const Equalizer9BandSettings& s) {
            mSettings = s;
            updateAllBands();

        }

        // Update specific band
        void setGain(int band, float db) {
            if (band < 0 || band >= 9) return;
            mSettings.gains[band] = db;
            calculateBand(band);
        }

        float getGain(int band) const {
            if (band >= 0 && band < 9) return mSettings.gains[band];
            return 0.0f;
        }

        void save(std::ostream& os) const override {
            Effect::save(os);              // Save mEnabled
            mSettings.getBinary(os);       // Save Settings
        }

        bool load(std::istream& is) override {
            if (!Effect::load(is)) return false; // Load mEnabled
            return mSettings.setBinary(is);      // Load Settings
        }


        virtual void process(float* buffer, int numSamples, int numChannels) override {
            if (!isEnabled()) return;

            // 1. Ensure we have state vectors for every channel
            if (mChannelStates.size() != static_cast<size_t>(numChannels)) {
                // Initialize numChannels vectors, each containing NUM_BANDS states
                mChannelStates.assign(numChannels, std::vector<FilterState>(NUM_BANDS));
            }

            for (int i = 0; i < numSamples; i++) {
                int channel = i % numChannels;
                float sample = buffer[i];

                // Get the specific state array for this channel
                std::vector<FilterState>& bands = mChannelStates[channel];

                // 2. Cascade the sample through all 9 filters for the current channel
                for (int b = 0; b < NUM_BANDS; b++) {
                    FilterState& s = bands[b];
                    BiquadCoeffs& c = mCoeffs[b];

                    // Standard Direct Form I Biquad
                    float out = c.b0 * sample + c.b1 * s.x1 + c.b2 * s.x2
                    - c.a1 * s.y1 - c.a2 * s.y2;

                    // Update history for this specific band and channel
                    s.x2 = s.x1;
                    s.x1 = sample;
                    s.y2 = s.y1;
                    s.y1 = out;

                    sample = out; // Current output becomes input for the next band
                }

                // 3. Store final processed sample back into the interleaved buffer
                buffer[i] = sample;
            }
        }

        //----------------------------------------------------------------------
        float getMagnitudeAtFrequency(float freq, float sampleRate) {
            // Nutze double für die Berechnung der Visualisierung (Präzision!)
            double phi = 2.0 * M_PI * (double)freq / (double)sampleRate;
            double cos1 = std::cos(phi);
            double cos2 = std::cos(2.0 * phi);

            double totalMag = 1.0;

            for (int b = 0; b < NUM_BANDS; b++) {
                const BiquadCoeffs& c = mCoeffs[b];

                double num = (double)c.b0*c.b0 + (double)c.b1*c.b1 + (double)c.b2*c.b2
                + 2.0 * ((double)c.b0*c.b1 + (double)c.b1*c.b2) * cos1
                + 2.0 * ((double)c.b0*c.b2) * cos2;

                double den = 1.0 + (double)c.a1*c.a1 + (double)c.a2*c.a2
                + 2.0 * ((double)c.a1 + (double)c.a1*c.a2) * cos1
                + 2.0 * ((double)c.a2) * cos2;

                if (den > 0.0) {
                    totalMag *= std::sqrt(num / den);
                }
            }
            return (float)totalMag;
        }


        virtual std::string getName() const override { return "9-BAND EQUALIZER";}
        #ifdef FLUX_ENGINE
        virtual ImVec4 getColor() const  override { return ImVec4(0.2f, 0.7f, 1.0f, 1.0f);}



        virtual void renderUIWide() override {
            ImGui::PushID("EQ9BAND_Effect_Row_WIDE");
            if (ImGui::BeginChild("EQ9BAND_W_BOX", ImVec2(-FLT_MIN,85.f) )) { //default if 65
                auto* eq = this;
                int currentIdx = 0; // Standard: "Custom"
                DSP::Equalizer9BandSettings currentSettings = eq->getSettings();
                ImFlux::GradientBox(ImVec2(-FLT_MIN, -FLT_MIN),0.f);
                ImGui::Dummy(ImVec2(2,0)); ImGui::SameLine();
                ImGui::BeginGroup();
                bool isEnabled = this->isEnabled();
                if (ImFlux::LEDCheckBox(getName(), &isEnabled, getColor())){
                    this->setEnabled(isEnabled);
                }
                if (!isEnabled) ImGui::BeginDisabled();
                ImGui::SameLine();
                // -------- stepper >>>>
                for (int i = 1; i < DSP::EQ9BAND_PRESETS.size(); ++i) {
                    if (currentSettings == DSP::EQ9BAND_PRESETS[i]) {
                        currentIdx = i;
                        break;
                    }
                }
                int displayIdx = currentIdx;  //<< keep currentIdx clean
                ImGui::SameLine(ImGui::GetWindowWidth() - 260.f); // Right-align reset button

                if (ImFlux::ValueStepper("##Preset", &displayIdx, EQ9BAND_PRESET_NAMES, IM_ARRAYSIZE(EQ9BAND_PRESET_NAMES))) {
                    if (displayIdx > 0 && displayIdx < DSP::EQ9BAND_PRESETS.size()) {
                        currentSettings =  DSP::EQ9BAND_PRESETS[displayIdx];
                        eq->setSettings(currentSettings);
                    }
                }
                ImGui::SameLine();
                if (ImFlux::ButtonFancy("RESET", ImFlux::SLATEDARK_BUTTON.WithSize(ImVec2(40.f, 20.f)) ))  {
                    eq->setSettings(DSP::FLAT_EQ);
                }
                ImGui::Separator();
                const float minGain = -12.0f;
                const float maxGain = 12.0f;
                const float sliderWidth = ImFlux::DARK_KNOB.radius * 2.f;
                for (int i = 0; i < 9; i++) {
                    ImGui::PushID(i);
                    float currentGain = eq->getGain(i);
                    ImGui::BeginGroup();
                    if (ImFlux::MiniKnobF("##v", &currentGain, minGain, maxGain)) {
                        eq->setGain(i, currentGain);
                    }
                    float textWidth = ImGui::CalcTextSize(EQ9BAND_LABELS[i]).x;
                    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (sliderWidth - textWidth) * 0.5f);
                    ImGui::TextUnformatted(EQ9BAND_LABELS[i]);
                    ImGui::EndGroup();
                    if (i < 8) ImGui::SameLine();
                    ImGui::PopID();
                }
                if (!isEnabled) ImGui::EndDisabled();
                ImGui::EndGroup();
            }
            ImGui::EndChild();
            ImGui::PopID();
        }

        virtual void renderUI() override {
            ImGui::PushID("EQ9_Effect_Row");
            ImGui::BeginGroup();
            auto* eq = this;
            bool isEnabled = eq->isEnabled();
            if (ImFlux::LEDCheckBox(getName(), &isEnabled, getColor())) eq->setEnabled(isEnabled);

            if (eq->isEnabled()) {
                if (ImGui::BeginChild("EQ_Box", ImVec2(0, 180), ImGuiChildFlags_Borders)) {
                    int currentIdx = 0;
                    DSP::Equalizer9BandSettings currentSettings = eq->getSettings();
                    for (int i = 1; i < DSP::EQ9BAND_PRESETS.size(); ++i) {
                        if (currentSettings == DSP::EQ9BAND_PRESETS[i]) {
                            currentIdx = i;
                            break;
                        }
                    }
                    int displayIdx = currentIdx;
                    ImGui::SetNextItemWidth(150);
                    if (ImFlux::ValueStepper("##Preset", &displayIdx, EQ9BAND_PRESET_NAMES, IM_ARRAYSIZE(EQ9BAND_PRESET_NAMES))) {
                        if (displayIdx > 0 && displayIdx < DSP::EQ9BAND_PRESETS.size()) {
                            currentSettings =  DSP::EQ9BAND_PRESETS[displayIdx];
                            eq->setSettings(currentSettings);
                        }
                    }
                    ImGui::SameLine(ImGui::GetWindowWidth() - 60);
                    if (ImFlux::FaderButton("Reset", ImVec2(40.f, 20.f)))  {
                        eq->setSettings(DSP::FLAT_EQ);
                    }
                    ImGui::Separator();


                    const float minGain = -12.0f;
                    const float maxGain = 12.0f;

                    float sliderWidth = 20.f; //35.0f;
                    float sliderHeight = 80.f; //150.0f;
                    float sliderSpaceing = 12.f ; //12.f;
                    ImVec2 padding = ImVec2(10, 50);


                    ImGui::SetCursorPos(padding);

                    for (int i = 0; i < 9; i++) {
                        ImGui::PushID(i);
                        float currentGain = eq->getGain(i);
                        ImGui::BeginGroup();
                        if (ImFlux::FaderVertical("##v", ImVec2(sliderWidth, sliderHeight), &currentGain, minGain, maxGain)) {
                            eq->setGain(i, currentGain);
                        }
                        float textWidth = ImGui::CalcTextSize(EQ9BAND_LABELS[i]).x;
                        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (sliderWidth - textWidth) * 0.5f);
                        ImGui::TextUnformatted(EQ9BAND_LABELS[i]);
                        ImGui::EndGroup();
                        if (i < 8) ImGui::SameLine(0, sliderSpaceing); // Spacing between sliders
                        ImGui::PopID();
                    }
                }
                ImGui::EndChild();
            } else {
                ImGui::Separator();
            }
            ImGui::EndGroup();
            ImGui::PopID();
            ImGui::Spacing();
        }


        //-----------------------------------------------------------------
        void renderCurve(ImVec2 size) {
            if (!isEnabled() ) return ;
            if (size.x < 10.f || size.y < 10.f  || !ImGui::IsRectVisible(size)) return;

            ImDrawList* dl = ImGui::GetWindowDrawList();
            ImVec2 pos = ImGui::GetCursorScreenPos();
            float midY = pos.y + size.y * 0.5f;

            static std::vector<ImVec2> points;
            points.resize(size.x);

            for (int x = 0; x < (int)size.x; x++) {
                float normX = (float)x / size.x;
                float minF = 20.0f;
                float maxF = 20000.0f;
                float freq = minF * std::pow(maxF / minF, normX);
                float mag = getMagnitudeAtFrequency(freq, mSampleRate);

                float db = 20.0f * std::log10(mag + 1e-6f);

                float y = midY - (db * (size.y / 30.0f));
                points[x] = ImVec2(pos.x + x, std::clamp(y, pos.y, pos.y + size.y));
            }

            dl->AddPolyline(points.data(), points.size(), IM_COL32(255, 255, 0, 255), 0, 2.0f);

            ImGui::Dummy(size);
        }


        #endif


    }; //CLASS

} // namespace DSP
