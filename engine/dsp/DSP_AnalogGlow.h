//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Digital Sound Processing : Analog Glow
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

namespace DSP {
struct AnalogGlowData {
    float cutoff;     // frequency cut 60..20000, default 20k
    float gain;       // gain  0.f, 2.f, default 1.f;
    float resonance;  // 0.0f .. 0.95f default 0.5
    float tilt; // 0.0 means Flat. -1.0 is Dark/Bass-heavy, +1.0 is Bright/Treble-heavy
    float wet;   // 0.0 = Pure Clean, 1.0 = Pure Glow

};

struct AnalogGlowSettings : public ISettings {
    AudioParam<float> cutoff { "Cut off" , 2000.f, 400.f,  8000.f, "%.f Hz" };
    AudioParam<float> gain   { "Gain", 10.f, 1.f, 100.f, "%.2f" };
    AudioParam<float> resonance { "Tube Resonance", 0.7f, 0.0f, 0.95f, "%.2f" };
    AudioParam<float> tilt { "Tilt", -0-25.0f, -1.0f, 1.0f, "%.2f" };
    AudioParam<float> wet { "Mix", 0.5f, 0.0f, 1.0f, "wet %.2f" };


    AnalogGlowSettings() = default;
    REGISTER_SETTINGS(AnalogGlowSettings, &cutoff, &gain, &resonance, &tilt, &wet)

    AnalogGlowData getData() const {
        return { cutoff.get(), gain.get(), resonance.get(), tilt.get(), wet.get() };
    }

    void setData(const AnalogGlowData& data) {
        cutoff.set(data.cutoff);
        gain.set(data.gain);
        resonance.set(data.resonance);
        tilt.set(data.tilt);
        wet.set(data.wet);
    }

    std::vector<std::shared_ptr<IPreset>> getPresets() const override {
        return {
            std::make_shared<Preset<AnalogGlowSettings, AnalogGlowData>>
                ("Custom", AnalogGlowData{2000.f, 10.f, 0.7f, 0.f, 0.5f}),
            std::make_shared<Preset<AnalogGlowSettings, AnalogGlowData>>
                ("Warm Glow", AnalogGlowData{2000.f, 10.f, 0.7f, -0.25f, 0.5f}),
            std::make_shared<Preset<AnalogGlowSettings, AnalogGlowData>>
                ("Tom Signature", AnalogGlowData{3400.f, 90.f, 0.95f,-0.15f, 0.32f}),
           std::make_shared<Preset<AnalogGlowSettings, AnalogGlowData>>
                ("High Gain", AnalogGlowData{8000.f, 100.f, 0.95f,1.f, 0.2f}),

            //  "Midnight Jazz" - Super smooth, bass-heavy, very warm
            std::make_shared<Preset<AnalogGlowSettings, AnalogGlowData>>
                ("Midnight Jazz", AnalogGlowData{ 650.f, 2.5f, 0.2f, -0.6f, 0.8f }),

            //  "Radio Lo-Fi" - Thin, resonant, like an old transistor radio
            std::make_shared<Preset<AnalogGlowSettings, AnalogGlowData>>
                ("Radio Lo-Fi", AnalogGlowData{ 1200.f, 15.f, 0.85f, 0.8f, 1.0f }),

            //  "Crushed Velvet" - Parallel Saturation: massive distortion but still clear
            std::make_shared<Preset<AnalogGlowSettings, AnalogGlowData>>
                ("Crushed Velvet", AnalogGlowData{ 4500.f, 85.f, 0.4f, 0.2f, 0.45f }),

            // "Vader's Breath" - Deep, dark, moving with the Pitch Shifter
            std::make_shared<Preset<AnalogGlowSettings, AnalogGlowData>>
                ("Vader's Breath", AnalogGlowData{ 350.f, 12.f, 0.9f, -0.9f, 0.75f }),

        };
    }
};

class AnalogGlow: public Effect{

private:
    struct FilterChannelState {
        float prevInput = 0.0f;
        float filterState = 0.0f; // Glow Filter
        float tiltLPState = 0.0f; // Tilt Low-Pass Component
    };

    float mSavCutOff = -1;
    float mAlpha = 1.0f;
    AnalogGlowSettings mSettings;
    // State for filtering and blending
    std::vector<FilterChannelState> mStates;

public:
    IMPLEMENT_EFF_CLONE(AnalogGlow)

