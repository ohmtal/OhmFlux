
#include <SDL3/SDL.h>
#include <mutex>

#include <fstream>
#include <vector>
#include <string>
#include <stdexcept>
#include <type_traits>

#include <audio/fluxAudio.h>
#include "soundMixModule.h"
//------------------------------------------------------------------------------
void SDLCALL FinalMixCallback(void *userdata, const SDL_AudioSpec *spec, float *buffer, int buflen) {
    if (!userdata) return;
    auto* soundMix = static_cast<SoundMixModule*>(userdata);

    if (!soundMix )
        return;



    //FIXME ALL EFFECTS NEED TO HANDLE CHANNELS spec->channels ....;
    //    ------- i added an example in limiter :D --------------

    if ( spec->format == SDL_AUDIO_F32 )
    {
        soundMix->getEffectsManager()->lock();
        if (buflen > 0 && soundMix->getEffectsManager()->getEffects().size() > 0)
        {
            int numSamples = buflen / sizeof(float);

            for (auto& effect : soundMix->getEffectsManager()->getEffects()) {
                effect->process(buffer, numSamples, spec->channels);
            }
        }
        soundMix->getEffectsManager()->unlock();
    }

    // NOTE: EXAMPLE CODE:
    // if (is_recording && recording_stream) {
    //     SDL_PutAudioStreamData(recording_stream, buffer, buflen);
    // }
}
//------------------------------------------------------------------------------
bool SoundMixModule::Initialize() {

        mEffectsManager = std::make_unique<DSP::EffectsManager>(true);
        //TEST load em all i dont have the count do i want an extra variable?
        // ... lets bruteforce for this test
        // so we can also test addEffect :P
        // for (S32 i = 0; i < 100; i++) {
        //     mEffectsManager->addEffect(DSP::EffectFactory::Create((DSP::EffectType) i));
        // }
        std::vector<DSP::EffectType> types = {
            DSP::EffectType::OverDrive,
            DSP::EffectType::Bitcrusher,
            DSP::EffectType::SoundCardEmulation,
            DSP::EffectType::RingModulator,
            DSP::EffectType::VoiceModulator,
            DSP::EffectType::Warmth,
            DSP::EffectType::Chorus,
            DSP::EffectType::Reverb,
            DSP::EffectType::Delay,
            DSP::EffectType::Equalizer9Band,
            DSP::EffectType::Limiter,
            DSP::EffectType::SpectrumAnalyzer,
            DSP::EffectType::VisualAnalyzer
        };

        for (auto type : types) {
            auto fx = DSP::EffectFactory::Create(type);
            if (fx) {
                // use defaults ! fx->setEnabled(false);
                mEffectsManager->addEffect(std::move(fx));
            }
        }


        for (const auto& fx : mEffectsManager->getEffects()) {
            Log("[info] Effect %s (Type: %d) is loaded.", fx->getName().c_str(), fx->getType());
        }

        // Setup PostMix
        if (!SDL_SetAudioPostmixCallback(AudioManager.getDeviceID(), FinalMixCallback, this)) {
            dLog("[error] can NOT open PostMix Device !!! %s", SDL_GetError());
        } else {
            Log("[info] SoundMixModule: PostMix Callback installed.");
        }


        Log("[info] SoundMixModule init done.");

        mInitialized = true;
        return true;

    }

