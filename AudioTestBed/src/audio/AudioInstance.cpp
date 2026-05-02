
#include "AudioInstance.h"
#include "utils/errorlog.h"

#undef STB_VORBIS_HEADER_ONLY
#include <stb_vorbis.c>


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
                wavOffset = 0;
                break;
            }
            case AudioType::OGG: {
                if (!vorbisDecoder) return false;
                stb_vorbis_seek_start(vorbisDecoder);
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
        if (resource) return false; // we have a resource!
        resource = lResource;
        doLoop = resource->enableLoop; //preset default
        switch (resource->fileType) {
            case AudioType::WAV: {

                srcSpec = resource->wavSrcSpec;
                dstSpec = srcSpec;
                dstSpec.format = SDL_AUDIO_F32;

                stream = SDL_CreateAudioStream(&srcSpec, &dstSpec);
                if (!stream) {
                    Log("[error]Couldn't create audio stream: %s", SDL_GetError());
                    return false;
                }

                if (!AudioManager.bindStream(stream)) {
                    Log("Failed to bind '%s' stream to device: %s", resource->fileName.c_str(), SDL_GetError());
                    SDL_DestroyAudioStream(stream);
                    stream = nullptr;
                    return false;
                }
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


                stream = SDL_CreateAudioStream(&srcSpec, &dstSpec);
                if (!AudioManager.bindStream(stream)) {
                    Log("Failed to bind '%s' stream: %s", resource->fileName.c_str(), SDL_GetError());
                    return false;
                }
                break;
            }

            default:
                Log("[error] AudioInstance::Init Unhandled AudioType %d", (int)resource->fileType);
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
    }
    //--------------------------------------------------------------------------
    bool AudioInstance::fillBuffer() {
        if (!resource || resource->mRawData.empty()) return false;

        const int CHUNK_SIZE = 4096;

        switch (resource->fileType) {

            // case AudioType::WAV: {
            //     const int CHUNK_SAMPLES = 4096; // Samples pro Kanal
            //     int numChannels = srcSpec.channels;
            //     size_t totalSamplesToRead = CHUNK_SAMPLES * numChannels;
            //
            //     mAudioBuffer.resize(totalSamplesToRead);
            //
            //     size_t bytesToRead = totalSamplesToRead * sizeof(int16_t);
            //
            //     if (wavOffset + bytesToRead > resource->mRawData.size()) {
            //         bytesToRead = resource->mRawData.size() - wavOffset;
            //         mAudioBuffer.resize(bytesToRead / sizeof(int16_t));
            //     }
            //
            //     const int16_t* sourcePtr = reinterpret_cast<const int16_t*>(resource->mRawData.data() + wavOffset);
            //
            //     for (size_t i = 0; i < mAudioBuffer.size(); ++i) {
            //         // S16 Range ist -32768 bis 32767. Wir teilen durch 32768.0, um auf float -1.0 bis 1.0 zu kommen.
            //         mAudioBuffer[i] = static_cast<float>(sourcePtr[i]) / 32768.0f;
            //     }
            //
            //     wavOffset += bytesToRead;
            //     break;
            // }


//lol hours spend for nothing ...
            // case AudioType::WAV: {
            //     int bytesPerSample = SDL_AUDIO_BYTESIZE(srcSpec.format);
            //     size_t chunkBytes = CHUNK_SIZE * srcSpec.channels * bytesPerSample;
            //
            //     if (wavOffset >= resource->mRawData.size()) {
            //         if (doLoop) {
            //             wavOffset = 0;
            //         } else {
            //             return false;
            //         }
            //     }
            //     if (wavOffset + chunkBytes > resource->mRawData.size()) {
            //         chunkBytes = resource->mRawData.size() - wavOffset;
            //     }
            //     if (!SDL_PutAudioStreamData(stream, resource->mRawData.data() + wavOffset, (int)chunkBytes)) {
            //         return false;
            //     }
            //     wavOffset += chunkBytes;
            //     dLog("WAV wavOffset: %d", (int)wavOffset);
            //     break;
            // }
            case AudioType::WAV: {
                if (mAudioBuffer.size() != CHUNK_SIZE * dstSpec.channels) {
                    mAudioBuffer.resize(CHUNK_SIZE * dstSpec.channels);
                }

                if (wavOffset >= resource->mRawData.size()) {
                    if (doLoop) wavOffset = 0;
                    else return false;
                }

                int bytesPerSampleSource = SDL_AUDIO_BYTESIZE(srcSpec.format);
                size_t bytesToReadFromSource = CHUNK_SIZE * srcSpec.channels * bytesPerSampleSource;

                if (wavOffset + bytesToReadFromSource > resource->mRawData.size()) {
                    bytesToReadFromSource = resource->mRawData.size() - wavOffset;
                }


                // bytesToReadFromSource = resource->mRawData.size(); //TEST: suck all in

                Uint8* tempDst = nullptr;
                int outLen = 0;

                if (SDL_ConvertAudioSamples(&srcSpec, resource->mRawData.data() + wavOffset, (int)bytesToReadFromSource,
                    &dstSpec, &tempDst, &outLen))
                {
                    mAudioBuffer.assign((float*)tempDst, (float*)(tempDst + outLen));
                    SDL_free(tempDst);
                } else {
                    return false;
                }

                wavOffset += bytesToReadFromSource;

                dLog("WAV wavOffset: %d", (int)wavOffset);

                break;
            }
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

                // dLog("OGG samplesRead: %d", samplesRead);
                break;
            }

            default:
                return false;
        }
        return true;
    }
    //--------------------------------------------------------------------------
    void AudioInstance::Update( const double& dt, Point3F* camPos ) {
        if (!isPlaying) return;

        // 0.1 = 100ms
        const int targetQueueSize = (int)(dstSpec.freq * dstSpec.channels * sizeof(float) * 0.1f);
        if (SDL_GetAudioStreamQueued(stream) < targetQueueSize) {
            if ( fillBuffer() ) {

                if ( dstSpec.format == SDL_AUDIO_F32 ) {
                    for (auto& sample : mAudioBuffer) {
                        dLog("sample: %f", sample);
                        sample *= volume;
                    }
                }

                SDL_PutAudioStreamData(stream, mAudioBuffer.data(), mAudioBuffer.size() * sizeof(float));

            } else {
                SDL_FlushAudioStream(stream);
                if (SDL_GetAudioStreamQueued(stream) == 0) {
                    isPlaying = false;
                    dLog("Sound finished playing.");
                }
            }
        }


    }




}; //namespace
