//-----------------------------------------------------------------------------
// Copyright (c) 2026 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Digital Sound Processing : KickSynt ( Kick-Drum )
//-----------------------------------------------------------------------------
// NOTE: audio_callback samplesPerStep = (SampleRate * 60.0f) / BPM / 4;
// TODO SnareSynt
// float whiteNoise() {return ((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f;}
//
// TODO HiHatSynt
// TODO TomTomSynt
//-----------------------------------------------------------------------------
#pragma once
#include "../DSP_Effect.h"


#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <cmath>
#include <atomic>
#include <algorithm>


namespace DSP {

struct KickData {
    float pitch;
    float decay;
    float click;
};


struct KickSettings : public ISettings {
    AudioParam<float> pitch      { "Pitch", 50.0f, 30.0f, 100.0f, "%.1f Hz" };
    AudioParam<float> decay      { "Decay", 0.3f, 0.01f, 2.0f, "%.2f s" };
    AudioParam<float> click      { "Click", 0.5f, 0.0f, 1.0f, "%.2f" };

    KickSettings() = default;
    REGISTER_SETTINGS(KickSettings, &pitch, &decay, &click)

    KickData getData() const {
        return { pitch.get(), decay.get(), click.get() };
    }

    void setData(const KickData& data) {
        pitch.set(data.pitch);
        click.set(data.click);
        decay.set(data.decay);
    }
    std::vector<std::shared_ptr<IPreset>> getPresets() const override {
        return {
            std::make_shared<Preset<KickSettings, KickData>>("Default", KickData{50.0f, 0.3f, 0.5f})
        };
    }
};

class KickSynth {
public:
    void trigger() {
        mPhase = 0.0f;
        mEnvelope = 1.0f; // full volume
        mPitchEnv = 1.0f; // highest pitch
        mActive = true;
    }

    float processSample(const KickSettings& settings, float sampleRate) {
        if (!mActive) return 0.0f;

        float startFreq = settings.pitch.get();
        float clickAmount = settings.click.get() * 500.0f; // Extra-Punch in Hz
        float currentFreq = startFreq + (mPitchEnv * clickAmount);

        float phaseIncrement = (2.0f * M_PI * currentFreq) / sampleRate;
        mPhase += phaseIncrement;
        if (mPhase >= 2.0f * M_PI) mPhase -= 2.0f * M_PI;

        float signal = std::sin(mPhase);
        float decayFactor = 1.0f / (settings.decay.get() * sampleRate);

        mEnvelope -= decayFactor;
        mPitchEnv -= decayFactor * 4.0f;

        if (mEnvelope <= 0.0f) {
            mEnvelope = 0.0f;
            mActive = false;
        }

        return signal * mEnvelope * 0.5f;
    }



private:
    float mPhase = 0.0f;
    float mEnvelope = 0.0f;
    float mPitchEnv = 0.0f;
    bool mActive = false;
};
}; //namespace
