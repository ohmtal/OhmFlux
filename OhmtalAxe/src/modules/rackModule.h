#pragma once

#include <SDL3/SDL.h>
#include <string>
#include <imgui.h>

#include <core/fluxBaseObject.h>
#include <DSP.h>
#include <DSP_EffectsManager.h>
#include <DSP_EffectFactory.h>


class RackModule : public FluxBaseObject {
private:

    std::unique_ptr<DSP::EffectsManager> mEffectsManager = nullptr;

    bool mInitialized = false;
    std::string mPresetsFile = "";
    std::string mFactoryPresetFile = "";

public:
    RackModule() = default;

    ~RackModule();


    bool Initialize() override;
    DSP::EffectsManager* getManager() const;

    void populateRack(DSP::EffectsRack* lRack);


    int mRackTabCurId = -1;
    int mRackTabNewId = -1;
    void DrawRack(bool* p_enabled);

    void DrawPresetList(DSP::EffectsManager* lManager) {
        lManager->DrawPresetList();
    }
    void DrawEffectManagerPresetListWindow(bool* p_enabled);


    void setSampleRate(float sampleRate);
    void process(float* buffer, int numSamples, int numChannels);

    void callSavePresets();
    void callLoadPresets();

};
