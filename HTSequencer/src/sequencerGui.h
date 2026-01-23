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

// ------------- Wav export in a thread >>>>>>>>>>>>>>
struct ExportTask {
    OPL3Controller* controller;
    opl3::SongData song;
    std::string filename;
    float progress = 0.0f; // Track progress here
    bool applyEffects = false;
    bool isFinished = false;
};

// This is the function the thread actually runs
static int SDLCALL ExportThreadFunc(void* data) {
    auto* task = static_cast<ExportTask*>(data);

    task->controller->exportToWav(task->song, task->filename, &task->progress, task->applyEffects);

    task->isFinished = true;
    return 0;
}


class SequencerGui: public FluxBaseObject
{
public:
    // dont forget to add a parameter 
    // a.) mDefaultEditorSettings
    // b.) on the bottom to the json macro!!! 
    struct GuiSettings {
        bool mEditorGuiInitialized;
        bool mShowFileManager;
        bool mShowConsole;
        bool mShowDSP;
        bool mShowSoundBankList;
        bool mShowFMEditor;
        bool mShowScalePlayer;
        bool mShowSongGui;
        bool mShowPiano;
    };


private:

    FluxRenderObject* mBackground = nullptr;
    FluxGuiGlue* mGuiGlue = nullptr;


    GuiSettings mGuiSettings;
    GuiSettings mDefaultGuiSettings = {
          .mEditorGuiInitialized = false
        , .mShowFileManager = true
        , .mShowConsole     = true
        , .mShowDSP         = true
        , .mShowSoundBankList = true
        , .mShowFMEditor = true
        , .mShowScalePlayer = true
        , .mShowSongGui = true
        , .mShowPiano = false
    };


    // -------- song -------
    opl3::SongData mCurrentSong;
    opl3::SongData mTestNoteSong;
    bool mLoopSong = false;
    bool mExportWithEffects = false;
    bool mInsertMode = false;



    //-------

    void InitDockSpace();
    void OnConsoleCommand(ImConsole* console, const char* cmdline);


    void ShowMenuBar();

    void ShowFileManager();


    // ----- Tests ------
    std::unique_ptr<OPL3Tests> mOpl3Tests;

    // ----- DSP ------
    void ShowDSPWindow();
    void RenderBitCrusherUI();
    void RenderChorusUI();
    void RenderReverbUI();
    void RenderWarmthUI();
    void RenderLimiterUI();
    void RenderEquilizer9BandUI();

    // ---- Bank / Instruments -----
    uint16_t mCurrentInstrumentId = 0;
    void ShowSoundBankWindow();
    void RenderInstrumentListUI(bool standAlone = false);

    void RenderInstrumentEditorUI(bool standAlone = false);
    void RenderOpParam(const ParamMeta& meta, OplInstrument::OpPair::OpParams& op, int metaIdx);



    // ------- songGui / Sequencer ---------
    void callSaveSong();
    void playSong(U8 playMode = 0);
    void stopSong();
    void newSong();
    void callExportSong();
    ExportTask* mCurrentExport = nullptr; //<<< for export to wav
    bool exportSongToWav(std::string filename);

    uint8_t getCurrentChannel();
    void insertTone( uint8_t midiNote);


    void RenderSequencerUI(bool standAlone = true);
    void DrawExportStatus();

    // ----- keyboards / scale player  -----
    void RenderScalePlayerUI(bool standAlone = false);
    void RenderPianoUI(bool standAlone = true);




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

    //------------------------------------------------------------------------------

}; //class

// macro for JSON support
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(SequencerGui::GuiSettings,
    mEditorGuiInitialized
    ,mShowFileManager
    ,mShowConsole
    ,mShowDSP
    ,mShowSoundBankList
    ,mShowFMEditor
    ,mShowScalePlayer
    ,mShowSongGui
    ,mShowPiano

)

namespace DSP {
    // Macros generate the to_json/from_json functions automatically
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(BitcrusherSettings, bits, sampleRate, wet)
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ChorusSettings, rate, depth, delayBase, wet, phaseOffset)
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ReverbSettings, decay, sizeL, sizeR, wet)
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(WarmthSettings, cutoff, drive, wet)
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Equalizer9BandSettings, gains)
}
