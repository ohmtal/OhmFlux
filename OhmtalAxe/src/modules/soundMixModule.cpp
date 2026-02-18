
#include <SDL3/SDL.h>
#include <mutex>

#include <fstream>
#include <vector>
#include <string>
#include <stdexcept>
#include <type_traits>

#include <src/appMain.h>

#include <audio/fluxAudio.h>
#include "soundMixModule.h"

//FIXME: Select Rack example :
// int currentIdx = getCurrentRackIndex();
// if (ImGui::BeginCombo("Select Rack", mPresets[currentIdx]->mName.c_str())) {
//     for (int n = 0; n < mPresets.size(); n++) {
//         bool is_selected = (currentIdx == n);
//         if (ImGui::Selectable(mPresets[n]->mName.c_str(), is_selected)) {
//             setActiveRack(n);
//         }
//     }
//     ImGui::EndCombo();
// }


//------------------------------------------------------------------------------
void SDLCALL FinalMixCallback(void *userdata, const SDL_AudioSpec *spec, float *buffer, int buflen) {
    if (!userdata || !spec || !buffer || buflen < 1) return;
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

            soundMix->getEffectsManager()->checkFrequence(spec->freq);
            soundMix->getEffectsManager()->process(buffer, numSamples, spec->channels);

            soundMix->mDrumManager->checkFrequence(spec->freq);
            soundMix->mDrumManager->process(buffer, numSamples, spec->channels);


            soundMix->mDrumKitLooper.process(buffer, numSamples, spec->channels);


            soundMix->mSpectrumAnalyzer->process(buffer, numSamples, spec->channels);
            soundMix->mVisualAnalyzer->process(buffer, numSamples, spec->channels);


            // for (auto& effect : soundMix->getEffectsManager()->getEffects()) {
            //     effect->process(buffer, numSamples, spec->channels);
            // }
        }
        soundMix->getEffectsManager()->unlock();
    }

    // NOTE: EXAMPLE CODE:
    // if (is_recording && recording_stream) {
    //     SDL_PutAudioStreamData(recording_stream, buffer, buflen);
    // }
}
//------------------------------------------------------------------------------
void SoundMixModule::DrawVisualAnalyzer(bool* p_enabled) {
    if (!mInitialized ||  mEffectsManager == nullptr || !*p_enabled) return;


        ImGui::SetNextWindowSizeConstraints(ImVec2(600.0f, 650.f), ImVec2(FLT_MAX, FLT_MAX));
        ImGui::Begin("Visualizer", p_enabled);

        {
            ImGui::PushID("SpectrumAnalyzer_Effect_Row");
            ImGui::BeginGroup();
            bool isEnabled = mSpectrumAnalyzer->isEnabled();
            if (ImFlux::LEDCheckBox(mSpectrumAnalyzer->getName(), &isEnabled, mSpectrumAnalyzer->getColor()))
                mSpectrumAnalyzer->setEnabled(isEnabled);
            float fullWidth = ImGui::GetContentRegionAvail().x;
            mSpectrumAnalyzer->DrawSpectrumAnalyzer(ImVec2(fullWidth, 80.0f));
            ImGui::EndGroup();
            ImGui::PopID();
            ImGui::Spacing();
        }

        {
            ImGui::PushID("VisualAnalyzer_Effect_Row");
            ImGui::BeginGroup();
            bool isEnabled = mVisualAnalyzer->isEnabled();
            if (ImFlux::LEDCheckBox(mVisualAnalyzer->getName(), &isEnabled, mVisualAnalyzer->getColor()))
                mVisualAnalyzer->setEnabled(isEnabled);
            float fullWidth = ImGui::GetContentRegionAvail().x;
            mVisualAnalyzer->renderPeakTest(); //FIXME
            ImGui::EndGroup();
            ImGui::PopID();
            ImGui::Spacing();
        }

        ImGui::End();

    }

