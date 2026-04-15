//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
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
        std::atomic<bool> mDecoderInitialized = false;
        std::atomic<bool> mDecoderPause = false;
        ma_decoder* mDecoder = nullptr;
        SDL_AudioStream* mStream = nullptr;

        std::vector<uint8_t> mRawBuffer;
        size_t mRawBufferLimit = 100 * 1024 * 1024; //100MB ... take a long time :P

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
        void populateRack(DSP::EffectsRack* lRack);

    public:
        AudioHandler();
        ~AudioHandler() {
            // stop DecoderWorker
            mDecoderThreadRunning.store( false );
            if (mDecoderThread.joinable()) {
                mDecoderThread.join();
            }

        }

        void RenderRack(int mode = 0);
        DSP::EffectsManager* getManager() const { return mEffectsManager.get();}

        float getVolume() {return mVolume.load(); }
        void setVolume(float value) {return mVolume.store(value); }

        bool getPause() { return mDecoderPause.load(); }
        void setPause(const bool value) { mDecoderPause.store(value); }


        std::function<void(const uint8_t*, size_t)> OnAudioStreamData = nullptr;
        std::function<void()> OnTitleTrigger = nullptr;
        std::string getCurrentTitle() const { return mCurrentTitle;}
        std::deque<MetaEvent> getPendingStreamTitles()  const { return mPendingStreamTitles; }
        std::string getNextTitle() const;

        bool init(StreamInfo* info);
        void shutDown() {

        }

        void OnStreamTitleUpdate(const std::string streamTitle, const size_t streamPosition);
        void OnAudioChunk(const void* buffer, const size_t size);
        void onDisConnected();

        void reset();

        std::string getEffectsSettingsBase64( );
        bool setEffectsSettingsBase64( std::string settingsBase64);
        size_t getRawBufferSize() const { return mRawBuffer.size(); }

        size_t getRingBufferAvailableForWrite()  { return mRingBuffer.getAvailableForWrite(); }
        size_t getRingBufferAvailableForRead()  { return mRingBuffer.getAvailableForRead(); }

        size_t getRingBufferCapacity()  { return mRingBuffer.getCapacity(); }
        bool setRingBufferCapacity(size_t value)  { return mRingBuffer.setCapacity(value); }

        // this would only make sense if we prebuffer a lot of data
        // when the ringbuffer gets empty it's not filled up again.
        bool fastForward( size_t bytes ) {
            if ( getRingBufferAvailableForRead() > bytes ) {
                //FIXME ringbuffer need a simple move of the read needle
                std::vector<float> dummyBuffer(bytes);
                return mRingBuffer.pop( dummyBuffer.data(), bytes) > 0;

            }
            return false;
        }

        void decoderDebug( );


    private:
        static void SDLCALL audio_callback(void* userdata, SDL_AudioStream* stream, int additional_amount, int total_amount);
        static ma_result OnReadFromRawBuffer(ma_decoder* pDecoder, void* pBufferOut, size_t bytesToRead, size_t* pBytesRead);
        static ma_result OnSeekDummy(ma_decoder* pDecoder, ma_int64 byteOffset, ma_seek_origin origin);

    };
}
