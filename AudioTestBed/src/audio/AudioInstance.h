//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Audio Renderer FIXME rename to AudioInstance!
//-----------------------------------------------------------------------------
// Audio Renderer prototype
// [ ] Initialize / Play
//  [X] Wav
//  [X] OGG
//  [X] MP3
//  [ ] SFX
// [ ] DSP::Processors::Panning3D
// [ ] FluxAudio::Manager handle list of instances
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

    void SDLCALL MyAudioLoopCallback(void *userdata, SDL_AudioStream *stream, int additional_amount, int total_amount); //fwd

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
        void Update( const double& dt, Point3F* camPos = nullptr );


        float getProgress() {
            if (mSampleLen == 0) return 0.f;
            return (float)mSamplePos / (float)mSampleLen;
        }

        // float* buffer, size_t numSamples
        std::function<void(const float*, size_t)> OnAudioProcess = nullptr;

        const SDL_AudioSpec getSpec() { return dstSpec; }
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
