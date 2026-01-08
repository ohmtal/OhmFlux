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
    int mSelectedCol = 1;
    bool mIsEditing = false;
    bool mJustOpenThisFrame = false;
    char mEditBuffer[32] = "";

    bool mScrollToSelected = false;

    int mCurrentPlayingRow = -1;


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


    bool  isPlaying() { return mController->getSequencerState().playing; }
    //-----------------------------------------------------------------------------------------------------
    // when playing live adding !!
    void insertTone(const char* lName)
    {
        int lChannel = mSelectedCol - 1;

        if (isPlaying())
        {
            mSongData.song[mCurrentPlayingRow][lChannel] = mController->getNoteWithOctave(lChannel, lName);
        } else {
            mSongData.song[mSelectedRow][lChannel] = mController->getNoteWithOctave(lChannel, lName);
            mController->playNoteDOS(lChannel, mSongData.song[mSelectedRow][lChannel]);
            mSelectedRow = std::min(FMS_MAX_SONG_LENGTH, mSelectedRow + mController->getStepByChannel(lChannel));
            if ( mSelectedRow > mSongData.song_length )
                mSongData.song_length = mSelectedRow;
        }
    }
    //--------------------------------------------------------------------------
    void DrawPianoScale()
    {
        struct PianoKey { const char* name; int offset; bool isBlack; };
        PianoKey keys[] = {
            {"C-", 0, false}, {"C#", 7, true}, {"D-", 1, false}, {"D#", 8, true},
            {"E-", 2, false},  {"F-", 3, false}, {"F#", 9, true}, {"G-", 4, false},
            {"G#", 10, true}, {"A-", 5, false}, {"A#", 11, true}, {"B-", 6, false}
        };
        int lChannel = mSelectedCol - 1;

        int  currentOctave = mController->getOctaveByChannel(lChannel);

        ImVec2 startPos = ImGui::GetCursorScreenPos();
        float whiteWidth = 40.0f, whiteHeight = 150.0f;
        float blackWidth = 26.0f, blackHeight = 90.0f;

        // 1. Draw White Keys (Allowing Overlap)
        int whiteKeyCount = 0;
        for (int i = 0; i < 12; i++) {
            if (keys[i].isBlack) continue;

            ImGui::PushID(i);
            ImGui::SetCursorScreenPos(ImVec2(startPos.x + (whiteKeyCount * whiteWidth), startPos.y));

            // FIX: Tell ImGui this item can be overlapped by items drawn later
            ImGui::SetNextItemAllowOverlap();

            bool isNull = (currentOctave == 0 && i == 0);
            if (isNull) ImGui::BeginDisabled();

            ImGui::Button("##white", ImVec2(whiteWidth, whiteHeight));

            // add note
            if (ImGui::IsItemActivated()) {
                insertTone(keys[i].name);
            }


            if (isNull) ImGui::EndDisabled();
            ImGui::PopID();
            whiteKeyCount++;
        }
        //--------------------------------------------------------------------------
        // 2. Draw Black Keys (On Top)
        whiteKeyCount = 0;
        for (int i = 0; i < 12; i++) {
            if (!keys[i].isBlack) { whiteKeyCount++; continue; }

            float xPos = startPos.x + (whiteKeyCount * whiteWidth) - (blackWidth / 2.0f);
            ImGui::SetCursorScreenPos(ImVec2(xPos, startPos.y));

            ImGui::PushID(i + 100);
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.05f, 0.05f, 0.05f, 1.0f)); // Darker black
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));

            bool isNull = (currentOctave == 7);
            if (isNull) ImGui::BeginDisabled();

            ImGui::Button("##black", ImVec2(blackWidth, blackHeight));

            if (ImGui::IsItemActivated()) {
                insertTone(keys[i].name);
            }


            if (isNull) ImGui::EndDisabled();
            ImGui::PopStyleColor(2);
            ImGui::PopID();
        }

        // Correctly extend window boundary
        ImVec2 finalPos = ImVec2(startPos.x + (whiteKeyCount * whiteWidth), startPos.y + whiteHeight);
        ImGui::SetCursorScreenPos(finalPos);
        ImGui::Dummy(ImVec2(0, 10));

    }

    //-----------------------------------------------------------------------------------------------------
    void DrawNoteCell(int row, int col, int16_t& note_val, FluxEditorOplController* controller)
    {
        ImGui::TableSetColumnIndex(col);
        bool is_selected = (row == mSelectedRow && col == mSelectedCol);

        // Use a simpler ID system to prevent ID collisions during clipping
        ImGui::PushID(col);

        if (is_selected && mIsEditing) {
            // --- EDIT MODE ---
            ImGui::SetKeyboardFocusHere();
            ImGui::SetNextItemWidth(-FLT_MIN);

            if (ImGui::InputText("##edit", mEditBuffer, sizeof(mEditBuffer),
                ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll)) {
                if (!mJustOpenThisFrame) {
                    note_val = (int16_t)controller->getIdFromNoteName(mEditBuffer);
                    mIsEditing = false;
                }
                }
                mJustOpenThisFrame = false;

            if (ImGui::IsItemDeactivated() && !ImGui::IsItemDeactivatedAfterEdit()) {
                mIsEditing = false;
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
                mSelectedCol = col;

                if (!isPlaying())
                    mScrollToSelected = true;

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
                    mIsEditing = true;
                    mJustOpenThisFrame = true;
                    snprintf(mEditBuffer, sizeof(mEditBuffer), "%s", display_text);
                }
            }
            ImGui::PopStyleColor(2);

            // 4. Draw text directly on top of the selectable
            ImGui::SameLine(ImGui::GetStyle().ItemSpacing.x);
            ImGui::TextColored(color, "%s", display_text);
        }
        ImGui::PopID();
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
                int lChannel = mSelectedCol - 1;

                insertTone(lShiftPressed ? sharp : natural);
                return true;
            }
            return false;
        };

        // -------------- check we are playing a song ------------------------
        // Get read-only state from your controller
        const auto& lSequencerState = mController->getSequencerState();
        if ( lSequencerState.playing )
        {
            mCurrentPlayingRow =lSequencerState.song_needle  ;
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
            if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && !ImGui::GetIO().WantTextInput && !mIsEditing)

            {
                // bool lAltPressed = ImGui::IsKeyDown(ImGuiKey_LeftAlt);
                bool lShiftPressed = ImGui::IsKeyDown(ImGuiKey_LeftShift) || ImGui::IsKeyDown(ImGuiKey_RightShift);
                bool lCtrlPressed = ImGui::IsKeyDown(ImGuiKey_LeftCtrl) || ImGui::IsKeyDown(ImGuiKey_RightCtrl);
                float lWheel = ImGui::GetIO().MouseWheel;




                if (lShiftPressed && mSelectionPivot == -1) {
                    mSelectionPivot = mSelectedRow;
                }

                mScrollToSelected = true;
                if (ImGui::IsKeyPressed(ImGuiKey_UpArrow, true) /*|| lWheel > 0*/)
                {
                    mSelectedRow = std::max(0, mSelectedRow - 1);
                    if (!lShiftPressed) mSelectionPivot = -1;
                }
                else
                if (ImGui::IsKeyPressed(ImGuiKey_DownArrow, true) /*|| lWheel < 0*/)
                {
                    mSelectedRow = std::min(static_cast<int>(mSongData.song_length), mSelectedRow + 1);
                    if (!lShiftPressed) mSelectionPivot = -1;
                }
                else
                if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow))  mSelectedCol = std::max(1, mSelectedCol - 1);
                else
                if (ImGui::IsKeyPressed(ImGuiKey_RightArrow)) mSelectedCol = std::min(9, mSelectedCol + 1);
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
                    mSelectedRow = std::min(static_cast<int>(mSongData.song_length), mSelectedRow + 16);
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
                } else {
                    mScrollToSelected = false;
                }



                if (ImGui::IsKeyPressed(ImGuiKey_Enter) || ImGui::IsKeyPressed(ImGuiKey_KeypadEnter)) {
                    mIsEditing = true;
                    mJustOpenThisFrame = true; // SET THIS FLAG

                    std::string current = mController->getNoteNameFromId(mSongData.song[mSelectedRow][mSelectedCol-1]);
                    snprintf(mEditBuffer, sizeof(mEditBuffer), "%s", current.c_str());
                } else {


                    int lChannel = mSelectedCol - 1;
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
                    if ( ImGui::IsKeyPressed( ImGuiKey_Insert ) )
                    {
                        if ( (mSelectedRow == mSongData.song_length) && (mSongData.song_length <= FMS_MAX_SONG_LENGTH) )
                        {
                            mSongData.song_length ++ ;
                            mSelectedRow ++;
                        }
                        else
                        {
                            mController->insertRowAt(mSongData, mSelectedRow);
                        }
                    }




                    if (ImGui::IsKeyPressed(ImGuiKey_KeypadAdd) || ImGui::IsKeyPressed(ImGuiKey_Q) )
                    {
                        mController->incOctaveByChannel(lChannel);
                    }

                    if (ImGui::IsKeyPressed(ImGuiKey_KeypadSubtract) || ImGui::IsKeyPressed(ImGuiKey_W) )
                    {
                        mController->decOctaveByChannel(lChannel);
                    }


                    if (ImGui::IsKeyPressed(ImGuiKey_S))
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


            DrawPianoScale();

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


                        if (ImGui::BeginPopupContextItem())
                        {
                            // fancy header
                            ImDrawList* draw_list = ImGui::GetWindowDrawList();
                            ImVec2 p0 = ImGui::GetCursorScreenPos();
                            ImVec2 p1 = ImVec2(p0.x + ImGui::GetContentRegionAvail().x, p0.y + ImGui::GetTextLineHeightWithSpacing());

                            // Zeichne eine gefüllte Fläche als Hintergrund
                            draw_list->AddRectFilled(p0, p1, ImGui::GetColorU32(ImGuiCol_HeaderActive), 4.0f);

                            // Text darüber platzieren
                            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2.0f);
                            ImGui::Indent(5.0f);
                            ImGui::Text("%s", mController->GetChannelName(lChannel));
                            ImGui::Unindent(5.0f);
                            //<<<


                            ImGui::Separator();

                            if (ImGui::MenuItem("Mute Channel")) { mController->setChannelMuted(lChannel, !lChannelIsMuted); }
                            // if (ImGui::MenuItem("Solo Channel")) { /* ... */ }

                            ImGui::Separator();

                            // Octave ...
                            int lOctave = mController->getOctaveByChannel(lChannel);
                            ImGui::Text("Octave:");
                            ImGui::SetNextItemWidth(100.0f); // Often needed as menus are narrow by default
                            if (ImGui::InputInt("##Octave", &lOctave)) {
                                mController->setOctaveByChannel(lChannel, lOctave);
                            }

                            ImGui::Separator();

                            // Step ..
                            int lStep = mController->getStepByChannel(lChannel);
                            ImGui::Text("Step:");
                            ImGui::SetNextItemWidth(100.0f); // Often needed as menus are narrow by default
                            if (ImGui::InputInt("##step", &lStep)) {
                                mController->setStepByChannel(lChannel, lStep);
                            }
                            //<<< Step

                            //-----
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
                                if (i == mCurrentPlayingRow) {
                                    // Highlight row using RowBg0 (standard for active rows)
                                    ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImColor(80, 80, 0, 255));
                                    ImGui::SetScrollHereY(0.5f); // Centers the playing row
                                }

                            } else if (isRowSelected(i))
                            {
                                // Different color for the selection range vs. the active cursor
                                ImVec4 color = (i == mSelectedRow) ? ImVec4(0.3f, 0.3f, 0.1f, 1.0f) : ImVec4(0.15f, 0.15f, 0.3f, 1.0f);
                                //FIXME when cols are selected (is mute at the moment) like this:
                                // speed up make ImGui::GetColorU32 only once !!!!
                                // ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, color1, 0); // Color Col 0
                                // ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, color2, 2); // Color Col 2


                                ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImGui::GetColorU32(color));


                                if (mScrollToSelected && (i == mSelectedRow))
                                {
                                    ImGui::SetScrollHereY(0.5f);
                                    mScrollToSelected = false;
                                }

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

                        // clipper.DisplayEnd - 1 => last row but i want to scroll earlier
                        // FIXME ? also bad here
                        // if ( mSelectedRow  > clipper.DisplayEnd - 3)
                        //     mScrollToSelected = true;
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
