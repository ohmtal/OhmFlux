#pragma once

#include <SDL3/SDL.h>
#include <string>
#include <imgui.h>

#include <core/fluxBaseObject.h>
#include <DSP.h>

class InputModule : public FluxBaseObject {
public:
    static const int BUFFER_SIZE = 4096;


private:

    bool mOpen = false;
    bool mInitialized = false;

    float buffer[BUFFER_SIZE] = {0};
    SDL_AudioStream *mOutStream = nullptr;
    SDL_AudioStream* mInStream   = nullptr;

    // NO ..effects here !! DSP::NoiseGate*  mNoiseGate = nullptr;


public:
    // set by hardware spec !!
    SDL_AudioSpec mInputSpec = { SDL_AUDIO_F32, 1, 48000 };


    InputModule() = default;
    ~InputModule() {
        close();
    }
    bool Initialize() override  {


        mInitialized = true;
        return true;
    }


    float (&getBuffer())[BUFFER_SIZE] {
        return buffer;
    }
    SDL_AudioStream *getStream()  { return mOutStream; }


    bool isOpen() const { return mOpen; }
    bool open(SDL_AudioSpec dstSpec = { SDL_AUDIO_F32, 2, 44100});
    bool close();


    virtual void Update(const double& dt) override {}


    void DrawInputModuleUI();
}; //CLASS
