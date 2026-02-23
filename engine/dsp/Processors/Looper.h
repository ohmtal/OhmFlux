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
        RecordingCountDown, //not really a countdown 10 to 0
        RecordingOverDupCountDown,
        Recording,
        RecordingOverDup,
        Playing
    };

    static const char* LooperModeNames[] = {
      "off",
      "Recording countdown",
      "Recording overdup countdown",
      "Recording",
      "Recording overdup",
      "Playing",
    };

    struct LooperPositionInfo {
        uint16_t beat = 0;
        uint16_t bar  = 0;
        uint16_t step = 0;
        uint32_t   bufferlen  = 0;
        float    seconds = 0.f;
        float    position = 0.f;
    };

    class Looper {
    private:
        //FIXME atomic
        std::vector<std::vector<float>> mLoopBuffers;
        uint32_t mBufferPos = 0;
        uint32_t mBufferLength = 0;
        LooperMode mMode = LooperMode::Off;


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
        void save(std::ostream& os)  const {
            uint32_t buflen = mBufferLength;
            if ( mLoopBuffers.size() == 0 && buflen > 0 ) {
                //assert ?!
                buflen = 0;
            }
            if ( buflen > 0 && mLoopBuffers[0].size() != buflen ) {
                //assert=!
                buflen= 0;
            }
            DSP_STREAM_TOOLS::write_binary(os, buflen);
            if (buflen == 0) return ;


            DSP_STREAM_TOOLS::write_binary(os, mBeatsPerBar);
            DSP_STREAM_TOOLS::write_binary(os, mSampleRate);
            DSP_STREAM_TOOLS::write_binary(os, mBeatsPerMinute);
            DSP_STREAM_TOOLS::write_binary(os, mBeatsPerSecond);
            DSP_STREAM_TOOLS::write_binary(os, mSecondsPerBeat);
            DSP_STREAM_TOOLS::write_binary(os, mSamplesPerBar);
            DSP_STREAM_TOOLS::write_binary(os, mSamplesPerBeat);
            DSP_STREAM_TOOLS::write_binary(os, mSamplesPerStep);


            uint8_t channels = static_cast<uint8_t>(mLoopBuffers.size());
            DSP_STREAM_TOOLS::write_binary(os, channels);


            for (uint8_t ch = 0; ch < channels; ch++)
            {
                for (uint32_t i =0; i < mBufferLength; i++)
                    DSP_STREAM_TOOLS::write_binary(os, mLoopBuffers[ch][i]);
            }

        }
        bool load(std::istream& is) {
            DSP_STREAM_TOOLS::read_binary(is, mBufferLength);
            if (mBufferLength == 0) return true; //empty one

            DSP_STREAM_TOOLS::read_binary(is, mBeatsPerBar);
            DSP_STREAM_TOOLS::read_binary(is, mSampleRate);
            DSP_STREAM_TOOLS::read_binary(is, mBeatsPerMinute);
            DSP_STREAM_TOOLS::read_binary(is, mBeatsPerSecond);
            DSP_STREAM_TOOLS::read_binary(is, mSecondsPerBeat);
            DSP_STREAM_TOOLS::read_binary(is, mSamplesPerBar);
            DSP_STREAM_TOOLS::read_binary(is, mSamplesPerBeat);
            DSP_STREAM_TOOLS::read_binary(is, mSamplesPerStep);

            uint8_t channels = 0;
            DSP_STREAM_TOOLS::read_binary(is, channels);
            //FIXME sanity bufferlen?!
            mLoopBuffers.assign(channels, std::vector<float>(mBufferLength, 0.0f));
            for (uint8_t ch = 0; ch < channels; ch++)
            {
                for (uint32_t i =0; i < mBufferLength; i++)
                    DSP_STREAM_TOOLS::read_binary(is, mLoopBuffers[ch][i]);
            }
            return is.good();
        }
        //----------------------------------------------------------------------
        void reset() {
            mBufferPos = 0;
            mBufferLength = 0;

            mBeatsPerBar = 4;
            mSampleRate = 48000;
            mBeatsPerMinute = 60;
            mBeatsPerSecond = 1.f;
            mSecondsPerBeat = 1.f;
            mSamplesPerStep = 0.f;
            mSamplesPerBar = 0.f;
            mSamplesPerBeat = 0.f;

            setMode(LooperMode::Off);

        }
        //----------------------------------------------------------------------

        // struct LooperPositionInfo {
        //     uint16_t beat = 0;
        //     unit16_t bar  = 0;
        //      uint32_t   bufferlen  = 0;
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
        //----------------------------------------------------------------------
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
        //----------------------------------------------------------------------
        uint16_t getBarsBySeconds(float requestedSeconds, uint16_t beatsPerMinute, uint8_t beatsPerBar = 4 ) {
            // 60 sec / 60 bpm = 1 beat / sec
            // 60 sec / 120 bpm = 0.5 beat / sec
            double secondsPerBeat = 60.0 / beatsPerMinute;
            // numBeats = sek / beatsPerSecond ==> 60 / 1 = 60
            double numBeats = requestedSeconds / secondsPerBeat;

            // bars  = numBeats / beatsPerBar => 60 / 4.f = 15
            uint16_t numBars = static_cast<int>(std::ceil(numBeats / (float)beatsPerBar));

            if (numBars < 1) numBars = 1;

            return numBars;
        }
        //----------------------------------------------------------------------
        float getSecondsByBars(uint16_t numBars, uint16_t beatsPerMinute, uint8_t beatsPerBar = 4) {
            double secondsPerBeat = 60.0 / beatsPerMinute;
            uint32_t totalBeats = static_cast<uint32_t>(numBars) * beatsPerBar;
            return static_cast<float>(totalBeats * secondsPerBeat);
        }
        //----------------------------------------------------------------------
        /*
         * Calculation:
         * SampleRate 48000 ==> 48000 Samples per second
         * BPM 60 = 1 beat per second
         * Bar (measure) beats per Bar usually 4 (in our drumkit)
         * 1 Bar = 4 steps !!!
         */
        void initWithSec(float requestedSeconds, uint16_t beatsPerMinute, float sampleRate, int numChannels, uint8_t beatsPerBar = 4 ) {

            // // 60 sec / 60 bpm = 1 beat / sec
            // // 60 sec / 120 bpm = 0.5 beat / sec
            // double secondsPerBeat = 60.0 / beatsPerMinute;
            // // numBeats = sek / beatsPerSecond ==> 60 / 1 = 60
            // double numBeats = requestedSeconds / secondsPerBeat;
            //
            // // bars  = numBeats / beatsPerBar => 60 / 4.f = 15
            // uint16_t numBars = static_cast<int>(std::ceil(numBeats / (float)beatsPerBar));
            //
            // // dLog("beatsPerSecond:%f, numBeats:%f, numBars:%d", beatsPerSecond, numBeats, numBars);
            //
            // if (numBars < 1) numBars = 1;

            uint16_t numBars =  getBarsBySeconds(requestedSeconds, beatsPerMinute, beatsPerBar);
            init(numBars,  beatsPerMinute, sampleRate, numChannels, beatsPerBar);
        }
        //----------------------------------------------------------------------
        void init(uint16_t bars,  uint16_t beatsPerMinute
                , float sampleRate, int numChannels, uint8_t beatsPerBar = 4) {

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


            mBufferLength = static_cast<uint32_t>((float)bars * mSamplesPerBar);

            mLoopBuffers.assign(numChannels, std::vector<float>(mBufferLength, 0.0f));
            mBufferPos = 0;

        }
        //----------------------------------------------------------------------
        //dont know if it's filled .... but init done we can play something
        bool bufferFilled() { return mBufferLength > 0 && !mLoopBuffers.empty();}
        int  getBPM() const { return mBeatsPerMinute;};
        //----------------------------------------------------------------------
        LooperMode  getMode() { return mMode;}
        bool setMode(LooperMode mode) {
            // why ? if (mBufferLength == 0) return false;
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
        void process(float* buffer, int numSamples, int numChannels, float playBackVolume = 1.f) {
            // Basic checks
            if ( mLoopBuffers.empty()
                || mMode == LooperMode::Off
                || mMode == LooperMode::RecordingCountDown
                || mMode == LooperMode::RecordingOverDupCountDown
            )  return;
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
                            inputSample += bufferSample * playBackVolume; // Add loop content to output
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
                        setMode(LooperMode::Playing);
                    }
                }
            } //outer loop
        } //process

    }; //CLASS

};}; //namespaces
