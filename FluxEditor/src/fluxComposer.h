//-----------------------------------------------------------------------------
// Copyright (c) 2026 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#pragma once

#include <core/fluxBaseObject.h>
#include <imgui.h>
#include "fluxEditorOplController.h"
#include "fluxEditorGlobals.h"




class FluxComposer : public FluxBaseObject
{
private:
    FluxEditorOplController* mController = nullptr;

    static const int MAX_PATTERNS = 16;
    static const int NOTES_PER_PATTERN = 64;

    // hackfest
    int mStartAt = 0;
    int mEndAt = -1;
    bool mLoop = false;


    int mSelectedRow = 0;
    int mSelectionPivot = -1;
    int selected_col = 1;
    bool is_editing = false;
    bool just_opened_this_frame = false;
    char edit_buf[32] = "";

    OplController::SongData mSongData;

    OplController::SongData mBufferSongData;

public:

    FluxComposer(FluxEditorOplController* lController)
    {
        mController = lController;
    }
    ~FluxComposer() { Deinitialize(); }
    //-----------------------------------------------------------------------------------------------------
    bool Initialize() override
    {
        // Assuming OplController is already initialized elsewhere or can be initialized here.
        // For now, let's just create it if it's not passed in.
        // In a real application, the OplController might be shared.
        if (!mController)
        {
            mController = new FluxEditorOplController();
            if (!mController || !mController->initController())
                return false;
        }

        // Initialize SongData
        mSongData.song_delay = 15; // Default from Pascal code
        mSongData.song_length = 32;
        for (int i = 0; i < 1001; ++i) {
            for (int j = 0; j < 10; ++j) {
                mSongData.song[i][j] = 0;
            }
        }

        return true;
    }
    //-----------------------------------------------------------------------------------------------------
    void Deinitialize() override
    {
        if (mController)
            SAFE_DELETE(mController);
    }
    //-----------------------------------------------------------------------------------------------------
    bool loadSong(const std::string& filename)
    {
        if (!mController)
            return false;

        if (mController->loadSongFMS(filename, mSongData))
        {
            mStartAt = 0;
            mEndAt = mSongData.song_length;
            return true;
        }
        return false;

    }

    //-----------------------------------------------------------------------------------------------------
    void setController(FluxEditorOplController* controller)  { mController = controller;  }
    FluxEditorOplController*  getController()   { return mController;  }
    //-----------------------------------------------------------------------------------------------------
    uint16_t getSelectionMin() { return (mSelectionPivot == -1) ? mSelectedRow : std::min(mSelectedRow, mSelectionPivot); }
    uint16_t getSelectionMax() { return (mSelectionPivot == -1) ? mSelectedRow : std::max(mSelectedRow, mSelectionPivot); }
    uint16_t getSelectionLen() { return (mSelectionPivot == -1) ? 1 : std::max(mSelectedRow, mSelectionPivot); }
    bool isRowSelected(int i) { return i >= getSelectionMin() && i <= getSelectionMax(); }

