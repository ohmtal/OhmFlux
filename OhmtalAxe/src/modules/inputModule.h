#pragma once

#include <SDL3/SDL.h>
#include <string>
#include <imgui.h>

#include <core/fluxBaseObject.h>
#include <DSP.h>
#include <DSP_EffectsManager.h>
#include <DSP_EffectFactory.h>


namespace SimpleDSP {

struct Visualizer {
    static constexpr int scope_size = 512;
    float scope_buffer[scope_size] = {0};
    std::atomic<int> scope_pos{0};

    void add_sample(float s) {
        int pos = scope_pos.load();
        scope_buffer[pos] = s;
        scope_pos = (pos + 1) % scope_size;
    }
};

} //namespace


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


        // NoiseGate && ToneControl
        mInputEffects = std::make_unique<DSP::EffectsManager>(true);

        std::vector<DSP::EffectType> effTypes = {
            DSP::EffectType::NoiseGate,
            // DSP::EffectType::ToneControl,
            DSP::EffectType::ChromaticTuner,
        };

        for (auto type : effTypes) {
            auto fx = DSP::EffectFactory::Create(type);
            if (fx) {
                // use defaults ! fx->setEnabled(false);
                mInputEffects->addEffect(std::move(fx));
            }
        }


        mInitialized = true;
        return true;
    }

    SimpleDSP::Visualizer mVisuallizer;
    std::unique_ptr<DSP::EffectsManager> mInputEffects = nullptr;



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
