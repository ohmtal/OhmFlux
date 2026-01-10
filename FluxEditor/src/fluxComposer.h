//-----------------------------------------------------------------------------
// Copyright (c) 2026 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// 2026-01-08
// * Menu play Selection
// * Piano: silence, insert or test  mode, note off
//
// 2026-01-09
// *  Piano:Sharp notes wrong !!
// * rewrite from handleNoteInput to keybord playing (y/x)
//     => USE SDL SCANCODE FOR keyboard layout same on all languages
//     => Note intput by name with [ALT]
// *  sync channel with fmEditor uint8_t mInstrumentChannel = 0; // 0 .. FMS_MAX_CHANNEL
//       ==> USE SDL Events for this !!!
//         event.type = FLUX_EVENT_OPL_CHANNEL_CHANGED; //FLUX_EVENT_SCALE_CHANGED;
//         event.user.code = lChannel;           // A custom integer code == channel
//
// * save ini to user path (test in emscripten => is not permanent)
// * ini not saved in path !!
//        => INI: Ini file set to:/home/tom/.local/share/Ohmflux/Flux_Editor/Flux_EditorGui.ini
//       made the variable static this works
// * song_length is +1 ?! is it only the display or also in the OplController ?
//   fixed at Clipper i had +1 no idea why
// * check for active window on event => note input
// *  OplController does not save the instruments ....!!!
//
// 2026-01-10
// * Instrument names.. added cache but name must be also set on resetInstrument and
//       presets ... also when it saved = overwritten
//
// * Save settings
//
// TODO: toggle melodic mode ++ save in settings
//
// TODO: Record mode with pre ticker like my guitar looper
//       Before i do this i should find out why record mode is so laggy!
//       i guess it's the mutex lock ?
//
// TODO:  I also like to have the fullscale als live insert ...
//
//
// TODO: Load / Save ==> also emscripten load will be tricky if done check SFXEditor
//       check => trigger_file_load (marked with TODO TEST)
//-----------------------------------------------------------------------------
#pragma once

#include <core/fluxBaseObject.h>
#include <utils/fluxSettingsManager.h>
#include <imgui.h>
#include "imgui_internal.h"
#include "fluxEditorOplController.h"
#include "fluxEditorGlobals.h"




class FluxComposer : public FluxBaseObject
{
private:
    FluxEditorOplController* mController = nullptr;
    bool mItsMyController = false;
    // std::unique_ptr<FluxEditorOplController> mController = nullptr;

    static const int MAX_PATTERNS = 16;
    static const int NOTES_PER_PATTERN = 64;

    // hackfest
    int mStartAt = 0;
    int mEndAt = -1;
    bool mLoop = false;


    int mSelectedRow = 0;
    int mSelectionPivot = -1;
    int mSelectedCol = 1;
    int mLastSetChannel = -1;
    // disabled !! dont need this anymore .. FIXME cleanup
    // bool mIsEditing = false;
    // bool mJustOpenThisFrame = false;
    char mEditBuffer[32] = "";

    bool mScrollToSelected = false;

    int mCurrentPlayingRow = -1;

    int mNewSongLen = 64;

    ImU32 mSelectedRowColor = 0;
    ImU32 mSelectedRowInactiveColor = 0;
    ImU32 mActiveRowColor = 0;
    ImU32 mPlayingRowColor = 0;


    // i use ALT for note input !! bool mKeyboardMode = true; //using keys yxcvbnnm instead of c,d,e
    bool mInsertMode   = true; //keyboard and piano insert tones
    bool mLiveMode     = false; //notes are insert where the play trigger is


    OplController::SongData mSongData;

    OplController::SongData mBufferSongData;

public:

    FluxComposer(FluxEditorOplController* lController)
    {
        mSongData.init();
        mBufferSongData.init();

        mController = lController;


        mSelectedRowColor = ImGui::GetColorU32(ImVec4(0.15f, 0.15f, 0.3f, 1.0f));
        mSelectedRowInactiveColor = ImGui::GetColorU32(ImVec4(0.15f, 0.15f, 0.3f, 0.3f));
        mActiveRowColor   = ImGui::GetColorU32(ImVec4(0.3f, 0.3f, 0.1f, 1.0f));

        mPlayingRowColor  = ImGui::GetColorU32(ImColor4F(cl_Coral));

    }
    ~FluxComposer() {
        Deinitialize();
    }
    //-----------------------------------------------------------------------------------------------------
    bool Initialize() override
    {

        if (!mController)
        {
            mController = new FluxEditorOplController();
            mItsMyController = true;
            if (!mController || !mController->initController())
                return false;
        }

        // Initialize SongData
        newSong();


        mInsertMode   = SettingsManager().get("fluxComposer::mInsertMode", true);
        mLiveMode     = SettingsManager().get("fluxComposer::mLiveMode", true);


        return true;
    }
    //-----------------------------------------------------------------------------------------------------
    void Deinitialize() override
    {
        SettingsManager().set("fluxComposer::mInsertMode", mInsertMode);
        SettingsManager().set("fluxComposer::mLiveMode", mLiveMode);


        // std::unique_ptr<Controller> mController; would be better ^^
        if (mController && mItsMyController)
        {
            SAFE_DELETE(mController);
        }

    }
    //-----------------------------------------------------------------------------------------------------
    bool loadSong(const std::string& filename)
    {
        if (!mController)
            return false;

        if (mController->loadSongFMS(filename, mSongData))
        {
            resetSongSettings();
            return true;
        }
        return false;

    }

