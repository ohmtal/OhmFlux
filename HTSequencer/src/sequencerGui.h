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
#include <gui/ImFlux.h>
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
        bool ShowPlayList;

    };


private:

    FluxRenderObject* mBackground = nullptr;
    FluxGuiGlue* mGuiGlue = nullptr;

    ImFont* mDefaultFont = nullptr;
    ImFont* mIconFont = nullptr; //<< font
    ImFont* mTinyFont = nullptr;

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
        , .ShowPlayList = false
    };


    // -------- song -------
    opl3::SongData mCurrentSong;
    opl3::SongData mTempSong;
    bool mLoopSong = false;
    bool mExportWithEffects = false;
    // bool mInsertMode = false;


    // -------- PatternSelection -------
    struct PatternSelection {
        bool active = false;
        std::array<int32_t, 2>  startPoint = {0, 0}; // [0]=row, [1]=col
        std::array<int32_t, 2>  endPoint = {0, 0};


        // -------------- sort
        void sort() {
            if (!active) return;
            Log("1: start: %d, %d end: %d, %d", startPoint[0], startPoint[1], endPoint[0], endPoint[1]);

            // Explicitly find the values first
            int32_t r0 = startPoint[0];
            int32_t r1 = endPoint[0];
            int32_t c0 = startPoint[1];
            int32_t c1 = endPoint[1];

            startPoint[0] = std::min(r0, r1);
            endPoint[0]   = std::max(r0, r1);

            startPoint[1] = std::min(c0, c1);
            endPoint[1]   = std::max(c0, c1);
            Log("2: start: %d, %d end: %d, %d", startPoint[0], startPoint[1], endPoint[0], endPoint[1]);

        }

        // -------------- init
        void init() {
            active = false;
            startPoint = {0, 0};
            endPoint = {0, 0};
        }
        // ----------- isSelected

        int minRow() const { return std::min((int)startPoint[0], (int)endPoint[0]); }
        int maxRow() const { return std::max((int)startPoint[0], (int)endPoint[0]); }
        int minCol() const { return std::min((int)startPoint[1], (int)endPoint[1]); }
        int maxCol() const { return std::max((int)startPoint[1], (int)endPoint[1]); }

        // Your requested action getters
        int getRowCount() const { return active ? (maxRow() - minRow() + 1) : 0; }
        int getColCount() const { return active ? (maxCol() - minCol() + 1) : 0; }
        int getCount() const { return getRowCount() * getColCount(); }

        bool isSelected(int r, int c) const {
            if (!active) return false;
            return (r >= minRow() && r <= maxRow() && c >= minCol() && c <= maxCol());
        }

    }; //PatternSelection
    //--------------------------------------------------------------------------
    struct PatternClipboard {
        std::vector<SongStep> data;
        int rows = 0;
        int cols = 0;
        bool active = false;

        void clear() {
            data.clear();
            rows = 0;
            cols = 0;
            active = false;
        }
    };
    //--------------------------------------------------------------------------
    // Pattern Editor:
    struct PatternEditorState {
        int currentPatternIdx = 0;
        int cursorRow = 0;
        int cursorCol = 0;
        bool scrollToSelected = false;
        bool following = false;

        PatternSelection selection;

        // for popup:
        int contextRow  = -1;
        int contextCol = -1;
        bool showContextRequest = false;

        // for shift + click
        int selectionAnchorRow = -1;
        int selectionAnchorCol = -1;

        bool visible = false;


        opl3::Pattern* pattern = nullptr; //mhhh we also have the index here ...

        // ---------- isSelected
        bool isSelected(int row, int col) const {
            return selection.isSelected(row,col);
        }
        // -------------- moveCursorPosition
        void moveCursorPosition(int rowAdd, int colAdd) {
            setCursorPosition(cursorRow + rowAdd, cursorCol+colAdd);
        }
        // -------------- setCursorPosition
        void setCursorPosition(int row, int col) {
            if (!pattern) return;
            row = std::clamp(row, 0, pattern->getRowCount() -1 );
            col = std::clamp(col, 0, pattern->getColCount() -1 );
            if ( row == cursorRow && col == cursorCol )
                return ;
            cursorRow = row;
            cursorCol = col;
            scrollToSelected = true;
        }
    }; //PatternEditorState

    struct NewPatternSettings {
        char name[64] = "New Pattern";
        ImVec4 color = ImVec4(0.2f, 0.6f, 0.2f, 1.f);
        int rowCount = 64; // Default tracker length
        bool isOpen = false;
    };


    NewPatternSettings mNewPatternSettings;
    PatternEditorState mPatternEditorState;
    PatternClipboard   mPatternClipBoard;

    bool playNote(uint8_t softwareChannel, SongStep step ); //play or insert a note
    bool stopNote(uint8_t softwareChannel );
    bool stopPlayedNotes( );
    uint8_t getCurrentChannel();   // get the current channel (mPatternEditorState.cursorCol)
    bool setCurrentChannel(uint8_t channel);


    int mDeletePatternScheduleId = -1;
    bool deletePattern (  opl3::SongData& song, int patternIndex);
    bool deletePatternIsPending();


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
    void RenderSoundCardEmuUI();



    // ---- Bank / Instruments -----
    uint16_t mCurrentInstrumentId = 0;
    void ShowSoundBankWindow();
    void RenderInstrumentListUI(bool standAlone = false);
    void RenderInstrumentListUI_OLD(bool standAlone = false);

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
    bool playSong();
    void stopSong();
    void newSong();

    bool playSelected(PatternEditorState& state, bool forcePatternPlay = false);
    bool clearSelectedSteps(PatternEditorState& state);
    void copyStepsToClipboard(PatternEditorState& state, PatternClipboard& cb);
    void pasteStepsFromClipboard(PatternEditorState& state, const PatternClipboard& cb, bool useContextPoint=false);
    void selectPatternAll(PatternEditorState& state);
    void selectPatternRow(PatternEditorState& state);
    void selectPatternCol(PatternEditorState& state);

    void setInstrumentSelection(PatternEditorState& state, uint16_t instrumentIndex);
    void transposeSelection(PatternEditorState& state, int semitones);
    void insertBlanksAndshiftDataDown(PatternEditorState& state);
    void deleteAndShiftDataUp(PatternEditorState& state);


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
    void DrawStepCell(SongStep& step, bool isSelected, int row, int col, PatternEditorState& state);
    void DrawStepCellPopup(PatternEditorState& state);
    void DrawPatternEditor(PatternEditorState& state);
    void ActionPatternEditor(PatternEditorState& state); //FIXME TO PRIVATE

    bool isPlaying();
    uint16_t getPlayingRow();
    uint16_t getPlayingSequenceIndex();

    bool Widget_InstrumentCombo(uint16_t& currentIdx, const std::vector<opl3::Instrument>& bank);

    ImU32 getInstrumentColor(uint16_t instrumentIndex) {
        // Use the Golden Ratio conjugate to distribute hues evenly
        // This prevents colors from repeating too early
        const float golden_ratio_conjugate = 0.618033988749895f;
        float hue = fmodf((float)instrumentIndex * golden_ratio_conjugate, 1.0f);

        ImVec4 colRGB;
        // Lower saturation or value slightly for better readability against white/dark backgrounds
        ImGui::ColorConvertHSVtoRGB(hue, 0.7f, 0.9f, colRGB.x, colRGB.y, colRGB.z);
        colRGB.w = 1.0f;

        return ImGui::GetColorU32(colRGB);
    }



    // -----
    void DrawMiniOrderList(SongData& song, int currentIndex = -1,  float buttonSize = 24.f , ImVec2 controlSize = {0,0});

    void DrawMiniOrderList1(SongData& song, bool standAlone = false, ImVec2 controlSize = {0,0});
    void DrawFancyOrderList(SongData& song, bool standAlone = true, ImVec2 controlSize = {0,0});
    void DrawOrderListEditor(SongData& song);

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
    ,EnhancedStepView
    ,ShowPlayList

)

namespace DSP {
    // Macros generate the to_json/from_json functions automatically
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(BitcrusherSettings, bits, sampleRate, wet)
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ChorusSettings, rate, depth, delayBase, wet, phaseOffset)
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ReverbSettings, decay, sizeL, sizeR, wet)
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(WarmthSettings, cutoff, drive, wet)
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Equalizer9BandSettings, gains)
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SoundCardEmulationSettings, renderMode)
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(LimiterSettings, Threshold, Attack, Release);


}
