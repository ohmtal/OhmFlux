#include <SDL3/SDL.h>

#include <src/appMain.h>
#include "src/appGlobals.h"

#include "drumKitLooper.h"
#include <src/fonts/IconsFontAwesome6.h>

//------------------------------------------------------------------------------
DSP::EffectsManager* DrumKitLooperModule::getManager() const{
    return mDrumKitManager.get();
}
//------------------------------------------------------------------------------
bool DrumKitLooperModule::Initialize() {

    mDrumKitFile =
    getGame()->mSettings.getPrefsPath()
    .append(getGame()->mSettings.getSafeCaption())
    .append(".drum");

    mDrumKitManager = std::make_unique<DSP::EffectsManager>(true);

    getManager()->getActiveRack()->setName("new DrumKit");

    addDrumKit();

    bool presetExits = std::filesystem::exists(mDrumKitFile);
    if (presetExits && !getManager()->LoadPresets(mDrumKitFile)) {
        LogFMT("[error] " + getManager()->getErrors());
    }

    mInitialized = true;
    Log("[info] DrumKit init done.");
    return true;
}
//------------------------------------------------------------------------------
DrumKitLooperModule::~DrumKitLooperModule(){
    if (mInitialized) {
        getManager()->setEnabled(false);
        if (!getManager()->SavePresets(mDrumKitFile)) LogFMT(getManager()->getErrors());
    }
}
//------------------------------------------------------------------------------
DSP::DrumKit* DrumKitLooperModule::getDrumKit() {
    if (!mInitialized) return nullptr;

    auto* fx = getManager()->getEffectByType(DSP::EffectType::DrumKit);
    if (!fx) return nullptr;
    return static_cast<DSP::DrumKit*>(fx);
}
//------------------------------------------------------------------------------
void DrumKitLooperModule::process(float* buffer, int numSamples, int numChannels){
    if (!mInitialized) return ;
    getManager()->process(buffer, numSamples, numChannels);
}
//------------------------------------------------------------------------------
void DrumKitLooperModule::start() {
    bool isEnabled = getDrumKit()->isEnabled();
    if (!isEnabled) getDrumKit()->setEnabled(true);
}
void DrumKitLooperModule::stop() {
    bool isEnabled = getDrumKit()->isEnabled();
    if (isEnabled) getDrumKit()->setEnabled(false);
}

void DrumKitLooperModule::toogleDrumKit(){
    if (!mInitialized) return ;
    bool isEnabled = getDrumKit()->isEnabled();
    // dLog("[info] toogle DrumKit to %d", !isEnabled);
    getDrumKit()->setEnabled(!isEnabled);
}
//------------------------------------------------------------------------------
void DrumKitLooperModule::DrawUI(bool* p_open){
    if (!mInitialized) return ;
    getDrumKit()->renderSequencerWindow(p_open);

    //FIXME ... extra window ...
    if (*p_open) {
        ImGui::SetNextWindowSizeConstraints(ImVec2(200.0f, 400.f), ImVec2(FLT_MAX, FLT_MAX));
        int currentIdx = getManager()->getActiveRackIndex();
        ImGui::Begin("DrumKit Presets"/*, p_enabled*/);

        float sizeLen = 120.f;
        ImVec2 p = ImGui::GetCursorScreenPos();
        p += ImGui::GetContentRegionAvail() - ImVec2(sizeLen + 20.f ,sizeLen + 20.f);

        ImFlux::DrawDrumKitSymbol(p, sizeLen, 0.f);


        ImGui::PushFont(gIconFont);

        ImFlux::LCDNumber(currentIdx , 3, 0, 24.0f, ImFlux::COL32_NEON_CYAN);
        ImFlux::SameLineBreak(gTBParams.size.x);

        if (ImFlux::ButtonFancy(ICON_FA_CIRCLE_PLUS "##New",gTBParams)) {
            int newId = getManager()->addRack();
            getManager()->setActiveRack(newId);
            addDrumKit();
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
        ImGui::PopFont();

        getManager()->DrawPresetList(0.1f);
        ImGui::End();
    }

}
//------------------------------------------------------------------------------
bool DrumKitLooperModule::addDrumKit(){
    auto fx = DSP::EffectFactory::Create(DSP::EffectType::DrumKit);
    if (!fx) return false;

    getManager()->getEffects().push_back(std::move(fx));
    getManager()->getActiveRack()->setName("new DrumKit");
    return true;
}
//------------------------------------------------------------------------------
void DrumKitLooperModule::callSavePresets() {
    g_FileDialog.setFileName(fluxStr::sanitizeFilenameWithUnderScores(getManager()->getName())+".drum");
    g_FileDialog.mSaveMode = true;
    g_FileDialog.mSaveExt = ".drum";
    g_FileDialog.mFilters = {".drum"};
    g_FileDialog.mLabel = "Save Drum Kit (.drum)";
    g_FileDialog.mDirty = true;
    g_FileDialog.mWasOpen = getMain()->getAppGui()->getAppSettings()->mShowFileBrowser;
    getMain()->getAppGui()->getAppSettings()->mShowFileBrowser = true;

}
void DrumKitLooperModule::callLoadPresets() {
    g_FileDialog.setFileName(fluxStr::sanitizeFilenameWithUnderScores(getManager()->getName())+".drum");
    g_FileDialog.mSaveMode = false;
    g_FileDialog.mSaveExt = ".drum";
    g_FileDialog.mFilters = {".drum"};
    g_FileDialog.mLabel = "Load Drum Kit Presets (.drum)";
    g_FileDialog.mDirty = true;

    g_FileDialog.mWasOpen = getMain()->getAppGui()->getAppSettings()->mShowFileBrowser;
    getMain()->getAppGui()->getAppSettings()->mShowFileBrowser = true;

}