    AnalogGlow(bool switchOn = false) :
        DSP::Effect(DSP::EffectType::AnalogGlow, switchOn)
        , mSettings()
    {
        initStates(2,false);
    }
    //----------------------------------------------------------------------
    virtual std::string getName() const override { return "Analog Glow";}
    //----------------------------------------------------------------------
    AnalogGlowSettings& getSettings() { return mSettings; }
    //----------------------------------------------------------------------
    void initStates(int numChannels, bool onlyIfChannelsChanged) {
        if (onlyIfChannelsChanged && (int)mStates.size() == numChannels) {
            return;
        }
        if ((int)mStates.size() != numChannels) {
            mStates.resize(numChannels, FilterChannelState());
        }
    }    //----------------------------------------------------------------------
    void updateAlpha() {
        float cutOff = mSettings.cutoff.get();

        if (std::abs(cutOff - mSavCutOff) < 0.1f) return;
        mSavCutOff = cutOff;

        // Standard One-Pole Alpha calculation via Omega
        float omega = 2.0f * (float)M_PI * cutOff / mSampleRate;

        // Low-pass Alpha formula: alpha = omega / (omega + 1)
        mAlpha = omega / (omega + 1.0f);

        // Safety: Cap alpha at 1.0 (fully open)
        if (mAlpha > 1.0f) mAlpha = 1.0f;
    }
    //----------------------------------------------------------------------
    void setSettings(const AnalogGlowSettings& s) {
        mSettings = s;
        updateAlpha();
    }
    //----------------------------------------------------------------------
    virtual void setSampleRate(float sampleRate) override {
        mSampleRate = sampleRate;
        updateAlpha();
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
    //----------------------------------------------------------------------
    virtual void process(float* buffer, int numSamples, int numChannels) override {
        initStates(numChannels, true);

        const float gain = mSettings.gain.get();
        if (!mEnabled || gain < 0.001f) return;

        // Cache parameters
        const float alpha = mAlpha;
        const float res = mSettings.resonance.get();
        const float tiltVal = mSettings.tilt.get();
        const float mixWet = mSettings.wet.get();
        const float mixDry = 1.0f - mixWet;

        const float tiltAlpha = 0.09f * (48000.0f / mSampleRate);

        FilterChannelState* statesPtr = mStates.data();
        int channel = 0;

        for (int i = 0; i < numSamples; i++) {
            const float drySignal = buffer[i]; // Store original for mixing
            FilterChannelState& s = statesPtr[channel];

            // --- STEP 1: TILT EQ ---
            s.tiltLPState += tiltAlpha * (drySignal - s.tiltLPState);
            float lowPart = s.tiltLPState;
            float highPart = drySignal - lowPart;
            float tilted = (lowPart * (1.0f - tiltVal)) + (highPart * (1.0f + tiltVal));

            // --- STEP 2: ANALOG GLOW ---
            float blended = (tilted + s.prevInput) * 0.5f;
            s.prevInput = tilted;

            float drive = blended + (res * s.filterState);
            s.filterState += alpha * (drive - s.filterState);
            s.filterState += 1e-18f; // Anti-Denormal

            // --- STEP 3: OUTPUT MIX ---
            // Saturated "Wet" signal
            float wetSignal = DSP::softClip(s.filterState * gain);

            // Linear crossfade between dry and wet
            buffer[i] = (drySignal * mixDry) + (wetSignal * mixWet);

            if (++channel >= numChannels) channel = 0;
        }
    }


    // virtual void process(float* buffer, int numSamples, int numChannels) override {
    //     initStates(numChannels, true);
    //
    //     // Cache params to avoid getter overhead in the loop
    //     const float gain = mSettings.gain.get();
    //     const float alpha = mAlpha; // From updateAlpha()
    //     const float res = mSettings.resonance.get();
    //     const float tiltVal = mSettings.tilt.get(); // -1.0 to 1.0
    //
    //     // Tilt-Alpha: Fixed center around 700Hz for guitar
    //     // Formula: 1 - exp(-2pi * 700 / 48000) approx 0.09
    //     const float tiltAlpha = 0.09f * (48000.0f / mSampleRate);
    //
    //     FilterChannelState* statesPtr = mStates.data();
    //     int channel = 0;
    //
    //     for (int i = 0; i < numSamples; i++) {
    //         float input = buffer[i];
    //         FilterChannelState& s = statesPtr[channel];
    //
    //         // --- STEP 1: TILT EQ (The Balance) ---
    //         // Create a gentle low-pass to split the signal
    //         s.tiltLPState += tiltAlpha * (input - s.tiltLPState);
    //         float lowPart = s.tiltLPState;
    //         float highPart = input - lowPart;
    //
    //         // Apply Tilt: Balance between low and high
    //         // Using (1.0 - tilt) for low and (1.0 + tilt) for high
    //         float tilted = (lowPart * (1.0f - tiltVal)) + (highPart * (1.0f + tiltVal));
    //
    //         // --- STEP 2: ANALOG GLOW (The Saturation) ---
    //         // Always Blending for that smooth feel
    //         float blended = (tilted + s.prevInput) * 0.5f;
    //         s.prevInput = tilted;
    //
    //         // Feedback Path (Resonance)
    //         float drive = blended + (res * s.filterState);
    //
    //         // One-Pole Low-Pass
    //         s.filterState += alpha * (drive - s.filterState);
    //         s.filterState += 1e-18f; // Anti-Denormal
    //
    //         // --- STEP 3: OUTPUT (The Warmth) ---
    //         // Final Gain and Soft-Clipping
    //         buffer[i] = DSP::softClip(s.filterState * gain);
    //
    //         if (++channel >= numChannels) channel = 0;
    //     }
    // }

    // virtual void process(float* buffer, int numSamples, int numChannels) override {
    //     initStates(numChannels, true);
    //
    //     const float gain = mSettings.gain.get();
    //     if (!mEnabled || gain < 0.001f) return;
    //
    //     const float alpha = mAlpha;
    //     const float res = mSettings.resonance.get(); // New Resonance param
    //     FilterChannelState* statesPtr = mStates.data();
    //
    //     int channel = 0;
    //     for (int i = 0; i < numSamples; i++) {
    //         FilterChannelState& s = statesPtr[channel];
    //         float input = buffer[i];
    //
    //         // Always Blending: Simple low-pass pre-filter
    //         float blended = (input + s.prevInput) * 0.5f;
    //         s.prevInput = input;
    //
    //         // Feedback Path: Mix some of the previous output back in
    //         // We use a small amount of the filter state as feedback
    //         float drive = blended + (res * s.filterState);
    //
    //         // One-Pole Filter Calculation
    //         s.filterState += alpha * (drive - s.filterState);
    //
    //         // Apply gain and the new Soft-Clip for "Warmth"
    //         float out = DSP::softClip(s.filterState * gain);
    //
    //         buffer[i] = out;
    //
    //         if (++channel >= numChannels) channel = 0;
    //     }
    // }


    // virtual void process(float* buffer, int numSamples, int numChannels) override {
    //     const float gain = mSettings.gain.get();
    //     if (!isEnabled() || gain < 0.001f) return;
    //
    //     const bool useBlending = mSettings.blendingOn.get();
    //
    //     initStates(numChannels, true);
    //     FilterChannelState* statesPtr = mStates.data();
    //
    //     int channel = 0;
    //     for (int i = 0; i < numSamples; i++) {
    //         float input = buffer[i];
    //
    //         FilterChannelState& s = statesPtr[channel];
    //         // Blending (Linear Interpolation)
    //         float blended = useBlending ? (input + s.prevInput) * 0.5f : input;
    //
    //         s.prevInput = input;
    //
    //         // Apply Low-Pass Filter (One-Pole IIR)
    //         s.filterState += mAlpha * (blended - s.filterState);
    //
    //         // nan save
    //         s.filterState += 1e-18f;
    //
    //         // Apply Gain and softClip
    //         float out = DSP::softClip(s.filterState * gain);
    //
    //
    //         buffer[i] = out;
    //         if (++channel >= numChannels) channel = 0;
    //     }
    // }

//     virtual void process(float* buffer, int numSamples, int numChannels) override {
//         const float gain = mSettings.gain.get();
//         if (!isEnabled() || gain < 0.001f) return;
//
//         const bool useBlending = mSettings.blendingOn.get();
//
//         initStates(numChannels, true);
//
//         int channel = 0;
//         for (int i = 0; i < numSamples; i++) {
//             float input = buffer[i];
//
//             // Access the state for the current channel
//             FilterChannelState& s = mStates[channel];
//
//             // 1. Blending (Linear Interpolation)
//             // Average of current input and previous input
//             float blended = useBlending ? (input + s.prevInput) * 0.5f : input;
//
//             // Save raw input for the next sample's blending
//             s.prevInput = input;
//
//             // 2. Low-Pass Filter (One-Pole IIR)
//             // Simple smoothing formula: y[n] = y[n-1] + alpha * (x[n] - y[n-1])
//             s.filterState = s.filterState + mAlpha * (blended - s.filterState);
//
//             // 3. Apply Gain and Clamp
//             float out = DSP::softClip(s.filterState * gain);
//
//             // Safety: Check for NaN and reset if necessary
//             if (std::isnan(out)) {
//                 s.filterState = 0.0f;
//                 out = 0.0f;
//             }
//
//             buffer[i] = out;
//             if (++channel >= numChannels) channel = 0;
//         }
//     }

    //----------------------------------------------------------------------
    //----------------------------------------------------------------------
    #ifdef FLUX_ENGINE
    virtual ImVec4 getColor() const  override { return ImVec4(0.9f, 0.2f, 0.2f, 1.0f);}

    virtual void renderPaddle() override {
        DSP::AnalogGlowSettings currentSettings = this->getSettings();
        currentSettings.gain.setKnobSettings(ImFlux::ksRed); // NOTE only works here !
        if (currentSettings.DrawPaddle(this)) {
            this->setSettings(currentSettings);
        }
    }
    virtual void renderUIWide() override {
        DSP::AnalogGlowSettings currentSettings = this->getSettings();
        if (currentSettings.DrawUIWide(this)) {
            this->setSettings(currentSettings);
        }
    }
    virtual void renderUI() override {
        DSP::AnalogGlowSettings currentSettings = this->getSettings();
        if (currentSettings.DrawUI(this, 160.f, true)) {
            this->setSettings(currentSettings);
        }
    }
    #endif
}; //class


}; //namespace
