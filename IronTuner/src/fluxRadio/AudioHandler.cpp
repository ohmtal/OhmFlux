//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#include <SDL3/SDL.h>
#include "AudioHandler.h"
#include "utils/errorlog.h"
#include "audio/fluxAudio.h"

#define MA_NO_DEVICE_IO
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

#include "dsp/MonoProcessors/Volume.h"
#include <mutex>



namespace FluxRadio {
    // -----------------------------------------------------------------------------
    AudioHandler::AudioHandler()
    : mRingBuffer(1024 * 512)
    {
        mDecoder = new ma_decoder();
        memset(mDecoder, 0, sizeof(ma_decoder));
        mStreamInfo = nullptr;

        mEffectsManager = std::make_unique<DSP::EffectsManager>(true);
        populateRack(mEffectsManager->getActiveRack());


    }
    // -----------------------------------------------------------------------------
    void SDLCALL AudioHandler::audio_callback(void* userdata, SDL_AudioStream* stream, int additional_amount, int total_amount) {
        auto* self = static_cast<AudioHandler*>(userdata);
        if (!self || !self->mStreamInfo) return;

        int channels = self->mStreamInfo->channels;
        size_t samplesNeeded = additional_amount / sizeof(float);

        std::vector<float> pcmBuffer(samplesNeeded);

        // new ringbuffer :D
        size_t samplesRead = self->mRingBuffer.pop(pcmBuffer.data(), samplesNeeded);

        if (samplesRead > 0) {

            float vol = self->mVolume.load();

            for (size_t i = 0; i < samplesRead; i++) {
                if ( self->mFadeInSamplesProcessed < self->FADE_IN_DURATION) {
                    pcmBuffer[i] = 0.f;
                    self->mFadeInSamplesProcessed++;
                } else {
                    pcmBuffer[i] = self->mVolProcessor.process(pcmBuffer[i], vol, false);
                }

            }

            if (self->mEffectsManager) {
                self->mEffectsManager->process(pcmBuffer.data(), (int)samplesRead, channels);
            }

            SDL_PutAudioStreamData(stream, pcmBuffer.data(), (int)(samplesRead * sizeof(float)));
        }

        if (samplesRead < samplesNeeded) {
            size_t missingSamples = samplesNeeded - samplesRead;
            std::vector<float> silence(missingSamples, 0.0f);
            SDL_PutAudioStreamData(stream, silence.data(), (int)(missingSamples * sizeof(float)));
        }
    }
    // -----------------------------------------------------------------------------
    bool AudioHandler::init(StreamInfo* info) {

        if (!info) {
            Log("[error] init StreamInfo in NULL!!");
            return false;
        }

        // audio/mpeg
        if (info->content_type != "audio/mpeg") {
            //FIXME
            Log("[error] Sorry only mp3 supported at the moment");
            if (isDebugBuild()) info->dump();

        }
        mStreamInfo = info;
        if (mInitialized ) {
            ma_decoder_uninit(mDecoder);
            mDecoderInitialized.store( false ) ;
            mInitialized = false;
        }

        if ( !mDecoderInitialized.load() )
            return false;


        mTotalAudioBytesPlayed = 0;
        mEffectsManager->setSampleRate(info->samplerate);

        if (mDecoder == nullptr) {
            dLog("mDecoder was nullptr?! ");
            mDecoder = new ma_decoder();
        }
        memset(mDecoder, 0, sizeof(ma_decoder));
        ma_decoder_config config = ma_decoder_config_init(ma_format_f32, info->channels, info->samplerate);


        if (info->content_type == "audio/aac") {
            // :/ lol bad evaluation !
            return false;
        } else {
            // Default: audio/mpeg
            config.encodingFormat = ma_encoding_format_mp3;
        }


        // dLog("this: %p, pDecoder: %p (Inhalt)", this, (void*)mDecoder);
        mDecoder->pUserData = this;
        ma_result result = ma_decoder_init(OnReadFromRawBuffer, OnSeekDummy, this, &config, mDecoder);


        if (result != MA_SUCCESS) {
            Log("[error] Unable to init decoder code:%d", result);
            return false;
        }

        // start DecoderWorker
        mDecoderThreadRunning.store( true );
        mDecoderThread = std::thread([this]() { DecoderWorker(); });


        // reset fade in !!
        mFadeInSamplesProcessed = 0;


        bool needsNewSDLStream = true;
        if (result == MA_SUCCESS) {

            if (mStream) {
                SDL_AudioSpec currentSpec;
                if (SDL_GetAudioStreamFormat(mStream, &currentSpec, nullptr)) {
                    if (currentSpec.freq == info->samplerate && currentSpec.channels == info->channels) {
                        needsNewSDLStream = false;
                        SDL_ResumeAudioStreamDevice(mStream);
                    }
                }
            }
        }


        if (needsNewSDLStream && result == MA_SUCCESS) {

            if (mStream) {
                SDL_DestroyAudioStream(mStream);
                mStream = nullptr;
            }


            SDL_AudioSpec spec;
            spec.format = SDL_AUDIO_F32;
            spec.channels = 2;
            spec.freq =  44100 ;
            mStream = SDL_CreateAudioStream(&spec, &spec);


            if (!mStream)
            {
                Log("[error] SDL_OpenAudioDeviceStream failed: %s", SDL_GetError());
                return false;
            }

            SDL_SetAudioStreamGetCallback(mStream, AudioHandler::audio_callback, this);

            #if defined(FLUX_ENGINE) && !defined(FLUX_ENGINE_FAKE)
            AudioManager.bindStream(mStream);
            #else
            SDL_AudioDeviceID dev = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, NULL);

            if (dev == 0) {
                Log("[error] Failed to open audio device: %s", SDL_GetError());
                return false;
            }

            if (!SDL_BindAudioStream(dev, mStream)) {
                Log("[error] SDL_OpenAudioDeviceStream failed to bind stream: %s", SDL_GetError());
                return false;
            }
            #endif

            SDL_ResumeAudioStreamDevice(mStream);

        }
        mInitialized = true;
        return true;
    }
    // -----------------------------------------------------------------------------
    void AudioHandler::OnAudioChunk(const void* buffer, size_t size) {
        std::lock_guard<std::recursive_mutex> lock(mBufferMutex);

        mRawBuffer.insert(mRawBuffer.end(), (uint8_t*)buffer, (uint8_t*)buffer + size);

        // init check
        if (mStreamInfo && !mDecoderInitialized.load() && mRawBuffer.size() >= mPreBufferSize) {
            mDecoderInitialized.store(true);
            this->init(mStreamInfo);
        }
    }
    // -----------------------------------------------------------------------------
    ma_result AudioHandler::OnReadFromRawBuffer(ma_decoder* pDecoder, void* pBufferOut, size_t bytesToRead, size_t* pBytesRead)
    {
        auto* self = static_cast<AudioHandler*>(pDecoder->pUserData);
        if (!self || !pBufferOut) {
            return MA_ERROR;
        }

        std::lock_guard<std::recursive_mutex> lock(self->mBufferMutex);

        size_t available = self->mRawBuffer.size();
        size_t actualRead = std::min(bytesToRead, available);

        if (actualRead > 0) {
            // fire OnAudioStreamData Event can be used for recording
            if (self->OnAudioStreamData) self->OnAudioStreamData(self->mRawBuffer.data(), actualRead);

            // Copy to pBufferOut and remove from mRawBuffer
            memcpy(pBufferOut, self->mRawBuffer.data(), actualRead);
            self->mRawBuffer.erase(self->mRawBuffer.begin(), self->mRawBuffer.begin() + actualRead);


            // Title Trigger / mTotalAudioBytesPlayed
            self->mTotalAudioBytesPlayed += actualRead;
            if ( !self->mPendingStreamTitles.empty() &&
                self->mTotalAudioBytesPlayed >= self->mPendingStreamTitles.front().byteOffset
            ) {
                bool newTitle = self->mCurrentTitle != self->mPendingStreamTitles.front().streamTitle;
                self->mCurrentTitle = self->mPendingStreamTitles.front().streamTitle;
                self->mPendingStreamTitles.pop_front();
                if ( newTitle &&  self->OnTitleTrigger ) self->OnTitleTrigger();
            }
        }
        if (pBytesRead) {
            *pBytesRead = actualRead;
        }

        return MA_SUCCESS;
    }
    // -----------------------------------------------------------------------------
    ma_result AudioHandler::OnSeekDummy(ma_decoder* pDecoder, ma_int64 byteOffset, ma_seek_origin origin){
        return MA_SUCCESS;
    }
    // -----------------------------------------------------------------------------
    void AudioHandler::reset ( ){
        if (!mDecoderInitialized.load()) return;
        std::lock_guard<std::recursive_mutex> lock(mBufferMutex);

        // stop DecoderWorker
        mDecoderThreadRunning.store( false );
        if (mDecoderThread.joinable()) {
            mDecoderThread.join();
        }
        mRawBuffer.clear();
        mRingBuffer.clear();

        mDecoderInitialized.store(false);
        if (mStream) {
            SDL_PauseAudioStreamDevice(mStream);
        }
        if (mDecoder) {
            ma_decoder_uninit(mDecoder);
            memset(mDecoder, 0, sizeof(ma_decoder));
            mInitialized = false;
        }
        mPendingStreamTitles.clear();
        mCurrentTitle = "";

    }
    // -----------------------------------------------------------------------------
    void AudioHandler::onDisConnected ( ){
        reset();
    }
    // -----------------------------------------------------------------------------
    void AudioHandler::OnStreamTitleUpdate(const std::string streamTitle, const size_t streamPosition){
        if (mCurrentTitle == "")  mCurrentTitle=streamTitle;
        MetaEvent newEvent;
        newEvent.streamTitle = streamTitle;
        newEvent.byteOffset = streamPosition;
        mPendingStreamTitles.push_back(newEvent);
    }
    // -----------------------------------------------------------------------------
    void AudioHandler::DecoderWorker() {
        const ma_uint64 framesToDecodePerLoop = 4096;
        std::vector<float> pcmBuffer;

        while (mDecoderThreadRunning.load()) {
            bool canDecode = false;
            {
                std::lock_guard<std::recursive_mutex> lock(mBufferMutex);
                canDecode = mDecoderInitialized.load() && mStreamInfo && (mRawBuffer.size() > framesToDecodePerLoop);
            }

            if (!canDecode) {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                continue;
            }

            if (pcmBuffer.empty()) pcmBuffer.resize(framesToDecodePerLoop * mStreamInfo->channels);

            // if (mRingBuffer.getAvailableForWrite() < (mRingBuffer.getCapacity() - (framesToDecodePerLoop * mStreamInfo->channels))) {
            if (mRingBuffer.getAvailableForWrite() > (framesToDecodePerLoop * mStreamInfo->channels)) {
                ma_uint64 framesRead = 0;

                ma_result result = ma_decoder_read_pcm_frames(mDecoder, pcmBuffer.data(), framesToDecodePerLoop, &framesRead);
                if (framesRead > 0) {
                    mRingBuffer.push(pcmBuffer.data(), framesRead * mStreamInfo->channels);
                    continue;
                } else  if (result == MA_AT_END && mRawBuffer.size() > 1024) {
                    if (isDebugBuild()) Log("[warn] manual seek decode is at end!!");
                    ma_uint64 currentFrame = 0;
                    ma_decoder_get_cursor_in_pcm_frames(mDecoder, &currentFrame);
                    ma_decoder_seek_to_pcm_frame(mDecoder, currentFrame);
                }

            }

            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }
    // -----------------------------------------------------------------------------
    void AudioHandler::populateRack(DSP::EffectsRack* lRack){
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
    // -----------------------------------------------------------------------------
    void AudioHandler::RenderRack(int mode){
        if (mEffectsManager) {
            mEffectsManager->renderUI(mode);
        }
    }
    // -----------------------------------------------------------------------------
    std::string AudioHandler::getEffectsSettingsBase64(){
        if (mEffectsManager->getActiveRack()) {
            std::stringstream oss;
            mEffectsManager->SaveRackStream(mEffectsManager->getActiveRack(), oss);
            ByteEncoder::Base64 lEncoder;
            return lEncoder.encode(oss);
        }
        return "";
    }
    // -----------------------------------------------------------------------------
    bool AudioHandler::setEffectsSettingsBase64(std::string settingsBase64) {

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
    // -----------------------------------------------------------------------------
    std::string AudioHandler::getNextTitle() const{
        if (!mPendingStreamTitles.empty()) {
            return mPendingStreamTitles.front().streamTitle;
        }
        return "";
    }
    // -----------------------------------------------------------------------------
    void AudioHandler::decoderDebug(){
        Log("Raw Buffer Size: %d, Ringbuffer spaceLeft:%d inUse:%d, Decode Running: %d",
            (int)getRawBufferSize(),
            (int)mRingBuffer.getAvailableForWrite(),
            (int)mRingBuffer.getAvailableForRead(),
            (int)mDecoderThreadRunning
        );
    }
}; //namespace

