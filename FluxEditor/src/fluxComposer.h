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


    int selected_row = 0;
    int selected_col = 1;
    bool is_editing = false;
    bool just_opened_this_frame = false;
    char edit_buf[32] = "";

    OplController::SongData mSongData;
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
    //-----------------------------------------------------------------------------------------------------
    //-----------------------------------------------------------------------------------------------------
    void DrawNoteCell(int row, int col, int16_t& note_val, FluxEditorOplController* controller)
    {
        ImGui::TableSetColumnIndex(col);
        bool is_selected = (row == selected_row && col == selected_col);



        if (is_selected) {
            ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImGui::GetColorU32(ImGuiCol_HeaderActive));
        }

        char id_buf[32];
        snprintf(id_buf, 32, "##cell_%d_%d", row, col);

        if (is_selected && is_editing) {
            ImGui::SetKeyboardFocusHere();
            ImGui::SetNextItemWidth(-FLT_MIN);

            // 1. Capture the result of the InputText
            bool submitted = ImGui::InputText(id_buf, edit_buf, sizeof(edit_buf),
                                              ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll);

            // 2. Logic: Only save if Enter was pressed AND it wasn't the frame we opened
            if (submitted) {
                if (!just_opened_this_frame) {
                    note_val = (int16_t)controller->getIdFromNoteName(edit_buf);
                    is_editing = false;
                }
            }

            // 3. Reset the shield flag ONLY after the first frame has successfully rendered
            // We do this at the very end of the block.
            just_opened_this_frame = false;

            if (ImGui::IsItemDeactivated() && !ImGui::IsItemDeactivatedAfterEdit()) {
                is_editing = false;
            }
        } else {
            // --- VIEW MODE ---
            // Single click to select, double click to edit

            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0, 0, 0, 0));
            ImGui::PushStyleColor(ImGuiCol_HeaderActive,  ImVec4(0, 0, 0, 0));

            if (ImGui::Selectable(id_buf, is_selected, ImGuiSelectableFlags_AllowDoubleClick, ImVec2(0, 0))) {
                selected_row = row;
                dLog("MO Selected row: %d", selected_row);
                selected_col = col;

                // THIS to prevent cursor goes to header after click ===>>>>
                ImGui::SetWindowFocus(nullptr);
                ImGui::SetWindowFocus("FM Song Composer");
                //<<<<<<


                if (ImGui::IsMouseDoubleClicked(0)) {
                    is_editing = true;
                    just_opened_this_frame = true;
                    std::string current = controller->getNoteNameFromId(note_val);
                    snprintf(edit_buf, sizeof(edit_buf), "%s", current.c_str());
                }
            }
            ImGui::PopStyleColor(2);

            // Display logic... (Keep your existing TextColored logic here)
            std::string display_text;
            if (note_val == -1)      display_text = "===";
            else if (note_val == 0)   display_text = "...";
            else                     display_text = controller->getNoteNameFromId(note_val);

            ImGui::SameLine();
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() - ImGui::GetStyle().ItemSpacing.x);
            if (note_val == -1)      ImGui::TextColored(ImVec4(1, 0, 1, 1), "===");
            else if (note_val == 0)   ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1), "...");
            else                     ImGui::TextUnformatted(display_text.c_str());
        }
    }

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

                mSongData.song[selected_row][lChannel] = mController->getNoteWithOctave(lChannel, lShiftPressed ? sharp : natural);

                mController->playNoteDOS(lChannel, mSongData.song[selected_row][lChannel]);

                selected_row = std::min(1000, selected_row + mController->getStepByChannel(lChannel));

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
        if (ImGui::Begin("FM Song Composer", nullptr, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoNav))
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
                if (ImGui::IsKeyPressed(ImGuiKey_UpArrow))    {


                    int newRow = std::max(0, selected_row - 1);
                    dLog("UP Selected row: %d => %d", selected_row, newRow);
                    selected_row = newRow;

                }
                if (ImGui::IsKeyPressed(ImGuiKey_DownArrow))  {
                    selected_row = std::min(1000, selected_row + 1);
                    dLog("DN Selected row: %d", selected_row);

                }
                if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow))  selected_col = std::max(1, selected_col - 1);
                if (ImGui::IsKeyPressed(ImGuiKey_RightArrow)) selected_col = std::min(9, selected_col + 1);

                if (ImGui::IsKeyPressed(ImGuiKey_Enter) || ImGui::IsKeyPressed(ImGuiKey_KeypadEnter)) {
                    is_editing = true;
                    just_opened_this_frame = true; // SET THIS FLAG

                    std::string current = mController->getNoteNameFromId(mSongData.song[selected_row][selected_col-1]);
                    snprintf(edit_buf, sizeof(edit_buf), "%s", current.c_str());
                } else {


                    int lChannel = selected_col - 1;
                    int16_t& current_note = mSongData.song[selected_row][lChannel];
                    // Check for specific keys to "push" values immediately

                    if      (handleNoteInput(ImGuiKey_C, "C-", "C#")) {}
                    else if (handleNoteInput(ImGuiKey_D, "D-", "D#")) {}
                    else if (handleNoteInput(ImGuiKey_E, "E-", "F-")) {} // E# is usually F
                    else if (handleNoteInput(ImGuiKey_F, "F-", "F#")) {}
                    else if (handleNoteInput(ImGuiKey_G, "G-", "G#")) {}
                    else if (handleNoteInput(ImGuiKey_A, "A-", "A#")) {}
                    else if (handleNoteInput(ImGuiKey_B, "B-", "C-")) {} // B# is usually C (next octave)

                    // special without step
                    if (ImGui::IsKeyPressed(ImGuiKey_Space))  {
                        current_note = -1; // "===" Note Off
                    }
                    else
                    if ( ImGui::IsKeyPressed(ImGuiKey_Delete)) {
                        current_note = 0;  // "..." Empty
                    }

                    // bool lAltPressed = ImGui::IsKeyDown(ImGuiKey_LeftAlt);
                    bool lShiftPressed = ImGui::IsKeyDown(ImGuiKey_LeftShift) || ImGui::IsKeyDown(ImGuiKey_RightShift);

                    if (ImGui::IsKeyPressed(ImGuiKey_1))
                        lShiftPressed ? mController->decOctaveByChannel(lChannel) :  mController->incOctaveByChannel(lChannel);

                    if (ImGui::IsKeyPressed(ImGuiKey_2))
                        lShiftPressed ? mController->decStepByChannel(lChannel) :  mController->incStepByChannel(lChannel);
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


                    for (int i = 0; i <= 1000; i++) // Match song[1001]
                    {
                        ImGui::TableNextRow();


                        bool is_active = lSequencerState.playing && (i == current_playing_row);
                        if (is_active) {
                            // Highlight row using RowBg0 (standard for active rows)
                            ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImColor(80, 80, 0, 255));

                            if (lSequencerState.playing) {
                                ImGui::SetScrollHereY(0.5f); // Centers the playing row
                            }
                        }

                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text("%03d", i+1);

                        for (int j = 1; j <= FMS_MAX_CHANNEL + 1; j++)
                        {
                            DrawNoteCell(i, j, mSongData.song[i][j-1], mController);
                        }
                    }
                    ImGui::EndTable();
                }
            }
            ImGui::EndChild();


            //---------------------- <<<<<<<<< SongDisplay


        }
        ImGui::End();
    }
};
