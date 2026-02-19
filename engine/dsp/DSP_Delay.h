//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Digital Sound Processing : Delay
//-----------------------------------------------------------------------------
// * using ISettings
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

struct DelayData  {
    float time;      //  10 - 2000ms max 2 sek
    float feedback;  //   0 - 0.95f
    float wet;       // 0.1 - 1.f
};

// default { 300.f, 0.4f, 0.3f }
struct DelaySettings : public ISettings {
    AudioParam<float> time     { "Time" ,  300.f, 10.f,  2000.f, "%.1f ms" };
    AudioParam<float> feedback { "Feedback", 0.4f, 0.1f,  0.95f, "%.2f" };
    AudioParam<float> wet      { "Mix",   0.3f ,  0.0f, 1.0f, "Wet %.2f"};

    DelaySettings() = default;
    REGISTER_SETTINGS(DelaySettings, &time, &feedback, &wet)

    DelayData getData() const {
        return { time.get(), feedback.get(), wet.get()};
    }

    void setData(const DelayData& data) {
        time.set(data.time);
        feedback.set(data.feedback);
        wet.set(data.wet);
    }
    std::vector<std::shared_ptr<IPreset>> getPresets() const override {
        return {
            std::make_shared<Preset<DelaySettings, DelayData>>
                ("Custom", DelayData{ 400.f, 0.4f, 0.3f }),

            std::make_shared<Preset<DelaySettings, DelayData>>
                ("Slapback", DelayData{ 50.f,  0.3f, 0.2f }),

            std::make_shared<Preset<DelaySettings, DelayData>>
                ("Short", DelayData{ 300.f, 0.4f, 0.3f }),

            std::make_shared<Preset<DelaySettings, DelayData>>
                ("Spacey", DelayData{ 800.f, 0.5f, 0.4f })
        };
    }

};

//-----
constexpr DelayData DEFAULT_DELAY_DATA = { 300.f, 0.4f, 0.3f }; // Standard echo
//-----


class Delay : public DSP::Effect {
private:

    std::vector<std::vector<float>> mBuffers;
    std::vector<uint32_t> mPositions;
    float* mChannelPtrs[8];

    DelaySettings mSettings;
    uint32_t mMaxBufSize;

    float mSmoothedDelaySamples = 0.f;


public:
    IMPLEMENT_EFF_CLONE(Delay)

