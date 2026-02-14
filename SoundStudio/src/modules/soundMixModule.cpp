
#include <SDL3/SDL.h>
#include <mutex>

#include <fstream>
#include <vector>
#include <string>
#include <stdexcept>
#include <type_traits>

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
void SoundMixModule::DrawRack()
    {
        if (!mInitialized ||  mEffectsManager == nullptr) return;
        ImGui::SetNextWindowSizeConstraints(ImVec2(600.0f, 650.f), ImVec2(FLT_MAX, FLT_MAX));
        ImGui::Begin("Post Digital Sound Effects Rack");
        mEffectsManager->renderUI(1);
        ImGui::End();

        ImGui::SetNextWindowSizeConstraints(ImVec2(600.0f, 650.f), ImVec2(FLT_MAX, FLT_MAX));
        ImGui::Begin("Post Digital Sound Effects Visualizer");
        if (auto* analyzer = getEffectsManager()->getSpectrumAnalyzer()) {
            ImGui::PushID("SpectrumAnalyzer_Effect_Row");
            ImGui::BeginGroup();
            bool isEnabled = analyzer->isEnabled();
            if (ImFlux::LEDCheckBox(analyzer->getName(), &isEnabled, analyzer->getColor()))
                analyzer->setEnabled(isEnabled);
            float fullWidth = ImGui::GetContentRegionAvail().x;
            analyzer->DrawSpectrumAnalyzer(ImVec2(fullWidth, 80.0f));
            ImGui::EndGroup();
            ImGui::PopID();
            ImGui::Spacing();
        }


        if (auto* analyzer = getEffectsManager()->getEqualizer9Band())
         if (analyzer->isEnabled()) {
            ImGui::PushID("Equalizer9Band_Effect_Row");
            ImGui::BeginGroup();
            float fullWidth = ImGui::GetContentRegionAvail().x;
            analyzer->renderCurve(ImVec2(fullWidth, 60.0f));
            ImGui::EndGroup();
            ImGui::PopID();
            ImGui::Spacing();
        }


        if (auto* analyzer = getEffectsManager()->getVisualAnalyzer()) {
            ImGui::PushID("VisualAnalyzer_Effect_Row");
            ImGui::BeginGroup();
            bool isEnabled = analyzer->isEnabled();
            if (ImFlux::LEDCheckBox(analyzer->getName(), &isEnabled, analyzer->getColor()))
                analyzer->setEnabled(isEnabled);
            float fullWidth = ImGui::GetContentRegionAvail().x;
            analyzer->renderPeakTest(); //FIXME
            ImGui::EndGroup();
            ImGui::PopID();
            ImGui::Spacing();
        }

        ImGui::End();


        //  ~~~~~~~~~~~ TEST RENDERIU ~~~~~~~~~~~~~~~~~~~~~
        ImGui::SetNextWindowSizeConstraints(ImVec2(600.0f, 650.f), ImVec2(FLT_MAX, FLT_MAX));
        ImGui::Begin("Post Digital Sound Effects Rack Alternate Rendering");
        mEffectsManager->renderUI(0);
        ImGui::End();

        //  ~~~~~~~~~~~ TEST RENDERPADDLE ~~~~~~~~~~~~~~~~~~~~~
        ImGui::SetNextWindowSizeConstraints(ImVec2(600.0f, 650.f), ImVec2(FLT_MAX, FLT_MAX));
        ImGui::Begin("Post Digital Sound Effects Rack Paddles TEST ");
        mEffectsManager->renderUI(2);
        ImGui::End();


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
            DSP::EffectType::NoiseGate,
            DSP::EffectType::ChromaticTuner,
            DSP::EffectType::DistortionBasic,
            DSP::EffectType::OverDrive,
            DSP::EffectType::Metal,
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
            DSP::EffectType::DrumKit,
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


         mEffectsManager->LoadRack("bla.rack", DSP::EffectsManager::OnlyUpdateExistingSingularity); //only existing ...

        mInitialized = true;
        return true;

    }
//------------------------------------------------------------------------------