//------------------------------------------------------------------------------
void SoundMixModule::DrawDrums(bool* p_enabled) {
    if (!mInitialized ||  mDrumManager == nullptr || !*p_enabled) return;

    ImGui::SetNextWindowSizeConstraints(ImVec2(600.0f, 650.f), ImVec2(FLT_MAX, FLT_MAX));
    ImGui::Begin("Drum Pads", p_enabled);
    mDrumManager->renderUI(3);
    ImGui::End();

}
//------------------------------------------------------------------------------
void SoundMixModule::DrawRack(bool* p_enabled)
{
    if (!mInitialized ||  mEffectsManager == nullptr || !*p_enabled) return;


    // if (!ImGui::Begin("Rack", p_enabled))
    // {
    //     ImGui::End();
    //     return;
    // }

    ImGui::SetNextWindowSizeConstraints(ImVec2(600.0f, 650.f), ImVec2(FLT_MAX, FLT_MAX));
    ImGui::Begin("Rack", p_enabled);


    //FIXME save last selected in settings !
    if (ImGui::BeginTabBar("##tabs", ImGuiTabBarFlags_None))
    {
        //  ~~~~~~~~~~~ RENDERIU ~~~~~~~~~~~~~~~~~~~~~
        if (ImGui::BeginTabItem("Effects Rack 80th")){
            mEffectsManager->renderUI(0);
            ImGui::EndTabItem();
        }
        //  ~~~~~~~~~~~ RENDERIU_WIDE ~~~~~~~~~~~~~~~~~~~~~
        if (ImGui::BeginTabItem("Effects Rack"))
        {
            mEffectsManager->renderUI(1);
            ImGui::EndTabItem();
        }
        //  ~~~~~~~~~~~ RENDERPADDLE ~~~~~~~~~~~~~~~~~~~~~
        if (ImGui::BeginTabItem("Effect Paddles")) {
            mEffectsManager->renderUI(2);
            ImGui::EndTabItem();
        }
    } //tabBar ....
    ImGui::EndTabBar();
    ImGui::End();


}
//------------------------------------------------------------------------------
bool SoundMixModule::Initialize() {

    mEffectsManager = std::make_unique<DSP::EffectsManager>(true);

    std::vector<DSP::EffectType> types = {
        DSP::EffectType::ToneControl,
        // DSP::EffectType::NoiseGate,
        // DSP::EffectType::ChromaticTuner,
        DSP::EffectType::DistortionBasic,
        DSP::EffectType::OverDrive,
        DSP::EffectType::Metal,
        // DSP::EffectType::Bitcrusher,
        DSP::EffectType::AnalogGlow,

        // DSP::EffectType::RingModulator,
        // DSP::EffectType::VoiceModulator,
        // DSP::EffectType::Warmth,

        DSP::EffectType::Chorus,
        DSP::EffectType::Reverb,
        DSP::EffectType::Delay,
        DSP::EffectType::Equalizer9Band,
        DSP::EffectType::Limiter,
    };


    // manually !!
    // DSP::EffectType::DrumKit,
    // DSP::EffectType::SpectrumAnalyzer,
    // DSP::EffectType::VisualAnalyzer


    for (auto type : types) {
        auto fx = DSP::EffectFactory::Create(type);
        if (fx) {
            // use defaults ! fx->setEnabled(false);
            mEffectsManager->addEffect(std::move(fx));
        }
    }

    mDrumManager = std::make_unique<DSP::EffectsManager>(true);

    std::vector<DSP::EffectType> drumTypes = {
        DSP::EffectType::KickDrum,
    };
    for (auto type : drumTypes) {
        auto fx = DSP::EffectFactory::Create(type);
        if (fx) {
            fx->setEnabled(true); //drums on be default
            mDrumManager->addEffect(std::move(fx));
        }
    }





    mSpectrumAnalyzer = cast_unique<DSP::SpectrumAnalyzer>(DSP::EffectFactory::Create(DSP::EffectType::SpectrumAnalyzer));
    mVisualAnalyzer = cast_unique<DSP::VisualAnalyzer>(DSP::EffectFactory::Create(DSP::EffectType::VisualAnalyzer));



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


    //FIXME TEST !!
        mEffectsManager->LoadRack("bla.rack", DSP::EffectsManager::OnlyUpdateExistingSingularity); //only existing ...

    mInitialized = true;
    return true;

}
//------------------------------------------------------------------------------
