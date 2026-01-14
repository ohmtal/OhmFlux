//-----------------------------------------------------------------------------
// Copyright (c) 2026 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------

#pragma once

#include <audio/fluxAudio.h>
#include <core/fluxBaseObject.h>
#include <core/fluxRenderObject.h>
#include <gui/fluxGuiGlue.h>

#include "sequencerGlobals.h"

// #include "fluxSfxEditor.h"
// #include "fluxFMEditor.h"
// #include "fluxComposer.h"



class SequencerGui: public FluxBaseObject
{
public:
    // dont forget to add a parameter 
    // a.) mDefaultEditorSettings
    // b.) on the bottom to the json macro!!! 
    struct GuiSettings {
        // bool mShowDemo;
        // bool mShowSFXEditor;
        // bool mShowFMInstrumentEditor;
        // bool mShowFMComposer;
        // bool mShowCompleteScale;
        bool mEditorGuiInitialized;
    };


private:
    FluxRenderObject* mBackground = nullptr;
    FluxGuiGlue* mGuiGlue = nullptr;
    // FluxSfxEditor* mSfxEditor = nullptr;
    // FluxFMEditor* mFMEditor = nullptr;
    // FluxComposer* mFMComposer = nullptr;


    GuiSettings mGuiSettings;
    GuiSettings mDefaultGuiSettings = {
        // .mShowDemo = false,
        // .mShowSFXEditor = true,
        // .mShowFMInstrumentEditor = true,
        // .mShowFMComposer = true,
        // .mShowCompleteScale = false,
        .mEditorGuiInitialized = false
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
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(SequencerGui::GuiSettings,
    // mShowDemo,
    // mShowSFXEditor,
    // mShowFMInstrumentEditor,
    // mShowFMComposer,
    // mShowCompleteScale,
    mEditorGuiInitialized
)
