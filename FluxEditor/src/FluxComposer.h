//-----------------------------------------------------------------------------
// Copyright (c) 2026 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#pragma once

#include <core/fluxBaseObject.h>
#include <imgui.h>
#include <OplController.h>
#include "fluxEditorGlobals.h"

class FluxComposer : public FluxBaseObject
{
private:
    OplController* mController = nullptr;
    uint8_t mCurrentChannel = 0; // 0-8 for 9 OPL channels

    // Placeholder for song data (e.g., patterns, notes)
    // For now, let's just have a simple structure, perhaps a 2D array representing notes over time
    // This is a very basic placeholder and would need significant design for a real composer.
    static const int MAX_PATTERNS = 16;
    static const int NOTES_PER_PATTERN = 64; // Example: 64 steps per pattern
    // A simple representation: channel, pattern, step -> note_id, duration, instrument_id
    // This will likely become much more complex, but for a starting point...
    // For now, let's keep it simpler and just focus on the UI for channels.


    OplController::SongData mSongData;
    uint16_t mSeqCounter = 1; // Current sequence/row in the song (1-indexed)
    uint16_t mAktSpur = 1;    // Current active track/channel (1-indexed)
public:

    FluxComposer(OplController* lController)
    {
        mController = lController;
    }
    ~FluxComposer() { Deinitialize(); }

    bool Initialize() override
    {
        // Assuming OplController is already initialized elsewhere or can be initialized here.
        // For now, let's just create it if it's not passed in.
        // In a real application, the OplController might be shared.
        if (!mController)
        {
            mController = new OplController();
            if (!mController || !mController->initController())
                return false;
        }

        // Initialize SongData
        mSongData.song_speed = 15; // Default from Pascal code
        mSongData.song_length = 0;
        for (int i = 0; i < 1001; ++i) {
            for (int j = 0; j < 10; ++j) {
                mSongData.song[i][j] = 0;
            }
        }

        return true;
    }

    void Deinitialize() override
    {
        if (mController)
            SAFE_DELETE(mController);
    }

    // Pass an existing OplController if it's shared across editors
    void SetOplController(OplController* controller)
    {
        mController = controller;
    }

    void DrawComposer()
    {
        if (!mController)
            return;

        ImGui::SetNextWindowSizeConstraints(ImVec2(800.0f, 600.0f), ImVec2(FLT_MAX, FLT_MAX));
        if (ImGui::Begin("FM Song Composer"))
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
                ImGui::EndMenuBar();
            }

            // Song Controls (Length, Speed)
            ImGui::SetNextItemWidth(120);
            ImGui::InputScalar("Song Length", ImGuiDataType_U16, &mSongData.song_length, nullptr, nullptr, "%u");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(120);
            ImGui::InputScalar("Song Speed", ImGuiDataType_U8, &mSongData.song_speed, nullptr, nullptr, "%u");

