//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Digital Sound Processing : 9 Band Equalizer
// ISO Standard Frequencies for the 9 bands (63Hz to 16kHz).
//-----------------------------------------------------------------------------
// * using ISettings
//-----------------------------------------------------------------------------
#pragma once
#include <cmath> 

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

    struct Equalizer9BandData {
        float b1;
        float b2;
        float b3;
        float b4;
        float b5;
        float b6;
        float b7;
        float b8;
        float b9;
    };

    struct Equalizer9BandSettings : public ISettings {
        AudioParam<float> b1 { "63",  0.f, -12.f, 12.f, "%.1f dB" };
        AudioParam<float> b2 { "125",  0.f, -12.f, 12.f, "%.1f dB" };
        AudioParam<float> b3 { "250",  0.f, -12.f, 12.f, "%.1f dB" };
        AudioParam<float> b4 { "500",  0.f, -12.f, 12.f, "%.1f dB" };
        AudioParam<float> b5 { "1k",  0.f, -12.f, 12.f, "%.1f dB" };
        AudioParam<float> b6 { "2k",  0.f, -12.f, 12.f, "%.1f dB" };
        AudioParam<float> b7 { "4k",  0.f, -12.f, 12.f, "%.1f dB" };
        AudioParam<float> b8 { "8k",  0.f, -12.f, 12.f, "%.1f dB" };
        AudioParam<float> b9 { "16k",  0.f, -12.f, 12.f, "%.1f dB" };

        Equalizer9BandSettings() = default;
        REGISTER_SETTINGS(Equalizer9BandSettings,
                &b1,
                &b2,
                &b3,
                &b4,
                &b5,
                &b6,
                &b7,
                &b8,
                &b9
        )

        Equalizer9BandData getData() const {
            return {
                b1.get(),
                b2.get(),
                b3.get(),
                b4.get(),
                b5.get(),
                b6.get(),
                b7.get(),
                b8.get(),
                b9.get(),
            };
        }

        void getGains(float* outArray) const {
            outArray[0] = b1.get();
            outArray[1] = b2.get();
            outArray[2] = b3.get();
            outArray[3] = b4.get();
            outArray[4] = b5.get();
            outArray[5] = b6.get();
            outArray[6] = b7.get();
            outArray[7] = b8.get();
            outArray[8] = b9.get();
        }

        void setGain(int band, float value) {
            switch (band) {
                case 0: b1.set(value); return;
                case 1: b2.set(value); return;
                case 2: b3.set(value); return;
                case 3: b4.set(value); return;
                case 4: b5.set(value); return;
                case 5: b6.set(value); return;
                case 6: b7.set(value); return;
                case 7: b8.set(value); return;
                case 8: b9.set(value); return;
            }
        }

        float getGain(int band) const {
            switch (band) {
                case 0: return b1.get();
                case 1: return b2.get();
                case 2: return b3.get();
                case 3: return b4.get();
                case 4: return b5.get();
                case 5: return b6.get();
                case 6: return b7.get();
                case 7: return b8.get();
                case 8: return b9.get();
            }
            return 0.f; // error?!
        }


        void setData(const Equalizer9BandData& data) {
            b1.set(data.b1);
            b2.set(data.b2);
            b3.set(data.b3);
            b4.set(data.b4);
            b5.set(data.b5);
            b6.set(data.b6);
            b7.set(data.b7);
            b8.set(data.b8);
            b9.set(data.b9);
        }

        std::vector<std::shared_ptr<IPreset>> getPresets() const override {
            return {
                std::make_shared<Preset<Equalizer9BandSettings, Equalizer9BandData>>
                    ("Custom", Equalizer9BandData{ 2.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 2.0f }),

                std::make_shared<Preset<Equalizer9BandSettings, Equalizer9BandData>>
                    ("Flat", Equalizer9BandData{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }),

                std::make_shared<Preset<Equalizer9BandSettings, Equalizer9BandData>>
                    ("Bass Boost", Equalizer9BandData{ 5.5f, 4.0f, 2.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }),

                std::make_shared<Preset<Equalizer9BandSettings, Equalizer9BandData>>
                    ("Smile", Equalizer9BandData{4.5f, 3.0f, 0.0f, -2.0f, -3.5f, -2.0f, 0.0f, 3.0f, 4.5f }),

                std::make_shared<Preset<Equalizer9BandSettings, Equalizer9BandData>>
                    ("Radio", Equalizer9BandData{-12.0f, -12.0f, -6.0f, 3.0f, 6.0f, 3.0f, -6.0f, -12.0f, -12.0f }),

                std::make_shared<Preset<Equalizer9BandSettings, Equalizer9BandData>>
                    ("Clarify", Equalizer9BandData{-2.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.5f, 3.0f, 4.5f, 6.0f }),

            };
        }
    };
    constexpr Equalizer9BandData DEFAULT_EQ9_DATA = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };

    // // Preset: Flat (No change)
    // constexpr Equalizer9BandSettings FLAT_EQ = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
    // // Preset: Bass Boost (Classic "V" shape but focused on low end)
    // constexpr Equalizer9BandSettings BASS_BOOST_EQ =  ;
    // // Preset: "Loudness" / Smile (Boosted lows and highs, dipped mids)
    // constexpr Equalizer9BandSettings SMILE_EQ = {4.5f, 3.0f, 0.0f, -2.0f, -3.5f, -2.0f, 0.0f, 3.0f, 4.5f };
    // // Preset: Radio / Telephone (Cuts lows and highs, boosts mids)
    // constexpr Equalizer9BandSettings RADIO_EQ = {-12.0f, -12.0f, -6.0f, 3.0f, 6.0f, 3.0f, -6.0f, -12.0f, -12.0f };
    // // Preset: Air / Clarity (Subtle low cut, boost in the presence and high frequencies)
    // constexpr Equalizer9BandSettings CLARITY_EQ = {-2.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.5f, 3.0f, 4.5f, 6.0f };
    //
    // constexpr Equalizer9BandSettings CUSTOM_EQ = FLAT_EQ; //DUMMY
    //
    //
    // static const char* EQ9BAND_PRESET_NAMES[] = {
    //     "Custom", "Flat", "Bass Boost", "Loudness", "Radio", "Clarity" };
    //
    // static const char* EQ9BAND_LABELS[] = { "63", "125", "250", "500", "1k", "2k", "4k", "8k", "16k" };
    //
    // static const std::array<DSP::Equalizer9BandSettings, 6> EQ9BAND_PRESETS = {
    //     CUSTOM_EQ,
    //     FLAT_EQ,
    //     BASS_BOOST_EQ,
    //     SMILE_EQ,
    //     RADIO_EQ,
    //     CLARITY_EQ
    // };
    //



    class Equalizer9Band : public DSP::Effect {
        struct FilterState {
            float x1 = 0, x2 = 0, y1 = 0, y2 = 0;
        };
    private:
        Equalizer9BandSettings mSettings;
        static constexpr int NUM_BANDS = 9;

        // Standard ISO 9-band center frequencies
        const float mFrequencies[NUM_BANDS] = { 63.0f, 125.0f, 250.0f, 500.0f, 1000.0f, 2000.0f, 4000.0f, 8000.0f, 16000.0f };

        BiquadCoeffs mCoeffs[NUM_BANDS];

        std::vector<std::vector<FilterState>> mChannelStates;

        void calculateBand(int band) {
            // float A = pow(10.0f, mSettings.gains[band] / 40.0f);
            float A = pow(10.0f, mSettings.getGain(band) / 40.0f);
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
        : Effect(DSP::EffectType::Equalizer9Band, switchOn)
        , mSettings()
        {
            mSampleRate = sampleRate;
            updateAllBands();
        }
        //----------------------------------------------------------------------
        virtual std::string getName() const override { return "9-BAND EQUALIZER";}
        //----------------------------------------------------------------------
        Equalizer9BandSettings getSettings() const { return mSettings; }
        //----------------------------------------------------------------------
        void setSettings(const Equalizer9BandSettings& s) {
            mSettings = s;
            updateAllBands();

        }
        //----------------------------------------------------------------------
        void setSampleRate(float newRate) override {
            if (newRate <= 0) return;
            mSampleRate = newRate;
            updateAllBands();
        }
        //----------------------------------------------------------------------
        // Update specific band
        void setGain(int band, float db) {
            if (band < 0 || band >= 9) return;
            mSettings.setGain(band, db);
            // mSettings.gains[band] = db;
            calculateBand(band);
        }
        //----------------------------------------------------------------------
        float getGain(int band) const {
            if (band >= 0 && band < 9) return mSettings.getGain(band);
            return 0.0f;
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
        }
        //----------------------------------------------------------------------
        // process
        //----------------------------------------------------------------------
        virtual void process(float* buffer, int numSamples, int numChannels) override {
            if (!isEnabled()) return;

            // 1. Ensure we have state vectors for every channel
            if (mChannelStates.size() != static_cast<size_t>(numChannels)) {
                // Initialize numChannels vectors, each containing NUM_BANDS states
                mChannelStates.resize(numChannels, std::vector<FilterState>(NUM_BANDS));
            }

            int channel = 0;
            for (int i = 0; i < numSamples; i++) {
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

                if (++channel >= numChannels) channel = 0;
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
        //----------------------------------------------------------------------
        //----------------------------------------------------------------------
        #ifdef FLUX_ENGINE
        virtual ImVec4 getColor() const  override { return ImVec4(0.2f, 0.7f, 1.0f, 1.0f);}

        virtual void renderPaddle() override {
            DSP::Equalizer9BandSettings currentSettings = this->getSettings();
            if (currentSettings.DrawPaddle(this)) {
                this->setSettings(currentSettings);
            }
        }

        virtual void renderUIWide() override {
            DSP::Equalizer9BandSettings currentSettings= this->getSettings();
            if (currentSettings.DrawUIWide(this)) {
                this->setSettings(currentSettings);
            }
        }

        virtual void renderUI() override {
            DSP::Equalizer9BandSettings currentSettings= this->getSettings();
            if (currentSettings.DrawUI_FaderVertical(this)) {
                this->setSettings(currentSettings);
            }
        }
        //----------------------------------------------------------------------


        // virtual void renderUI() override {
        //     ImGui::PushID("EQ9_Effect_Row");
        //     ImGui::BeginGroup();
        //     auto* eq = this;
        //     bool isEnabled = eq->isEnabled();
        //     if (ImFlux::LEDCheckBox(getName(), &isEnabled, getColor())) eq->setEnabled(isEnabled);
        //
        //     if (eq->isEnabled()) {
        //         if (ImGui::BeginChild("EQ_Box", ImVec2(0, 180), ImGuiChildFlags_Borders)) {
        //             int currentIdx = 0;
        //             DSP::Equalizer9BandSettings currentSettings = eq->getSettings();
        //             for (int i = 1; i < DSP::EQ9BAND_PRESETS.size(); ++i) {
        //                 if (currentSettings == DSP::EQ9BAND_PRESETS[i]) {
        //                     currentIdx = i;
        //                     break;
        //                 }
        //             }
        //             int displayIdx = currentIdx;
        //             ImGui::SetNextItemWidth(150);
        //             if (ImFlux::ValueStepper("##Preset", &displayIdx, EQ9BAND_PRESET_NAMES, IM_ARRAYSIZE(EQ9BAND_PRESET_NAMES))) {
        //                 if (displayIdx > 0 && displayIdx < DSP::EQ9BAND_PRESETS.size()) {
        //                     currentSettings =  DSP::EQ9BAND_PRESETS[displayIdx];
        //                     eq->setSettings(currentSettings);
        //                 }
        //             }
        //             ImGui::SameLine(ImGui::GetWindowWidth() - 60);
        //             if (ImFlux::FaderButton("Reset", ImVec2(40.f, 20.f)))  {
        //                 eq->setSettings(DSP::FLAT_EQ);
        //             }
        //             ImGui::Separator();
        //
        //
        //             const float minGain = -12.0f;
        //             const float maxGain = 12.0f;
        //
        //             float sliderWidth = 20.f; //35.0f;
        //             float sliderHeight = 80.f; //150.0f;
        //             float sliderSpaceing = 12.f ; //12.f;
        //             ImVec2 padding = ImVec2(10, 50);
        //
        //
        //             ImGui::SetCursorPos(padding);
        //
        //             for (int i = 0; i < 9; i++) {
        //                 ImGui::PushID(i);
        //                 float currentGain = eq->getGain(i);
        //                 ImGui::BeginGroup();
        //                 if (ImFlux::FaderVertical("##v", ImVec2(sliderWidth, sliderHeight), &currentGain, minGain, maxGain)) {
        //                     eq->setGain(i, currentGain);
        //                 }
        //                 float textWidth = ImGui::CalcTextSize(EQ9BAND_LABELS[i]).x;
        //                 ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (sliderWidth - textWidth) * 0.5f);
        //                 ImGui::TextUnformatted(EQ9BAND_LABELS[i]);
        //                 ImGui::EndGroup();
        //                 if (i < 8) ImGui::SameLine(0, sliderSpaceing); // Spacing between sliders
        //                 ImGui::PopID();
        //             }
        //         }
        //         ImGui::EndChild();
        //     } else {
        //         ImGui::Separator();
        //     }
        //     ImGui::EndGroup();
        //     ImGui::PopID();
        //     ImGui::Spacing();
        // }


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
