//-----------------------------------------------------------------------------
// Copyright (c) 2026 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// TODO: to options:
//              FluxEditorOplController => mSyncInstrumentChannel
//              fileDialog => mDefaultPath
// TODO: save your own settings file !! and ImGui ==> SaveIniSettingsToMemory
//-----------------------------------------------------------------------------

#pragma once

#include <audio/fluxAudio.h>
#include <core/fluxBaseObject.h>
#include <core/fluxRenderObject.h>
#include <gui/fluxGuiGlue.h>

#include "fluxSfxEditor.h"
#include "fluxFMEditor.h"
#include "fluxEditorGlobals.h"
#include "fluxComposer.h"


class EditorGui: public FluxBaseObject
{
public:
    struct soundToolParameter {
        bool mShowDemo                = false;
        bool mShowSFXEditor           = false;
        bool mShowFMInstrumentEditor  = true;
        bool mShowFMComposer          = true;
        bool mShowCompleteScale       = false;
        // bool mShowPianoScale          = false;

    };

private:
    FluxRenderObject* mBackground = nullptr;
    FluxGuiGlue* mGuiGlue = nullptr;
    FluxSfxEditor* mSfxEditor = nullptr;
    FluxFMEditor* mFMEditor = nullptr;
    FluxComposer* mFMComposer = nullptr;

    soundToolParameter mParameter;
public:

    bool Initialize() override;
    void Deinitialize() override;
    void onEvent(SDL_Event event);
    void DrawMsgBoxPopup();
    void ShowManuBar();
    void DrawGui( );
    void onKeyEvent(SDL_KeyboardEvent event);


}; //class
