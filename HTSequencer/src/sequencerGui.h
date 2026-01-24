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
        bool InsertMode;
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
    Pattern* getCurrentPattern()
    {
        PatternEditorState& state = mPatternEditorState;
        SongData& song = mCurrentSong;

        if (song.patterns.empty())
            return nullptr;

        // Safety clamping
        if (state.currentPatternIdx < 0)
            state.currentPatternIdx = 0;

        if (state.currentPatternIdx >= (int)song.patterns.size()) {
            state.currentPatternIdx = (int)song.patterns.size() - 1;
        }

        // Return the memory address of the pattern in the vector
        return &song.patterns[state.currentPatternIdx];
    }
    //------------------------------------------------------------------------------

    //------------------------------------------------------------------------------
    void InsertRow(Pattern& pat, int rowIndex) {
        auto& steps = pat.getStepsMutable();
        auto pos = steps.begin() + (rowIndex * SOFTWARE_CHANNEL_COUNT);
        // Insert 12 empty steps
        steps.insert(pos, SOFTWARE_CHANNEL_COUNT, SongStep{});
    }
    //------------------------------------------------------------------------------
    bool DrawNewPatternModal(SongData& song, NewPatternSettings& settings) {
        bool result = false;
        uint8_t patCount =  (uint8_t)song.patterns.size();


        if (ImGui::BeginPopupModal("New Pattern Configuration", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::InputText("Pattern Name", settings.name, 64);
            ImGui::ColorEdit4("Pattern Color", (float*)&settings.color);

            ImGui::SliderInt("Rows", &settings.rowCount, 1, 1024);
            ImGui::SameLine();
            if(ImGui::Button("256")) settings.rowCount = 256;

            // ImGui::Separator();
            // ImGui::Text("Software Channels: %d", opl3::SOFTWARE_CHANNEL_COUNT);
            // ImGui::BulletText("0-5: 4-Operator capable");
            // ImGui::BulletText("6-11: 2-Operator");

            if (ImGui::Button("OK", ImVec2(120, 0))) {
                Pattern p;
                p.mName = settings.name;
                p.mColor = ImGui::ColorConvertFloat4ToU32(settings.color);

                // Allocation: Rows * 12 channels
                p.getStepsMutable().resize(settings.rowCount * opl3::SOFTWARE_CHANNEL_COUNT);

                // Initialize with "None" notes
                for(auto& s : p.getStepsMutable()) {
                    s.note = 255;
                    s.volume = 63;
                    s.panning = 32;
                }

                song.patterns.push_back(std::move(p));
                // automatic add to orderlist ! you can edit it later
                uint8_t newPatternIdx = patCount - 1;
                song.orderList.push_back(newPatternIdx);
                ImGui::CloseCurrentPopup();
                result = true;
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(120, 0))) {
                ImGui::CloseCurrentPopup();
                result = true;
            }

            ImGui::EndPopup();
        }
        return result;
    }
    //------------------------------------------------------------------------------
    void DrawPatternSelector(SongData& song, PatternEditorState& state) {


        if (song.patterns.empty()) {
            ImGui::Text("No patterns created.");
            state.currentPatternIdx = -1;
            return;
        }


        if (state.currentPatternIdx < 0) state.currentPatternIdx = 0;
        else
        if (state.currentPatternIdx >= (int)song.patterns.size()) {
            state.currentPatternIdx = (int)song.patterns.size() - 1;
        }
        //-----------

        if (ImGui::BeginTabBar("PatternTabs", ImGuiTabBarFlags_AutoSelectNewTabs | ImGuiTabBarFlags_Reorderable)) {
            for (int i = 0; i < (int)song.patterns.size(); i++) {
                Pattern& p = song.patterns[i];

                // 1. Convert color and Push Styles BEFORE BeginTabItem
                ImVec4 tabColor = ImGui::ColorConvertU32ToFloat4(p.mColor);
                ImVec4 inactiveColor = tabColor;
                inactiveColor.w *= 0.6f;

                ImGui::PushStyleColor(ImGuiCol_Tab, inactiveColor);
                ImGui::PushStyleColor(ImGuiCol_TabActive, tabColor);
                ImGui::PushStyleColor(ImGuiCol_TabHovered, tabColor);

                char label[64];
                snprintf(label, sizeof(label), "%s###Tab%d",p.mName.c_str(), i);

                // 2. Begin the Tab
                bool tabOpen = ImGui::BeginTabItem(label);

                // Context menu logic (associated with the last item, the tab)
                char popupId[32];
                snprintf(popupId, sizeof(popupId), "TabCtx%d", i);
                if (ImGui::BeginPopupContextItem(popupId)) {
                    ImGui::TextColored(ImColor4F(cl_DarkGray), "Pattern settings");
                    char patName[64];
                    strncpy(patName, p.mName.c_str(), sizeof(patName));
                    ImGui::TextDisabled("Pattern Name");
                    if (ImGui::InputText("##Pattern Name", patName, 64))
                    {
                        p.mName = patName;
                    }
                    ImVec4 tempCol = ImGui::ColorConvertU32ToFloat4(p.mColor);
                    ImGui::TextDisabled("Pattern Color");
                    if (ImGui::ColorEdit4("##Pattern Color", (float*)&tempCol)) {
                        p.mColor = ImGui::ColorConvertFloat4ToU32(tempCol);
                    }
                    ImGui::EndPopup();
                }

                // 3. Handle Tab Content if Open
                if (tabOpen) {
                    state.currentPatternIdx = i; // Mark as selected
                    ImGui::EndTabItem();
                }

                // 4. IMPORTANT: Always Pop here, outside the 'if (tabOpen)' block
                // This ensures every push is matched by a pop every frame
                ImGui::PopStyleColor(3);
            }
            ImGui::EndTabBar();
        }

    }
    //------------------------------------------------------------------------------
    // void RenderStepCell(SongStep& step, bool selected) {
    //     // Formatting the strings using your existing opl3 namespace
    //     std::string noteStr = opl3::ValueToNote(step.note);
    //
    //     // Format: Note(3) Inst(2) Vol(2) FX(1) Param(2)
    //     // Example: "C-4 01 v63 A0F"
    //     char buf[32];
    //
    //     // Volume representation (v00-v63)
    //     char volBuf[4];
    //     if (step.note != 255) snprintf(volBuf, sizeof(volBuf), "v%02d", step.volume);
    //     else strcpy(volBuf, " ..");
    //
    //     // Effect representation (e.g., A0F)
    //     char fxBuf[5];
    //     if (step.effectType != 0) snprintf(fxBuf, sizeof(fxBuf), "%c%02X", step.effectType, step.effectVal);
    //     else strcpy(fxBuf, " ...");
    //
    //     snprintf(buf, sizeof(buf), "%s %02X %s %s",
    //              noteStr.c_str(), step.instrument, volBuf, fxBuf);
    //
    //     if (selected) {
    //         ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImGui::GetColorU32(ImGuiCol_HeaderActive));
    //     }
    //
    //     ImGui::Selectable(buf, selected);
    // }
    void RenderStepCell(SongStep& step, bool isSelected, int r, int c, PatternEditorState& state) {
        // 1. Construct the tracker-style string: "C-4 01 v63 A0F"
        std::string noteStr = opl3::ValueToNote(step.note);

        char buf[32];
        snprintf(buf, sizeof(buf), "%s %02X %02d %c%02X",
                 noteStr.c_str(),
                 step.instrument,
                 step.volume,
                 (step.effectType == 0 ? '.' : (char)step.effectType),
                 step.effectVal);

        // 2. Highlight if this is the active selection/cursor
        // FIXME  only is ! state.ScrollToSelected
        if (isSelected) {
            ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImGui::GetColorU32(ImGuiCol_HeaderActive));
        }

        // 3. Make the cell clickable to set the cursor
        // Use "##" to create a unique ID for the Selectable without showing the label twice
        // if you prefer to draw text separately, or just pass 'buf'.
        if (ImGui::Selectable(buf, isSelected, ImGuiSelectableFlags_AllowOverlap)) {
            state.cursorRow = r;
            state.cursorCol = c;
        }
    }


    //------------------------------------------------------------------------------
    void DrawPatternEditor(Pattern& pattern, PatternEditorState& state) {
        const int numRows = (int)pattern.getSteps().size() / opl3::SOFTWARE_CHANNEL_COUNT;

        if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && !ImGui::IsAnyItemActive()) {
            ImGui::SetKeyboardFocusHere();
        }

        // Style the table for a tight tracker look
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1));

        static ImGuiTableFlags flags = ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersOuterH |
        ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable;

        if (ImGui::BeginTable("PatternGrid", opl3::SOFTWARE_CHANNEL_COUNT + 1, flags, ImVec2(0, 600))) {

            // Setup Row Index Column
            ImGui::TableSetupScrollFreeze(0, 1); // Freeze header row
            ImGui::TableSetupColumn("##Row", ImGuiTableColumnFlags_WidthFixed, 35.0f);

            // Setup Channel Columns
            for (int i = 0; i < opl3::SOFTWARE_CHANNEL_COUNT; i++) {
                char name[16];
                if (i < 6) {
                    snprintf(name, sizeof(name), "CH %02d (4OP)", i + 1);
                } else {
                    snprintf(name, sizeof(name), "CH %02d (2OP)", i + 1);
                }

                ImGui::TableSetupColumn(name, ImGuiTableColumnFlags_WidthFixed, 120.0f);
            }
            ImGui::TableHeadersRow();

            // High-performance clipping
            ImGuiListClipper clipper;
            clipper.Begin(numRows);

            //FIXME  only if request scroll  add something like state.ScrollToSelected
            clipper.IncludeItemByIndex(state.cursorRow);

            while (clipper.Step()) {

                for (int r = clipper.DisplayStart; r < clipper.DisplayEnd; r++) {
                    ImGui::TableNextRow();

                    if (r == state.cursorRow /*&& state.cursorRow > clipper.DisplayEnd - 3*/) //
                        ImGui::SetScrollHereY(0.5f);

                    // Column 0: Row Number
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextDisabled("%03d", r);

                    // Columns 1-12: Channel Steps
                    for (int c = 0; c < opl3::SOFTWARE_CHANNEL_COUNT; c++) {
                        ImGui::TableSetColumnIndex(c + 1);

                        // Visual distinction for 4-Operator channels
                        if (c < 6) {
                            ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImGui::GetColorU32(ImVec4(0.2f, 0.2f, 0.4f, 0.3f)));
                        }

                        SongStep& step = pattern.getStep(r, c);

                        ImGui::PushID(r * opl3::SOFTWARE_CHANNEL_COUNT + c);

                        // Pass current row/column and state to the cell renderer
                        bool isCursorPos = (state.cursorRow == r && state.cursorCol == c);
                        RenderStepCell(step, isCursorPos, r, c, state);

                        ImGui::PopID();
                    } //columns


                }

            } //while
            ImGui::EndTable();

            if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) {
                if (ImGui::IsKeyPressed(ImGuiKey_UpArrow))    state.cursorRow = std::max(0, state.cursorRow - 1);
                if (ImGui::IsKeyPressed(ImGuiKey_DownArrow))  state.cursorRow = std::min((int)pattern.getSteps().size()/12 - 1, state.cursorRow + 1);
                if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow))  state.cursorCol = std::max(0, state.cursorCol - 1);
                if (ImGui::IsKeyPressed(ImGuiKey_RightArrow)) state.cursorCol = std::min(opl3::SOFTWARE_CHANNEL_COUNT - 1, state.cursorCol + 1);

                if ( ImGui::IsKeyPressed(ImGuiKey_Space)) pattern.getStep(state.cursorRow, state.cursorCol).note = opl3::STOP_NOTE;

                // //FIXME delete selected func
                if ( ImGui::IsKeyPressed(ImGuiKey_Delete)) pattern.getStep(state.cursorRow, state.cursorCol).init();

                if (ImGui::IsKeyPressed(ImGuiKey_PageUp)) state.cursorRow =std::max(0, state.cursorRow - 16);
                if (ImGui::IsKeyPressed(ImGuiKey_PageDown)) state.cursorRow = std::min((int)pattern.getSteps().size()/12 - 1, state.cursorRow + 16);
                if (ImGui::IsKeyPressed(ImGuiKey_Home)) state.cursorRow = 0;
                if (ImGui::IsKeyPressed(ImGuiKey_End)) state.cursorRow = (int)pattern.getSteps().size()/12 - 1;

            } // is focused

        }
        ImGui::PopStyleVar();
    }
    //------------------------------------------------------------------------------
    SongData CreateTempSelection(const Pattern& activePattern, const PatternEditorState& state) {
        SongData temp;
        int minR = std::min(state.selectStartRow, state.selectEndRow);
        int maxR = std::max(state.selectStartRow, state.selectEndRow);

        Pattern subPattern;
        subPattern.mName = "SelectionClip";

        // Resize to accommodate the selected rows
        auto& steps = subPattern.getStepsMutable();
        int rowCount = (maxR - minR) + 1;
        steps.resize(rowCount * SOFTWARE_CHANNEL_COUNT);

        for (int r = minR; r <= maxR; r++) {
            for (int c = 0; c < SOFTWARE_CHANNEL_COUNT; c++) {
                if (state.isSelected(r, c)) {
                    steps[(r - minR) * SOFTWARE_CHANNEL_COUNT + c] = activePattern.getStep(r, c);
                } else {
                    // Fill unselected channels in the range with empty notes
                    steps[(r - minR) * SOFTWARE_CHANNEL_COUNT + c].note = 255;
                }
            }
        }

        temp.patterns.push_back(subPattern);
        temp.orderList.push_back(0);
        return temp;
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

)

namespace DSP {
    // Macros generate the to_json/from_json functions automatically
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(BitcrusherSettings, bits, sampleRate, wet)
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ChorusSettings, rate, depth, delayBase, wet, phaseOffset)
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ReverbSettings, decay, sizeL, sizeR, wet)
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(WarmthSettings, cutoff, drive, wet)
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Equalizer9BandSettings, gains)
}
