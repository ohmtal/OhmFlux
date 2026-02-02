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



class EditorGui: public FluxBaseObject
{
public:
    // dont forget to add a parameter 
    // a.) mDefaultEditorSettings
    // b.) on the bottom to the json macro!!! 
    struct EditorSettings {
        bool mShowDemo;
        bool mShowSFXEditor;
        bool mEditorGuiInitialized;
        bool mShowImFluxWidgets;
    };


private:
    FluxRenderObject* mBackground = nullptr;
    FluxGuiGlue* mGuiGlue = nullptr;
    FluxSfxEditor* mSfxEditor = nullptr;


    EditorSettings mEditorSettings;
    EditorSettings mDefaultEditorSettings = {
        .mShowDemo = false,
        .mShowSFXEditor = true,
        .mEditorGuiInitialized = false,
        .mShowImFluxWidgets = false
    };

public:

    bool Initialize() override;
    void Deinitialize() override;
    void onEvent(SDL_Event event);
    void DrawMsgBoxPopup();
    void ShowMenuBar();
    void DrawGui( );
    void onKeyEvent(SDL_KeyboardEvent event);
    void InitDockSpace(); 


}; //class

// macro for JSON support
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(EditorGui::EditorSettings,
    mShowDemo,
    mShowSFXEditor,
    mEditorGuiInitialized,
    mShowImFluxWidgets
)
