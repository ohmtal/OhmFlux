#include "sequencerGui.h"
#include "sequencerMain.h"
#include <gui/ImFileDialog.h>
#include <imgui_internal.h>
#include <utils/fluxSettingsManager.h>

#include <algorithm>
#include <string>
#include <cctype>

//------------------------------------------------------------------------------
void SequencerGui::ShowSoundBankWindow()
{
    if (!mGuiSettings.mShowSoundBankEditor) return;
    ImGui::SetNextWindowSize(ImVec2(600, 600), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Sound Bank", &mGuiSettings.mShowSoundBankEditor))
    {
        ImGui::End();
        return;
    }

    if (ImGui::BeginPopupContextItem())
    {
        if (ImGui::MenuItem("Close"))
            mGuiSettings.mShowSoundBankEditor = false;
        ImGui::EndPopup();
    }

    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 12.0f);


    // Outer table with 2 columns
    if (ImGui::BeginTable("InstrumentLayoutTable", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerV)) {

        // --- COLUMN 1: Sidebar (List) ---
        ImGui::TableSetupColumn("Sidebar", ImGuiTableColumnFlags_WidthFixed, 150.0f);
        // --- COLUMN 2: Content (Editor & Scale Player) ---
        ImGui::TableSetupColumn("Content", ImGuiTableColumnFlags_WidthStretch);

        ImGui::TableNextRow();

        // Left Side
        ImGui::TableSetColumnIndex(0);
        if (ImGui::BeginChild("ListRegion")) {
            RenderInstrumentListUI();
        }
        ImGui::EndChild();

        // Right Side
        ImGui::TableSetColumnIndex(1);
        if (ImGui::BeginChild("ContentRegion")) {

            // Top: Instrument Editor
            RenderInstrumentEditorUI();

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            // Bottom: Scale Player
            // RenderScalePlayerUI();
        }
        ImGui::EndChild();

        ImGui::EndTable();
    }

    // RenderScalePlayerUI(true);

    ImGui::PopStyleVar();

    ImGui::End();

}
//------------------------------------------------------------------------------
void SequencerGui::RenderInstrumentListUI(bool standAlone)
{
    if (standAlone)
    {
        ImGui::SetNextWindowSize(ImVec2(200, 600), ImGuiCond_FirstUseEver);
        ImGui::Begin("Instruments");
    }

    // CHANGED: Use ImVec2(0, 0) so it fills the Table Column/Sidebar width and height
    if (ImGui::BeginChild("BC_Box", ImVec2(0, 0), ImGuiChildFlags_Borders)) {
        std::vector<opl3::OplInstrument>& bank = getMain()->getController()->mSoundBank;

        // Use -FLT_MIN to ensure the ListBox fills the entire child area
        if (ImGui::BeginListBox("##InstList", ImVec2(-FLT_MIN, -FLT_MIN))) {
            for (int n = 0; n < (int)bank.size(); n++) {
                const bool is_selected = (mCurrentInstrumentId == n);

                // std::format is great for 2026!
                std::string instrumentCaption = std::format("{}##{}", bank[n].name, n);

                if (ImGui::Selectable(instrumentCaption.c_str(), is_selected)) {
                    mCurrentInstrumentId = n;
                }

                if (ImGui::IsItemHovered()) {

                    if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                        Log("Using Instrument %d", mCurrentInstrumentId);
                        myTestSong = mOpl3Tests->createScaleSong(mCurrentInstrumentId);
                        getMain()->getController()->playSong(myTestSong);
                    }

                    ImGui::SetTooltip("#%d %s", n, instrumentCaption.c_str());
                }

                if (is_selected) ImGui::SetItemDefaultFocus();
            }
            ImGui::EndListBox();
        }
    }
    ImGui::EndChild();

    if (standAlone) ImGui::End();
}
//------------------------------------------------------------------------------
void SequencerGui::RenderScalePlayerUI(bool standAlone) {
    if (standAlone) {
        ImGui::SetNextWindowSize(ImVec2(520, 450), ImGuiCond_FirstUseEver);
        if (!ImGui::Begin("Scale Player")) { ImGui::End(); return; }
    }

    // --- SINGLE COMBO BOX FOR MODE ---
    static int selectedTypeIdx = 0;
    const char* typeNames[] = {
        "Single Note", "Chord: Major", "Chord: Minor",
        "Chord: Augmented", "Chord: Diminished", "Chord: Major 7", "Chord: Minor 7"
    };

    // Helper map to match the combo index to the offset vectors
    // Index 0 is nullptr because it's handled as a single note
    const std::vector<int>* chordOffsets[] = {
        nullptr,
        &opl3::CHORD_MAJOR, &opl3::CHORD_MINOR, &opl3::CHORD_AUGMENTED,
        &opl3::CHORD_DIMINISHED, &opl3::CHORD_MAJOR_7, &opl3::CHORD_MINOR_7
    };

    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "Scale Player");

    ImGui::SameLine();
    ImGui::SetNextItemWidth(200);
    ImGui::Combo("##Play Mode", &selectedTypeIdx, typeNames, IM_ARRAYSIZE(typeNames));

    // --- MIDI GRID ---
    static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit;

    if (ImGui::BeginTable("MidiGrid", 13, flags)) {
        ImGui::TableSetupColumn("Oct", ImGuiTableColumnFlags_WidthFixed, 40.0f);
        const char* noteNames[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
        for (int i = 0; i < 12; i++) {
            ImGui::TableSetupColumn(noteNames[i], ImGuiTableColumnFlags_WidthFixed, 25.0f);
        }
        ImGui::TableHeadersRow();

        for (int octave = 0; octave <= 7; octave++) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);

            // Roman Numerals for Octaves
            const char* octLabels[] = { "0", "I", "II", "III", "IV", "V", "VI", "VII" };
            ImGui::Text("%s", octLabels[octave]);

            for (uint8_t n = 0; n < 12; n++) {
                uint8_t midiNote = (octave + 1) * 12 + n;
                if (midiNote > 127) break;

                ImGui::TableSetColumnIndex(n + 1);
                bool isSharp = (n == 1 || n == 3 || n == 6 || n == 8 || n == 10);

                ImGui::PushID(midiNote);

                // Styling
                if (isSharp) {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                } else {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
                }

                ImGui::Button(std::to_string(midiNote).c_str(), ImVec2(-FLT_MIN, 0.0f));
                ImGui::PopStyleColor(2);

                // --- LOGIC ---
                if (ImGui::IsItemActivated()) {
                    if (selectedTypeIdx == 0) {
                        // Single Note
                        SongStep step{midiNote, mCurrentInstrumentId};
                        getMain()->getController()->playNote(MAX_CHANNELS - 1, step);
                    } else {
                        // Chord - using the pointer from our helper array
                        getMain()->getController()->playChord(mCurrentInstrumentId, midiNote, *chordOffsets[selectedTypeIdx]);
                    }
                }

                if (ImGui::IsItemDeactivated()) {
                    // i play on last channels !
                    // Safety: Stop channels 1 through 4 to cover all possible notes in the chord
                    for (uint8_t ch = MAX_CHANNELS - 5; ch < MAX_CHANNELS; ch++) {
                        getMain()->getController()->stopNote(ch);
                    }
                }

                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("%s%d (MIDI %d)", noteNames[n], octave, midiNote);
                }
                ImGui::PopID();
            }
        }
        ImGui::EndTable();

        ImGui::Separator();
        if (ImGui::Button("Silence all (Panic)", ImVec2(-FLT_MIN, 35))) {
            getMain()->getController()->silenceAll(false);
        }
    }
    if (standAlone) ImGui::End();
}