    //-----------------------------------------------------------------------------------------------------
    void setController(FluxEditorOplController* controller)  { mController = controller;  }
    FluxEditorOplController*  getController()   { return mController;  }
    //-----------------------------------------------------------------------------------------------------
    void resetSelection() { mSelectionPivot = -1;}
    uint16_t getSelectionMin() { return (mSelectionPivot == -1) ? mSelectedRow : std::min(mSelectedRow, mSelectionPivot); }
    uint16_t getSelectionMax() { return (mSelectionPivot == -1) ? mSelectedRow : std::max(mSelectedRow, mSelectionPivot); }
    uint16_t getSelectionLen() { return (mSelectionPivot == -1) ? 1 : std::max(mSelectedRow, mSelectionPivot); }
    bool isRowSelected(int i) { return i >= getSelectionMin() && i <= getSelectionMax(); }


    bool  isPlaying() { return mController->getSequencerState().playing; }
    //-----------------------------------------------------------------------------------------------------
    // when playing live adding !!
    void insertTone(const char* lName, int lOctaveAdd = 0)
    {
        int lChannel = getCurrentChannel();


        int lNewTone = mController->getNoteWithOctave(lChannel, lName, lOctaveAdd);
        if (isPlaying() && mLiveMode)
        {
            mSongData.song[mCurrentPlayingRow][lChannel] = lNewTone;
        } else {
            if (std::strcmp(lName, "===") == 0)
            {
                mSongData.song[mSelectedRow][lChannel] = -1;
                mSelectedRow = std::min(FMS_MAX_SONG_LENGTH, mSelectedRow + mController->getStepByChannel(lChannel));
                mController->stopNote(lChannel);
                return ;
            }
            if (std::strcmp(lName, "...") == 0)
            {
                mSongData.song[mSelectedRow][lChannel] = 0;
                mSelectedRow = std::min(FMS_MAX_SONG_LENGTH, mSelectedRow + mController->getStepByChannel(lChannel));
                mController->stopNote(lChannel);
                return ;
            }


            mSongData.song[mSelectedRow][lChannel] = lNewTone;
            mController->playNoteDOS(lChannel, mSongData.song[mSelectedRow][lChannel]);
            mSelectedRow = std::min(FMS_MAX_SONG_LENGTH, mSelectedRow + mController->getStepByChannel(lChannel));
            if ( mSelectedRow > mSongData.song_length )
                mSongData.song_length = mSelectedRow;
        }
    }
    //--------------------------------------------------------------------------
    void DrawPianoScale()
    {
       int lChannel = getCurrentChannel();
       ImVec2 lButtonSize = ImVec2(100, 0);

        // ---- Header -----

        ImGui::SetNextItemWidth(200.0f);
        if (ImGui::SliderInt("##ChannelSlider", &lChannel,
            FMS_MIN_CHANNEL, FMS_MAX_CHANNEL,
            mController->GetChannelName(lChannel)))
        {
            lChannel = setChannel(lChannel);
        }

        ImGui::SameLine();
        ImGui::Checkbox("Insert Mode",&mInsertMode);
        ImGui::SameLine();
        ImGui::Checkbox("Live Mode",&mLiveMode);


        ImGui::BeginDisabled(!mInsertMode);
        ImGui::SameLine();
        if (ImGui::Button("add (===)", lButtonSize)) {
            insertTone("===");
        }
        ImGui::SameLine();
        if (ImGui::Button("add (...)", lButtonSize)) {
            insertTone("...");
        }
        // if (ImGui::Button("Add: ...", lButtonSize)) {}
        ImGui::EndDisabled();

        // ImGui::SameLine();
        // if (ImGui::Button("Silence all.", lButtonSize)) {
        //     mController->silenceAll(false);
        // }



        // ---- Scale ------
        int lScaleCount = 12 * 3;
        int lOctaveAdd = -1;
        int  currentOctave = 0;

        struct PianoKey { const char* name; int offset; bool isBlack; };
        PianoKey keys[] = {
            {"C-", 0, false}, {"C#", 7, true}, {"D-", 1, false}, {"D#", 8, true},
            {"E-", 2, false},  {"F-", 3, false}, {"F#", 9, true}, {"G-", 4, false},
            {"G#", 10, true}, {"A-", 5, false}, {"A#", 11, true}, {"B-", 6, false}
        };



        ImVec2 startPos = ImGui::GetCursorScreenPos();
        float whiteWidth = 40.0f, whiteHeight = 90.0f;
        float blackWidth = 26.0f, blackHeight = 50.0f;

        ImVec4 lWhileColor = ImColor4F(cl_LightGray);
        ImVec4 lWhileColorHoover = ImColor4F(cl_SkyBlue);
        if (mInsertMode) {
            lWhileColor = ImColor4F(cl_White);
        }

        // 1. Draw White Keys (Allowing Overlap)
        int whiteKeyCount = 0;
        for (int i = 0; i < lScaleCount; i++) {
            if (keys[i % 12].isBlack) continue;
            lOctaveAdd = std::trunc( i / 12) - 1;
            currentOctave = mController->getOctaveByChannel(lChannel) + lOctaveAdd;

            ImGui::PushID(i);
            ImGui::SetCursorScreenPos(ImVec2(startPos.x + (whiteKeyCount * whiteWidth), startPos.y));

            // FIX: Tell ImGui this item can be overlapped by items drawn later
            ImGui::SetNextItemAllowOverlap();

            bool isNull = (currentOctave == 0 && i == 0);
            if (isNull) ImGui::BeginDisabled();


            ImGui::PushStyleColor(ImGuiCol_Button, lWhileColor);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, lWhileColorHoover);


            ImGui::Button("##white", ImVec2(whiteWidth, whiteHeight));
            // add or play note
            if (ImGui::IsItemActivated()) {

                if (mInsertMode)
                    insertTone(keys[i % 12].name, lOctaveAdd);
                else {
                    mController->playNoteDOS(lChannel, ((currentOctave - 1) * 12) + keys[i % 12].offset);
                }

            }
            if (ImGui::IsItemDeactivated() && !mInsertMode)
                mController->stopNote(lChannel);

            if ((i % 12) == 0)
            {
                ImGui::SetCursorScreenPos(ImVec2(startPos.x + 5 + (whiteKeyCount * whiteWidth), startPos.y + whiteHeight - 20.f));
                ImGui::TextColored(ImColor4F(cl_Black), "%s%d",keys[i % 12].name,  mController->getOctaveByChannel(lChannel)+lOctaveAdd);
            }

            if (isNull) ImGui::EndDisabled();
            ImGui::PopStyleColor(2);
            ImGui::PopID();
            whiteKeyCount++;
        }
        //--------------------------------------------------------------------------
        // 2. Draw Black Keys (On Top)
        whiteKeyCount = 0;
        for (int i = 0; i < lScaleCount; i++)
        {
            lOctaveAdd = std::trunc( i / 12) - 1;
            currentOctave = mController->getOctaveByChannel(lChannel) + lOctaveAdd;
            if (!keys[i % 12].isBlack) { whiteKeyCount++; continue; }

            float xPos = startPos.x + (whiteKeyCount * whiteWidth) - (blackWidth / 2.0f);
            ImGui::SetCursorScreenPos(ImVec2(xPos, startPos.y));

            ImGui::PushID(i + 100);
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.05f, 0.05f, 0.05f, 1.0f)); // Darker black
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));

            bool isNull = (currentOctave == 7);
            if (isNull) ImGui::BeginDisabled();

            ImGui::Button("##black", ImVec2(blackWidth, blackHeight));

            // i hate redundancy ...
            // add or play note
            if (ImGui::IsItemActivated()) {

                if (mInsertMode)
                    insertTone(keys[i % 12].name, lOctaveAdd);
                else {
                    mController->playNoteDOS(lChannel, ((currentOctave - 1) * 12) + keys[i % 12].offset);
                }

            }
            if (ImGui::IsItemDeactivated() && !mInsertMode)
                mController->stopNote(lChannel);


            if ((i % 12) == 0)
            {
                ImGui::SetCursorScreenPos(ImVec2(xPos + 5, startPos.y + blackHeight - 20.f));
                ImGui::TextColored(ImColor4F(cl_Aquamarine), "%s%d",keys[i % 12].name,  mController->getOctaveByChannel(lChannel)+lOctaveAdd);
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
    void DrawNoteCell(int lRow, int lCol, int16_t& lNoteValue, FluxEditorOplController* lController, Color4F lNoteColor)
    {
        ImGui::TableSetColumnIndex(lCol);
        bool is_selected = (lRow == mSelectedRow && lCol == mSelectedCol);
        bool is_active   = mController->getChannelActive( lCol - 1 );

        // Use a simpler ID system to prevent ID collisions during clipping
        ImGui::PushID(lCol);
/*
        if (is_selected && mIsEditing) {
            // --- EDIT MODE ---
            ImGui::SetKeyboardFocusHere();
            ImGui::SetNextItemWidth(-FLT_MIN);

            if (ImGui::InputText("##edit", mEditBuffer, sizeof(mEditBuffer),
                ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll)) {
                if (!mJustOpenThisFrame) {
                    lNoteValue = (int16_t)lController->getIdFromNoteName(mEditBuffer);
                    mIsEditing = false;
                }
                }
                mJustOpenThisFrame = false;

            if (ImGui::IsItemDeactivated() && !ImGui::IsItemDeactivatedAfterEdit()) {
                mIsEditing = false;
            }
        } else*/
        {
            // --- VIEW MODE ---
            const char* lDisplayText = "...";
            ImVec4 lColor = ImColor4F(cl_Gray);

            if (lNoteValue == -1) {
                lDisplayText = "===";
                lColor = ImColor4F(cl_Magenta);
            } else if (lNoteValue > 0) {
                static std::string note_name;
                note_name = lController->getNoteNameFromId(lNoteValue);
                lDisplayText = note_name.c_str();
                lColor = ImColor4F(lNoteColor);
            }

            if (!is_active) {
                lColor = ImVec4(0.4f, 0.4f, 0.4f, 1.f); // Default Gray
            }


            // Disable hover highlights
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0,0,0,0));
            ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0,0,0,0));

            // The Selectable MUST be exactly the size of the cell to keep the clipper stable
            if (ImGui::Selectable("##select", is_selected, ImGuiSelectableFlags_AllowDoubleClick | ImGuiSelectableFlags_AllowOverlap, ImVec2(0, 0))) {
                mSelectedRow = lRow;
                mSelectedCol = lCol;
                setChannel( getCurrentChannel() );

                if (!isPlaying())
                    mScrollToSelected = true;

                if (  mSelectionPivot >= 0)
                {
                    bool lShiftPressed = ImGui::IsKeyDown(ImGuiKey_LeftShift) || ImGui::IsKeyDown(ImGuiKey_RightShift);
                    if  (!lShiftPressed)
                        resetSelection();
                }

                // This resets focus to the parent window to keep keyboard nav clean
                ImGui::SetWindowFocus(nullptr);
                ImGui::SetWindowFocus("FM Song Composer");

                // if (ImGui::IsMouseDoubleClicked(0)) {
                //     mIsEditing = true;
                //     mJustOpenThisFrame = true;
                //     snprintf(mEditBuffer, sizeof(mEditBuffer), "%s", lDisplayText);
                // }
            }
            ImGui::PopStyleColor(2);

            // 4. Draw text directly on top of the selectable
            ImGui::SameLine(ImGui::GetStyle().ItemSpacing.x);
            ImGui::TextColored(lColor, "%s", lDisplayText);
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
                // int lChannel = getCurrentChannel();

                insertTone(lShiftPressed ? sharp : natural);
                return true;
            }
            return false;
        };

        // -------------- check we are playing a song ------------------------
        // Get read-only state from your controller
        const auto& lSequencerState = mController->getSequencerState();
        if ( isPlaying() )
        {
            mCurrentPlayingRow =lSequencerState.song_needle  ;
            // mController->consoleSongOutput(true); // DEBUG
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
                    if (ImGui::MenuItem("New Song")) {
                        newSong();
                    }
                    if (ImGui::MenuItem("Load Song")) { showMessage("Open", "Use the File Browser to open a Song (fms)"); }
                    if (ImGui::MenuItem("Save Song")) {
                        g_FileDialog.mSaveMode = true;
                        g_FileDialog.mSaveExt = ".fms";
                        g_FileDialog.mLabel = "Save Song (.fms)";
                    }
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Edit"))
                {
                    if (ImGui::MenuItem("Insert emtpy row", "INS")) { insertEmpty(); }
                    ImGui::Separator();
                    if (ImGui::MenuItem("Copy", "Ctrl+c")) { copySelected(); }
                    if (ImGui::MenuItem("Paste", "Ctrl+v")) { pasteSelected(); }
                    ImGui::Separator();
                    if (ImGui::MenuItem("Clear", "DEL")) {  clearSelected();}
                    if (ImGui::MenuItem("Delete (rows)", "Ctrl+DEL")) { deleteSelected(); }

                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("Action"))
                {

                    if (ImGui::MenuItem("Play selected")) { playSong(1); }
                    if (ImGui::MenuItem("Silence all.")) { mController->silenceAll(false); }
                    ImGui::Separator();
                    if (ImGui::MenuItem("Activate all channel")) {mController->setAllChannelActive(true);}
                    if (ImGui::MenuItem("Deactivate all channel")) {mController->setAllChannelActive(false);}
                    ImGui::Separator();
                    if (ImGui::MenuItem("Octave + ","+, w")) { incOctave(); }
                    if (ImGui::MenuItem("Octave - ","-, q")) { decOctave(); }

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
                if (isPlaying())
                {
                    if (ImGui::Button("Stop",lButtonSize))
                    {
                        mController->setPlaying(false);
                    }
                } else {
                    if (ImGui::Button("Play",lButtonSize))
                    {
                        playSong(3); //autodetect
                    }
                }

                ImGui::TableNextColumn();
                ImGui::BeginDisabled(isPlaying());
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
            if (isPlaying() && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
                ImGui::SetTooltip("Cannot change while playing.");
            }

            ImGui::Separator();

            //---------------------- SongDisplay >>>>>>>>>>>>>>>>>>>

            // if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && !is_editing)
            if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && !ImGui::GetIO().WantTextInput/* && !mIsEditing*/)

            {
                bool lAltPressed = ImGui::IsKeyDown(ImGuiKey_LeftAlt) || ImGui::IsKeyDown(ImGuiKey_RightAlt);
                bool lShiftPressed = ImGui::IsKeyDown(ImGuiKey_LeftShift) || ImGui::IsKeyDown(ImGuiKey_RightShift);
                bool lCtrlPressed = ImGui::IsKeyDown(ImGuiKey_LeftCtrl) || ImGui::IsKeyDown(ImGuiKey_RightCtrl);
                // float lWheel = ImGui::GetIO().MouseWheel;




                if (lShiftPressed && mSelectionPivot == -1) {
                    mSelectionPivot = mSelectedRow;
                }

                mScrollToSelected = true;
                if (ImGui::IsKeyPressed(ImGuiKey_UpArrow, true) /*|| lWheel > 0*/)
                {
                    mSelectedRow = std::max(0, mSelectedRow - 1);
                    if (!lShiftPressed) resetSelection();
                }
                else
                if (ImGui::IsKeyPressed(ImGuiKey_DownArrow, true) /*|| lWheel < 0*/)
                {
                    mSelectedRow = std::min(static_cast<int>(mSongData.song_length), mSelectedRow + 1);
                    if (!lShiftPressed) resetSelection();
                }
                else
                if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow))  channelDown();
                else
                if (ImGui::IsKeyPressed(ImGuiKey_RightArrow)) channelUp();
                else
                if (ImGui::IsKeyPressed(ImGuiKey_PageUp))
                {
                    int newRow = std::max(0, mSelectedRow - 16);
                    mSelectedRow = newRow;
                    if (!lShiftPressed) resetSelection();
                }
                else
                if (ImGui::IsKeyPressed(ImGuiKey_PageDown))
                {
                    mSelectedRow = std::min(static_cast<int>(mSongData.song_length), mSelectedRow + 16);
                    if (!lShiftPressed) resetSelection();
                }
                else
                if (ImGui::IsKeyPressed(ImGuiKey_Home)) {
                    mSelectedRow = 0;
                    if (!lShiftPressed) resetSelection();
                }
                else
                if (ImGui::IsKeyPressed(ImGuiKey_End)) {
                    // Use your song_length or the absolute max row (999)
                    mSelectedRow = mSongData.song_length > 0 ? (mSongData.song_length - 1) : 0;
                    if (!lShiftPressed) resetSelection();
                } else {
                    mScrollToSelected = false;
                }



