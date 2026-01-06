//-----------------------------------------------------------------------------
// Copyright (c) 2026 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#pragma once

#include <core/fluxBaseObject.h>
#include <imgui.h>
#include <OplController.h>
#include "fluxEditorGlobals.h"

#ifdef __EMSCRIPTEN__
#ifdef __cplusplus
extern "C" {
    #endif
    // Forward declaration
    void emscripten_trigger_download(const char* name);
    #ifdef __cplusplus
}
#endif
#endif

// enum OPL_FILE_ACTION_TYPE :int {
//     fa_none     = 0,
//     fa_load     = 1,
//     fa_save     = 2,
//     fa_export   = 3
// };

class FluxFMEditor : public FluxBaseObject
{
private:
    OplController* mController = nullptr;
    uint8_t mInstrumentChannel = 0; // 0 .. FMS_MAX_CHANNEL
    bool mInstrumentEditorAutoPlay = true;
    bool mInstrumentEditorAutoPlayStarted = false;

public:
    ~FluxFMEditor() { Deinitialize();}

    bool Initialize() override
    {
        mController = new OplController();
        if (!mController || !mController->initController())
            return false;



        mController->loadInstrumentPreset();
        // mController->dumpInstrumentFromCache(mInstrumentChannel);

        return true;
    }
    void Deinitialize() override
    {
        if (mController)
            SAFE_DELETE(mController);
    }
    //--------------------------------------------------------------------------
    void resetInstrument()
    {
        mController->resetInstrument(mInstrumentChannel);
    }
    //--------------------------------------------------------------------------
    void DrawScalePlayer()
    {
        if (!mController)
            return;

        const char* noteNames[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
        // Offsets based on your table: C=null, C#=7, D=1, D#=8, E=2, F=3, F#=9, G=4, G#=10, A=5, A#=11, B=6
        const int noteOffsets[] = { 0, 7, 1, 8, 2, 3, 9, 4, 10, 5, 11, 6 };

        ImGui::Begin("FM Full Scale");

        if (ImGui::BeginTable("ScaleTable", 13, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchSame))
        {
            ImGui::TableSetupColumn("Octave", ImGuiTableColumnFlags_WidthFixed, 60.0f);
            for (int i = 0; i < 12; i++) ImGui::TableSetupColumn(noteNames[i]);
            ImGui::TableHeadersRow();

            for (int oct = 0; oct < 8; oct++)
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("Oct %s",
                            (oct == 0) ? "I" : (oct == 1) ? "II" : (oct == 2) ? "III" :
                            (oct == 3) ? "IV" : (oct == 4) ? "V" : (oct == 5) ? "VI" :
                            (oct == 6) ? "VII" : "VIII");

                for (int noteIdx = 0; noteIdx < 12; noteIdx++)
                {
                    ImGui::TableSetColumnIndex(noteIdx + 1);

                    int noteIndex = (oct * 12) + noteOffsets[noteIdx];

                    // Handle the "//" null markers from your table
                    bool isNull = (oct == 0 && noteIdx == 0) || (oct == 7 && noteIdx > 0);

                    if (isNull)
                    {
                        ImGui::TextDisabled("//");
                    }
                    else
                    {
                        char label[8];
                        snprintf(label, sizeof(label), "%d", noteIndex);

                        // Style black keys (sharps)
                        bool isSharp = strchr(noteNames[noteIdx], '#') != nullptr;
                        if (isSharp) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.15f, 0.15f, 1.0f));

                        // Create the button
                        ImGui::Button(label, ImVec2(-FLT_MIN, 0.0f));

                        if (ImGui::IsItemActivated()) {
                            mController->playNoteDOS(mInstrumentChannel, noteIndex);
                        }
                        if (ImGui::IsItemDeactivated()) {
                            mController->stopNote(mInstrumentChannel);
                        }

                        if (isSharp) ImGui::PopStyleColor();
                    }
                }
            }
            ImGui::EndTable();
        }

        ImGui::Separator();
        if (ImGui::Button("Stop Channel", ImVec2(-FLT_MIN, 0))) {
            mController->stopNote(mInstrumentChannel);
        }

        ImGui::End();
    }
    //--------------------------------------------------------------------------
    void DrawPianoScale()
    {
        static int currentOctave = 3;
        const char* octaves[] = { "I", "II", "III", "IV", "V", "VI", "VII", "VIII" };

        ImGui::Begin("Test Instrument Piano");

        // Octave Selection
        ImGui::SetNextItemWidth(120);
        if (ImGui::BeginCombo("Octave", octaves[currentOctave])) {
            for (int i = 0; i < 8; i++) {
                if (ImGui::Selectable(octaves[i], currentOctave == i)) currentOctave = i;
            }
            ImGui::EndCombo();
        }

        struct PianoKey { const char* name; int offset; bool isBlack; };
        PianoKey keys[] = {
            {"C", 0, false}, {"C#", 7, true}, {"D", 1, false}, {"D#", 8, true},
            {"E", 2, false},  {"F", 3, false}, {"F#", 9, true}, {"G", 4, false},
            {"G#", 10, true}, {"A", 5, false}, {"A#", 11, true}, {"B", 6, false}
        };

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

            if (ImGui::IsItemActivated()) mController->playNoteDOS(mInstrumentChannel, (currentOctave * 12) + keys[i].offset);
            if (ImGui::IsItemDeactivated()) mController->stopNote(mInstrumentChannel);

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

            if (ImGui::IsItemActivated()) mController->playNoteDOS(mInstrumentChannel, (currentOctave * 12) + keys[i].offset);
            if (ImGui::IsItemDeactivated()) mController->stopNote(mInstrumentChannel);

            if (isNull) ImGui::EndDisabled();
            ImGui::PopStyleColor(2);
            ImGui::PopID();
        }

        // Correctly extend window boundary
        ImVec2 finalPos = ImVec2(startPos.x + (whiteKeyCount * whiteWidth), startPos.y + whiteHeight);
        ImGui::SetCursorScreenPos(finalPos);
        ImGui::Dummy(ImVec2(0, 10));

        ImGui::End();
    }
    //--------------------------------------------------------------------------
    void DrawInstrumentEditor()
    {
        // 1. Get the current data from the controller to fill our local editor
        // We use a static or persistent buffer so the UI is responsive
        static uint8_t editBuffer[24];
        static int lastChannel = -1;

        // If we switched channels, load the new instrument data into our buffer
        // if (mInstrumentChannel != lastChannel)
        {
            const uint8_t* currentIns = mController->getInstrument(mInstrumentChannel);
            if (currentIns) {
                std::copy(currentIns, currentIns + 24, editBuffer);
            }
            lastChannel = mInstrumentChannel;
        }

        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar; //<<< For menubar !!
        ImGui::SetNextWindowSizeConstraints(ImVec2(400.0f, 400.0f), ImVec2(FLT_MAX, FLT_MAX));
        if (ImGui::Begin("FM Instrument Editor", nullptr, window_flags))
        {

            bool isAnySliderActive = false; // Track if any slider is being dragged


            // 2. Use BeginMenuBar instead of just BeginMenu
            if (ImGui::BeginMenuBar())
            {
                if (ImGui::BeginMenu("File"))
                {
                    if (ImGui::MenuItem("Open")) { POPUP_NOT_IMPEMENTED_ACTIVE = true; }
                    if (ImGui::MenuItem("Save")) { POPUP_NOT_IMPEMENTED_ACTIVE = true; }
                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("Options"))
                {
                    ImGui::MenuItem("Autoplay C-4", "", &mInstrumentEditorAutoPlay);

                    // --- NEW CHANNEL SUB-MENU ---
                    if (ImGui::BeginMenu("Select Channel"))
                    {
                        for (uint8_t i = 0; i < 9; i++)
                        {
                            char label[32];
                            snprintf(label, sizeof(label), "Channel %d", i + 1); // Display 1-9 for users

                            // MenuItem returns true when clicked
                            // The third parameter (bool selected) shows the checkmark
                            if (ImGui::MenuItem(label, nullptr, mInstrumentChannel == i))
                            {
                                mInstrumentChannel = i;
                                // Optional: Reset the local buffer to the new channel's data immediately
                                lastChannel = -1;
                            }
                        }
                        ImGui::EndMenu();
                    }
                    // ----------------------------
                    ImGui::Separator();
                    if (ImGui::MenuItem("Reset")) { resetInstrument(); }
                    ImGui::EndMenu();
                }
                //-------------- channel select combo and volume
                // Add spacing for combo and volume
                float rightOffset = 230.0f;
                ImGui::SameLine(ImGui::GetWindowWidth() - rightOffset);

                ImGui::SetNextItemWidth(100);
                float currentVol = mController->getVolume();
                if (ImGui::SliderFloat("##FMVol", &currentVol, 0.0f, 2.0f, "Vol %.1f"))
                {
                    mController->setVolume(currentVol);
                }
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("FM Volume");
                ImGui::SameLine(); // Place the combo box immediately to the right of the slider

                // 3. Add Channel Selection Combo
                ImGui::SetNextItemWidth(120);
                if (ImGui::BeginCombo("##ChannelSelect", mController->GetChannelName(mInstrumentChannel), ImGuiComboFlags_HeightLarge))
                {
                    for (int i = 0; i <= FMS_MAX_CHANNEL; i++)
                    {
                        bool is_selected = (mInstrumentChannel == i);
                        if (ImGui::Selectable(mController->GetChannelName(i), is_selected)) {
                            mInstrumentChannel = i;
                        }
                        if (is_selected) ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }
                //------------------

                ImGui::EndMenuBar(); // 3. EndMenuBar, not EndMenu
            }


            if (ImGui::BeginTable("Params", 4, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV))
            {
                // Setup columns: [Mod Label] [Mod Slider] [Car Label] [Car Slider]
                ImGui::TableSetupColumn("Mod Label", ImGuiTableColumnFlags_WidthFixed, 130.0f);
                ImGui::TableSetupColumn("Mod Slider", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("Car Label", ImGuiTableColumnFlags_WidthFixed, 130.0f);
                ImGui::TableSetupColumn("Car Slider", ImGuiTableColumnFlags_WidthStretch);

                // Iterate 2 parameters at a time (total 24 params = 12 rows)
                for (size_t i = 0; i < OplController::INSTRUMENT_METADATA.size(); i += 2)
                {
                    ImGui::TableNextRow();

                    // --- Column 0 & 1: Modulator ---
                    {
                        const auto& meta = OplController::INSTRUMENT_METADATA[i];
                        ImGui::TableSetColumnIndex(0);
                        ImGui::TextUnformatted(meta.name.c_str());

                        ImGui::TableSetColumnIndex(1);
                        ImGui::PushID(static_cast<int>(i));
                        uint8_t minV = 0;

                        // Fixed width for sliders to keep the middle of the table aligned
                        ImGui::SetNextItemWidth(-FLT_MIN);
                        if (ImGui::SliderScalar("##v", ImGuiDataType_U8, &editBuffer[i], &minV, &meta.maxValue, "%u")) {
                            mController->setInstrument(mInstrumentChannel, editBuffer);
                        }

                        if (ImGui::IsItemActive()) isAnySliderActive = true;
                        if (ImGui::IsItemDeactivated()) {
                            mController->stopNote(mInstrumentChannel);
                            mInstrumentEditorAutoPlayStarted = false;
                        }
                        ImGui::PopID();
                    }

                    // --- Column 2 & 3: Carrier ---
                    if (i + 1 < OplController::INSTRUMENT_METADATA.size())
                    {
                        const auto& meta = OplController::INSTRUMENT_METADATA[i + 1];
                        ImGui::TableSetColumnIndex(2);
                        ImGui::TextUnformatted(meta.name.c_str());

                        ImGui::TableSetColumnIndex(3);
                        ImGui::PushID(static_cast<int>(i + 1));
                        uint8_t minV = 0;

                        ImGui::SetNextItemWidth(-FLT_MIN);
                        if (ImGui::SliderScalar("##v", ImGuiDataType_U8, &editBuffer[i + 1], &minV, &meta.maxValue, "%u")) {
                            mController->setInstrument(mInstrumentChannel, editBuffer);
                        }

                        if (ImGui::IsItemActive()) isAnySliderActive = true;
                        if (ImGui::IsItemDeactivated()) {
                            mController->stopNote(mInstrumentChannel);
                            mInstrumentEditorAutoPlayStarted = false;
                        }
                        ImGui::PopID();
                    }
                }
                ImGui::EndTable();
            }

            // if (ImGui::BeginTable("Params", 2, ImGuiTableFlags_RowBg))
            // {
            //     for (size_t i = 0; i < OplController::INSTRUMENT_METADATA.size(); ++i)
            //     {
            //         const auto& meta = OplController::INSTRUMENT_METADATA[i];
            //         ImGui::TableNextRow();
            //         ImGui::TableSetColumnIndex(0);
            //         ImGui::TextUnformatted(meta.name.c_str());
            //
            //         ImGui::TableSetColumnIndex(1);
            //         ImGui::PushID(i);
            //
            //         if (meta.maxValue == 1) {
            //             bool val = (editBuffer[i] != 0);
            //             if (ImGui::Checkbox("##v", &val)) {
            //                 editBuffer[i] = val ? 1 : 0;
            //                 mController->setInstrument(mInstrumentChannel, editBuffer);
            //             }
            //         } else { // slider >>
            //             ImGui::PushID(i);
            //             uint8_t minV = 0;
            //
            //             // 1. Draw the slider
            //             if (ImGui::SliderScalar("##v", ImGuiDataType_U8, &editBuffer[i], &minV, &meta.maxValue, "%u")) {
            //                 mController->setInstrument(mInstrumentChannel, editBuffer);
            //             }
            //
            //             if (ImGui::IsItemActive()) {
            //                 isAnySliderActive = true;
            //             }
            //
            //             if (ImGui::IsItemDeactivated()) {
            //                 mController->stopNote(mInstrumentChannel);
            //                 mInstrumentEditorAutoPlayStarted = false;
            //             }
            //             ImGui::PopID();
            //         } //<<< slider
            //         ImGui::PopID();
            //     }
            //     ImGui::EndTable();
            // }


            if (mInstrumentEditorAutoPlay && isAnySliderActive && !mInstrumentEditorAutoPlayStarted)
            {
                mController->playNoteDOS(mInstrumentChannel, mController->getIdFromNoteName("C-4"));
                mInstrumentEditorAutoPlayStarted = true;
            }

        }
        ImGui::End();
    }

    //--------------------------------------------------------------------------
    void DrawComposer()
    {
        if (!mController)
            return ;

        ImGui::SetNextWindowSizeConstraints(ImVec2(800.0f, 400.f), ImVec2(FLT_MAX, FLT_MAX));
        ImGui::Begin("FM Composer");

        ImGui::End();
    }




};
