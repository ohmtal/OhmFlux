#pragma once

#include <SDL3/SDL.h>
#include <string>
#include <atomic>
#include <imgui.h>

#include <DSP.h>
#include <DSP_EffectsManager.h>
#include <DSP_EffectFactory.h>

#include <core/fluxBaseObject.h>
#include <gui/ImFlux.h>


class KeyBoardModule : public FluxBaseObject {

public:
    void onKeyEvent(SDL_KeyboardEvent event);



};
