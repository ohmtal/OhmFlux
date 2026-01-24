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
#include <opl3_base.h>
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

    if (task->isFinished)
        return 0;

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
        bool mShowFileBrowser;
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

    DSP::SpectrumAnalyzer* mSpectrumAnalyzer;

    GuiSettings mGuiSettings;
    GuiSettings mDefaultGuiSettings = {
          .mEditorGuiInitialized = false
        , .mShowFileBrowser = true
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
    void RenderSpectrumAnalyzer();

    // ---- Bank / Instruments -----
    uint16_t mCurrentInstrumentId = 0;
    void ShowSoundBankWindow();
    void RenderInstrumentListUI(bool standAlone = false);

    void RenderInstrumentEditorUI(bool standAlone = false);
    void RenderOpParam(const ParamMeta& meta, OplInstrument::OpPair::OpParams& op, int metaIdx);
    void DrawOperatorGrid(OplInstrument::OpPair::OpParams& op);
    void DrawWaveformIcon(ImDrawList* drawList, ImVec2 pos, int waveIdx, float size, ImU32 color);
    void RenderWaveformSelector(uint8_t& waveVal);
    void DrawADSRGraphBezier(ImVec2 size, const opl3::OplInstrument::OpPair::OpParams& op, int virtualNote = 60, float pulseVol = 0.f);

    // fancy 4OP overlays
    void DrawAlgorithmHoverFunc(const OplInstrument inst);
    void Draw4OP_Algorithm0Overlay(float nodeSize = 35.0f, float spacing = 20.0f);
    void Draw4OP_Algorithm1Overlay(float nodeSize = 35.0f, float spacing = 20.0f);
    void Draw4OP_Algorithm3Overlay(float nodeSize = 35.0f, float spacing = 20.0f);
    void Draw4OP_Algorithm2Overlay(float nodeSize = 35.0f, float spacing = 20.0f);
    void Draw2OP_FM_Overlay(float nodeSize = 35.0f, float spacing = 20.0f);
    void Draw2OP_Additive_Overlay(float nodeSize = 35.0f, float spacing = 20.0f);



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
    void onKeyEventKeyBoard(SDL_KeyboardEvent event);
    void Update(const double& dt) override;

    //------------------------------------------------------------------------------

}; //class

// macro for JSON support
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(SequencerGui::GuiSettings,
    mEditorGuiInitialized
    ,mShowFileBrowser
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
