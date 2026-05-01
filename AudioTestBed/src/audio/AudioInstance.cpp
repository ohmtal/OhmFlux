
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
    void SDLCALL MyAudioLoopCallback(void *userdata, SDL_AudioStream *stream, int additional_amount, int total_amount) {
        if (!userdata) return;
        AudioInstance* instance = (AudioInstance*)userdata;
        if (!instance) return;

        if ( additional_amount > 0 ) {
            // SDL_PutAudioStreamData(stream, data->buffer, data->len);
            if ( instance->doLoop ) {
                // switch (instance->resource->fileType) {
                //     case AudioType::WAV:
                //         break;
                //     case AudioType::
                // }
                // SDL_PutAudioStreamDataNoCopy(stream,  instance->resource->mRawData.data(), instance->resource->len, NULL, nullptr);
                instance->Play();
            } else {
                instance->Stop();
            }

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

        switch (resource->fileType) {
            case AudioType::WAV: {
                dLog("playing wav :D");

                // used for loop / isPlaying and position Panning
                SDL_SetAudioStreamGetCallback(stream, MyAudioLoopCallback, this);

                if (!SDL_PutAudioStreamDataNoCopy(stream,  resource->mRawData.data(), resource->len, NULL, nullptr)) {
                    Log("[error] failed to play %s ERROR:%s", resource->fileName.c_str(), SDL_GetError());
                    return false;
                }
                break;
            }

            //............ OGG ..............
            case AudioType::OGG: {
                dLog("FIXME BAD IDEA WHEN LOOPING AUDIO ... ADD UPDATE like before !!!!!");

                if (!vorbisDecoder) {
                    Log("[error] Can play ogg decoder not available!");
                    return false;
                }

                SDL_SetAudioStreamGetCallback(stream, MyAudioLoopCallback, this);

                const int chunkSamples = 4096;
                std::vector<float> floatBuffer(chunkSamples * srcSpec.channels);

                int samplesRead = 0;
                while ((samplesRead = stb_vorbis_get_samples_float_interleaved(vorbisDecoder, srcSpec.channels, floatBuffer.data(), floatBuffer.size())) > 0) {
                    int byteSize = samplesRead * srcSpec.channels * sizeof(float);
                    if (!SDL_PutAudioStreamData(stream, floatBuffer.data(), byteSize)) {
                        Log("[error] SDL_PutAudioStreamData failed: %s", SDL_GetError());
                        return false;
                    }
                }
                SDL_FlushAudioStream(stream);

                stb_vorbis_seek_start(vorbisDecoder);
                // stb_vorbis_close(vorbisDecoder); //FIXME on STOP ?!
                // vorbisDecoder = nullptr;

                break;
            }

            //
            //                 case AudioType::MP3:
            //                     // Using miniaudio to decode from memory
            //                     ma_decoder_init_memory(res->mRawData.data(), res->mRawData.size(), nullptr, &slot.mp3Decoder);
            //                     break;
            //                 case AudioType::SFX:
            //                     break;

            default:
                Log("[error] AudioInstance::Play Unhandled AudioType %d", (int)resource->fileType);
                return false;
        }
        isPlaying = true;
        return true;
    }
    //--------------------------------------------------------------------------
    bool AudioInstance::Stop() {
        if (!resource || !stream || badData || !isInitialized ) return false;
        dLog("Stop playing ...");
        SDL_ClearAudioStream(stream);
        SDL_SetAudioStreamGetCallback(stream, nullptr, nullptr);
        switch (resource->fileType) {

            case AudioType::OGG:
                // if (vorbisDecoder) {
                //     stb_vorbis_close(vorbisDecoder);
                //     vorbisDecoder = nullptr;
                // }

            default: {

            }

        };
        SDL_ClearAudioStream(stream);
        isPlaying = false;

        return true;
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

                Uint8* wavBuf;
                SDL_IOStream* io = SDL_IOFromConstMem(resource->mRawData.data(), resource->mRawData.size());
                if (SDL_LoadWAV_IO(io, true, &srcSpec, &wavBuf, &resource->len)) {
                    stream = SDL_CreateAudioStream(&srcSpec, nullptr);
                    if (!stream) {
                        Log("[error]Couldn't create audio stream: %s", SDL_GetError());
                        SDL_free(wavBuf);
                        return false;
                    }

                    if (!AudioManager.bindStream(stream)) {
                        Log("Failed to bind '%s' stream to device: %s", resource->fileName.c_str(), SDL_GetError());
                        SDL_DestroyAudioStream(stream);
                        stream = nullptr;
                        SDL_free(wavBuf);
                        return false;
                    }
                    SDL_free(wavBuf);
                } else {
                    setBad();
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

                stream = SDL_CreateAudioStream(&srcSpec, nullptr); // Target NULL = SDL konvertiert automatisch zu Device Spec
                if (!AudioManager.bindStream(stream)) {
                    Log("Failed to bind '%s' stream: %s", resource->fileName.c_str(), SDL_GetError());
                    return false;
                }
                break;
            }

            default:
                Log("[error] AudioInstance::Init Unhandled AudioType %d", (int)resource->fileType);
                return false;
        }
        isInitialized = true;
        return true;
    }
    //--------------------------------------------------------------------------
    AudioInstance::~AudioInstance() {
        if ( stream ) { SDL_DestroyAudioStream(stream); stream = nullptr; }
        if ( vorbisDecoder ) { stb_vorbis_close(vorbisDecoder); vorbisDecoder = nullptr; }
    }
    //--------------------------------------------------------------------------



}; //namespace
