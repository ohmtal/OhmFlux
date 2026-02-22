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

    struct LooperPositionInfo {
        uint16_t beat = 0;
        uint16_t bar  = 0;
        uint16_t step = 0;
        size_t   bufferlen  = 0;
        float    seconds = 0.f;
        float    position = 0.f;
    };

    class Looper {
    private:
        std::vector<std::vector<float>> mLoopBuffers;
        size_t mBufferPos = 0;
        size_t mBufferLength = 0;
        LooperMode mMode = LooperMode::Off;
        std::function<void(LooperMode)> mOnModeChanged = nullptr;
        // ---- statistics:
        int mBeatsPerBar = 4;
        float mSampleRate = 48000;
        uint16_t mBeatsPerMinute = 60;
        double mBeatsPerSecond = 1.f;
        double mSecondsPerBeat = 1.f;
        double mSamplesPerBar = 0.f;
        double mSamplesPerBeat = 0.f;
        double mSamplesPerStep = 0.f;

    public:
        //----------------------------------------------------------------------
        void reset() {
            mBufferPos = 0;
            mBufferLength = 0;

            mBeatsPerBar = 4;
            mSampleRate = 48000;
            mBeatsPerMinute = 60;
            mBeatsPerSecond = 1.f;
            mSamplesPerStep = 0.f;
            mSamplesPerBar = 0.f;
            mSamplesPerBeat = 0.f;

        }
        //----------------------------------------------------------------------
        void setModeChangedCallback(std::function<void(LooperMode)> callback) {
            mOnModeChanged = callback;
        }
        //----------------------------------------------------------------------

        // struct LooperPositionInfo {
        //     uint16_t beat = 0;
        //     unit16_t bar  = 0;
        //      size_t   bufferlen  = 0;
        //     float    seconds = 0.f;
        //     float    position = 0.f;
        // };

        // info of the buffer position
        LooperPositionInfo getPositionInfo() {
            LooperPositionInfo result = {};
            if (mBufferLength == 0) return result;
            result.bufferlen = mBufferLength;
            result.position =  static_cast<float>(mBufferPos) / static_cast<float>(mBufferLength);

            result.bar  = (uint16_t)(mBufferPos / mSamplesPerBar);
            result.beat = (uint16_t)(mBufferPos / mSamplesPerBeat);
            result.step = (uint16_t)(mBufferPos / mSamplesPerStep);

            result.seconds =  (float)(mBufferPos / mSamplesPerBeat * mSecondsPerBeat );
            // result.seconds =  static_cast<float>(mBufferPos) / mSampleRate;

            return result;
        }

        // info of the complete buffer with running position
        LooperPositionInfo getInfo() {
            LooperPositionInfo result = {};
            if (mBufferLength == 0) return result;
            result.bufferlen = mBufferLength;
            result.position =  static_cast<float>(mBufferPos) / static_cast<float>(mBufferLength);

            result.bar  = (uint16_t)(mBufferLength / mSamplesPerBar);
            result.beat = (uint16_t)(mBufferLength / mSamplesPerBeat);
            result.step = (uint16_t)(mBufferLength / mSamplesPerStep);


            // result.seconds =  static_cast<float>(mBufferLength) / mSampleRate;
            result.seconds =  (float)(mBufferLength / mSamplesPerBeat * mSecondsPerBeat );

            return result;
        }



        /*
         * Calculation:
         * SampleRate 48000 ==> 48000 Samples per second
         * BPM 60 = 1 beat per second
         * Bar (measure) beats per Bar usually 4 (in our drumkit)
         * 1 Bar = 4 steps !!!
         */
        void initWithSec(float requestedSeconds, uint16_t beatsPerMinute, float sampleRate, int numChannels, uint8_t beatsPerBar = 4 ) {

            // 60 sec / 60 bpm = 1 beat / sec
            // 60 sec / 120 bpm = 0.5 beat / sec
            double secondsPerBeat = 60.0 / beatsPerMinute;
            // numBeats = sek / beatsPerSecond ==> 60 / 1 = 60
            double numBeats = requestedSeconds / secondsPerBeat;

            // bars  = numBeats / beatsPerBar => 60 / 4.f = 15
            uint16_t numBars = static_cast<int>(std::ceil(numBeats / (float)beatsPerBar));

            // dLog("beatsPerSecond:%f, numBeats:%f, numBars:%d", beatsPerSecond, numBeats, numBars);

            if (numBars < 1) numBars = 1;

            init(numBars, beatsPerBar, beatsPerMinute, sampleRate, numChannels);
        }
        //----------------------------------------------------------------------
        void init(uint16_t bars, uint8_t beatsPerBar, uint16_t beatsPerMinute
                , float sampleRate, int numChannels) {

            if (bars < 1 ) bars = 1;

            //--- parameters
            mSampleRate     = sampleRate;
            mBeatsPerBar    = beatsPerBar;
            mBeatsPerMinute = beatsPerMinute;
            //--- calculated:
            mSecondsPerBeat = 60.f / beatsPerMinute;
            mSamplesPerBeat = mSecondsPerBeat * mSampleRate;
            mSamplesPerBar  = mSamplesPerBeat * mBeatsPerBar;
            mSamplesPerStep = mSamplesPerBeat / (double)mBeatsPerBar;
            mBeatsPerSecond = mBeatsPerMinute / 60.0;

            // mSamplesPerStep = ( sampleRate * 60.f ) / (beatsPerMinute * beatsPerBar);
            // mBeatsPerSecond =
            // mSamplesPerBeat = mBeatsPerSecond * mSampleRate;
            // mSamplesPerBar  = mSamplesPerBeat * (double)mBeatsPerBar;

            // double beatsPerSecond = 60.f / beatsPerMinute;
            // double samplesPerBeat = beatsPerSecond * sampleRate;
            // double samplesPerBar  = samplesPerBeat * (double)beatsPerBar;
            //
            // // statistics only....
            // mBeatsPerBar = beatsPerBar;
            //
            // mBeatsPerMinute = beatsPerMinute;
            // mSamplesPerBar = samplesPerBar;
            // mSamplesPerBeat = samplesPerBeat;


            mBufferLength = static_cast<size_t>((float)bars * mSamplesPerBar);

            mLoopBuffers.assign(numChannels, std::vector<float>(mBufferLength, 0.0f));
            mBufferPos = 0;

        }



        // void init(float requestedSeconds, float bpm, float sampleRate, int numChannels) {
        //     int beatsPerBar = 4; // Standard 4/4 Takt
        //
        //     // 1. Samples per single beat (e.g. at 120 BPM: 0.5 seconds)
        //     float samplesPerBeat = (60.0f / bpm) * sampleRate;
        //
        //     // 2. Samples per full bar (e.g. 4 beats = 2.0 seconds)
        //     float samplesPerBar = samplesPerBeat * (float)beatsPerBar;
        //
        //     // 3. How many full bars are needed to cover 'requestedSeconds'?
        //     float requestedSamples = requestedSeconds * sampleRate;
        //
        //     // ceil rounds up: e.g. 7 seconds @ 120 BPM (8s per 2 bars) -> returns 2 bars
        //     int numBars = static_cast<int>(std::ceil(requestedSamples / samplesPerBar));
        //
        //     // Ensure we have at least one bar
        //     if (numBars < 1) numBars = 1;
        //
        //     // 4. Set final buffer length to exactly X bars
        //     mBufferLength = static_cast<size_t>((float)numBars * samplesPerBar);
        //
        //     mLoopBuffers.assign(numChannels, std::vector<float>(mBufferLength, 0.0f));
        //     mBufferPos = 0;
        // }
        // //----------------------------------------------------------------------
        // void initByBpm(float bpm, int beatsPerBar, float sampleRate, int numChannels) {
        //     // 1. Samples per beat = (64.0 / bpm) * sampleRate
        //     // 2. Samples per bar = samplesPerBeat * beatsPerBar
        //     float samplesPerBeat = (60.0f / bpm) * sampleRate;
        //     size_t totalSamplesPerBar = static_cast<size_t>(samplesPerBeat * beatsPerBar);
        //
        //     mBufferLength = totalSamplesPerBar;
        //     mLoopBuffers.assign(numChannels, std::vector<float>(mBufferLength, 0.0f));
        //     mBufferPos = 0;
        // }
        // //----------------------------------------------------------------------
        // // FIXME Call this when the BPM or SampleRate changes
        // //
        // void initBySteps(int numSteps, float samplesPerStep, int numChannels) {
        //     // Exact length of one full 16-step loop
        //     mBufferLength = static_cast<size_t>(static_cast<float>(numSteps) * samplesPerStep);
        //
        //     mLoopBuffers.assign(numChannels, std::vector<float>(mBufferLength, 0.0f));
        //     mBufferPos = 0; // Start at the beginning
        // }

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
