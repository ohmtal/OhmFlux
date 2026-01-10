//-----------------------------------------------------------------------------
// Copyright (c) 2026 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// TODO: to options:
//              FluxEditorOplController => mSyncInstrumentChannel
//              fileDialog => mDefaultPath
// 2026-01-10
// * save your own settings file !! and ImGui ==> SaveIniSettingsToMemory
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
    struct EditorSettings {
        bool mShowDemo;
        bool mShowSFXEditor;
        bool mShowFMInstrumentEditor;
        bool mShowFMComposer;
        bool mShowCompleteScale;
    };


private:
    FluxRenderObject* mBackground = nullptr;
    FluxGuiGlue* mGuiGlue = nullptr;
    FluxSfxEditor* mSfxEditor = nullptr;
    FluxFMEditor* mFMEditor = nullptr;
    FluxComposer* mFMComposer = nullptr;


    EditorSettings mEditorSettings;
    EditorSettings mDefaultEditorSettings = {
        .mShowDemo = false,
        .mShowSFXEditor = true,
        .mShowFMInstrumentEditor = true,
        .mShowFMComposer = true,
        .mShowCompleteScale = false
    };

public:

    bool Initialize() override;
    void Deinitialize() override;
    void onEvent(SDL_Event event);
    void DrawMsgBoxPopup();
    void ShowManuBar();
    void DrawGui( );
    void onKeyEvent(SDL_KeyboardEvent event);


}; //class

// macro for JSON support
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(EditorGui::EditorSettings,
                                                mShowDemo,
                                                mShowSFXEditor,
                                                mShowFMInstrumentEditor,
                                                mShowFMComposer,
                                                mShowCompleteScale
)