// void SequencerGui::RenderScalePlayerUI(bool standAlone) {
//
//
//     if (standAlone)
//     {
//         ImGui::SetNextWindowSize(ImVec2(480, 300)); //, ImGuiCond_FirstUseEver);
//         ImGui::Begin("Scale Player");
//     }
//
//
//     ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "Scale Player");
//
//     // 13 columns: 1 for Octave Label + 12 for Notes
//     static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit;
//
//     if (ImGui::BeginTable("MidiGrid", 13, flags)) {
//         // --- Header Row ---
//         ImGui::TableSetupColumn("Oct", ImGuiTableColumnFlags_WidthFixed, 40.0f);
//         const char* noteNames[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
//         for (int i = 0; i < 12; i++) {
//             ImGui::TableSetupColumn(noteNames[i], ImGuiTableColumnFlags_WidthFixed, 25.0f);
//         }
//         ImGui::TableHeadersRow();
//
//         // --- Data Rows (Octaves -1 to 9) ---
//         // for (int octave = -1; octave <= 9; octave++) {
//         for (int octave = 0; octave <= 7; octave++) {
//             ImGui::TableNextRow();
//
//             // Column 0: Octave Label
//             ImGui::TableSetColumnIndex(0);
//             // ImGui::Text("%d", octave);
//             ImGui::Text("%s",
//                         (octave == -1) ? "-" : (octave == 0) ? "0" : (octave == 1) ? "I" :
//                         (octave == 2) ? "II" :(octave == 3) ? "III" :
//                         (octave == 4) ? "IV" : (octave == 5) ? "V" : (octave == 6) ? "VI" :
//                         (octave == 7) ? "VII" : "VIII");
//
//             // Columns 1-12: MIDI Note Numbers
//             for (uint8_t n = 0; n < 12; n++) {
//                 uint8_t midiNote = (octave + 1) * 12 + n;
//                 if (midiNote > 127) break; // MIDI cap
//
//                 ImGui::TableSetColumnIndex(n + 1);
//
//                 // Check if it's a sharp note (C#, D#, F#, G#, A#)
//                 bool isSharp = (n == 1 || n == 3 || n == 6 || n == 8 || n == 10);
//
//                 ImGui::PushID(midiNote);
//
//
//                 if (isSharp)
//                 {
//                     // Sharp: Black button with White text
//                     ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
//                     ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
//                 }
//                 else
//                 {
//                     // Natural: White button with Black text
//                     ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
//                     ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
//                 }
//
//                 ImGui::Button(std::to_string(midiNote).c_str(), ImVec2(-FLT_MIN, 0.0f));
//
//                 // Pop both the Button color and the Text color
//                 ImGui::PopStyleColor(2);
//
//
//                 if (ImGui::IsItemActivated()) {
//                     SongStep step{midiNote,mCurrentInstrumentId};
//                     getMain()->getController()->playNote(1,step); //FIXME CHANNEL ?!
//                 }
//                 if (ImGui::IsItemDeactivated()) {
//                     getMain()->getController()->stopNote(1); //FIXME CHANNEL ?!
//                 }
//
//
//
//                 // Tooltip to show Note Name + Octave on hover
//                 if (ImGui::IsItemHovered()) {
//                     ImGui::SetTooltip("%s%d (MIDI %d)", noteNames[n], octave, midiNote);
//                 }
//                 ImGui::PopID();
//             } //for ...
//         }
//         ImGui::EndTable();
//
//         ImGui::Separator();
//         if (ImGui::Button("Silence all.", ImVec2(-FLT_MIN, 0))) {
//             getMain()->getController()->silenceAll(false);
//         }
//
//     }
//     if (standAlone) ImGui::End();
// }
//------------------------------------------------------------------------------

