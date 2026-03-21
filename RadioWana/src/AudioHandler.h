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
#include <functional>
#include <deque>
#include "DSP.h"
#include "DSP_EffectsManager.h"
#include "dsp/MonoProcessors/Volume.h"

namespace FluxRadio {


    class AudioHandler {
    protected:
        bool mDecoderInitialized = false;
        ma_decoder* mDecoder = nullptr;
        SDL_AudioStream* mStream = nullptr;

        std::vector<uint8_t> mRawBuffer; // ringbuffer
        std::recursive_mutex mBufferMutex;

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
                // high cpu usage! DSP::EffectType::SpectrumAnalyzer,
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


        AudioHandler() {
            mDecoder = new ma_decoder();
            memset(mDecoder, 0, sizeof(ma_decoder));
            mStreamInfo = nullptr;

            mEffectsManager = std::make_unique<DSP::EffectsManager>(true);
            populateRack(mEffectsManager->getActiveRack());
        }

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
            // if (mStream) {
            //     SDL_DestroyAudioStream(mStream);
            //     mStream = nullptr;
            // }
            // mDecoderInitialized = false;
        }

        void OnStreamTitleUpdate(const std::string streamTitle, const size_t streamPosition);
        void OnAudioChunk(const void* buffer, const size_t size);
        void onDisConnected(bool doLock = true);

    private:
        static void SDLCALL audio_callback(void* userdata, SDL_AudioStream* stream, int additional_amount, int total_amount);
        static ma_result OnReadFromRawBuffer(ma_decoder* pDecoder, void* pBufferOut, size_t bytesToRead, size_t* pBytesRead);
        static ma_result OnSeekDummy(ma_decoder* pDecoder, ma_int64 byteOffset, ma_seek_origin origin);


    };
}
