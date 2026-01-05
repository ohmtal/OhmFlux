//-----------------------------------------------------------------------------
// Copyright (c) 2026 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#pragma once

#include <core/fluxBaseObject.h>
#include <imgui.h>
#include <OplController.h>

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

public:
    ~FluxFMEditor() { Deinitialize();}

    bool Initialize() override
    {
        mController = new OplController();
        if (!mController || !mController->initController())
            return false;

        mController->setInstrument(mInstrumentChannel, mController->GetDefaultInstrument().data());

        mController->dumpInstrumentFromCache(mInstrumentChannel);

        return true;
    }
    void Deinitialize() override
    {
        if (mController)
            SAFE_DELETE(mController);
    }
    //--------------------------------------------------------------------------
    void DrawScalePlayer()
    {
        if (!mController)
            return;

        const char* noteNames[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
        // Offsets based on your table: C=null, C#=7, D=1, D#=8, E=2, F=3, F#=9, G=4, G#=10, A=5, A#=11, B=6
        const int noteOffsets[] = { 0, 7, 1, 8, 2, 3, 9, 4, 10, 5, 11, 6 };

        ImGui::Begin("OPL Scale Player");

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
        if (ImGui::Button("Panic: Stop Channel", ImVec2(-FLT_MIN, 0))) {
            mController->silenceAll();
        }

        ImGui::End();
    }
    //--------------------------------------------------------------------------
    // void DrawPianoScale()
    // {
    //     static int currentOctave = 3; // Default to Octave IV (index 3)
    //     const char* octaves[] = { "I", "II", "III", "IV", "V", "VI", "VII", "VIII" };
    //
    //     ImGui::Begin("OPL Piano Editor");
    //
    //     // 1. Octave Selection
    //     ImGui::SetNextItemWidth(120);
    //     if (ImGui::BeginCombo("Octave", octaves[currentOctave])) {
    //         for (int i = 0; i < 8; i++) {
    //             if (ImGui::Selectable(octaves[i], currentOctave == i)) currentOctave = i;
    //         }
    //         ImGui::EndCombo();
    //     }
    //
    //     ImGui::Spacing();
    //
    //     // 2. Keyboard Layout Logic
    //     // Offsets: C=null, C#=7, D=1, D#=8, E=2, F=3, F#=9, G=4, G#=10, A=5, A#=11, B=6
    //     struct PianoKey { const char* name; int offset; bool isBlack; };
    //     PianoKey keys[] = {
    //         {"C", -1, false}, {"C#", 7, true}, {"D", 1, false}, {"D#", 8, true},
    //         {"E", 2, false},  {"F", 3, false}, {"F#", 9, true}, {"G", 4, false},
    //         {"G#", 10, true}, {"A", 5, false}, {"A#", 11, true}, {"B", 6, false}
    //     };
    //
    //     ImVec2 startPos = ImGui::GetCursorScreenPos();
    //     float whiteWidth = 40.0f;
    //     float whiteHeight = 150.0f;
    //     float blackWidth = 25.0f;
    //     float blackHeight = 90.0f;
    //
    //     // Draw White Keys First
    //     int whiteKeyCount = 0;
    //     for (int i = 0; i < 12; i++) {
    //         if (keys[i].isBlack) continue;
    //
    //         int noteIndex = (currentOctave * 12) + keys[i].offset;
    //         bool isNull = (currentOctave == 0 && i == 0); // Octave I, C is //
    //
    //         ImGui::PushID(i);
    //         ImGui::SetCursorScreenPos(ImVec2(startPos.x + (whiteKeyCount * whiteWidth), startPos.y));
    //
    //         if (isNull) ImGui::BeginDisabled();
    //         if (ImGui::Button("##white", ImVec2(whiteWidth, whiteHeight))) {}
    //
    //         // Use the same activation logic from your earlier scale
    //         if (ImGui::IsItemActivated()) mController->playNoteDOS(mInstrumentChannel, noteIndex);
    //         if (ImGui::IsItemDeactivated()) mController->stopNote(mInstrumentChannel);
    //         if (isNull) ImGui::EndDisabled();
    //
    //         ImGui::PopID();
    //         whiteKeyCount++;
    //     }
    //
    //     // Draw Black Keys (Overlapping)
    //     whiteKeyCount = 0;
    //     for (int i = 0; i < 12; i++) {
    //         if (!keys[i].isBlack) {
    //             whiteKeyCount++;
    //             continue;
    //         }
    //
    //         int noteIndex = (currentOctave * 12) + keys[i].offset;
    //         bool isNull = (currentOctave == 7); // Octave VIII sharps are //
    //
    //         // Position black keys between white keys
    //         float xPos = startPos.x + (whiteKeyCount * whiteWidth) - (blackWidth / 2.0f);
    //         ImGui::SetCursorScreenPos(ImVec2(xPos, startPos.y));
    //
    //         ImGui::PushID(i + 100);
    //         ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
    //
    //         if (isNull) ImGui::BeginDisabled();
    //         if (ImGui::Button("##black", ImVec2(blackWidth, blackHeight))) {}
    //
    //         if (ImGui::IsItemActivated()) mController->playNoteDOS(mInstrumentChannel, noteIndex);
    //         if (ImGui::IsItemDeactivated()) mController->stopNote(mInstrumentChannel);
    //         if (isNull) ImGui::EndDisabled();
    //
    //         ImGui::PopStyleColor();
    //         ImGui::PopID();
    //     } // black keys
    //
    //     // Reset cursor to end of piano block
    //     // Calculate the final "bottom-right" corner of your keyboard
    //     ImVec2 finalPos = ImVec2(startPos.x + (whiteKeyCount * whiteWidth), startPos.y + whiteHeight);
    //     // Instead of just setting the cursor, submit a dummy to extend the boundaries
    //     ImGui::SetCursorScreenPos(finalPos);
    //     ImGui::Dummy(ImVec2(0, 10)); // Extends boundary and adds 10px padding at the bottom
    //
    //     ImGui::End();
    // }
    void DrawPianoScale()
    {
        static int currentOctave = 3;
        const char* octaves[] = { "I", "II", "III", "IV", "V", "VI", "VII", "VIII" };

        ImGui::Begin("OPL Piano Editor");

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
    // void DrawInstrumentEditor()
    // {
    //     if (!mController)
    //         return ;
    //
    //     uint8_t* instrumentData =  mController->getInstrumentMutable( mInstrumentChannel );
    //
    //     ImGui::SetNextWindowSizeConstraints(ImVec2(450.0f, 400.0f), ImVec2(FLT_MAX, FLT_MAX));
    //
    //     if (ImGui::Begin("FM Instrument Editor"))
    //     {
    //
    //         if (ImGui::BeginCombo("Select Instrument", ("Instrument " + std::to_string(mInstrumentChannel)).c_str())) {
    //             for (int n = 0; n <= FMS_MAX_CHANNEL; n++) {
    //                 const bool is_selected = (mInstrumentChannel == n);
    //                 if (ImGui::Selectable(("Instrument " + std::to_string(n)).c_str(), is_selected))
    //                     mInstrumentChannel = n;
    //                 if (is_selected)
    //                     ImGui::SetItemDefaultFocus();
    //             }
    //             ImGui::EndCombo();
    //         }
    //         // 2-column table: Parameter name | Control widget
    //         if (ImGui::BeginTable("InstrumentParams", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg))
    //         {
    //             ImGui::TableSetupColumn("Parameter", ImGuiTableColumnFlags_WidthFixed, 180.0f);
    //             ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);
    //             ImGui::TableHeadersRow();
    //
    //             for (size_t i = 0; i < OplController::INSTRUMENT_METADATA.size(); ++i)
    //             {
    //                 const auto& meta = OplController::INSTRUMENT_METADATA[i];
    //
    //                 ImGui::TableNextRow();
    //                 ImGui::TableSetColumnIndex(0);
    //                 ImGui::TextUnformatted(meta.name.c_str());
    //
    //                 ImGui::TableSetColumnIndex(1);
    //                 ImGui::PushID(static_cast<int>(i)); // Prevent ID collisions for duplicate names
    //
    //                 // OPTIMIZATION: Check if the parameter is a binary flag (0 or 1)
    //                 if (meta.maxValue == 1)
    //                 {
    //                     bool isChecked = (instrumentData[i] != 0);
    //                     if (ImGui::Checkbox("##v", &isChecked))
    //                     {
    //                         instrumentData[i] = isChecked ? 1 : 0;
    //                     }
    //                 }
    //                 else
    //                 {
    //                     // Use SliderScalar for uint8_t data (0-maxValue)
    //                     uint8_t minVal = 0;
    //                     ImGui::SliderScalar("##v", ImGuiDataType_U8, &instrumentData[i], &minVal, &meta.maxValue, "%u");
    //                 }
    //
    //                 ImGui::PopID();
    //             }
    //             ImGui::EndTable();
    //         }
    //     }
    //     ImGui::End();
    // }
    void DrawInstrumentEditor()
    {
        // 1. Get the current data from the controller to fill our local editor
        // We use a static or persistent buffer so the UI is responsive
        static uint8_t editBuffer[24];
        static int lastChannel = -1;

        // If we switched channels, load the new instrument data into our buffer
        if (mInstrumentChannel != lastChannel) {
            const uint8_t* currentIns = mController->getInstrument(mInstrumentChannel);
            if (currentIns) {
                std::copy(currentIns, currentIns + 24, editBuffer);
            }
            lastChannel = mInstrumentChannel;
        }

        ImGui::SetNextWindowSizeConstraints(ImVec2(400.0f, 400.0f), ImVec2(FLT_MAX, FLT_MAX));
        if (ImGui::Begin("FM Instrument Editor"))
        {
            bool changed = false;

            if (ImGui::BeginTable("Params", 2, ImGuiTableFlags_RowBg))
            {
                for (size_t i = 0; i < OplController::INSTRUMENT_METADATA.size(); ++i)
                {
                    const auto& meta = OplController::INSTRUMENT_METADATA[i];
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted(meta.name.c_str());

                    ImGui::TableSetColumnIndex(1);
                    ImGui::PushID(i);

                    if (meta.maxValue == 1) {
                        bool val = (editBuffer[i] != 0);
                        if (ImGui::Checkbox("##v", &val)) {
                            editBuffer[i] = val ? 1 : 0;
                            changed = true; // Mark for update
                        }
                    } else {
                        uint8_t minV = 0;
                        if (ImGui::SliderScalar("##v", ImGuiDataType_U8, &editBuffer[i], &minV, &meta.maxValue, "%u")) {
                            changed = true; // Mark for update
                        }
                    }
                    ImGui::PopID();
                }
                ImGui::EndTable();
            }

            // 2. If any slider or checkbox was moved, send the WHOLE buffer to the OPL chip
            if (changed) {
                mController->setInstrument(mInstrumentChannel, editBuffer);
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
