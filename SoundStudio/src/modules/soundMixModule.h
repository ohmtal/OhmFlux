#pragma once

#include <SDL3/SDL.h>
#include <DSP.h>
#include <DSP_EffectsManager.h>
#include <DSP_EffectFactory.h>

#include <imgui.h>
#include <gui/ImFlux.h>


class SoundMixModule : public FluxBaseObject {
private:
    std::unique_ptr<DSP::EffectsManager> mEffectsManager = nullptr;
    bool mInitialized = false;

public:
    SoundMixModule() = default;

    bool Initialize() override {

        mEffectsManager = std::make_unique<DSP::EffectsManager>(true);


        //TEST load em all i dont have the count do i want an extra variable?
        // ... lets bruteforce for this test
        // so we can also test addEffect :P
        for (S32 i = 0; i < 100; i++) {
            mEffectsManager->addEffect(DSP::EffectFactory::Create((DSP::EffectType) i));
        }

        for (const auto& fx : mEffectsManager->getEffects()) {
            Log("[info] Effect %s (Type: %d) is loaded.", fx->getName().c_str(), fx->getType());
        }


        // FIXME LATER WE DO :

        // // Loaded Effects
        // std::vector<DSP::EffectType> types = {
        //     DSP::EffectType::Bitcrusher,
        //     DSP::EffectType::SoundCardEmulation,
        //     DSP::EffectType::Warmth,
        //     DSP::EffectType::Chorus,
        //     DSP::EffectType::Reverb,
        //     DSP::EffectType::Equalizer9Band,
        //     DSP::EffectType::Limiter
        // };
        //
        // for (auto type : types) {
        //     auto fx = DSP::EffectFactory::Create(type);
        //     if (fx) {
        //         fx->setEnabled(false);
        //         mEffectsManager->addEffect(std::move(fx));
        //     }
        // }

        Log("[info] SoundMixModule init done.");

        mInitialized = true;
        return true;

    }
    //--------------------------------------------------------------------------
    void DrawRack()
    {
        if (!mInitialized ||  mEffectsManager == nullptr) return;
        ImGui::SetNextWindowSizeConstraints(ImVec2(600.0f, 650.f), ImVec2(FLT_MAX, FLT_MAX));
        ImGui::Begin("Post Digital Sound Effects Rack");
        mEffectsManager->renderUI(true);
        ImGui::End();

        //  ~~~~~~~~~~~ TEST RENDERIU ~~~~~~~~~~~~~~~~~~~~~
        ImGui::SetNextWindowSizeConstraints(ImVec2(600.0f, 650.f), ImVec2(FLT_MAX, FLT_MAX));
        ImGui::Begin("Post Digital Sound Effects Rack Alternate Rendering");
        mEffectsManager->renderUI(false);
        ImGui::End();


    }



};
