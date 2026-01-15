//-----------------------------------------------------------------------------
// Copyright (c) 2026 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------

#pragma once

#include <audio/fluxAudio.h>
#include <core/fluxBaseObject.h>
#include <core/fluxRenderObject.h>
#include <gui/fluxGuiGlue.h>
#include <gui/ImConsole.h>

#include "sequencerGlobals.h"
#include <opl3.h>

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
        bool mShowFileManager;
        bool mShowConsole;

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

        .mEditorGuiInitialized = false,
        .mShowFileManager = true,
        .mShowConsole     = true
    };


    void InitDockSpace();
    void OnConsoleCommand(ImConsole* console, const char* cmdline);

    opl3::SongData myTestSong;

public:
    ImConsole mConsole;

    bool Initialize() override;
    void Deinitialize() override;
    void onEvent(SDL_Event event);
    void DrawMsgBoxPopup();
    void ShowMenuBar();
    void DrawGui( );
    void onKeyEvent(SDL_KeyboardEvent event);
    void Update(const double& dt) override;




}; //class

// macro for JSON support
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(SequencerGui::GuiSettings,
    // mShowDemo,
    // mShowSFXEditor,
    // mShowFMInstrumentEditor,
    // mShowFMComposer,
    // mShowCompleteScale,
    mEditorGuiInitialized
    ,mShowFileManager
    ,mShowConsole
)
