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

class DrumKitLooper : public FluxBaseObject {
private:
    std::unique_ptr<DSP::DrumKit> mDrumKit = nullptr;
public:
    DrumKitLooper() {
        dLog("DrumKitLooper construct ...");
        mDrumKit = cast_unique<DSP::DrumKit>(DSP::EffectFactory::Create(DSP::EffectType::DrumKit));
    }

    //----------------------------------------------------------------------
    void process(float* buffer, int numSamples, int numChannels) {
        mDrumKit->process(buffer, numSamples, numChannels);
    }

    void DrawUI(bool *p_open) {
            mDrumKit->renderSequencerWindow(p_open);
    }


    //FIXME
    // mDrumKit->loadFromFile("bla.drum");
    // mDrumKit->saveToFile("bla.drum");


}; //class
