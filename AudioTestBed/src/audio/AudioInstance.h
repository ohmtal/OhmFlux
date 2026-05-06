//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Audio Renderer prototype
// [ ] Initialize / Play
//  [X] Wav
//  [X] OGG
//  [X] MP3
//  [X] SFX
//  [X] FLAC
// [ ] DSP::Processors::Panning3D
// [ ] FluxAudio::Manager handle list of instances
//-----------------------------------------------------------------------------
// NOTE: MiniAudio rocks
// I could have used miniaudio for WAV and OGG too but since i did WAV and OGG
// first I keep what i added and tested before.
//
// This would be the key to use OGG with miniaudio and stb_vorbis:
// #define MA_HAS_VORBIS
// #include "stb_vorbis.c"
// #include "miniaudio.h"
//-----------------------------------------------------------------------------
#pragma once

#include "SDL3/SDL.h"
#include <string>

#include "audio/fluxAudio.h"
#include "AudioType.h"
#include "AudioResourceManager.h"
#include <core/fluxGlobals.h>
#include <core/fluxBaseObject.h>
#include <miniaudio.h>
#include "SFXGeneratorStereo.h"
#include <functional>
//----
#include "utils/errorlog.h"

struct stb_vorbis; //FWD

namespace FluxAudio {


    struct AudioInstance {
        //--------
        // params
        bool doLoop = false;
        // volume
        float volume = 1.f;
        // position :
        bool usePosition    = false;
        Point3F position    = {};
        // in 2d it is x,y,cam zoom
        Point3F box         = {100.f, 100.f, 2.f};
        //--------
        //Resource
        ResourceData* resource = nullptr;

        // save cpu power
        bool mAutoConvertSfxToWav = true;

        // status
        bool isPlaying    = false; //<< fixme !!

        bool isInitialized = false;
        bool badData = false;



        //-------- Initialize
        bool Initialize( ResourceData* lResource);
        bool Initialize(std::string fileName);

        ~AudioInstance();

        //-------- Tag as bad data
        void setBad();

        //-------- Stop / Play
        bool Play();
        bool Stop();
        bool Resume();

        //------- Put Data in
        // void Update( const double& dt, Point3F* camPos = nullptr );
        void UpdateStream();


        float getProgress() {
            if (mSampleLen == 0) return 0.f;
            return (float)mSamplePos / (float)mSampleLen;
        }

        // float* buffer, size_t numSamples
        std::function<void(const float*, size_t)> OnAudioProcess = nullptr;
        const SDL_AudioSpec getSpec() { return dstSpec; }

        //events
        std::function<void()> OnStreamEnds = nullptr;
        std::function<void(std::string)> OnFatalError = nullptr;


        bool ConvertToWav();

    private:

        // decoder / steam
        SDL_AudioStream* stream = nullptr;
        stb_vorbis* vorbisDecoder = nullptr;
        ma_decoder maDecoder;

        SFXGeneratorStereo*  mSFXGen = nullptr;

        // we hold the spec
        SDL_AudioSpec srcSpec;
        SDL_AudioSpec dstSpec;


        // Buffer
        size_t mSamplePos = 0;
        size_t mSampleLen = 0;
        std::vector<float> mAudioBuffer;
        bool fillBuffer();
    }; //AudioInstance

    //--------------------------------------------------------------------------




}; //namespace
