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
void FluxAudioStream::Update(const double& dt)
{
    if (!mInitDone || !mPlaying) return;


    if (mUsePostion) {
        RectF view = Render2D.getCamera()->getVisibleWorldRect(false);
        //FIXME float zoom = Render2D.getCamera()->getZoom();

        // Calculate the center of the camera in world space
        Point2F camCenter = { view.x + (view.w * 0.5f), view.y + (view.h * 0.5f) };

        // Dynamic Max Distance
        // We base the hearing range on the current visible width.
        // This ensures that if you zoom out, your "hearing range" expands.
        float maxDist = (view.w * 0.5f) * 1.5f;
        float maxDistSq = maxDist * maxDist;

        float dSq = camCenter.distSq(mPosition);

        if (dSq > maxDistSq) {
            SDL_SetAudioStreamGain(mStream, 0.0f);
            return;
        }

        // 2. Volume Calculation
        float actualDist = std::sqrt(dSq);
        float volume = 1.0f - (actualDist / maxDist);

        // Apply mGain (user setting) * distance falloff
        SDL_SetAudioStreamGain(mStream, mGain * (volume * volume));

        // Panning
        // Panning also needs to be relative to the visible width
        // float pan = (mPosition.x - camCenter.x) / (view.w * 0.5f);
        // SDL_SetAudioStreamPan(mStream, std::clamp(pan, -1.0f, 1.0f));
    }


    // <<<<<<<<<<<<<<
    // Maintain a 500ms buffer to prevent stuttering
    int targetBytes = mSpec.freq * sizeof(float) * mSpec.channels / 2;

    while (SDL_GetAudioStreamAvailable(mStream) < targetBytes)
    {
        if (mIsOgg && mVorbis)
        {
            // Read 1024 samples per channel at a time
            const int samplesToRead = 1024;
            const int totalFloats = samplesToRead * mSpec.channels;

            // Ensure our conversion buffer is large enough
            if (mConversionBuffer.size() < (size_t)totalFloats) {
                mConversionBuffer.resize(totalFloats);
            }

            // stb_vorbis returns the number of samples per channel actually read
            int samplesRead = stb_vorbis_get_samples_float_interleaved(
                mVorbis,
                mSpec.channels,
                mConversionBuffer.data(),
                                                                       totalFloats
            );

            if (samplesRead > 0) {
                // Shove the interleaved LRLR data into the SDL stream
                SDL_PutAudioStreamData(mStream, mConversionBuffer.data(),
                                       samplesRead * mSpec.channels * sizeof(float));
            }
            else {
                // End of file / Loop logic
                if (mLooping) {
                    stb_vorbis_seek_start(mVorbis);
                } else {
                    mPlaying = false;
                    SDL_FlushAudioStream(mStream);
                    break;
                }
            }
        }  // if (mIsOgg && mVorbis)
        else if (!mIsOgg)
        {
            if (mLooping) {
                if (SDL_GetAudioStreamAvailable(mStream) < (int)mWaveDataLen) {
                    SDL_PutAudioStreamData(mStream, mWavData, (int)mWaveDataLen);
                }
            } else {
                mPlaying = false;
                SDL_FlushAudioStream(mStream);
            }
            break;
        } // if (!mIsOgg)

    } //  while (SDL_GetAudioStreamAvailable(mStream) < targetBytes)
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
    }
    else if (mWavData) {
        // Only WAVs use this direct data push
        if (!SDL_PutAudioStreamData(mStream, mWavData, (int)mWaveDataLen)) {
            Log("Failed to put WAV data: %s", SDL_GetError());
            return false;
        }
        // Do NOT call SDL_FlushAudioStream if you want to loop later
    }

    return SDL_ResumeAudioStreamDevice(mStream);
}//-----------------------------------------------------------------------------
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
