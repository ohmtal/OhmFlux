//-----------------------------------------------------------------------------
// Copyright (c) 2026 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#pragma once

#include <audio/fluxAudio.h>
#include <core/fluxBaseObject.h>
#include <core/fluxRenderObject.h>
#include <gui/fluxGuiGlue.h>

#include "fluxSfxEditor.h"
#include "fluxFMEditor.h"
#include "fluxEditorGlobals.h"



class EditorGui: public FluxBaseObject
{
public:
    struct soundToolParameter {
        bool mShowDemo                = false;
        bool mShowSFXEditor           = true;
        bool mShowFMInstrumentEditor  = true;
        bool mShowFMComposer          = false;
        bool mShowCompleteScale       = false;
        bool mShowPianoScale          = true;

    };

private:
    FluxRenderObject* mBackground = nullptr;
    FluxGuiGlue* mGuiGlue = nullptr;
    FluxSfxEditor* mSfxEditor = nullptr;
    FluxFMEditor* mFMEditor = nullptr;

    soundToolParameter mParameter;
public:

    bool Initialize() override;
    void Deinitialize() override;
    void onEvent(SDL_Event event);
    void DrawMsgBoxPopup();
    void ShowManuBar();
    void DrawGui( );

}; //class
