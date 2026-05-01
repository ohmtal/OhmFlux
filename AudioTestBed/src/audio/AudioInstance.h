//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Audio Renderer FIXME rename to AudioInstance!
//-----------------------------------------------------------------------------
// Audio Renderer prototype
// [ ] Initialize / Play
//  [ ] Wav
//  [ ] OGG
//  [ ] MP3
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
        Point2F position    = {};
        Point3F box         = {100.f, 100.f, 2.f};
        //--------
        //Resource
        ResourceData* resource = nullptr;
        // decoder / steam
        SDL_AudioStream* stream = nullptr;
        stb_vorbis* vorbisDecoder = nullptr;
        // we hold the spec
        SDL_AudioSpec srcSpec;
        SDL_AudioSpec dstSpec;

        // Buffer
        std::vector<float> mAudioBuffer;

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

        //------- Put Data in
        void Update( Point3F camPos );

    private:
        void fillBuffer();

    }; //AudioInstance

    //--------------------------------------------------------------------------




}; //namespace