    Delay(bool switchOn = false) :
    Effect(DSP::EffectType::Delay, switchOn)
    ,mSettings()
    {
        setSampleRate(mSampleRate);
        mSmoothedDelaySamples = 0.f;
    }
    //----------------------------------------------------------------------
    virtual std::string getName() const override { return "DELAY";}
    //----------------------------------------------------------------------
    DelaySettings& getSettings() { return mSettings; }
    //----------------------------------------------------------------------
    void setSettings(const DelaySettings& s) {
        mSettings = s;
        reset();

    }
    //----------------------------------------------------------------------
    void updateBuffers( int numChannels) {
        mBuffers.resize(numChannels, std::vector<float>(mMaxBufSize, 0.0f));
        mPositions.resize(numChannels, 0);

        for (int i = 0; i < numChannels; ++i) {
            mChannelPtrs[i] = mBuffers[i].data();
        }

    }
    //----------------------------------------------------------------------
    void init(int numChannels, float sampleRate) {
        mMaxBufSize = 32768;
        while (mMaxBufSize < (int)(sampleRate * 2.0f)) mMaxBufSize <<= 1;
        updateBuffers(numChannels);
    }
    //----------------------------------------------------------------------
    void setSampleRate(float sampleRate) override {

        mSampleRate = sampleRate;
        int curChannels = (int) mBuffers.size();
        init(curChannels, sampleRate);
    }
    //----------------------------------------------------------------------
    void reset() override {

        // int curChannels = (int) mBuffers.size();
        // updateBuffers(curChannels);

        mSmoothedDelaySamples = 0.f;

        // clear buffers
        for (auto& channelBuffer : mBuffers) {
            std::fill(channelBuffer.begin(), channelBuffer.end(), 0.0f);
        }
        std::fill(mPositions.begin(), mPositions.end(), 0);

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
    virtual float getTailLengthSeconds() const override {
        if (!isEnabled()) return 0.0f;

        float feedback = mSettings.feedback.get();
        float delayMs = mSettings.time.get();

        // If feedback is very low, tail is just one delay cycle
        if (feedback < 0.01f) return delayMs / 1000.0f;

        // Safety: Clamp feedback to avoid infinite tail or log(1) error
        if (feedback >= 0.99f) feedback = 0.99f;

        // n = log(threshold) / log(feedback)
        // -60dB threshold (0.001) is standard for "silence"
        float iterations = logf(0.001f) / logf(feedback);
        return (iterations * delayMs) / 1000.0f;
    }
    //----------------------------------------------------------------------
    // Process
    //----------------------------------------------------------------------
    virtual void process(float* buffer, int numSamples, int numChannels) override {
        const float wet = mSettings.wet.get();
        if (!isEnabled() || wet <= 0.001f) return;


        int curChannels = (int) mBuffers.size();
        if (numChannels != curChannels) updateBuffers(numChannels);


        // prepare ( mMaxBufSize must have power of 2 !!! )
        const uint32_t mask = mMaxBufSize - 1;
        const float targetDelaySamples = (mSettings.time.get() / 1000.0f) * mSampleRate;
        const float feedback = mSettings.feedback.get();
        const float dryGain = 1.0f - wet;

        // Pointer-Array
        for (int c = 0; c < numChannels; ++c) mChannelPtrs[c] = mBuffers[c].data();

        int channel = 0;
        for (int i = 0; i < numSamples; i++) {
            //  Smoothing (
            if (channel == 0) {
                mSmoothedDelaySamples += 0.001f * (targetDelaySamples - mSmoothedDelaySamples);
            }

            float dry = buffer[i];
            float* activeBuf = mChannelPtrs[channel];
            uint32_t& activePos = mPositions[channel];

            //  Read Position (no Modulo!)
            float readPos = (float)activePos - mSmoothedDelaySamples;
            while (readPos < 0.0f) readPos += (float)mMaxBufSize;

            uint32_t indexA = (uint32_t)readPos & mask;
            uint32_t indexB = (indexA + 1) & mask;
            float fraction = readPos - (float)((uint32_t)readPos);

            // Linear Interpolation
            float delayed = activeBuf[indexA] + fraction * (activeBuf[indexB] - activeBuf[indexA]);

            // Update Buffer & Mix
            activeBuf[activePos] = dry + (delayed * feedback);
            buffer[i] = (dry * dryGain) + (delayed * wet);

            // Increment (Bitmasking instead of %)
            activePos = (activePos + 1) & mask;

            if (++channel >= numChannels) channel = 0;
        }
    }


#ifdef FLUX_ENGINE
    virtual ImVec4 getColor() const  override { return ImVec4(0.3f, 0.8f, 0.6f, 1.0f);}

    virtual void renderPaddle() override {
        DSP::DelaySettings currentSettings = this->getSettings();
        currentSettings.wet.setKnobSettings(ImFlux::ksBlue); // NOTE only works here !
        if (currentSettings.DrawPaddle(this)) {
            this->setSettings(currentSettings);
        }
    }

    virtual void renderUIWide() override {
        DSP::DelaySettings currentSettings = this->getSettings();
        if (currentSettings.DrawUIWide(this)) {
            this->setSettings(currentSettings);
        }
    }
    virtual void renderUI() override {
        DSP::DelaySettings currentSettings = this->getSettings();
        if (currentSettings.DrawUI(this, 110.f, true)) {
            this->setSettings(currentSettings);
        }
    }
    #endif

}; //CLASS
}; //namespace
