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
    struct SeqSettings {
        bool EditorGuiInitialized;
        bool ShowFileBrowser;
        bool ShowConsole;
        bool ShowDSP;
        bool ShowSoundBankList;
        bool ShowFMEditor;
        bool ShowScalePlayer;
        bool ShowSongGui;
        bool ShowPiano;
        bool InsertMode;  // when playing a note is insert
        bool EnhancedStepView; // if false only the note is displayed
    };

private:

    FluxRenderObject* mBackground = nullptr;
    FluxGuiGlue* mGuiGlue = nullptr;

    DSP::SpectrumAnalyzer* mSpectrumAnalyzer;

    SeqSettings mSettings;
    SeqSettings mDefaultSettings = {
          .EditorGuiInitialized = false
        , .ShowFileBrowser = true
        , .ShowConsole     = true
        , .ShowDSP         = true
        , .ShowSoundBankList = true
        , .ShowFMEditor = true
        , .ShowScalePlayer = true
        , .ShowSongGui = true
        , .ShowPiano = false
        , .InsertMode = false
        , .EnhancedStepView = false
    };


    // -------- song -------
    opl3::SongData mCurrentSong;
    opl3::SongData mTestNoteSong;
    bool mLoopSong = false;
    bool mExportWithEffects = false;
    // bool mInsertMode = false;


    // Pattern Editor:
    struct PatternEditorState {
        int currentPatternIdx = 0;
        int cursorRow = 0;
        int cursorCol = 0;
        bool scrollToSelected = false;
        bool following = false;

        // Selection (start and end points for range-based operations)
        int selectStartRow = -1, selectStartCol = -1;
        int selectEndRow = -1, selectEndCol = -1;



        bool isSelected(int r, int c) const {
            if (selectStartRow == -1) return false;
            int minR = std::min(selectStartRow, selectEndRow);
            int maxR = std::max(selectStartRow, selectEndRow);
            int minC = std::min(selectStartCol, selectEndCol);
            int maxC = std::max(selectStartCol, selectEndCol);
            return (r >= minR && r <= maxR && c >= minC && c <= maxC);
        }
    };

    struct NewPatternSettings {
        char name[64] = "New Pattern";
        ImVec4 color = ImVec4(0.2f, 0.2f, 0.2f, 1.f);
        int rowCount = 64; // Default tracker length
        bool isOpen = false;
    };


    NewPatternSettings mNewPatternSettings;
    PatternEditorState mPatternEditorState;

    bool playNote(uint8_t softwareChannel, SongStep step ); //play or insert a note
    uint8_t getCurrentChannel();   // get the current channel (mPatternEditorState.cursorCol)



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
    void RenderOpParam(const ParamMeta& meta, Instrument::OpPair::OpParams& op, int metaIdx);
    void DrawOperatorGrid(Instrument::OpPair::OpParams& op);
    void DrawWaveformIcon(ImDrawList* drawList, ImVec2 pos, int waveIdx, float size, ImU32 color);
    void RenderWaveformSelector(uint8_t& waveVal);
    void DrawADSRGraphBezier(ImVec2 size, const opl3::Instrument::OpPair::OpParams& op, int virtualNote = 60, float pulseVol = 0.f);

    // fancy 4OP overlays
    void DrawAlgorithmHoverFunc(const Instrument inst);
    void Draw4OP_Algorithm0Overlay(float nodeSize = 35.0f, float spacing = 20.0f);
    void Draw4OP_Algorithm1Overlay(float nodeSize = 35.0f, float spacing = 20.0f);
    void Draw4OP_Algorithm3Overlay(float nodeSize = 35.0f, float spacing = 20.0f);
    void Draw4OP_Algorithm2Overlay(float nodeSize = 35.0f, float spacing = 20.0f);
    void Draw2OP_FM_Overlay(float nodeSize = 35.0f, float spacing = 20.0f);
    void Draw2OP_Additive_Overlay(float nodeSize = 35.0f, float spacing = 20.0f);



    // ------- songGui / Sequencer ---------
    bool mLiveInsert = false; // TODO IMPORTANT! (see also songGui todo's )

    void callSaveSong();
    void playSong(U8 playMode = 0);
    void stopSong();
    void newSong();


    void RenderSequencerUI(bool standAlone = true);

    // ----- keyboards / scale player  -----
    void RenderScalePlayerUI(bool standAlone = false);
    void RenderPianoUI(bool standAlone = true);


    // --------- export -----------------
    void callExportSong();
    ExportTask* mCurrentExport = nullptr; //<<< for export to wav
    bool exportSongToWav(std::string filename);
    void DrawExportStatus();



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
    // ---- songGui / pattern ----
    Pattern* getCurrentPattern();
    void InsertRow(Pattern& pat, int rowIndex);

    bool DrawNewPatternModal(SongData& song, NewPatternSettings& settings);
    void DrawPatternSelector(SongData& song, PatternEditorState& state);
    void RenderStepCell(SongStep& step, bool isSelected, int r, int c, PatternEditorState& state);
    void setCursorPosition(int row, int col);
    void moveCursorPosition(int rowAdd, int colAdd);
    void DrawPatternEditor(Pattern& pattern, PatternEditorState& state);
    SongData CreateTempSelection(const Pattern& activePattern, const PatternEditorState& state);

    bool isPlaying();
    uint16_t getPlayingRow();
    uint16_t getPlayingSequenceIndex();


    /**
     * Widget_InstrumentCombo
     * return -1 if no instrument is selected
     */
    int Widget_InstrumentCombo(int currentIdx, const std::vector<opl3::Instrument>& bank) {
        int result = -1; // Default: no change

        // 1. Determine the text to show in the collapsed box
        const char* previewText = (currentIdx >= 0 && currentIdx < (int)bank.size())
        ? bank[currentIdx].name.c_str()
        : "Select Instrument...";

        // 2. Start the Combo
        if (ImGui::BeginCombo("##instrunmentCombo", previewText)) {
            for (int i = 0; i < (int)bank.size(); i++) {
                const bool isSelected = (currentIdx == i);

                // Use ## suffix to ensure unique IDs even if names are identical
                std::string itemLabel = bank[i].name + "##" + std::to_string(i);

                if (ImGui::Selectable(itemLabel.c_str(), isSelected)) {
                    result = i; // Selection occurred!
                }

                // Set initial focus to the current selection when opening
                if (isSelected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }

        return result;
    }


    //--------------------------------------------------------------------------

}; //class

// macro for JSON support
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(SequencerGui::SeqSettings,
    EditorGuiInitialized
    ,ShowFileBrowser
    ,ShowConsole
    ,ShowDSP
    ,ShowSoundBankList
    ,ShowFMEditor
    ,ShowScalePlayer
    ,ShowSongGui
    ,ShowPiano
    ,InsertMode
    , EnhancedStepView

)

namespace DSP {
    // Macros generate the to_json/from_json functions automatically
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(BitcrusherSettings, bits, sampleRate, wet)
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ChorusSettings, rate, depth, delayBase, wet, phaseOffset)
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ReverbSettings, decay, sizeL, sizeR, wet)
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(WarmthSettings, cutoff, drive, wet)
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Equalizer9BandSettings, gains)
}
