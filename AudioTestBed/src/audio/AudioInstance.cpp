
#include "AudioInstance.h"
#include "utils/errorlog.h"

#undef STB_VORBIS_HEADER_ONLY
#include <stb_vorbis.c>

#define MA_NO_DEVICE_IO
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

namespace FluxAudio {

    //--------------------------------------------------------------------------
    inline const char* get_vorbis_error_string(int error) {
        switch (error) {
            case VORBIS__no_error:                  return "No error";
            case VORBIS_need_more_data:             return "Need more data";
            case VORBIS_invalid_setup:              return "Invalid setup (header corrupt)";
            case VORBIS_invalid_stream:             return "Invalid stream / Not a Vorbis file";
            case VORBIS_missing_capture_pattern:    return "Missing capture pattern (OggS)";
            case VORBIS_invalid_stream_structure_version: return "Invalid stream structure version";
            case VORBIS_continued_packet_flag_invalid: return "Continued packet flag invalid";
            case VORBIS_incorrect_stream_serial_number: return "Incorrect stream serial number";
            case VORBIS_invalid_first_page:         return "Invalid first page";
            case VORBIS_bad_packet_type:            return "Bad packet type";
            case VORBIS_cant_find_last_page:        return "Can't find last page";
            case VORBIS_seek_failed:                return "Seek failed";
            case VORBIS_outofmem:                   return "Out of memory";
            case VORBIS_seek_without_length:        return "Seek without length";
            case VORBIS_file_open_failure:          return "File open failure";
            case VORBIS_unexpected_eof:             return "Unexpected EOF";
            default:                                return "Unknown error";
        }
    }