    //-----------------------------------------------------------------------------------------------------
    //-----------------------------------------------------------------------------------------------------
    void DrawNoteCell(int row, int col, int16_t& note_val, FluxEditorOplController* controller)
    {
        ImGui::TableSetColumnIndex(col);
        bool is_selected = (row == mSelectedRow && col == selected_col);

        // Use a simpler ID system to prevent ID collisions during clipping
        ImGui::PushID(col);

        if (is_selected && is_editing) {
            // --- EDIT MODE ---
            ImGui::SetKeyboardFocusHere();
            ImGui::SetNextItemWidth(-FLT_MIN);

            if (ImGui::InputText("##edit", edit_buf, sizeof(edit_buf),
                ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll)) {
                if (!just_opened_this_frame) {
                    note_val = (int16_t)controller->getIdFromNoteName(edit_buf);
                    is_editing = false;
                }
                }
                just_opened_this_frame = false;

            if (ImGui::IsItemDeactivated() && !ImGui::IsItemDeactivatedAfterEdit()) {
                is_editing = false;
            }
        } else {
            // --- VIEW MODE ---
            // 1. Get the text once to avoid multiple string allocations
            const char* display_text = "...";
            ImVec4 color = ImVec4(0.5f, 0.5f, 0.5f, 1); // Default Gray

            if (note_val == -1) {
                display_text = "===";
                color = ImVec4(1, 0, 1, 1); // Magenta
            } else if (note_val > 0) {
                // Note: Use a static or persistent string to avoid heap allocation every frame
                static std::string note_name;
                note_name = controller->getNoteNameFromId(note_val);
                display_text = note_name.c_str();
                color = ImVec4(1, 1, 1, 1); // White
            }

            // 2. Disable hover highlights that cause jitter
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0,0,0,0));
            ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0,0,0,0));

            // 3. The Selectable MUST be exactly the size of the cell to keep the clipper stable
            if (ImGui::Selectable("##select", is_selected, ImGuiSelectableFlags_AllowDoubleClick | ImGuiSelectableFlags_AllowOverlap, ImVec2(0, 0))) {
                mSelectedRow = row;
                selected_col = col;


                if (  mSelectionPivot >= 0)
                {
                    bool lShiftPressed = ImGui::IsKeyDown(ImGuiKey_LeftShift) || ImGui::IsKeyDown(ImGuiKey_RightShift);
                    if  (!lShiftPressed)
                        mSelectionPivot = -1;
                }



                // This resets focus to the parent window to keep keyboard nav clean
                ImGui::SetWindowFocus(nullptr);
                ImGui::SetWindowFocus("FM Song Composer");

                if (ImGui::IsMouseDoubleClicked(0)) {
                    is_editing = true;
                    just_opened_this_frame = true;
                    snprintf(edit_buf, sizeof(edit_buf), "%s", display_text);
                }
            }
            ImGui::PopStyleColor(2);

            // 4. Draw text directly on top of the selectable
            ImGui::SameLine(ImGui::GetStyle().ItemSpacing.x);
            ImGui::TextColored(color, "%s", display_text);
        }
        ImGui::PopID();
    }

    // void DrawNoteCell(int row, int col, int16_t& note_val, FluxEditorOplController* controller)
    // {
    //     ImGui::TableSetColumnIndex(col);
    //     bool is_selected = (row == selected_row && col == selected_col);
    //
    //
    //
    //     if (is_selected) {
    //         ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImGui::GetColorU32(ImGuiCol_HeaderActive));
    //     }
    //
    //     char id_buf[32];
    //     snprintf(id_buf, 32, "##cell_%d_%d", row, col);
    //
    //     if (is_selected && is_editing) {
    //         ImGui::SetKeyboardFocusHere();
    //         ImGui::SetNextItemWidth(-FLT_MIN);
    //
    //         // 1. Capture the result of the InputText
    //         bool submitted = ImGui::InputText(id_buf, edit_buf, sizeof(edit_buf),
    //                                           ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll);
    //
    //         // 2. Logic: Only save if Enter was pressed AND it wasn't the frame we opened
    //         if (submitted) {
    //             if (!just_opened_this_frame) {
    //                 note_val = (int16_t)controller->getIdFromNoteName(edit_buf);
    //                 is_editing = false;
    //             }
    //         }
    //
    //         // 3. Reset the shield flag ONLY after the first frame has successfully rendered
    //         // We do this at the very end of the block.
    //         just_opened_this_frame = false;
    //
    //         if (ImGui::IsItemDeactivated() && !ImGui::IsItemDeactivatedAfterEdit()) {
    //             is_editing = false;
    //         }
    //     } else {
    //         // --- VIEW MODE ---
    //         // Single click to select, double click to edit
    //
    //         ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0, 0, 0, 0));
    //         ImGui::PushStyleColor(ImGuiCol_HeaderActive,  ImVec4(0, 0, 0, 0));
    //
    //         if (ImGui::Selectable(id_buf, is_selected, ImGuiSelectableFlags_AllowDoubleClick, ImVec2(0, 0))) {
    //             selected_row = row;
    //             dLog("MO Selected row: %d", selected_row);
    //             selected_col = col;
    //
    //             // IMPORTANT THIS to prevent cursor goes to header after click ===>>>>
    //             ImGui::SetWindowFocus(nullptr);
    //             ImGui::SetWindowFocus("FM Song Composer");
    //             //<<<<<<
    //
    //
    //             if (ImGui::IsMouseDoubleClicked(0)) {
    //                 is_editing = true;
    //                 just_opened_this_frame = true;
    //                 std::string current = controller->getNoteNameFromId(note_val);
    //                 snprintf(edit_buf, sizeof(edit_buf), "%s", current.c_str());
    //             }
    //         }
    //         ImGui::PopStyleColor(2);
    //
    //         // Display logic... (Keep your existing TextColored logic here)
    //         std::string display_text;
    //         if (note_val == -1)      display_text = "===";
    //         else if (note_val == 0)   display_text = "...";
    //         else                     display_text = controller->getNoteNameFromId(note_val);
    //
    //         ImGui::SameLine();
    //         ImGui::SetCursorPosX(ImGui::GetCursorPosX() - ImGui::GetStyle().ItemSpacing.x);
    //         if (note_val == -1)      ImGui::TextColored(ImVec4(1, 0, 1, 1), "===");
    //         else if (note_val == 0)   ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1), "...");
    //         else                     ImGui::TextUnformatted(display_text.c_str());
    //     }
    // }

    //-----------------------------------------------------------------------------------------------------
    //-----------------------------------------------------------------------------------------------------
    void DrawComposer()
    {
        if (!mController)
            return;

        auto handleNoteInput = [&](ImGuiKey key, const char* natural, const char* sharp) {
            if (ImGui::IsKeyPressed(key))
            {
                bool lShiftPressed = ImGui::IsKeyDown(ImGuiKey_LeftShift) || ImGui::IsKeyDown(ImGuiKey_RightShift);
                int lChannel = selected_col - 1;

                mSongData.song[mSelectedRow][lChannel] = mController->getNoteWithOctave(lChannel, lShiftPressed ? sharp : natural);

                mController->playNoteDOS(lChannel, mSongData.song[mSelectedRow][lChannel]);

                mSelectedRow = std::min(1000, mSelectedRow + mController->getStepByChannel(lChannel));

                return true;
            }
            return false;
        };

        // -------------- check we are playing a song ------------------------
        // Get read-only state from your controller
        const auto& lSequencerState = mController->getSequencerState();
        int current_playing_row = 0;
        if ( lSequencerState.playing )
        {
            current_playing_row =lSequencerState.song_needle  ;
            mController->consoleSongOutput(true); //FIXME DEBUG

        }
        //---------------
        ImVec2 lButtonSize = ImVec2(100, 0);
        //---------------

        ImGui::SetNextWindowSizeConstraints(ImVec2(800.0f, 600.0f), ImVec2(FLT_MAX, FLT_MAX));
        // if (ImGui::Begin("FM Song Composer", nullptr, ImGuiWindowFlags_MenuBar))
        if (ImGui::Begin("FM Song Composer", nullptr, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoScrollWithMouse))
        {
            // Menu bar for file operations, options, etc.
            if (ImGui::BeginMenuBar())
            {
                if (ImGui::BeginMenu("File"))
                {
                    if (ImGui::MenuItem("New Song")) { /* TODO: Implement new song logic */ }
                    if (ImGui::MenuItem("Load Song")) { /* TODO: Implement load song logic */ }
                    if (ImGui::MenuItem("Save Song")) { /* TODO: Implement save song logic */ }
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Options"))
                {
                    // Add composer-specific options here
                    ImGui::EndMenu();
                }


                ImGui::SameLine();
                ImGui::Dummy(ImVec2(5.0f, 0.0f));


                if (ImGui::Button("Load TestSong"))
                {
                    loadSong("assets/fm/songs/test.fms");
                }

                ImGui::EndMenuBar();
            }

            // Song Controls (Length, Speed)
            if (ImGui::BeginTable("SongControls", 4, ImGuiTableFlags_SizingFixedFit))
            {
                ImGui::TableNextColumn();
                ImGui::Text("Song Length:");
                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(120);
                ImGui::InputScalar("##Length", ImGuiDataType_U16, &mSongData.song_length, nullptr, nullptr, "%u");

                ImGui::TableNextColumn();
                ImGui::Text("Song Delay:");
                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(120);
                ImGui::InputScalar("##Speed", ImGuiDataType_U8, &mSongData.song_delay, nullptr, nullptr, "%u");


                ImGui::TableNextRow(); // <<< row ----
                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(120);
                if (lSequencerState.playing)
                {
                    if (ImGui::Button("Stop",lButtonSize))
                    {
                        mController->setPlaying(false);
                    }
                } else {
                    if (ImGui::Button("Play",lButtonSize))
                    {
                        // For demonstration, play a C-4 note on the current channel
                        mController->playSong(mSongData, mLoop, mStartAt, mEndAt);
                    }
                }

                ImGui::TableNextColumn();
                ImGui::BeginDisabled(lSequencerState.playing);
                ImGui::DragIntRange2("##SongRange", &mStartAt, &mEndAt, 1.0f, 0, mSongData.song_length);
                ImGui::EndDisabled();

                ImGui::TableNextColumn();
                if (ImGui::Checkbox("Loop", &mLoop))
                {
                    mController->setLoop( mLoop );
                }

                ImGui::TableNextColumn();


                ImGui::EndTable();
            }


            // hint for readonly
            if (lSequencerState.playing && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
                ImGui::SetTooltip("Cannot change while playing.");
            }

            ImGui::Separator();

            //---------------------- SongDisplay >>>>>>>>>>>>>>>>>>>

            // if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && !is_editing)
            if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && !ImGui::GetIO().WantTextInput && !is_editing)

            {
                // bool lAltPressed = ImGui::IsKeyDown(ImGuiKey_LeftAlt);
                bool lShiftPressed = ImGui::IsKeyDown(ImGuiKey_LeftShift) || ImGui::IsKeyDown(ImGuiKey_RightShift);
                bool lCtrlPressed = ImGui::IsKeyDown(ImGuiKey_LeftCtrl) || ImGui::IsKeyDown(ImGuiKey_RightCtrl);
                float lWheel = ImGui::GetIO().MouseWheel;

                if (lShiftPressed && mSelectionPivot == -1) {
                    mSelectionPivot = mSelectedRow;
                }

                if (ImGui::IsKeyPressed(ImGuiKey_UpArrow, true) || lWheel > 0)    {
                    mSelectedRow = std::max(0, mSelectedRow - 1);
                    if (!lShiftPressed) mSelectionPivot = -1;
                }
                else
                if (ImGui::IsKeyPressed(ImGuiKey_DownArrow, true) || lWheel < 0)  {
                    mSelectedRow = std::min(static_cast<int>(mSongData.song_length), mSelectedRow + 1);
                    if (!lShiftPressed) mSelectionPivot = -1;
                }
                else
                if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow))  selected_col = std::max(1, selected_col - 1);
                else
                if (ImGui::IsKeyPressed(ImGuiKey_RightArrow)) selected_col = std::min(9, selected_col + 1);
                else
                if (ImGui::IsKeyPressed(ImGuiKey_PageUp))
                {
                    int newRow = std::max(0, mSelectedRow - 16);
                    mSelectedRow = newRow;
                    if (!lShiftPressed) mSelectionPivot = -1;
                }
                else
                if (ImGui::IsKeyPressed(ImGuiKey_PageDown))
                {
                    mSelectedRow = std::min(static_cast<int>(mSongData.song_length), mSelectedRow + 1);
                    if (!lShiftPressed) mSelectionPivot = -1;
                }
                else
                if (ImGui::IsKeyPressed(ImGuiKey_Home)) {
                    mSelectedRow = 0;
                    if (!lShiftPressed) mSelectionPivot = -1;
                }
                else
                if (ImGui::IsKeyPressed(ImGuiKey_End)) {
                    // Use your song_length or the absolute max row (999)
                    mSelectedRow = mSongData.song_length > 0 ? (mSongData.song_length - 1) : 0;
                    if (!lShiftPressed) mSelectionPivot = -1;
                }



                if (ImGui::IsKeyPressed(ImGuiKey_Enter) || ImGui::IsKeyPressed(ImGuiKey_KeypadEnter)) {
                    is_editing = true;
                    just_opened_this_frame = true; // SET THIS FLAG

                    std::string current = mController->getNoteNameFromId(mSongData.song[mSelectedRow][selected_col-1]);
                    snprintf(edit_buf, sizeof(edit_buf), "%s", current.c_str());
                } else {


                    int lChannel = selected_col - 1;
                    int16_t& current_note = mSongData.song[mSelectedRow][lChannel];
                    // Check for specific keys to "push" values immediately

                    if (!lCtrlPressed)
                    {
                        if      (handleNoteInput(ImGuiKey_C, "C-", "C#")) {}
                        else if (handleNoteInput(ImGuiKey_D, "D-", "D#")) {}
                        else if (handleNoteInput(ImGuiKey_E, "E-", "F-")) {} // E# is usually F
                        else if (handleNoteInput(ImGuiKey_F, "F-", "F#")) {}
                        else if (handleNoteInput(ImGuiKey_G, "G-", "G#")) {}
                        else if (handleNoteInput(ImGuiKey_A, "A-", "A#")) {}
                        else if (handleNoteInput(ImGuiKey_B, "B-", "C-")) {} // B# is usually C (next octave)
                    }

                    // special without step
                    if (ImGui::IsKeyPressed(ImGuiKey_Space))  {
                        current_note = -1; // "===" Note Off
                    }
                    else
                    if ( ImGui::IsKeyPressed(ImGuiKey_Delete))
                    {
                        if (mSelectionPivot >= 0)
                            mController->clearSongRange(mSongData,  getSelectionMin(), getSelectionMax());
                        else
                            current_note = 0;  // "..." Empty

                    }


                    if (ImGui::IsKeyPressed(ImGuiKey_1))
                        lShiftPressed ? mController->decOctaveByChannel(lChannel) :  mController->incOctaveByChannel(lChannel);

                    if (ImGui::IsKeyPressed(ImGuiKey_2))
                        lShiftPressed ? mController->decStepByChannel(lChannel) :  mController->incStepByChannel(lChannel);


                    // --- Handling Operations on Range ---

                    if (lCtrlPressed && ImGui::IsKeyPressed(ImGuiKey_C)) {
                        mController->clearSong(mBufferSongData);
                        mController->copySongRange(mSongData,getSelectionMin(), mBufferSongData, 0, getSelectionLen());

                    }
                    if (lCtrlPressed && ImGui::IsKeyPressed(ImGuiKey_V))
                    {
                        mController->copySongRange(mBufferSongData, 0, mSongData, getSelectionMin(), mBufferSongData.song_length);
                    }
                    //FIXME Ctrl + x
                    //FIXME Ctrl + v




                }
            } //is_eding


            // --------------- Songdata -----------------
            if (ImGui::BeginChild("##SongDataTable", ImVec2(0, -ImGui::GetTextLineHeightWithSpacing()), ImGuiChildFlags_Borders, ImGuiWindowFlags_AlwaysVerticalScrollbar))
            {
                if (ImGui::BeginTable("SongTable", 10, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY))
                {
                    // 1. Setup ALL columns BEFORE any row rendering
                    ImGui::TableSetupScrollFreeze(0, 2); // Freeze Header + Oct/Step row

                    ImGui::TableSetupColumn("Seq", ImGuiTableColumnFlags_WidthFixed, 40.0f);
                    for (int j = 0; j < 9; j++) // Channels 0 to 8
                    {
                        ImGui::TableSetupColumn(mController->GetChannelNameShort(j), ImGuiTableColumnFlags_WidthFixed, 60.0f);
                    }

                    // 2. Render the actual Header row
                    // ImGui::TableHeadersRow();
                    ImGui::TableNextRow(ImGuiTableRowFlags_Headers);

                    for (int j = 0; j <= FMS_MAX_CHANNEL + 1; j++)
                    {
                        int lChannel = j - 1;
                        bool lChannelIsMuted = mController->getChannelMuted(lChannel);

                        // 2. Move to the correct column
                        if (!ImGui::TableSetColumnIndex(j)) continue;

                        // 3. Submit a manual header with its label
                        const char* col_name = ImGui::TableGetColumnName(j);

                        if (lChannelIsMuted)
                            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 0.f, 0.f, 1.0f));

                        ImGui::TableHeader(col_name);

                        if (lChannelIsMuted)
                            ImGui::PopStyleColor();

                        // ----  Header popup menu and header click -----


                        if (ImGui::BeginPopupContextItem()) { // Standard right-click context menu
                            if (ImGui::MenuItem("Mute Channel")) { mController->setChannelMuted(lChannel, !lChannelIsMuted); }
                            // if (ImGui::MenuItem("Solo Channel")) { /* ... */ }
                            ImGui::EndPopup();
                        }

                        // 4. DETECT THE CLICK
                        if (ImGui::IsItemClicked() && j > 0)
                        {
                            mController->setChannelMuted(lChannel, !mController->getChannelMuted(lChannel));
                            dLog("Header clicked for column %d", j);
                        }
                    }





                    // 3. Render the Second Fixed Row (Octave | Step info)
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextDisabled("O,S");

                    for (int j = 0; j < 9; j++)
                    {
                        if (ImGui::TableSetColumnIndex(j + 1))
                        {
                            // Just use Text here. You cannot use TableSetupColumn here.
                            ImGui::TextColored(ImVec4(0.7f, 0.7f, 1.0f, 1.0f), "%0d | %0d",
                                               mController->getOctaveByChannel(j),
                                               mController->getStepByChannel(j));
                        }
                    }

                    // -------------------- MAIN TABLE RENDERING --------------------------
                    ImGuiListClipper clipper;

                    // clipper.Begin(FMS_MAX_SONG_LENGTH, 20.f);
                    clipper.Begin(mSongData.song_length+1, 20.f);

                    clipper.IncludeItemByIndex(mSelectedRow);

                    while (clipper.Step())
                    {
                        for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
                        {
                            ImGui::TableNextRow(ImGuiTableRowFlags_None, 20.0f);
                            ImGui::PushID(i);
                            if (lSequencerState.playing)
                            {
                                if (i == current_playing_row) {
                                    // Highlight row using RowBg0 (standard for active rows)
                                    ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImColor(80, 80, 0, 255));
                                    ImGui::SetScrollHereY(0.5f); // Centers the playing row
                                }

                            } else if (isRowSelected(i))
                            {
                                // Different color for the selection range vs. the active cursor
                                ImVec4 color = (i == mSelectedRow) ? ImVec4(0.3f, 0.3f, 0.1f, 1.0f) : ImVec4(0.15f, 0.15f, 0.3f, 1.0f);
                                ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImGui::GetColorU32(color));


                                //FIXME but how ?
                                if (i == mSelectedRow)
                                    ImGui::SetScrollHereY(0.5f);
                            }


                            // Draw Sequence Number
                            ImGui::TableSetColumnIndex(0);
                            ImGui::Text("%03d", i + 1);

                            // Draw Channels
                            for (int j = 1; j <= 9; j++) {
                                DrawNoteCell(i, j, mSongData.song[i][j-1], mController);
                            }

                            ImGui::PopID();
                        } //for display ....
                    } //while clipper...

                    ImGui::EndTable();
                }
            }
            ImGui::EndChild();


            //---------------------- <<<<<<<<< SongDisplay


        }
        ImGui::End();
    }
};
