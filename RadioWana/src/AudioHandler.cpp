#include <SDL3/SDL.h>
#include "AudioHandler.h"
#include "utils/errorlog.h"
#include "audio/fluxAudio.h"

#define MA_NO_DEVICE_IO
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
//FIXME  #include "extras/miniaudio_libav.h"

#include "dsp/MonoProcessors/Volume.h"

#include <mutex>


namespace FluxRadio {
    // -----------------------------------------------------------------------------

    void SDLCALL AudioHandler::audio_callback(void* userdata, SDL_AudioStream* stream, int additional_amount, int total_amount) {
        auto* self = static_cast<AudioHandler*>(userdata);

        int channels = self->mStreamInfo->channels;
        ma_uint32 framesToRead = additional_amount / (sizeof(float) * channels);

        // Buffer
        std::vector<float> pcmBuffer(framesToRead * channels);

        ma_uint64 framesRead;
        ma_result result = ma_decoder_read_pcm_frames(self->mDecoder, pcmBuffer.data(), framesToRead, &framesRead);

        if (framesRead > 0) {
            size_t totalSamplesRead = framesRead * channels;


            float vol = self->mVolume.load();
            for (uint32_t i = 0; i < totalSamplesRead; i++)
                pcmBuffer.data()[i] = self->mVolProcessor.process(pcmBuffer.data()[i], vol, false);

            if (self->mEffectsManager) {
                self->mEffectsManager->process(pcmBuffer.data(), (int)totalSamplesRead, channels);
            }


            int bytesToWrite = (int)(totalSamplesRead * sizeof(float));
            SDL_PutAudioStreamData(stream, pcmBuffer.data(), bytesToWrite);
        }

        if (framesRead < framesToRead) {
            int missingFrames = framesToRead - (int)framesRead;
            int silenceBytes = missingFrames * channels * sizeof(float);
            std::vector<float> silence(missingFrames * channels, 0.0f);
            SDL_PutAudioStreamData(stream, silence.data(), silenceBytes);
        }
    }


    // void SDLCALL AudioHandler::audio_callback(void* userdata, SDL_AudioStream* stream, int additional_amount, int total_amount) {
    //     auto* self = static_cast<AudioHandler*>(userdata);
    //
    //     int bytesPerFrame = sizeof(float) * self->mStreamInfo->channels;
    //     ma_uint32 framesToRead = additional_amount / bytesPerFrame;
    //
    //     //  PCM-Data
    //     std::vector<float> pcmBuffer(framesToRead * self->mStreamInfo->channels);
    //
    //     // miniaudio
    //     ma_uint64 framesRead;
    //     ma_result result = ma_decoder_read_pcm_frames(self->mDecoder, pcmBuffer.data(), framesToRead, &framesRead);
    //
    //     if (framesRead > 0) {
    //         int numSamples =  (int)(framesRead * bytesPerFrame);
    //         self->mEffectsManager->process(pcmBuffer.data(), numSamples, self->mStreamInfo->channels);
    //
    //         SDL_PutAudioStreamData(stream, pcmBuffer.data(), numSamples);
    //     }
    //
    //     if (framesRead < framesToRead) {
    //         int silenceBytes = (framesToRead - (int)framesRead) * bytesPerFrame;
    //         std::vector<uint8_t> silence(silenceBytes, 0);
    //
    //         SDL_PutAudioStreamData(stream, silence.data(), silenceBytes);
    //     }
    // }

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

        // FIXME reconnect [error] Unable to init decoder code:-203


        if (mInitialized ) {
            ma_decoder_uninit(mDecoder);
            mDecoderInitialized = false;
            mInitialized = false;
        }

        if ( !mDecoderInitialized )
            return false;


        mTotalAudioBytesPlayed = 0;
        mEffectsManager->setSampleRate(info->samplerate);

        if (mDecoder == nullptr) {
            dLog("mDecoder was nullptr?! ");
            mDecoder = new ma_decoder();
        }
        memset(mDecoder, 0, sizeof(ma_decoder));
        ma_decoder_config config = ma_decoder_config_init(ma_format_f32, info->channels, info->samplerate);

        // ma_encoding_format_unknown = 0,
        // ma_encoding_format_wav,
        // ma_encoding_format_flac,
        // ma_encoding_format_mp3,
        // ma_encoding_format_vorbis

        if (info->content_type == "audio/aac") {
            // :/ lol bad evaluation !
        } else {
            // Default: audio/mpeg
            config.encodingFormat = ma_encoding_format_mp3;
        }


        // dLog("this: %p, pDecoder: %p (Inhalt)", this, (void*)mDecoder);
        mDecoder->pUserData = this;
        ma_result result = ma_decoder_init(OnReadFromRawBuffer, OnSeekDummy, this, &config, mDecoder);

// FIXME ffmpeg backend
//         ma_libav_decoder_config config = ma_libav_decoder_config_init(ma_format_f32, 2, 44100);
//
//         ma_libav_decoder decoder;
//         ma_result result = ma_libav_decoder_init_file("url", &config, &decoder);
//
//         if (result != MA_SUCCESS) {
//
//         }
// ------------- CALLBACK MUST BE LIKE:
// ma_libav_vfs_callbacks vfsCallbacks;
// vfsCallbacks.onRead = my_vfs_read_callback;
// vfsCallbacks.onSeek = my_vfs_seek_callback;
//
// ma_libav_decoder_init_vfs(my_vfs_userdata, &vfsCallbacks, &config, &decoder);




        if (result != MA_SUCCESS) {
            Log("[error] Unable to init decoder code:%d", result);
            return false;
        }

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

        if (mStreamInfo && !mDecoderInitialized && mRawBuffer.size() >= mPreBufferSize) {
            mDecoderInitialized = true;
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
                self->mCurrentTitle = self->mPendingStreamTitles.front().streamTitle;
                self->mPendingStreamTitles.pop_front();
                if ( self->OnTitleTrigger ) self->OnTitleTrigger();
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
    void AudioHandler::reset ( bool doLock ){
        if (!mDecoderInitialized) return;
        //FIXME on exit: Fatal glibc error: pthread_mutex_lock.c:426 (__pthread_mutex_lock_full): assertion failed: e != ESRCH || !robust
        if (doLock) std::lock_guard<std::recursive_mutex> lock(mBufferMutex);
        mRawBuffer.clear();
        mDecoderInitialized = false;
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
    void AudioHandler::onDisConnected ( bool doLock ){
        reset(doLock);
    }
    // -----------------------------------------------------------------------------
    void AudioHandler::OnStreamTitleUpdate(const std::string streamTitle, const size_t streamPosition){
        if (mCurrentTitle == "")  mCurrentTitle=streamTitle;
        MetaEvent newEvent;
        newEvent.streamTitle = streamTitle;
        newEvent.byteOffset = streamPosition;
        mPendingStreamTitles.push_back(newEvent);
    }

}; //namespace

