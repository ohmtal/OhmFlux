#include <SDL3/SDL.h>
#include <imgui.h>
#include <imgui_internal.h>

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
#include <src/fonts/IconsFontAwesome6.h>

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

        DSP::EffectType::AutoWah,
        DSP::EffectType::Tremolo,

        DSP::EffectType::DistortionBasic,
        DSP::EffectType::OverDrive,
        DSP::EffectType::Metal,
        //  DSP::EffectType::Bitcrusher,
        DSP::EffectType::AnalogGlow,

        // FIXME rack editor
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
//------------------------------------------------------------------------------
void RackModule::DrawRack(bool* p_enabled)
{
    if (!mInitialized
        ||  mEffectsManager == nullptr
        || !*p_enabled) return;


    ImGui::SetNextWindowSizeConstraints(ImVec2(600.0f, 650.f), ImVec2(FLT_MAX, FLT_MAX));
    ImGui::Begin("Rack", p_enabled);


    // ImGui::PushFont(gIconFont);
    ImGui::SetWindowFontScale(2.f);
    ImGui::SeparatorText(getManager()->getActiveRack()->getName().c_str());
    ImGui::SetWindowFontScale(1.f);
    // ImGui::PopFont();

    if (ImGui::BeginTabBar("##RackModule_Effect_Tabs", ImGuiTabBarFlags_None))
    {
        //  ~~~~~~~~~~~ RENDERIU ~~~~~~~~~~~~~~~~~~~~~
        if (ImGui::BeginTabItem("Effects Rack 80th",NULL, ( mRackTabNewId == 1) ? ImGuiTabItemFlags_SetSelected : ImGuiTabItemFlags_None)){
            mEffectsManager->renderUI(0);
            mRackTabCurId= 1;
            ImGui::EndTabItem();
        }
        //  ~~~~~~~~~~~ RENDERIU_WIDE ~~~~~~~~~~~~~~~~~~~~~
        if (ImGui::BeginTabItem("Effects Rack",NULL, (mRackTabNewId == 2) ? ImGuiTabItemFlags_SetSelected : ImGuiTabItemFlags_None))
        {
            mEffectsManager->renderUI(1);
            mRackTabCurId= 2;
            ImGui::EndTabItem();
        }
        //  ~~~~~~~~~~~ RENDERPADDLE ~~~~~~~~~~~~~~~~~~~~~
        if (ImGui::BeginTabItem("Effect Paddles",NULL, (mRackTabNewId == 3) ? ImGuiTabItemFlags_SetSelected : ImGuiTabItemFlags_None)) {
            mEffectsManager->renderUI(2);
            mRackTabCurId= 3;
            ImGui::EndTabItem();
        }
        mRackTabNewId = -1;
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

    DSP::EffectsManager* lManager = getManager();
    int currentIdx = lManager->getActiveRackIndex();


    ImGui::SetNextWindowSizeConstraints(ImVec2(200.0f, 400.f), ImVec2(FLT_MAX, FLT_MAX));
    ImGui::Begin("Rack Presets", p_enabled);

    ImGui::PushFont(gIconFont);

    ImFlux::LCDNumber(currentIdx , 3, 0, 24.0f);
    ImFlux::SameLineBreak(gTBParams.size.x);

    if (ImFlux::ButtonFancy(ICON_FA_CIRCLE_PLUS "##New",gTBParams)) {
        int newId = lManager->addRack();
        lManager->setActiveRack(newId);
        populateRack(lManager->getActiveRack());
    }
    ImFlux::Hint("New default Rack.");

    ImFlux::SameLineBreak(gTBParams.size.x);
    if (ImFlux::ButtonFancy(ICON_FA_FOLDER_OPEN "##Load",gTBParams)) {
        callLoadPresets();
    }
    ImFlux::Hint("Load Presets");

    ImFlux::SameLineBreak(gTBParams.size.x);
    if (ImFlux::ButtonFancy(ICON_FA_FLOPPY_DISK "##Save",gTBParams)) {
        callSavePresets();
    }
    ImFlux::Hint("Save Presets");

    ImFlux::SameLineBreak( 8.f );
    ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
    ImFlux::SameLineBreak(gTBParams.size.x);
    if (ImFlux::ButtonFancy(ICON_FA_EJECT "##Reset",gTBParams.WithColor(IM_COL32(50,20,20,255)))) {
        ImGui::OpenPopup("ConfirmRESETRackPresets");
    }
    ImFlux::Hint("Restore Factory defaults, current Presets will be replaced!");
    // ImFlux::SameLineBreak(gTBParams.size.x);

    ImGui::PopFont(/*gIconFont*/);

    if (ImGui::BeginPopup("ConfirmRESETRackPresets"))
    {
        ImGui::SeparatorText("Confirm to reset the Rack");
        if (ImFlux::ButtonFancy("Yes",gTextButtonParams ))
        {
            if (!lManager->LoadPresets(mFactoryPresetFile)) {
                showMessage("ERROR", "Failed to load Factory Presets.");
            }
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImFlux::ButtonFancy("Cancel",gTextButtonParams )) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }

    ImGui::SeparatorText(std::format("Preset: {}",lManager->getName()).c_str());
    DrawPresetList(lManager);
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



