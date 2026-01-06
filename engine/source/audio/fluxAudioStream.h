//-----------------------------------------------------------------------------
// Copyright (c) 2024 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#pragma once
#ifndef _FLUXAUDIOSTREAM_H_
#define _FLUXAUDIOSTREAM_H_


#include <SDL3/SDL.h>
#include <vector>
#include <string>

#include "core/fluxGlobals.h"
#include "core/fluxBaseObject.h"
#include "utils/errorlog.h"

#define STB_VORBIS_HEADER_ONLY
#include "stb_vorbis.c"

class FluxAudioStream : public FluxBaseObject
{
private:
    const char* mFileName = nullptr;
    bool mInitDone = false;
    bool mPlaying = false;
    bool mLooping = false;
    SDL_AudioStream *mStream = nullptr;
    Uint8 *mWavData = nullptr;
    Uint32 mWaveDataLen = 0;
    // SDL_AudioDeviceID mAudioDevice;
    float mGain = 1.f; //volume 0.f..1.f

    // position
    bool mUsePostion = false;
    Point2F mPosition = { 0.f, 0.f };

    //OGG
    stb_vorbis* mVorbis = nullptr;
    void* mRawFileData = nullptr;  //for SDL_LoadFile

    SDL_AudioSpec mSpec;
    bool mIsOgg = false;
    std::vector<float> mConversionBuffer;

    void clearResources();
    void FillOggBuffer();

protected:
    bool loadWAV(const char * lFilename);
    bool loadOGG(const char* lFilename);

public:
    FluxAudioStream( const char* lFilename);
    ~FluxAudioStream();

    bool play();
    bool stop();
    bool resume();
    void Update(const double& dt) override;
    bool isPlaying() {return mPlaying;}
    void setLooping(bool value) {mLooping = value;}
    bool setGain(float value);
    float getGain() { return mGain; }

    void setPositon( Point2F lPos)  {
        mPosition = lPos;
        mUsePostion = true;
    }
    bool getInitDone() { return mInitDone; }

}; //class

#endif //_FLUXAUDIOSTREAM_H_
