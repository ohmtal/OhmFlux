//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// not really mono using numChannels!
//-----------------------------------------------------------------------------
#pragma once

#include <vector>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <atomic>
#include <fstream>
#include <cstdlib>
#include <functional>

#include "../DSP_Math.h"

#ifdef FLUX_ENGINE
#include <utils/errorlog.h>
#endif

namespace DSP {
namespace Processors {

    enum class LooperMode {
        Off,
        Recording,
        RecordingOverDup,
        Playing
    };

    class Looper {
    private:
        std::vector<std::vector<float>> mLoopBuffers;
        size_t mBufferPos = 0;
        size_t mBufferLength = 0;
        LooperMode mMode = LooperMode::Off;
        std::function<void(LooperMode)> mOnModeChanged = nullptr;

    public:
        //----------------------------------------------------------------------
        void setModeChangedCallback(std::function<void(LooperMode)> callback) {
            mOnModeChanged = callback;
        }
        //----------------------------------------------------------------------
        void init(float requestedSeconds, float bpm, float sampleRate, int numChannels) {
            int beatsPerBar = 4; // Standard 4/4 Takt

            // 1. Samples per single beat (e.g. at 120 BPM: 0.5 seconds)
            float samplesPerBeat = (60.0f / bpm) * sampleRate;

            // 2. Samples per full bar (e.g. 4 beats = 2.0 seconds)
            float samplesPerBar = samplesPerBeat * (float)beatsPerBar;

            // 3. How many full bars are needed to cover 'requestedSeconds'?
            float requestedSamples = requestedSeconds * sampleRate;

            // ceil rounds up: e.g. 7 seconds @ 120 BPM (8s per 2 bars) -> returns 2 bars
            int numBars = static_cast<int>(std::ceil(requestedSamples / samplesPerBar));

            // Ensure we have at least one bar
            if (numBars < 1) numBars = 1;

            // 4. Set final buffer length to exactly X bars
            mBufferLength = static_cast<size_t>((float)numBars * samplesPerBar);

            mLoopBuffers.assign(numChannels, std::vector<float>(mBufferLength, 0.0f));
            mBufferPos = 0;
        }
        //----------------------------------------------------------------------
        void initByBpm(float bpm, int beatsPerBar, float sampleRate, int numChannels) {
            // 1. Samples per beat = (64.0 / bpm) * sampleRate
            // 2. Samples per bar = samplesPerBeat * beatsPerBar
            float samplesPerBeat = (60.0f / bpm) * sampleRate;
            size_t totalSamplesPerBar = static_cast<size_t>(samplesPerBeat * beatsPerBar);

            mBufferLength = totalSamplesPerBar;
            mLoopBuffers.assign(numChannels, std::vector<float>(mBufferLength, 0.0f));
            mBufferPos = 0;
        }
        //----------------------------------------------------------------------
        // FIXME Call this when the BPM or SampleRate changes
        void initBySteps(int numSteps, float samplesPerStep, int numChannels) {
            // Exact length of one full 16-step loop
            mBufferLength = static_cast<size_t>(static_cast<float>(numSteps) * samplesPerStep);

            mLoopBuffers.assign(numChannels, std::vector<float>(mBufferLength, 0.0f));
            mBufferPos = 0; // Start at the beginning
        }

        //----------------------------------------------------------------------
        //dont know if it's filled .... but init done we can play something
        bool bufferFilled() { return mBufferLength > 0 && !mLoopBuffers.empty();}
        //----------------------------------------------------------------------
        LooperMode  getMode() { return mMode;}
        bool setMode(LooperMode mode) {
            if (mBufferLength == 0) return false;
            mMode = mode;
            mBufferPos = 0;
            return true;
        }
        //----------------------------------------------------------------------
        float getPosition() const {
            if (mBufferLength == 0) return 0.0f;
            return static_cast<float>(mBufferPos) / static_cast<float>(mBufferLength);
        }
        //----------------------------------------------------------------------
        void process(float* buffer, int numSamples, int numChannels) {
            // Basic checks
            if (mLoopBuffers.empty() || mMode == LooperMode::Off) return;
            if (numChannels != static_cast<int>(mLoopBuffers.size())) return;

            // Outer loop: iterate through time frames (interleaved)
            // numSamples is usually the total number of floats in the buffer (samples * channels)
            for (int i = 0; i < numSamples; i += numChannels) {

                // Inner loop: iterate through each channel in the current frame
                for (int ch = 0; ch < numChannels; ch++) {
                    float& bufferSample = mLoopBuffers[ch][mBufferPos];
                    float& inputSample = buffer[i + ch];

                    switch (mMode) {
                        case LooperMode::Recording:
                            bufferSample = inputSample;
                            // output is usually just the input (monitoring)
                            break;

                        case LooperMode::RecordingOverDup:
                            bufferSample += inputSample;
                            inputSample += bufferSample; // Mix existing loop into output
                            break;

                        case LooperMode::Playing:
                            inputSample += bufferSample; // Add loop content to output
                            break;

                        default:
                            break;
                    }
                }

                // Advance position once per frame (after all channels are processed)
                mBufferPos++;
                if (mBufferPos >= mBufferLength) {
                    mBufferPos = 0;
                    if (mMode == LooperMode::Recording) {
                        mMode = LooperMode::Playing;
                        if (mOnModeChanged) {
                            mOnModeChanged(mMode);
                        }
                    }
                }
            } //outer loop
        } //process

    }; //CLASS

};}; //namespaces
