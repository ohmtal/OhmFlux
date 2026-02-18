
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

    float vol = soundMix->getMasterVolume();

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

            soundMix->getDrumManager()->checkFrequence(spec->freq);
            soundMix->getDrumManager()->process(buffer, numSamples, spec->channels);


            soundMix->mDrumKitLooper.process(buffer, numSamples, spec->channels);


            // master Volume
            for (int i = 0; i < numSamples; i++) {
                buffer[i] *= vol;
            }

            // analyzer
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

    //FIXME move the rack managemane to taskbar
    if (ImGui::BeginChild("RackManagement", ImVec2(0.f, 50.f))) {
        DSP::EffectsManager* lManager = getEffectsManager();

        int currentIdx = lManager->getActiveRackIndex();
        int count = lManager->getPresetsCount();


        ImGui::Text("Preset count: %d, Active Rack: [%d] %s (%d effects)",
                    count,
                    currentIdx,
                    lManager->getActiveRack()->getName().c_str(),
                    lManager->getActiveRack()->getEffectsCount()
        );

        if (count > 0)
        {
            if (ImFlux::ButtonFancy("<")) {
                currentIdx--;
                if (currentIdx < 0) currentIdx = count -1;
                lManager->setActiveRack(currentIdx);
            }
            ImGui::SameLine();
            ImFlux::LCDNumber(currentIdx , 3, 0, 24.0f);
            ImGui::SameLine();
            if (ImFlux::ButtonFancy(">")) {
                currentIdx++;
                if (currentIdx >= count) currentIdx = 0;
                lManager->setActiveRack(currentIdx);
            }
            ImGui::SameLine();

            char nameBuf[64];
            strncpy(nameBuf, lManager->getActiveRack()->getName().c_str(), sizeof(nameBuf));
            ImGui::Text("Name");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(200);
            if (ImGui::InputText("##Rack Name", nameBuf, sizeof(nameBuf))) {
                lManager->getActiveRack()->setName(nameBuf);
            }

            ImGui::SameLine();
            if (ImFlux::ButtonFancy("New")) {
                int newId = lManager->addRack();
                lManager->setActiveRack(newId);
                populateRack(lManager->getActiveRack());
            }
            ImGui::SameLine();
            if (ImFlux::ButtonFancy("Clone")) {
                int newId = lManager->cloneCurrent();
                lManager->setActiveRack(newId);
            }
            ImGui::SameLine();
            if (count < 2) ImGui::BeginDisabled();
            if (ImFlux::ButtonFancy("Delete")) {
                int newId = lManager->removeRack(currentIdx);
                lManager->setActiveRack(newId);
            }
            if (count < 2) ImGui::EndDisabled();
        } // count > 0



    }
    ImGui::EndChild();
    //<<< rack management
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
void SoundMixModule::populateRack(DSP::EffectsRack* lRack){
    std::vector<DSP::EffectType> types = {
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
        DSP::EffectType::ToneControl,
        DSP::EffectType::Limiter,
    };


    // manually !!
    // DSP::EffectType::DrumKit,
    // DSP::EffectType::SpectrumAnalyzer,
    // DSP::EffectType::VisualAnalyzer


    for (auto type : types) {
        auto fx = DSP::EffectFactory::Create(type);
        if (fx) {
            lRack->getEffects().push_back(std::move(fx));
        }
    }


}

//------------------------------------------------------------------------------
bool SoundMixModule::Initialize() {

    mPresetsFile =
    getGame()->mSettings.getPrefsPath()
    .append(getGame()->mSettings.getSafeCaption())
    .append(".rack.presets");

    mDrumKitFile =
    getGame()->mSettings.getPrefsPath()
    .append(getGame()->mSettings.getSafeCaption())
    .append(".drum");



    mEffectsManager = std::make_unique<DSP::EffectsManager>(true);
    populateRack(mEffectsManager->getActiveRack());


    // for (auto type : types) {
    //     auto fx = DSP::EffectFactory::Create(type);
    //     if (fx) {
    //         // use defaults ! fx->setEnabled(false);
    //         mEffectsManager->addEffect(std::move(fx));
    //     }
    // }

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


    if (!mEffectsManager->LoadPresets(mPresetsFile)) LogFMT(mEffectsManager->getErrors());

    mInitialized = true;
    return true;

}
//------------------------------------------------------------------------------
