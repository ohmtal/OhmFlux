#pragma once

#include <SDL3/SDL.h>
#include <DSP.h>
#include <DSP_EffectsManager.h>
#include <DSP_EffectFactory.h>

class SoundMixModule : public FluxBaseObject {
private:
    DSP::EffectsManager* mEffectsManager;

public:
    SoundMixModule() = default;

    bool Initialize() override {

        auto effman = std::make_unique<DSP::EffectsManager>();
        mEffectsManager =  effman.get();

        //TEST load em all i dont have the count do i want an extra variable?
        // ... lets bruteforce for this test
        // so we can also test addEffect :P
        for (S32 i = 0; i < 100; i++) {
            mEffectsManager->addEffect(DSP::EffectFactory::Create((DSP::EffectType) i));
        }

        for (const auto& fx : mEffectsManager->getEffects()) {
            Log("[info] Effect %s (Type: %d) is loaded.", fx->getName().c_str(), fx->getType());
        }

        Log("[info] SoundMixModule init done.");

        return true;

    }



};
