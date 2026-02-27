
#include <SDL3/SDL.h>
#include <mutex>

#include <fstream>
#include <vector>
#include <string>
#include <stdexcept>
#include <type_traits>
#include <filesystem>

#include <src/appMain.h>

#include <audio/fluxAudio.h>
#include "soundMixModule.h"

#include "../appGlobals.h"
//------------------------------------------------------------------------------
void SDLCALL FinalMixCallback(void *userdata, const SDL_AudioSpec *spec, float *buffer, int buflen) {
    if (!userdata || !spec || !buffer || buflen < 1) return;
    auto* soundMix = static_cast<SoundMixModule*>(userdata);

    if (!soundMix )
        return;

    float vol = soundMix->getMasterVolume();

    if ( spec->format == SDL_AUDIO_F32 )
    {
        if (buflen > 0)
        {
            int numSamples = buflen / sizeof(float);
            soundMix->getDrumManager()->checkFrequence(spec->freq);
            soundMix->getDrumManager()->process(buffer, numSamples, spec->channels);

            if (getMain()->getAppGui()->getDrumKitLooperModule()) getMain()->getAppGui()->getDrumKitLooperModule()->process(buffer, numSamples, spec->channels);

            // master Volume
            for (int i = 0; i < numSamples; i++) {
                buffer[i] *= vol;
            }

            // analyzer
            soundMix->mSpectrumAnalyzer->process(buffer, numSamples, spec->channels);
            soundMix->mVisualAnalyzer->process(buffer, numSamples, spec->channels);
        }
    }

    // NOTE: EXAMPLE CODE:
    // if (is_recording && recording_stream) {
    //     SDL_PutAudioStreamData(recording_stream, buffer, buflen);
    // }
}
//------------------------------------------------------------------------------
void SoundMixModule::DrawVisualAnalyzer(bool* p_enabled) {
    if (!mInitialized ||  !*p_enabled) return;
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

    ImGui::PushID(this);
    ImGui::SeparatorText("Pads");
    ImVec2 padSize = ImVec2(100.f,100.f);
    for (auto& drum : mDrumManager->getActiveRack()->getEffects()) {
        drum->renderDrumPad(padSize);
        ImFlux::SameLineBreak(padSize.x);
    }
    ImGui::PopID();


    ImGui::End();

}
//------------------------------------------------------------------------------
bool SoundMixModule::close() {

    bool result = false;
    if (!SDL_SetAudioPostmixCallback(AudioManager.getDeviceID(), nullptr, nullptr)) {
        Log("[error] can NOT unbind PostMix Device !!! %s", SDL_GetError());
        result |= false;
    } else {
        Log("[info] SoundMixModule: PostMix callback uninstalled.");
        result |= true;

    }

    return result;
}
//------------------------------------------------------------------------------
struct DrumCreateConfig {
    const DSP::EffectType Type = DSP::EffectType::KickDrum;
    const std::string customName = "";
    const uint32_t customColor = 0;
};

bool SoundMixModule::Initialize() {


    mDrumManager = std::make_unique<DSP::EffectsManager>(true);
    std::vector<DrumCreateConfig> drumList = {
        { DSP::EffectType::KickDrum},
        { DSP::EffectType::SnareDrum},
        { DSP::EffectType::HiHat, "Open Hat"},
        { DSP::EffectType::HiHat, "Closed Hat", IM_COL32(10,100,10,255)},
        { DSP::EffectType::TomDrum},
        { DSP::EffectType::Cymbals},
    };
    for (auto drum : drumList) {
        auto fx = DSP::EffectFactory::Create(drum.Type);
        if (fx) {
            fx->setEnabled(true); //drums on be default
            fx->setCustomName(drum.customName);
            fx->setCustomColor32(drum.customColor);
            if ( drum.Type == DSP::EffectType::HiHat ) {
                if (drum.customName == "Closed Hat") {
                    auto* hat = static_cast<DSP::HiHat*>(fx.get());
                    auto s = hat->getSettings();
                    if (auto p = s.findPresetByName("Closed")) {
                        p->apply(s);
                        hat->setSettings(s);
                    } else {
                        Log("[error] Can NOT find preset for closed HiHat!");
                    }
                }
            }
            mDrumManager->addEffect(std::move(fx));
        }
    }





    mSpectrumAnalyzer = cast_unique<DSP::SpectrumAnalyzer>(DSP::EffectFactory::Create(DSP::EffectType::SpectrumAnalyzer));
    mVisualAnalyzer = cast_unique<DSP::VisualAnalyzer>(DSP::EffectFactory::Create(DSP::EffectType::VisualAnalyzer));


    // Setup PostMix
    if (!SDL_SetAudioPostmixCallback(AudioManager.getDeviceID(), FinalMixCallback, this)) {
        Log("[error] can NOT open PostMix Device !!! %s", SDL_GetError());
    } else {
        Log("[info] SoundMixModule: PostMix Callback installed.");
    }

    Log("[info] SoundMixModule init done.");

    mInitialized = true;
    return true;
}

