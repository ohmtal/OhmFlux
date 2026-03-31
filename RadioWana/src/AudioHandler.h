//-----------------------------------------------------------------------------
// Copyright (c) 2012 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#pragma once

#include <SDL3/SDL.h>
#include <iostream>
#include "StreamInfo.h"
#include <miniaudio.h>
#include <vector>
#include <mutex>
#include <thread>
#include <atomic>
#include <chrono>
#include <functional>
#include <deque>
#include "DSP.h"
#include "DSP_EffectsManager.h"
#include "dsp/MonoProcessors/Volume.h"
#include "utils/byteEncoder.h"
#include "audio/fluxAudioBuffer.h"

namespace FluxRadio {


    class AudioHandler {
    protected:
        bool mDecoderInitialized = false;
        ma_decoder* mDecoder = nullptr;
        SDL_AudioStream* mStream = nullptr;

        std::vector<uint8_t> mRawBuffer;
        FluxAudio::AudioBuffer mRingBuffer;
        std::recursive_mutex mBufferMutex;

        std::thread mDecoderThread;
        std::atomic<bool> mDecoderThreadRunning{true};
        void DecoderWorker();

        size_t mFadeInSamplesProcessed = 0;
        const size_t FADE_IN_DURATION = 44100 * 0.1;



        bool mInitialized = false;
        StreamInfo* mStreamInfo = nullptr;

        uint16_t mPreBufferSize = 16384;
        size_t mTotalAudioBytesPlayed = 0;

        std::string mCurrentTitle = "";
        std::deque<MetaEvent> mPendingStreamTitles;


        DSP::MonoProcessors::Volume mVolProcessor;
        std::atomic<float>mVolume = 1.f;


        std::unique_ptr<DSP::EffectsManager> mEffectsManager = nullptr;
        void populateRack(DSP::EffectsRack* lRack) {
            std::vector<DSP::EffectType> types = {
                DSP::EffectType::Equalizer9Band,
                // DSP::EffectType::Limiter,
                // high cpu usage!
                DSP::EffectType::SpectrumAnalyzer,
                DSP::EffectType::VisualAnalyzer,
            };
            for (auto type : types) {
                auto fx = DSP::EffectFactory::Create(type);
                if (fx) {
                    fx->setEnabled(true);
                    lRack->getEffects().push_back(std::move(fx));
                }
            }
        }

    public:
        AudioHandler()  : mRingBuffer(1024 * 512) {
            mDecoder = new ma_decoder();
            memset(mDecoder, 0, sizeof(ma_decoder));
            mStreamInfo = nullptr;

            mEffectsManager = std::make_unique<DSP::EffectsManager>(true);
            populateRack(mEffectsManager->getActiveRack());


        }
        // ~AudioHandler() {
        //     mDecoderThreadRunning.store(false);
        //     if (mDecoderThread.joinable()) mDecoderThread.join();
        // }


        void RenderRack(int mode = 0) {
            if (mEffectsManager) {
                    mEffectsManager->renderUI(mode);
            }
        }
        DSP::EffectsManager* getManager() const { return mEffectsManager.get();}

        float getVolume() {return mVolume.load(); }
        void setVolume(float value) {return mVolume.store(value); }


        std::function<void(const uint8_t*, size_t)> OnAudioStreamData = nullptr;
        std::function<void()> OnTitleTrigger = nullptr;
        std::string getCurrentTitle() const { return mCurrentTitle;}
        std::deque<MetaEvent> getPendingStreamTitles()  const { return mPendingStreamTitles; }
        std::string getNextTitle() const {
            if (!mPendingStreamTitles.empty()) {
                return mPendingStreamTitles.front().streamTitle;
            }
            return "";
        }

        bool init(StreamInfo* info);
        void shutDown() {
            onDisConnected(false);
        }

        void OnStreamTitleUpdate(const std::string streamTitle, const size_t streamPosition);
        void OnAudioChunk(const void* buffer, const size_t size);
        void onDisConnected(bool doLock = true);

        void reset(bool doLock = true);

    private:
        static void SDLCALL audio_callback(void* userdata, SDL_AudioStream* stream, int additional_amount, int total_amount);
        static ma_result OnReadFromRawBuffer(ma_decoder* pDecoder, void* pBufferOut, size_t bytesToRead, size_t* pBytesRead);
        static ma_result OnSeekDummy(ma_decoder* pDecoder, ma_int64 byteOffset, ma_seek_origin origin);


    public:
        std::string getEffectsSettingsBase64( ) {
            if (mEffectsManager->getActiveRack()) {
                std::stringstream oss;
                mEffectsManager->SaveRackStream(mEffectsManager->getActiveRack(), oss);
                ByteEncoder::Base64 lEncoder;
                return lEncoder.encode(oss);
            }
            return "";
        }
        bool setEffectsSettingsBase64( std::string settingsBase64) {

            if (settingsBase64.empty()) {
                LogFMT("[error] setInputEffectsSettings failed! Empty INPUT Stream!");
                return false;
            }

            if (mEffectsManager->getActiveRack()) {
                ByteEncoder::Base64 lEncoder;
                std::stringstream iss;
                if (!lEncoder.decode(settingsBase64, iss)) {
                    LogFMT("[error] setInputEffectsSettings failed! Invalid Stream!");
                    return false;
                }
                if (!mEffectsManager->LoadRackStream(iss, DSP::EffectsManager::OnlyUpdateExistingSingularity, true)) {
                    LogFMT("[error] setInputEffectsSettings failed!\n{}", mEffectsManager->getErrors());
                    return false;
                }

                return true;
            }
            return false;
        }


        size_t getRawBufferSize() const { return mRawBuffer.size(); }
        void decoderDebug( ) {
            // if (!isDebugBuild()) return;
            Log("Raw Buffer Size: %d, Decode Running: %d",(int)getRawBufferSize(), (int)mDecoderThreadRunning);
        }

    };
}
