#pragma once

#include <SDL3/SDL.h>
#include <string>
#include <imgui.h>

#include <utils/byteEncoder.h>
#include <core/fluxBaseObject.h>

#include <DSP.h>
#include <DSP_EffectsManager.h>
#include <DSP_EffectFactory.h>
#include <MonoProcessors/Visualizer.h>



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

    DSP::MonoProcessors::BasicVisualizer mVisuallizer;
    std::unique_ptr<DSP::EffectsManager> mInputEffects = nullptr;



    float (&getBuffer())[BUFFER_SIZE] {
        return buffer;
    }
    SDL_AudioStream *getStream()  { return mOutStream; }


    bool isOpen() const { return mOpen; }
    bool open(SDL_AudioSpec dstSpec = { SDL_AUDIO_F32, 2, 48000});
    bool close();
    bool toggle();

    virtual void Update(const double& dt) override {}
    void DrawInputModuleUI();


    std::string getInputEffectsSettingsBase64( ) {
        if (mInputEffects->getActiveRack()) {
            std::stringstream oss;
            mInputEffects->SaveRackStream(mInputEffects->getActiveRack(), oss);
            ByteEncoder::Base64 lEncoder;
            return lEncoder.encode(oss);
        }
        return "";
    }
    bool setInputEffectsSettingsBase64( std::string settingsBase64) {

        if (settingsBase64.empty()) {
            LogFMT("[error] setInputEffectsSettings failed! Empty INPUT Stream!");
            return false;
        }

        if (mInputEffects->getActiveRack()) {
            ByteEncoder::Base64 lEncoder;
            std::stringstream iss;
            if (!lEncoder.decode(settingsBase64, iss)) {
                LogFMT("[error] setInputEffectsSettings failed! Invalid Stream!");
                return false;
            }



            if (!mInputEffects->LoadRackStream(iss, DSP::EffectsManager::OnlyUpdateExistingSingularity, true)) {
                LogFMT("[error] setInputEffectsSettings failed!\n{}", mInputEffects->getErrors());
                return false;
            }

            return true;
        }
        return false;
    }



}; //CLASS