            // Channel Selection
            ImGui::Text("Current Channel: %d", mCurrentChannel + 1); // Display 1-9
            ImGui::SameLine();
            ImGui::SetNextItemWidth(200);
            if (ImGui::BeginCombo("##ChannelSelectComposer", mController->GetChannelName(mCurrentChannel), ImGuiComboFlags_HeightLarge))
            {
                for (int i = 0; i < 9; i++) // OPL has 9 channels
                {
                    bool is_selected = (mCurrentChannel == i);
                    if (ImGui::Selectable(mController->GetChannelName(i), is_selected))
                    {
                        mCurrentChannel = i;
                    }
                    if (is_selected)
                        ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }

            ImGui::Separator();

            ImGui::Separator();

            // Song Editing Area: Sequence and Channel Navigation, and the main song data table
            ImGui::Text("Current Sequence: %d", mSeqCounter); // Display 1-indexed
            ImGui::SameLine();
            if (ImGui::SmallButton("<- Seq")) { if (mSeqCounter > 1) mSeqCounter--; }
            ImGui::SameLine();
            if (ImGui::SmallButton("Seq ->")) { if (mSeqCounter < 1000) mSeqCounter++; }
            ImGui::SameLine();
            ImGui::Text("Active Track: %d", mAktSpur); // Display 1-indexed
            ImGui::SameLine();
            if (ImGui::SmallButton("<- Track")) { if (mAktSpur > 1) mAktSpur--; }
            ImGui::SameLine();
            if (ImGui::SmallButton("Track ->")) { if (mAktSpur < 9) mAktSpur++; }

            ImGui::Dummy(ImVec2(0, 10)); // Some vertical space

            // Song Data Table (Replicating schreibneu functionality)
            if (ImGui::BeginChild("##SongDataTable", ImVec2(0, -ImGui::GetTextLineHeightWithSpacing()), ImGuiChildFlags_Borders, ImGuiWindowFlags_AlwaysVerticalScrollbar))
            {
                if (ImGui::BeginTable("SongTable", 10, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_ScrollY))
                {
                    ImGui::TableSetupColumn("Seq", ImGuiTableColumnFlags_WidthFixed, 60.0f);
                    for (int j = 0; j < 9; ++j) {
                        char col_name[8];
                        snprintf(col_name, sizeof(col_name), "S%d", j + 1);
                        ImGui::TableSetupColumn(col_name);
                    }
                    ImGui::TableHeadersRow();

                    int start_seq = mSeqCounter - 5; // Display 10 rows, centered around mSeqCounter
                    if (start_seq < 1) start_seq = 1;
                    int end_seq = start_seq + 9;     // Display 10 rows
                    if (end_seq > 1000) {             // Adjust if we go past the end
                        end_seq = 1000;
                        start_seq = end_seq - 9;
                        if (start_seq < 1) start_seq = 1; // Ensure start is not less than 1
                    }

                    for (int i = start_seq; i <= end_seq; ++i)
                    {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text("%03d", i);

                        for (int j = 0; j < 9; ++j)
                        {
                            ImGui::TableSetColumnIndex(j + 1);
                            // Highlight current active track
                            if (j + 1 == mAktSpur) {
                                ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.2f, 0.2f, 0.4f, 1.0f));
                            }

                            char id_buf[32];
                            snprintf(id_buf, sizeof(id_buf), "##seq%dchannel%d", i, j);

                            int16_t note_value = mSongData.song[i][j + 1]; // +1 because Pascal arrays are 1-indexed

                            ImGui::SetNextItemWidth(-FLT_MIN);
                            if (ImGui::InputScalar(id_buf, ImGuiDataType_S16, &note_value))
                            {
                                // Update song data if value changed
                                mSongData.song[i][j + 1] = note_value;
                                // Update song_length if we are editing past current length
                                if (i > mSongData.song_length) {
                                    mSongData.song_length = i;
                                }
                            }

                            // Color coding based on note value
                            // case song[anfangsseq,j] of
                            // -1:outcolor:=magenta;
                            //  0:outcolor:=lightblue;
                            //	else outcolor:=yellow;
                            // end;
                            if (ImGui::IsItemHovered() && !ImGui::IsItemActive())
                            {
                                ImVec4 color;
                                if (note_value == -1) color = ImVec4(1.0f, 0.0f, 1.0f, 1.0f); // Magenta
                                else if (note_value == 0) color = ImVec4(0.5f, 0.5f, 1.0f, 1.0f); // Light Blue
                                else color = ImVec4(1.0f, 1.0f, 0.0f, 1.0f); // Yellow
                                ImGui::SetTooltip("Note: %d", note_value);
                                ImGui::GetWindowDrawList()->AddRectFilled(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), ImGui::GetColorU32(color));
                            }

                            if (j + 1 == mAktSpur) {
                                ImGui::PopStyleColor();
                            }
                        }
                    }
                    ImGui::EndTable();
                }
            }
            ImGui::EndChild();

            ImGui::Separator();

            // Example: Play/Stop controls for the current channel
            if (ImGui::Button("Play Channel"))
            {
                // For demonstration, play a C-4 note on the current channel
                mController->playNoteDOS(mCurrentChannel, mController->getIdFromNoteName("C-4"));
            }
            ImGui::SameLine();
            if (ImGui::Button("Stop Channel"))
            {
                mController->stopNote(mCurrentChannel);
            }        }
        ImGui::End();
    }
};