/*                if (ImGui::IsKeyPressed(ImGuiKey_Enter) || ImGui::IsKeyPressed(ImGuiKey_KeypadEnter)) {
                    mIsEditing = true;
                    mJustOpenThisFrame = true; // SET THIS FLAG

                    std::string current = mController->getNoteNameFromId(mSongData.song[mSelectedRow][mSelectedCol-1]);
                    snprintf(mEditBuffer, sizeof(mEditBuffer), "%s", current.c_str());
                } else */{


                    int lChannel = getCurrentChannel();
                    int16_t& current_note = mSongData.song[mSelectedRow][lChannel];
                    // Check for specific keys to "push" values immediately

                    // if (!mKeyboardMode)
                    {
                        if (!lCtrlPressed && lAltPressed)
                        {
                            if      (handleNoteInput(ImGuiKey_C, "C-", "C#")) {}
                            else if (handleNoteInput(ImGuiKey_D, "D-", "D#")) {}
                            else if (handleNoteInput(ImGuiKey_E, "E-", "F-")) {} // E# is usually F
                            else if (handleNoteInput(ImGuiKey_F, "F-", "F#")) {}
                            else if (handleNoteInput(ImGuiKey_G, "G-", "G#")) {}
                            else if (handleNoteInput(ImGuiKey_A, "A-", "A#")) {}
                            else if (handleNoteInput(ImGuiKey_B, "B-", "C-")) {} // B# is usually C (next octave)
                        }
                    }

                    // special without step
                    if (ImGui::IsKeyPressed(ImGuiKey_Space))  {
                        current_note = -1; // "===" Note Off
                    }
                    // else
                    if ( ImGui::IsKeyPressed(ImGuiKey_Delete))
                    {
                        if ( lCtrlPressed )
                        {
                            deleteSelected();
                        } else {
                            if (mSelectionPivot >= 0)
                                clearSelected();
                            else
                                current_note = 0;  // "..." Empty
                        }

                    }
                    if ( ImGui::IsKeyPressed( ImGuiKey_Insert ) )
                    {
                        insertEmpty();
                    }




                    if (ImGui::IsKeyPressed(ImGuiKey_KeypadAdd) || ImGui::IsKeyPressed(ImGuiKey_W) )
                    {
                        incOctave();
                    }

                    if (ImGui::IsKeyPressed(ImGuiKey_KeypadSubtract) || ImGui::IsKeyPressed(ImGuiKey_Q) )
                    {
                        decOctave();
                    }


                    // do this in menu!
                    // if (ImGui::IsKeyPressed(ImGuiKey_S))
                    //     lShiftPressed ? mController->decStepByChannel(lChannel) :  mController->incStepByChannel(lChannel);


                    // --- Handling Operations on Range ---


                    if (lCtrlPressed && ImGui::IsKeyPressed(ImGuiKey_C)) {
                        copySelected();

                    }
                    if (lCtrlPressed && ImGui::IsKeyPressed(ImGuiKey_V))
                    {
                      pasteSelected();
                    }
                    //FIXME Ctrl + x





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
                        bool lChannelIsActive = mController->getChannelActive(lChannel);

                        // 2. Move to the correct column
                        if (!ImGui::TableSetColumnIndex(j)) continue;

                        // 3. Submit a manual header with its label
                        static std::string lColCaption;
                        lColCaption = ImGui::TableGetColumnName(j);

                        if (j == 0) {

                        }
                        else if (!lChannelIsActive)
                            ImGui::PushStyleColor(ImGuiCol_Text, ImColor4F(cl_Gray));
                        else {
                            if (lChannel == getCurrentChannel() )
                                ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, Color4FIm(cl_Black));

                            ImGui::PushStyleColor(ImGuiCol_Text, ImColor4F(cl_Lime));
                        }

                        ImGui::TableHeader(lColCaption.c_str());

                        if (j > 0)
                            ImGui::PopStyleColor();

                        // ----  Header p menu and header click -----


                        if (ImGui::BeginPopupContextItem())
                        {
                            std::string lCaption;

                            if ( j == 0 )
                                lCaption = "All Channel";
                            else
                                lCaption = mController->GetChannelName(lChannel);


                            // fancy header
                            ImDrawList* draw_list = ImGui::GetWindowDrawList();
                            ImVec2 p0 = ImGui::GetCursorScreenPos();
                            ImVec2 p1 = ImVec2(p0.x + ImGui::GetContentRegionAvail().x, p0.y + ImGui::GetTextLineHeightWithSpacing());
                            draw_list->AddRectFilled(p0, p1, ImGui::GetColorU32(ImGuiCol_HeaderActive), 4.0f);
                            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2.0f);
                            ImGui::Indent(5.0f);
                            ImGui::Text("%s", lCaption.c_str());
                            ImGui::Unindent(5.0f);
                            //<<<

                            if ( j > 0)
                            {
                                ImGui::Separator();
                                ImGui::TextColored(Color4FIm(cl_AcidGreen), "%s", mController->GetInstrumentName(mSongData,lChannel).c_str());
                            }


                            ImGui::Separator();
                            if ( j == 0 ) {
                                if (ImGui::MenuItem("Activate all channel")) {
                                    mController->setAllChannelActive(true);
                                }
                                if (ImGui::MenuItem("Deactivate all channel")) {
                                    mController->setAllChannelActive(false);
                                }


                            } else {
                                ImGui::Separator();

                                if (ImGui::MenuItem("Toggle Active")) { mController->setChannelActive(lChannel, !lChannelIsActive); }
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
                            } // channels
                            ImGui::EndPopup();
                        }

                        //  DETECT THE left click
                        if (ImGui::IsItemClicked() && j > 0)
                        {
                            mController->setChannelActive(lChannel, !mController->getChannelActive(lChannel));
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
                    clipper.Begin(mSongData.song_length, 20.f);

                    clipper.IncludeItemByIndex(mSelectedRow);

                    while (clipper.Step())
                    {
                        for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++)
                        {
                            ImGui::TableNextRow(ImGuiTableRowFlags_None, 20.0f);
                            ImGui::PushID(row);

                            static Color4F lNoteColor;
                            lNoteColor = cl_White;

                            // ----- Draw Sequence Number -----
                            // need it after !
                            // ImGui::TableSetColumnIndex(0);
                            // ImGui::Text("%03d", i + 1);
                            //
                            // // Draw Channels
                            // for (int j = 1; j <= 9; j++) {
                            //     DrawNoteCell(i, j, mSongData.song[i][j-1], mController);
                            // }


                            // ----- Draw Selectiontion --------

                            if (isPlaying())
                            {
                                if (row == mCurrentPlayingRow)
                                {
                                    // Highlight row using RowBg0 (standard for active rows)
                                    ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, mPlayingRowColor);
                                    lNoteColor = cl_Black;

                                    if (mLiveMode &&  (mCurrentPlayingRow > clipper.DisplayEnd - 3)) {
                                        // ImGui::SetScrollHereY(0.5f); // Centers the playing row
                                        ImGui::SetScrollHereY(0.f); // to top
                                    }

                                }
                                // show selection on row 0 only
                                if (isRowSelected(row))
                                    ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, mSelectedRowColor, 0 );

                            }
                            else if (isRowSelected(row)) {

                                static bool sSelectedRow =  row == mSelectedRow;

                                if ( sSelectedRow || mSelectionPivot >= 0 )
                                {
                                    ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, mSelectedRowColor, 0 );
                                    for (int ch = FMS_MIN_CHANNEL; ch <= FMS_MAX_CHANNEL; ch++ )
                                        if (mController->getChannelActive(ch))
                                            ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, mSelectedRowColor, ch + 1 );
                                        else
                                            ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, mSelectedRowInactiveColor, ch + 1 );

                                } else {

                                    ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, mActiveRowColor);
                                }


                                if (!isPlaying() && mScrollToSelected && mSelectedRow > clipper.DisplayEnd - 3)
                                {
                                    ImGui::SetScrollHereY(0.5f);
                                    // ImGui::SetScrollHereY(0.0f);
                                }
                                // if (/*mScrollToSelected && */sSelectedRow)
                                // {
                                //     ImGui::SetScrollHereY(0.5f);
                                //     mScrollToSelected = false;
                                // }
                            }


                            // ----- Draw Sequence Number -----
                            ImGui::TableSetColumnIndex(0);
                            ImGui::Text("%03d", row + 1);

                            // Draw Channels
                            for (int col = 1; col <= 9; col++) {
                                DrawNoteCell(row, col, mSongData.song[row][col-1], mController, lNoteColor);
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
    //-----------------------------------------------------------------------------------------------------
    // Edit:
    //-----------------------------------------------------------------------------------------------------
    void copySelected(){
        mController->clearSong(mBufferSongData);
        mController->copySongRange(mSongData,getSelectionMin(), mBufferSongData, 0, getSelectionLen());
    }
    void pasteSelected(){
        mController->copySongRange(mBufferSongData, 0, mSongData, getSelectionMin(), mBufferSongData.song_length);
    }
    void clearSelected(){
        mController->clearSongRange(mSongData,  getSelectionMin(), getSelectionMax());
    }

    void deleteSelected() {
        mController->deleteSongRange(mSongData,  getSelectionMin(), getSelectionMax());
        resetSelection();
    }
    void insertEmpty() {
        if ( (mSelectedRow == mSongData.song_length) && (mSongData.song_length <= FMS_MAX_SONG_LENGTH) ) {
            mSongData.song_length ++ ;
            mSelectedRow ++;
        } else {
            mController->insertRowAt(mSongData, mSelectedRow);
        }
    }
    void incOctave()
    {
        mController->incOctaveByChannel(getCurrentChannel());
    }
    void decOctave()
    {
        mController->decOctaveByChannel(getCurrentChannel());
    }
    //--------------------------------------------------------------------------
    /*
     * Playmodes:
     *      0 (default)= With startat/endAt
     *      1 = only selection
     *      2 = full Song
     *      3 = autodetect
     */
    void playSong(U8 playMode = 0)
    {
        switch (playMode)
        {
            case 1: mController->playSong(mSongData, mLoop, getSelectionMin(), getSelectionMax());break;
            case 2: mController->playSong(mSongData, mLoop); break;
            case 3:
                if (getSelectionLen() > 1)
                    playSong(1);
                else
                    playSong(0);
                break;
            default: mController->playSong(mSongData, mLoop, mStartAt, mEndAt); break;
        }
    }

    //--------------------------------------------------------------------------

    int getCurrentChannel()
    {
        return  std::clamp(mSelectedCol - 1, FMS_MIN_CHANNEL, FMS_MAX_CHANNEL);
    }

    int setChannel( int lChannel, bool fireEvent = true)
    {
        lChannel = std::clamp(lChannel, FMS_MIN_CHANNEL, FMS_MAX_CHANNEL);
        if (mLastSetChannel == lChannel)
            return lChannel;

        mLastSetChannel = lChannel;
        mSelectedCol = lChannel + 1;


        //fire EVENT >>>
        if ( mController->mSyncInstrumentChannel && fireEvent)
        {
            SDL_Event event;
            SDL_zero(event);
            event.type = FLUX_EVENT_COMPOSER_OPL_CHANNEL_CHANGED;
            event.user.code = lChannel;
            SDL_PushEvent(&event);
        }

        return lChannel;
    }
    int channelUp( )
    {
        return setChannel(getCurrentChannel() + 1 );
    }
    int channelDown( )
    {
        return setChannel(getCurrentChannel() - 1 );
    }
    //--------------------------------------------------------------------------
    void onEvent(SDL_Event event)
    {
        if (event.type == FLUX_EVENT_INSTRUMENT_OPL_CHANNEL_CHANGED) {
            // dLog("Instrument changed channel to: %d",event.user.code);
            setChannel( event.user.code, false);
        }
        else
        if (event.type == FLUX_EVENT_INSTRUMENT_OPL_INSTRUMENT_NAME_CHANGED) {
            // event.user.code is the channel !
            mController->SetInstrumentName(mSongData
                    ,event.user.code
                    ,mController->getInstrumentNameFromCache(event.user.code).c_str()
                    );
            // dLog("Instrument name changed on channel %d", event.user.code);
        }

    }
    //--------------------------------------------------------------------------
    void onKeyEvent(SDL_KeyboardEvent event)
    {
        ImGuiWindow* window = ImGui::FindWindowByName("FM Song Composer");
        bool isFocused = (window && window == GImGui->NavWindow);
        if (!isFocused)
            return ;

        bool isKeyUp = (event.type == SDL_EVENT_KEY_UP);
        bool isAlt =  event.mod & SDLK_LALT || event.mod & SDLK_RALT;
        bool isCtrl =  event.mod & SDLK_LCTRL || event.mod & SDLK_RCTRL;

        /*
           Mapping:
                    d   f       h   j   k
              z   x   c   v   b   n   m   ,   .
              B-1 C   D   E   F   G   A   B   C+1
        */

        //FIXME mInsertMode ?

        if (/*mKeyboardMode && */!isAlt && !isCtrl && !isKeyUp)
        {
                 if (event.scancode == SDL_SCANCODE_Z)  insertTone("B-", -1);
            else if (event.scancode == SDL_SCANCODE_X)  insertTone("C-",  0);
            else if (event.scancode == SDL_SCANCODE_D)  insertTone("C#",  0);
            else if (event.scancode == SDL_SCANCODE_C)  insertTone("D-",  0);
            else if (event.scancode == SDL_SCANCODE_F)  insertTone("D#",  0);
            else if (event.scancode == SDL_SCANCODE_V)  insertTone("E-",  0);
            else if (event.scancode == SDL_SCANCODE_B)  insertTone("F-",  0);
            else if (event.scancode == SDL_SCANCODE_H)  insertTone("F#",  0);
            else if (event.scancode == SDL_SCANCODE_N)  insertTone("G-",  0);
            else if (event.scancode == SDL_SCANCODE_J)  insertTone("G#",  0);
            else if (event.scancode == SDL_SCANCODE_M)  insertTone("A-",  0);
            else if (event.scancode == SDL_SCANCODE_K)  insertTone("A#",  0);
            else if (event.scancode == SDL_SCANCODE_COMMA)  insertTone("B-",  0);
            else if (event.scancode == SDL_SCANCODE_PERIOD)  insertTone("C-",  1);
            // else if (event.scancode == SDL_SCANCODE_SPACE)  insertTone("===",  0);



            // Log("Scancode is: %d",event.scancode );
        } // no mods
    }

    bool saveSong(std::string filename)
    {
        return mController->saveSongFMS(filename, mSongData);
    }

    void resetSongSettings()
    {
        mStartAt = 0;
        mEndAt = -1;
        resetSelection();
        // maybe mLoop = false;
        mController->setAllChannelActive(true);

    }



    void newSong(bool resetInstruments = true) {
        mSongData.init();
        mSongData.song_delay = 15;
        mSongData.song_length = mNewSongLen;
        resetSongSettings();
        if (resetInstruments)
            mController->loadInstrumentPresetSyncSongName(mSongData);
    }
}; //class