void SequencerGui::RenderOpParam(const opl3::ParamMeta& meta, opl3::OplInstrument::OpPair::OpParams& op, int metaIdx) {
    static const uint8_t zero = 0; // Create a zero constant for the min pointer

    uint8_t* val = nullptr;

    // Mapping metadata index to struct members
    switch (metaIdx) {
        case 0:  val = &op.multi;   break;
        case 1:  val = &op.tl;      break;
        case 2:  val = &op.attack;  break;
        case 3:  val = &op.decay;   break;
        case 4:  val = &op.sustain; break;
        case 5:  val = &op.release; break;
        case 6:  val = &op.wave;    break;
        case 7:  val = &op.ksr;     break;
        case 8:  val = &op.egTyp;   break;
        case 9:  val = &op.vib;     break;
        case 10: val = &op.am;      break;
        case 11: val = &op.ksl;     break;
    }

    if (val) {
        // Create a unique ID scope for this specific slider/checkbox
        // This combines the column (Mod/Car) and the row (Freq/Attack/etc)
        ImGui::PushID(metaIdx);

        ImGui::SetNextItemWidth(-FLT_MIN);

        if (meta.maxValue == 1) {
            bool bVal = (*val != 0);
            // We can now use a simple label because PushID handles the uniqueness
            if (ImGui::Checkbox("##value", &bVal)) {
                *val = bVal ? 1 : 0;
            }
        } else {
            static const uint8_t zero = 0;
            // Use the address of the maxValue from the metadata
            ImGui::SliderScalar("##value", ImGuiDataType_U8, val, &zero, &meta.maxValue, "%d");
        }

        ImGui::PopID();
    }
}
//------------------------------------------------------------------------------
void SequencerGui::RenderInstrumentEditorUI(bool standAlone) {

    if (getMain()->getController()->mSoundBank.size() < mCurrentInstrumentId+1)
    {
        Log("[error] Current Instrument out of bounds!");
        mCurrentInstrumentId = 0;
        return;
    }

    if (standAlone)
    {
        ImGui::SetNextWindowSize(ImVec2(200, 600), ImGuiCond_FirstUseEver);
        ImGui::Begin("Instrument Frequenz Modulatuion");
    }


    OplInstrument& inst = getMain()->getController()->mSoundBank[mCurrentInstrumentId];

    ImGui::PushID("OPL_Editor");

    // Define the ranges as variables of the same type (int8_t)
    static const int8_t minFine = -128, maxFine = 127;
    static const int8_t minOff = -24, maxOff = 24;
    static const uint8_t minFeed = 0, maxFeed = 7;
    static const int8_t minFixed = 0, maxFixed = 127;



    // --- Header Section ---



    char nameBuf[64];
    strncpy(nameBuf, inst.name.c_str(), sizeof(nameBuf));
    ImGui::Text("Instrument Name");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(200);
    if (ImGui::InputText("##Instrument Name", nameBuf, sizeof(nameBuf))) {
        inst.name = nameBuf;
    }

    ImGui::SameLine();
    ImGui::Checkbox("4-Op Mode", &inst.isFourOp);
    ImGui::SameLine();
    ImGui::Checkbox("Double Voice", &inst.isDoubleVoice);

    if (ImGui::TreeNodeEx("Global Tuning", ImGuiTreeNodeFlags_DefaultOpen)) {
        // 3 columns for your 3 sliders
        if (ImGui::BeginTable("SliderTable", 3))
        {
            // --- ROW 1: Labels ---
            ImGui::TableNextRow();

            ImGui::TableSetColumnIndex(0);
            ImGui::TextUnformatted("Fine Tune");

            ImGui::TableSetColumnIndex(1);
            ImGui::TextUnformatted("Note Offset");

            ImGui::TableSetColumnIndex(2);
            ImGui::TextUnformatted("Fixed Note");

            // --- ROW 2: Sliders ---
            ImGui::TableNextRow();

            ImGui::TableSetColumnIndex(0);
            ImGui::SetNextItemWidth(-FLT_MIN); // -FLT_MIN tells the slider to use all available column width
            ImGui::SliderScalar("##FineTune", ImGuiDataType_S8, &inst.fineTune, &minFine, &maxFine);

            ImGui::TableSetColumnIndex(1);
            ImGui::SetNextItemWidth(-FLT_MIN);
            ImGui::SliderScalar("##NoteOffset", ImGuiDataType_S8, &inst.noteOffset, &minOff, &maxOff);

            ImGui::TableSetColumnIndex(2);
            ImGui::SetNextItemWidth(-FLT_MIN);
            ImGui::SliderScalar("##FixedNote", ImGuiDataType_S8, &inst.fixedNote, &minFixed, &maxFixed);

            ImGui::EndTable();
        }

        ImGui::TreePop();
    }

    ImGui::Separator();

    // --- Operator Pairs Section ---
    int numPairs = (inst.isFourOp  || inst.isDoubleVoice) ? 2 : 1;
    for (int p = 0; p < numPairs; p++) {
        ImGui::PushID(p);
        auto& pair = inst.pairs[p];

        if (ImGui::CollapsingHeader(p == 0 ? "Operator Pair 1 (Primary)" : "Operator Pair 2 (Secondary)", ImGuiTreeNodeFlags_DefaultOpen)) {

            // 3 Columns, fixed widths as requested (150, 150, and remaining)
            if (ImGui::BeginTable("pair_cfg", 3))
            {
                // --- ROW 1: CAPTIONS ---
                ImGui::TableNextRow();

                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted("Feedback");

                ImGui::TableSetColumnIndex(1);
                ImGui::TextUnformatted("Connection");

                ImGui::TableSetColumnIndex(2);
                ImGui::TextUnformatted("Panning");

                // --- ROW 2: WIDGETS ---
                ImGui::TableNextRow();

                // Feedback Slider
                ImGui::TableSetColumnIndex(0);
                ImGui::SetNextItemWidth(-FLT_MIN);
                ImGui::SliderScalar("##Feedback", ImGuiDataType_U8, &pair.feedback, &minFeed, &maxFeed);

                // Connection Combo
                ImGui::TableSetColumnIndex(1);
                ImGui::SetNextItemWidth(-FLT_MIN);
                const char* connTypes[] = { "FM (Serial)", "Additive (Parallel)" };
                int connIdx = (int)pair.connection;
                if (ImGui::Combo("##Connection", &connIdx, connTypes, 2)) {
                    pair.connection = (uint8_t)connIdx;
                }

                // Panning Combo
                ImGui::TableSetColumnIndex(2);
                ImGui::SetNextItemWidth(-FLT_MIN);
                const char* panTypes[] = { "Mute", "Left", "Right", "Center" };
                int panIdx = (int)pair.panning;
                if (ImGui::Combo("##Panning", &panIdx, panTypes, 4)) {
                    pair.panning = (uint8_t)panIdx;
                }

                ImGui::EndTable();
            }

            ImGui::Columns(1);

            ImGui::Spacing();

            // --- The Operator Slider Grid ---
            // Columns: [Parameter Label] | [Modulator Slider] | [Carrier Slider]
            ImGui::Columns(3, "op_grid", true);
            ImGui::SetColumnWidth(0, 120); // Label column

            // Headers
            ImGui::NextColumn();
            ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "MODULATOR (Op 0)");
            ImGui::NextColumn();
            ImGui::TextColored(ImVec4(1.0f, 0.7f, 0.4f, 1.0f), "CARRIER (Op 1)");
            ImGui::Separator();

            // Iterate through metadata to generate rows
            for (size_t i = 0; i < OPL_OP_METADATA.size(); i++) {
                const auto& meta = OPL_OP_METADATA[i];

                ImGui::NextColumn();
                ImGui::AlignTextToFramePadding();
                ImGui::TextUnformatted(meta.name.c_str());

                // Modulator Column
                ImGui::NextColumn();
                ImGui::PushID(0); // Modulator index
                RenderOpParam(meta, pair.ops[0], i);
                ImGui::PopID();

                // Carrier Column
                ImGui::NextColumn();
                ImGui::PushID(1); // Carrier index
                RenderOpParam(meta, pair.ops[1], i);
                ImGui::PopID();
            }
            ImGui::Columns(1);
        }
        ImGui::PopID();
        ImGui::Spacing();
    }
    ImGui::PopID();
    if (standAlone) ImGui::End();
}
