#include <SDL3/SDL.h>

#include <src/appMain.h>
#include "src/appGlobals.h"

#include "drumKitLooper.h"

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

    auto fx = DSP::EffectFactory::Create(DSP::EffectType::DrumKit);
    if (!fx) return false;
    getManager()->getEffects().push_back(std::move(fx));

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
void DrumKitLooperModule::toogleDrumKit(){
    if (!mInitialized) return ;
    bool isEnabled = getDrumKit()->isEnabled();
    dLog("[info] toogle DrumKit to %d", !isEnabled);
    getDrumKit()->setEnabled(!isEnabled);
}
//------------------------------------------------------------------------------
void DrumKitLooperModule::DrawUI(bool* p_open){
    if (!mInitialized) return ;
    getDrumKit()->renderSequencerWindow(p_open);

    //FIXME ... extra window ...
    if (*p_open) {
        ImGui::SetNextWindowSizeConstraints(ImVec2(200.0f, 400.f), ImVec2(FLT_MAX, FLT_MAX));
        ImGui::Begin("DrumKit Presets"/*, p_enabled*/);
        getManager()->DrawPresetList(0.1f);
        ImGui::End();
    }

}
//------------------------------------------------------------------------------


