//-----------------------------------------------------------------------------
// Copyright (c) 2024 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#include "fluxAudioStream.h"
#include "errorlog.h"
#include "fluxRender2D.h"

#include <algorithm>

#undef STB_VORBIS_HEADER_ONLY
#include "stb/stb_vorbis.c"
//-----------------------------------------------------------------------------
FluxAudioStream::FluxAudioStream( const char* lFilename)
{
    setVisible(false); //sound does not need draw

    mAudioDevice =  SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr);
    if (mAudioDevice == 0)
    {
        Log("Failed to open audio device!");
        return;
    }

    // Determine file type and load accordingly
    std::string path = lFilename;
    if (path.find(".ogg") != std::string::npos || path.find(".OGG") != std::string::npos) {
        mInitDone = loadOGG(lFilename);

    } else {
        mInitDone = loadWAV(lFilename);
    }

    if (!mInitDone) {
        Log("Failed to initialize audio stream for file: %s", lFilename);
    } else {
        dLog("Audio stream loaded for file: %s", lFilename);
    }
}
//-----------------------------------------------------------------------------
FluxAudioStream::~FluxAudioStream()
{
    clearResources();
}
//-----------------------------------------------------------------------------
void FluxAudioStream::clearResources()
{
    if (mStream) { SDL_DestroyAudioStream(mStream); mStream = nullptr; }
    if (mWavData) { SDL_free(mWavData); mWavData = nullptr; }
    if (mVorbis) { stb_vorbis_close(mVorbis); mVorbis = nullptr; }
    mInitDone = false;
    mIsOgg = false;
}
//-----------------------------------------------------------------------------
void FluxAudioStream::FillOggBuffer()
{
    if (!mIsOgg || !mVorbis || !mStream) return;

    // Maintain a 500ms buffer to prevent stuttering
    int targetBytes = mSpec.freq * sizeof(float) * mSpec.channels / 2;

    // Use Available (what's in the resampler) + Queued (what's waiting)
    // to determine if we need more data
    while ((SDL_GetAudioStreamAvailable(mStream) + SDL_GetAudioStreamQueued(mStream)) < targetBytes)
    {
        const int samplesToRead = 1024;
        const int totalFloats = samplesToRead * mSpec.channels;

        if (mConversionBuffer.size() < (size_t)totalFloats) {
            mConversionBuffer.resize(totalFloats);
        }

        int samplesRead = stb_vorbis_get_samples_float_interleaved(
            mVorbis, mSpec.channels, mConversionBuffer.data(), totalFloats
        );

        if (samplesRead > 0) {
            SDL_PutAudioStreamData(mStream, mConversionBuffer.data(),
                                   samplesRead * mSpec.channels * sizeof(float));
        }
        else {
            // End of file logic
            if (mLooping) {
                stb_vorbis_seek_start(mVorbis);
            } else {
                // If we aren't looping and ran out of data, stop and flush
                SDL_FlushAudioStream(mStream);
                break;
            }
        }
    }
}
//-----------------------------------------------------------------------------
void FluxAudioStream::Update(const double& dt)
{
    if (!mInitDone || !mPlaying) return;

    if (mUsePostion) {
        auto camera = Render2D.getCamera();
        RectF view = camera->getVisibleWorldRect(false);
        float zoom = camera->getZoom(); // 1.0 = Default, 0.1 = Far, 5.0 = Close

        Point2F camCenter = { view.x + (view.w * 0.5f), view.y + (view.h * 0.5f) };

        // 1. Calculate the "Hearing Range" based on the current view
        // As you zoom out, view.w increases, naturally expanding the hearing area.
        float maxDist = (view.w * 0.5f) * 1.5f;
        float distSq = camCenter.distSq(mPosition);
        float maxDistSq = maxDist * maxDist;

        if (distSq > maxDistSq) {
            SDL_SetAudioStreamGain(mStream, 0.0f);
            return;
        }

        // 2. Calculate Distance Falloff (0.0 to 1.0)
        float dist = std::sqrt(distSq);
        float falloff = 1.0f - (dist / maxDist);
        falloff = falloff * falloff; // Exponential curve for smoother fading

        // 3. Apply Zoom Scaling
        // Zoom 1.0 should be full volume. Zoom 0.1 (far) should be quieter.
        // We use a linear scale here, but you can clamp it to prevent it from getting too silent.
        float zoomEffect = std::clamp(zoom, 0.f, 1.0f);
        // dLog("zoomEffect: %8.6f", zoomEffect);

        // 4. Final Gain Calculation
        // Total Gain = User Setting * Distance Falloff * Zoom Level
        float finalGain = mGain * falloff * zoomEffect;

        SDL_SetAudioStreamGain(mStream, std::clamp(finalGain, 0.0f, 1.0f));
    }



    // <<<<<<<<<<<<<<

    if (mIsOgg) {
        FillOggBuffer();

        // If we finished playing (non-looping), update state
        if (!mLooping && SDL_GetAudioStreamAvailable(mStream) == 0 && SDL_GetAudioStreamQueued(mStream) == 0) {
            mPlaying = false;
        }
    } else {
        // WAV Loop Logic
        if (mLooping) {
            if (SDL_GetAudioStreamQueued(mStream) < (int)mWaveDataLen / 2) {
                SDL_PutAudioStreamData(mStream, mWavData, (int)mWaveDataLen);
            }
        }
    }
    // // Maintain a 500ms buffer to prevent stuttering
    // int targetBytes = mSpec.freq * sizeof(float) * mSpec.channels / 2;
    //
    // while (SDL_GetAudioStreamAvailable(mStream) < targetBytes)
    // {
    //     if (mIsOgg && mVorbis)
    //     {
    //         // Read 1024 samples per channel at a time
    //         const int samplesToRead = 1024;
    //         const int totalFloats = samplesToRead * mSpec.channels;
    //
    //         // Ensure our conversion buffer is large enough
    //         if (mConversionBuffer.size() < (size_t)totalFloats) {
    //             mConversionBuffer.resize(totalFloats);
    //         }
    //
    //         // stb_vorbis returns the number of samples per channel actually read
    //         int samplesRead = stb_vorbis_get_samples_float_interleaved(
    //             mVorbis,
    //             mSpec.channels,
    //             mConversionBuffer.data(),
    //                                                                    totalFloats
    //         );
    //
    //         if (samplesRead > 0) {
    //             // Shove the interleaved LRLR data into the SDL stream
    //             SDL_PutAudioStreamData(mStream, mConversionBuffer.data(),
    //                                    samplesRead * mSpec.channels * sizeof(float));
    //         }
    //         else {
    //             // End of file / Loop logic
    //             if (mLooping) {
    //                 stb_vorbis_seek_start(mVorbis);
    //             } else {
    //                 mPlaying = false;
    //                 SDL_FlushAudioStream(mStream);
    //                 break;
    //             }
    //         }
    //     }  // if (mIsOgg && mVorbis)
    //     else if (!mIsOgg)
    //     {
    //         if (mLooping) {
    //             if (SDL_GetAudioStreamAvailable(mStream) < (int)mWaveDataLen) {
    //                 SDL_PutAudioStreamData(mStream, mWavData, (int)mWaveDataLen);
    //             }
    //         } else {
    //             mPlaying = false;
    //             SDL_FlushAudioStream(mStream);
    //         }
    //         break;
    //     } // if (!mIsOgg)
    //
    // } //  while (SDL_GetAudioStreamAvailable(mStream) < targetBytes)
}