    //--------------------------------------------------------------------------
    void AudioInstance::setBad(){
        badData = true;
        if (resource) {
            AudioResourceManager.blacklist(resource->fileName);
            resource = nullptr;
        }
    }
    //--------------------------------------------------------------------------
    bool AudioInstance::Play() {
        if (!resource || !stream || badData || !isInitialized ) return false;

        SDL_ClearAudioStream(stream);
        switch (resource->fileType) {
            case AudioType::WAV: {
                mSamplePos = 0;
                break;
            }
            case AudioType::OGG: {
                if (!vorbisDecoder) return false;
                stb_vorbis_seek_start(vorbisDecoder);
                break;
            }

            case AudioType::MP3:
            case AudioType::FLAC:
            {
                if (!maDecoder.pBackend) return false;
                ma_result res = ma_decoder_seek_to_pcm_frame(&maDecoder, 0);
                if (res != MA_SUCCESS) {
                    Log("[error] Audio: MP3/FLAC Seek failed (%s)", resource->fileName.c_str());
                    if (OnFatalError) OnFatalError("MP3/FLAC Seek failed!");
                    return false;
                }
                mSamplePos = 0;
                break;
            }

            case AudioType::SFX: {
                if (!mSFXGen) return false;
                mSFXGen->PlaySample();
                mSamplePos = 0;
                break;
            }

            default:
                return false;
        }

        AudioManager.bindStream(stream);
        isPlaying = true;
        return isPlaying;
    }
    //--------------------------------------------------------------------------
    bool AudioInstance::Stop() {
        if (!resource || !stream || badData || !isInitialized ) return false;
        if (!isPlaying) return false;

        isPlaying = false;

        return AudioManager.unBindStream(stream);
    }
    //--------------------------------------------------------------------------
    bool AudioInstance::Resume() {
        if (!resource || !stream || badData || !isInitialized ) return false;
        if (isPlaying) return false;

        isPlaying=true;
        return AudioManager.bindStream(stream);

    }
    //--------------------------------------------------------------------------
    bool AudioInstance::Initialize(std::string fileName){
        if (badData) return false;
        if (resource) return false; //allready done !
        return Initialize( AudioResourceManager.get(fileName) );
    }
    //--------------------------------------------------------------------------
    bool AudioInstance::Initialize(ResourceData* lResource) {
        if (badData) return false;
        // allow resource overwrite ! if (resource) return false; // we have a resource!
        if (isInitialized) return false;
        resource = lResource;
        doLoop = resource->enableLoop; //preset default
        switch (resource->fileType) {
            case AudioType::WAV: {

                srcSpec = resource->wavSrcSpec;
                dstSpec = srcSpec;
                dstSpec.format = SDL_AUDIO_F32;


                //NOTE: dstSpec is correct since it's converted to this!
                stream = SDL_CreateAudioStream( &dstSpec, nullptr);
                // stream = SDL_CreateAudioStream(&srcSpec, &dstSpec);
                if (!stream) {
                    Log("[error]Couldn't create audio stream: %s", SDL_GetError());
                    if (OnFatalError) OnFatalError(std::format("Couldn't create audio stream: {}", SDL_GetError()));
                    return false;
                }

                if (!AudioManager.bindStream(stream)) {
                    Log("Failed to bind '%s' stream to device: %s", resource->fileName.c_str(), SDL_GetError());
                    SDL_DestroyAudioStream(stream);
                    stream = nullptr;
                    return false;
                }

                setSampleLenAndDuration();

                // mSamplePos = 0;
                // mSampleLen = resource->mRawData.size();
                // int bytesPerFrame = (SDL_AUDIO_BITSIZE(dstSpec.format) / 8) * dstSpec.channels;
                // double wavFrames = mSampleLen / bytesPerFrame;
                // mSampleDuration =  wavFrames / dstSpec.freq;


                break;
            }
            //............ OGG ..............
            case AudioType::OGG: {
                int error;
                vorbisDecoder = stb_vorbis_open_memory(resource->mRawData.data(), (int)resource->mRawData.size(), &error, nullptr);
                if (!vorbisDecoder) {
                    Log("Failed to open OGG: %s ERROR:%s", resource->fileName.c_str(), get_vorbis_error_string(error));
                    setBad();
                    return false;
                }

                stb_vorbis_info info = stb_vorbis_get_info(vorbisDecoder);

                srcSpec.format = SDL_AUDIO_F32;
                srcSpec.channels = info.channels;
                srcSpec.freq = info.sample_rate;

                dstSpec = srcSpec;


                // stream = SDL_CreateAudioStream(&srcSpec, &dstSpec);
                stream = SDL_CreateAudioStream(&srcSpec, nullptr);
                if (!AudioManager.bindStream(stream)) {
                    Log("Failed to bind '%s' stream: %s", resource->fileName.c_str(), SDL_GetError());
                    return false;
                }

                setSampleLenAndDuration();

                dLog("SAMPLE LEN = %d", (int)mSampleLen);

                break;
            }

            // ............ MP3/FLAC (miniaudio) ..............
            case AudioType::MP3:
            case AudioType::FLAC:
            {
                ma_decoder_config config = ma_decoder_config_init(ma_format_f32, 0, 0);

                ma_result res = ma_decoder_init_memory(resource->mRawData.data(), resource->mRawData.size(), &config, &maDecoder);

                if (res != MA_SUCCESS) {
                    Log("[error] Audio: Failed to open MP3/FLAC: %s", resource->fileName.c_str());
                    if (OnFatalError) OnFatalError("Failed to open MP3/FLAC.");
                    setBad();
                    return false;
                }

                srcSpec.format   = SDL_AUDIO_F32;
                srcSpec.channels = maDecoder.outputChannels;
                srcSpec.freq     = maDecoder.outputSampleRate;

                dstSpec = srcSpec;

                stream = SDL_CreateAudioStream(&srcSpec, nullptr);
                if (!AudioManager.bindStream(stream)) {
                    Log("[error] Audio: Failed to bind '%s' stream: %s", resource->fileName.c_str(), SDL_GetError());
                    if (OnFatalError) OnFatalError(std::format("Failed to bind stream: {}", SDL_GetError()));
                    ma_decoder_uninit(&maDecoder);
                    return false;
                }

                setSampleLenAndDuration();

                dLog("MP3/FLAC SAMPLE LEN = %d", (int)mSampleLen);
                break;
            }

            // ........ SFX .............
            case AudioType::SFX: {
                mSFXGen = new SFXGeneratorStereo();
                if (!mSFXGen->LoadFromMemory(resource->mRawData)) {
                    Log("[error] Audio SFX. Failed to load data.");
                    if (OnFatalError) OnFatalError("Failed to load SFX Data.");
                }

                srcSpec.format = SDL_AUDIO_F32;
                srcSpec.channels = 2;
                srcSpec.freq =  44100 ;
                dstSpec = srcSpec;

                // stream = SDL_CreateAudioStream(&srcSpec, &dstSpec);
                stream = SDL_CreateAudioStream(&srcSpec, nullptr);
                if (!AudioManager.bindStream(stream)) {
                    Log("Failed to bind '%s' stream: %s", resource->fileName.c_str(), SDL_GetError());
                    return false;
                }

                setSampleLenAndDuration();

                if ( mAutoConvertSfxToWav ) {
                    FluxSchedule.callDeferred([this](){
                        this->ConvertToWav();
                    });
                }


                break;
            }



            default:
                Log("[error] AudioInstance::Init Unhandled AudioType %d", (int)resource->fileType);
                if (OnFatalError) OnFatalError(std::format("AudioInstance::Init Unhandled AudioType: {}", (int)resource->fileType));
                setBad();
                return false;
        }
        isInitialized = true;
        return true;
    }
    //--------------------------------------------------------------------------
    AudioInstance::~AudioInstance( ) {
        if ( stream ) { SDL_DestroyAudioStream(stream); stream = nullptr; }
        if ( vorbisDecoder ) { stb_vorbis_close(vorbisDecoder); vorbisDecoder = nullptr; }
        if ( maDecoder.pBackend ) {ma_decoder_uninit(&maDecoder); maDecoder.pBackend=nullptr; }
        if ( mSFXGen ) { SAFE_DELETE(mSFXGen); mSFXGen = nullptr; }

    }
    //--------------------------------------------------------------------------
    bool AudioInstance::fillBuffer() {
        if (!resource || resource->mRawData.empty()) return false;

        const int CHUNK_SIZE = 4096;

        switch (resource->fileType) {

            case AudioType::WAV: {
                if (mAudioBuffer.size() != CHUNK_SIZE * dstSpec.channels) {
                    mAudioBuffer.resize(CHUNK_SIZE * dstSpec.channels);
                }

                if (mSamplePos >= resource->mRawData.size()) {
                    if (doLoop) mSamplePos = 0;
                    else return false;
                }

                int bytesPerSampleSource = SDL_AUDIO_BYTESIZE(srcSpec.format);
                size_t bytesToReadFromSource = CHUNK_SIZE * srcSpec.channels * bytesPerSampleSource;

                if (mSamplePos + bytesToReadFromSource > resource->mRawData.size()) {
                    bytesToReadFromSource = resource->mRawData.size() - mSamplePos;
                }

                Uint8* tempDst = nullptr;
                int outLen = 0;

                if (SDL_ConvertAudioSamples(&srcSpec, resource->mRawData.data() + mSamplePos, (int)bytesToReadFromSource,
                    &dstSpec, &tempDst, &outLen))
                {
                    mAudioBuffer.assign((float*)tempDst, (float*)(tempDst + outLen));
                    SDL_free(tempDst);
                } else {
                    return false;
                }

                mSamplePos += bytesToReadFromSource;

                // dLog("WAV wavOffset: %d", (int)wavOffset);

                break;
            }

            // .......... OGG .............
            case AudioType::OGG: {
                if (mAudioBuffer.size() != CHUNK_SIZE * dstSpec.channels) {
                    mAudioBuffer.resize(CHUNK_SIZE * dstSpec.channels);
                }

                int samplesRead = stb_vorbis_get_samples_float_interleaved(
                    vorbisDecoder, dstSpec.channels, mAudioBuffer.data(), (int)mAudioBuffer.size());

                if (samplesRead <= 0) {
                    if (doLoop) {
                        stb_vorbis_seek_start(vorbisDecoder);
                        samplesRead = stb_vorbis_get_samples_float_interleaved(
                            vorbisDecoder, dstSpec.channels, mAudioBuffer.data(), (int)mAudioBuffer.size());
                    } else {
                        return false; // EOF
                    }
                }

                if (samplesRead < CHUNK_SIZE) {
                    mAudioBuffer.resize(samplesRead * dstSpec.channels);
                }

                mSamplePos = stb_vorbis_get_sample_offset(vorbisDecoder);
                // dLog("OGG samplesRead: %d, %d of %d, %f played", samplesRead, (int)mSamplePos, (int)mSampleLen, getProgress());
                break;
            }

            // .......... MP3 (miniaudio) .............
            case AudioType::MP3:
            case AudioType::FLAC:
            {
                if (mAudioBuffer.size() != CHUNK_SIZE * dstSpec.channels) {
                    mAudioBuffer.resize(CHUNK_SIZE * dstSpec.channels);
                }

                ma_uint64 framesRead = 0;
                ma_result res = ma_decoder_read_pcm_frames(&maDecoder, mAudioBuffer.data(), CHUNK_SIZE, &framesRead);

                if (framesRead == 0) {
                    if (doLoop) {
                        ma_decoder_seek_to_pcm_frame(&maDecoder, 0);
                        res = ma_decoder_read_pcm_frames(&maDecoder, mAudioBuffer.data(), CHUNK_SIZE, &framesRead);
                    } else {
                        return false; // EOF
                    }
                }

                if (framesRead < CHUNK_SIZE) {
                    mAudioBuffer.resize(framesRead * dstSpec.channels);
                }

                ma_uint64 currentFrame;
                ma_decoder_get_cursor_in_pcm_frames(&maDecoder, &currentFrame);
                mSamplePos = (uint32_t)currentFrame;
                // dLog("MP3 %d of %d, %f played",  (int)mSamplePos, (int)mSampleLen, getProgress());

                break;
            }
            case AudioType::SFX: {
                if (!mSFXGen) return false;

                if ( !mSFXGen->mState.playing_sample ) {
                    if (doLoop)  { mSFXGen->PlaySample();  mSamplePos = 0; }
                    else return false; //DONE
                }

                int framesToRead = CHUNK_SIZE;
                int remainingFrames = mSampleLen - mSamplePos;
                if ( framesToRead >  remainingFrames  )
                    framesToRead = remainingFrames;


                if (mAudioBuffer.size() != framesToRead * dstSpec.channels) {
                    mAudioBuffer.resize(framesToRead * dstSpec.channels);
                }

                mSFXGen->SynthSample(framesToRead, mAudioBuffer.data());
                mSamplePos += framesToRead;

                break;
            }

            // ....... UNKNOWN ........
            default:
                return false;
        }
        return true;
    }
    //--------------------------------------------------------------------------
    // void AudioInstance::Update( const double& dt, Point3F* camPos ) {
    void AudioInstance::UpdateStream() {
        if (!isPlaying) return;

        const float cacheSec = 0.250f; //was 0.1 => 100
        const int targetQueueSize = (int)(dstSpec.freq * dstSpec.channels * sizeof(float) * cacheSec);
        if (SDL_GetAudioStreamQueued(stream) < targetQueueSize) {
            if ( fillBuffer() ) {
                for (auto& sample : mAudioBuffer) {
                    // dLog("sample: %f", sample);
                    sample *= volume;
                }
                if (OnAudioProcess) OnAudioProcess(mAudioBuffer.data(),mAudioBuffer.size());
                SDL_PutAudioStreamData(stream, mAudioBuffer.data(), mAudioBuffer.size() * sizeof(float));

            } else {
                SDL_FlushAudioStream(stream);
                if (SDL_GetAudioStreamQueued(stream) == 0) {
                    isPlaying = false;
                    if (OnStreamEnds) OnStreamEnds();
                    dLog("Sound finished playing.");
                }
            }
        }
    }
    //--------------------------------------------------------------------------
    bool AudioInstance::ConvertToWav() {
        if (!resource || resource->mRawData.empty()) return false;
        if ( isPlaying ) {
            Log("[error] ConvertToWav only allowed when not playing!");
            if (OnFatalError) OnFatalError("ConvertToWav only allowed when not playing!");
            return false;
        }
        switch (resource->fileType) {
            case AudioType::SFX: {
                if (!mSFXGen) {
                    Log("[error] Convert SFX To WAV: SFX Generator not Initialized!");
                    if (OnFatalError) OnFatalError("Convert SFX To WAV: SFX Generator not Initialized!");
                    return false;
                }
                resource->fileType = FluxAudio::AudioType::WAV;
                resource->wavSrcSpec = mSFXGen->getSpec();
                std::vector<float> f32ExportBuffer;
                mSFXGen->exportToBuffer(f32ExportBuffer, nullptr, false);
                resource->mRawData.resize(f32ExportBuffer.size() * sizeof(float));
                std::memcpy(resource->mRawData.data(), f32ExportBuffer.data(), resource->mRawData.size());
                // no need to reinitialize since both are F32 but i need to update the len:
                setSampleLenAndDuration();

                // finally delete the mSFXGen
                SAFE_DELETE(mSFXGen);
                mSFXGen = nullptr;

                Log("[info] SFX converted to wav (%s)", resource->fileName.c_str());
                return true;

            }
            default:
                // i also could do this on other formats using miniaudio but why.
                Log("[warn] ConvertToWav unsupported Audioformat.");
                return false;
        }

    }
    // -------------------------------------------------------------------------
    const size_t AudioInstance::getFrames() {
        if (!isInitialized) return false;
        switch (resource->fileType) {
            case AudioType::WAV: {
                return mWavFrames;
                break;
            }
            case AudioType::OGG:
            case AudioType::MP3:
            case AudioType::FLAC:
            case AudioType::SFX:
            {
                // all this formats use frames as sampleLen
                return mSampleLen;
                break;
            }
            default: {
                Log("Invalid AudioType (%d) in AudioInstance::getFrames", (int)resource->fileType);
                return 0;
                break;
            }
        }
        return 0;
    }

