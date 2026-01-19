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
#include <DSP.h>
#include "sequencerGlobals.h"
#include <opl3.h>
#include <OPL3Tests.h>

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
        bool mEditorGuiInitialized;
        bool mShowFileManager;
        bool mShowConsole;
        bool mShowDSP;
        bool mShowSoundBankEditor;

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
        , .mShowFileManager = true
        , .mShowConsole     = true
        , .mShowDSP         = false
        , .mShowSoundBankEditor  = true
    };



    void InitDockSpace();
    void OnConsoleCommand(ImConsole* console, const char* cmdline);


    void ShowMenuBar();


    // ----- Tests ------
    std::unique_ptr<OPL3Tests> mOpl3Tests;
    opl3::SongData myTestSong;

    // ----- DSP ------
    void ShowDSPWindow();
    void RenderBitCrusherUI();
    void RenderChorusUI();
    void RenderReverbUI();
    void RenderWarmthUI();

    // ---- Bank / Instruments -----
    int mCurrentInstrumentId = 0;
    void ShowSoundBankWindow();
    void RenderInstrumentListUI(bool standAlone = false);
    void RenderInstrumentEditorUI(bool standAlone = false);
    void RenderScalePlayerUI(bool standAlone = false);


public:
    SequencerGui() {}
    ~SequencerGui() {}

    ImConsole mConsole;
    bool Initialize() override;
    void Deinitialize() override;
    void onEvent(SDL_Event event);
    void DrawMsgBoxPopup();

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
    ,mShowDSP
    ,mShowSoundBankEditor
)

namespace DSP {
    // Macros generate the to_json/from_json functions automatically
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(BitcrusherSettings, bits, sampleRate, wet)
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ChorusSettings, rate, depth, delayBase, wet, phaseOffset)
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ReverbSettings, decay, sizeL, sizeR, wet)
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(WarmthSettings, cutoff, drive, wet)
}
