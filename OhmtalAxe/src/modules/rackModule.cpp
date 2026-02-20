
#include <SDL3/SDL.h>
#include <mutex>

#include <fstream>
#include <vector>
#include <string>
#include <stdexcept>
#include <type_traits>
#include <filesystem>

#include <audio/fluxAudio.h>

#include <src/appMain.h>
#include "src/appGlobals.h"

#include "soundMixModule.h"
//------------------------------------------------------------------------------
RackModule::~RackModule(){
    if (!mEffectsManager->SavePresets(mPresetsFile)) LogFMT(mEffectsManager->getErrors());
}
//------------------------------------------------------------------------------
DSP::EffectsManager* RackModule::getManager() const {
    return mEffectsManager.get();
}
//------------------------------------------------------------------------------
bool RackModule::Initialize() {
    mPresetsFile =
    getGame()->mSettings.getPrefsPath()
    .append("Save.axe");


    mFactoryPresetFile = getGamePath().append("assets/Factory.axe");

    mEffectsManager = std::make_unique<DSP::EffectsManager>(true);
    getManager()->setName("Sound Effects");
    populateRack(mEffectsManager->getActiveRack());

    bool presetExits = std::filesystem::exists(mPresetsFile);
    if (!mEffectsManager->LoadPresets(presetExits ? mPresetsFile : mFactoryPresetFile)) {
        LogFMT("[error] "+mEffectsManager->getErrors());
    }

    mInitialized = true;
    Log("[info] Rack init done.");
    return true;

}
//------------------------------------------------------------------------------
void RackModule::populateRack(DSP::EffectsRack* lRack) {
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
void RackModule::DrawRack(bool* p_enabled)
{
    if (!mInitialized
        ||  mEffectsManager == nullptr
        || !*p_enabled) return;



    // if (!ImGui::Begin("Rack", p_enabled))
    // {
    //     ImGui::End();
    //     return;
    // }

    ImGui::SetNextWindowSizeConstraints(ImVec2(600.0f, 650.f), ImVec2(FLT_MAX, FLT_MAX));
    ImGui::Begin("Rack", p_enabled);

    //FIXME move the rack managemane to taskbar
    if (ImGui::BeginChild("RackManagement", ImVec2(0.f, 50.f))) {
        DSP::EffectsManager* lManager = getManager();

        int currentIdx = lManager->getActiveRackIndex();
        int count = lManager->getPresetsCount();



        if (count  > 0 && currentIdx >= 0)
        {
            ImGui::Text("Preset count: %d, Active Rack: [%d] %s (%d effects)",
                        count,
                        currentIdx,
                        lManager->getActiveRack()->getName().c_str(),
                        lManager->getActiveRack()->getEffectsCount()
            );

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
            if (ImFlux::ButtonFancy("Save")) {
                callSavePresets();
            }
            ImGui::SameLine();
            if (ImFlux::ButtonFancy("Load")) {
                callLoadPresets();
            }
            ImGui::SameLine();
            if (count < 2) ImGui::BeginDisabled();
            if (ImFlux::ButtonFancy("Delete")) {
                int newId = lManager->removeRack(currentIdx);
                lManager->setActiveRack(newId);
            }
            if (count < 2) ImGui::EndDisabled();

            ImGui::SameLine();
            if (ImFlux::ButtonFancy("restore Factory defaults", ImFlux::RED_BUTTON.WithSize(ImVec2(200.f,24.f)))) {

                if (!lManager->LoadPresets(mFactoryPresetFile)) {
                    showMessage("ERROR", "Failed to load Factory Presets.");

                }
            }
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
void RackModule::setSampleRate(float sampleRate){
    if (!mInitialized) return;
    getManager()->checkFrequence(sampleRate);
}
//------------------------------------------------------------------------------
void RackModule::process(float* buffer, int numSamples, int numChannels) {
    if (!mInitialized) return;
    getManager()->process(buffer, numSamples, numChannels);
}
//------------------------------------------------------------------------------
void RackModule::DrawEffectManagerPresetListWindow(bool* p_enabled) {
    if (!mInitialized ||  mEffectsManager == nullptr || !*p_enabled) return;
    ImGui::SetNextWindowSizeConstraints(ImVec2(200.0f, 400.f), ImVec2(FLT_MAX, FLT_MAX));
    ImGui::Begin("Rack Presets", p_enabled);
    DrawPresetList(mEffectsManager.get());
    ImGui::End(/*"Rack Presets", p_enabled*/);

}
//------------------------------------------------------------------------------
void RackModule::callSavePresets() {
    g_FileDialog.setFileName(fluxStr::sanitizeFilenameWithUnderScores(getManager()->getName())+".axe");
    g_FileDialog.mSaveMode = true;
    g_FileDialog.mSaveExt = ".axe";
    g_FileDialog.mFilters = {".axe"};
    g_FileDialog.mLabel = "Save Rack Presets (.axe)";
    g_FileDialog.mDirty = true;
    g_FileDialog.mWasOpen = getMain()->getAppGui()->getAppSettings()->mShowFileBrowser;
    getMain()->getAppGui()->getAppSettings()->mShowFileBrowser = true; //FIXME RESET ?!

}
void RackModule::callLoadPresets() {
    g_FileDialog.setFileName(fluxStr::sanitizeFilenameWithUnderScores(getManager()->getName())+".axe");
    g_FileDialog.mSaveMode = false;
    g_FileDialog.mSaveExt = ".axe";
    g_FileDialog.mFilters = {".axe"};
    g_FileDialog.mLabel = "Load Rack Presets (.axe)";
    g_FileDialog.mDirty = true;

    g_FileDialog.mWasOpen = getMain()->getAppGui()->getAppSettings()->mShowFileBrowser;
    getMain()->getAppGui()->getAppSettings()->mShowFileBrowser = true; //FIXME RESET ?!

}