//-----------------------------------------------------------------------------
bool FluxAudioStream::loadOGG(const char* lFilename) {
    int error;
    mVorbis = stb_vorbis_open_filename(lFilename, &error, nullptr);
    if (!mVorbis) {
        Log("Failed to open OGG: %s", lFilename);
        return false;
    }

    stb_vorbis_info info = stb_vorbis_get_info(mVorbis);

    // Set up SDL spec to match OGG file
    mSpec.format = SDL_AUDIO_F32; // stb_vorbis outputs floats easily
    mSpec.channels = info.channels;
    mSpec.freq = info.sample_rate;

    mStream = SDL_CreateAudioStream(&mSpec, nullptr);
    SDL_BindAudioStream(mAudioDevice, mStream);

    mIsOgg = true;
    mInitDone = true;
    return true;
}

//-----------------------------------------------------------------------------
bool FluxAudioStream::loadWAV(const char * lFilename)
{
    if (mAudioDevice == 0)
        return false;

    mInitDone = false;

    if (mStream) {
        SDL_DestroyAudioStream(mStream);
        mStream = nullptr;
    }

    if (mWavData) {
        SDL_free(mWavData);
        mWavData = nullptr;
        mWaveDataLen = 0;
    }

    mFileName = lFilename;

    SDL_AudioSpec spec;
    /* Load the .wav file from wherever the app is being run from. */
    if (!SDL_LoadWAV(lFilename, &spec, &mWavData, &mWaveDataLen)) {
        Log("Couldn't load .wav file: %s", SDL_GetError());
        return false;
    }

    mStream = SDL_CreateAudioStream(&spec, nullptr);
    if (!mStream) {
        Log("Couldn't create audio stream: %s", SDL_GetError());
        return false;
    }

    if (!SDL_BindAudioStream(mAudioDevice, mStream)) {
        Log("Failed to bind '%s' stream to device: %s", lFilename, SDL_GetError());
        SDL_DestroyAudioStream(mStream);
        mStream = nullptr;
        return false;
    }

    mInitDone = true;
    return true;
}
//-----------------------------------------------------------------------------
bool FluxAudioStream::play()
{
    if (!mInitDone || !mStream) {
        dLog("Cant play audio Stream. initDone:%d, stream:%d", mInitDone, mStream != 0 );
        return false;
    }

    mPlaying = true;
    SDL_ClearAudioStream(mStream); // Clear old leftover data

    if (mIsOgg && mVorbis) {
        // OGGs don't use PutAudioStreamData here;
        // they start empty and are filled in Update()
        stb_vorbis_seek_start(mVorbis);

        this->FillOggBuffer();
    }
    else if (mWavData) {
        // Only WAVs use this direct data push

        if (!SDL_PutAudioStreamData(mStream, mWavData, (int)mWaveDataLen)) {
            Log("Failed to put WAV data: %s", SDL_GetError());
            return false;
        }
        if (! mLooping ) {
            return SDL_FlushAudioStream(mStream);
        }

        // Do NOT call SDL_FlushAudioStream if you want to loop later
    }

    return SDL_ResumeAudioStreamDevice(mStream);
}
//-----------------------------------------------------------------------------
bool FluxAudioStream::stop()
{
    if (!mInitDone)
        return false;

    mPlaying = false;
    return SDL_PauseAudioStreamDevice(mStream);
}
//-----------------------------------------------------------------------------
bool FluxAudioStream::resume()
{
    if (!mInitDone || mPlaying)
        return false;

    mPlaying=true;
    return SDL_ResumeAudioStreamDevice(mStream);
}
//-----------------------------------------------------------------------------
bool FluxAudioStream::setGain(float value) {
    mGain = std::clamp(value, 0.0f, 1.0f);
    if (mStream) {
        return SDL_SetAudioStreamGain(mStream, mGain);
    }
    return false;
}//-----------------------------------------------------------------------------
