#pragma once
#pragma once

#include <string>

#include <SDL3/SDL.h>
#include <DSP.h>
#include <DSP_EffectsManager.h>
#include <DSP_EffectFactory.h>

#include <imgui.h>
#include <gui/ImFlux.h>

#include <core/fluxBaseObject.h>

class DrumKitLooperModule : public FluxBaseObject {
private:
    std::unique_ptr<DSP::EffectsManager> mDrumKitManager = nullptr;

    std::string mDrumKitFile = "";

    bool mInitialized = false;

public:
    DrumKitLooperModule() = default;
    ~DrumKitLooperModule();

    bool Initialize() override;

    DSP::DrumKit* getDrumKit();
    void process(float* buffer, int numSamples, int numChannels);

    void start();
    void stop();
    void toogleDrumKit();
    void DrawUI(bool *p_open);

    DSP::EffectsManager* getManager() const;



    bool addDrumKit();;
    void callSavePresets();
    void callLoadPresets();



}; //class