    void AudioInstance::setSampleLenAndDuration() {

        mSamplePos = 0;
        mBytesPerFrame = (SDL_AUDIO_BITSIZE(dstSpec.format) / 8) * dstSpec.channels;

        switch (resource->fileType) {
            case AudioType::WAV: {
                mSampleLen = resource->mRawData.size();
                mWavFrames = resource->mRawData.size() / mBytesPerFrame;
                mSampleDuration =  (double)mWavFrames / dstSpec.freq;

                break;
            }
            case AudioType::OGG: {
                if (!vorbisDecoder) {
                     if (OnFatalError)  OnFatalError("missing vorbisDecoder");
                     return;
                }

                mSampleLen = stb_vorbis_stream_length_in_samples(vorbisDecoder);
                mSampleDuration = (double)mSampleLen / vorbisDecoder->sample_rate;

                break;
            }
            case AudioType::MP3:
            case AudioType::FLAC: {

                if (!maDecoder.pBackend) {
                  if (OnFatalError) OnFatalError("miniaudio Decoder");
                  return;
                }
                ma_uint64 totalFrames;
                ma_decoder_get_length_in_pcm_frames(&maDecoder, &totalFrames);
                mSampleLen = (uint32_t)totalFrames;
                mSampleDuration = (double)mSampleLen / maDecoder.outputSampleRate;

                break;
            }
            case AudioType::SFX: {
                mSampleLen = mSFXGen->getSyntFrames();
                mSampleDuration =  (double)mSampleLen / dstSpec.freq;

                break;
            }
            default: {
                if (OnFatalError)  OnFatalError("setSampleLenAndDuration unknown audio filetype!");
                break;
            }
        }

        dLog("SampleDuration %.2f", mSampleDuration);

    }


}; //namespace
