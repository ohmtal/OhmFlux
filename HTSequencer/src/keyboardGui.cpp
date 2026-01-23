#include "sequencerGui.h"
#include "sequencerMain.h"
#include <imgui_internal.h>

#include <algorithm>
#include <string>
#include <cctype>
//------------------------------------------------------------------------------
// FIXME
// - channel !!
// - show what i'am playing
// - save / restore : start/end octave
//------------------------------------------------------------------------------
uint8_t SequencerGui::getCurrentChannel(){
    // LogFMT("[warn] FIXME insertTone!! getCurrentChannel");
    return 6; //first 2OP channel FIXME

}
void SequencerGui::insertTone( uint8_t midiNote)  {
    LogFMT("[warn] FIXME insertTone!! {}", midiNote);
}


//------------------------------------------------------------------------------
namespace chords {

    static int selectedTypeIdx = 0; //chords

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
}


//------------------------------------------------------------------------------
void SequencerGui::RenderScalePlayerUI(bool standAlone) {
    if (standAlone) {
        ImGui::SetNextWindowSize(ImVec2(520, 450), ImGuiCond_FirstUseEver);
        if (!ImGui::Begin("Scale Player")) { ImGui::End(); return; }
    }

    uint8_t channel = getCurrentChannel();

    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "Scale Player");

    ImGui::SameLine();
    ImGui::SetNextItemWidth(200);
    ImGui::Combo("##Play Mode", &chords::selectedTypeIdx, chords::typeNames, IM_ARRAYSIZE(chords::typeNames));

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
                    if (chords::selectedTypeIdx == 0) {
                        // Single Note
                        SongStep step{midiNote, mCurrentInstrumentId};
                        getMain()->getController()->playNote(channel, step);
                    } else {
                        // Chord - using the pointer from our helper array
                        getMain()->getController()->playChord(channel, mCurrentInstrumentId, midiNote, *chords::chordOffsets[chords::selectedTypeIdx]);
                    }
                }

                if (ImGui::IsItemDeactivated()) {
                    // i play on last channels !
                    // Safety: Stop channels 1 through 4 to cover all possible notes in the chord
                    for (uint8_t ch = channel; ch < channel+4 ; ch++) {
                        if (ch < SOFTWARE_CHANNEL_COUNT)
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
//------------------------------------------------------------------------------
void SequencerGui::RenderPianoUI(bool standAlone )
{
    OPL3Controller*  controller = getMain()->getController();
    if (!controller)
        return;

    uint8_t channel = getCurrentChannel();

    if (standAlone) {
        ImGui::SetNextWindowSize(ImVec2(1100, 200), ImGuiCond_FirstUseEver);
        if (!ImGui::Begin("Piano")) { ImGui::End(); return; }
    }


    static int startOctave = 2;
    static int endOctave = 5;
    ImGui::AlignTextToFramePadding();
    ImGui::TextColored(ImColor4F(cl_Yellow),"Octaves");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(70);
    if (ImGui::InputInt("##startOctave", &startOctave))
        startOctave = std::clamp(startOctave, 0,6);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(70);
    if (ImGui::InputInt("##endOctave", &endOctave))
        endOctave = std::clamp(endOctave, startOctave,7);

    ImGui::SameLine();

    ImVec2 lButtonSize = ImVec2(80, 0);

    // ---- Header -----

    ImGui::SameLine();
    ImGui::TextColored(ImColor4F(cl_Green),"Play Mode:");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(200);
    ImGui::Combo("##Play Mode", &chords::selectedTypeIdx, chords::typeNames, IM_ARRAYSIZE(chords::typeNames));


    ImGui::BeginDisabled(!mInsertMode);
    ImGui::SameLine();
    if (ImGui::Button("add (===)", lButtonSize)) {
        insertTone(opl3::STOP_NOTE);
    }
    ImGui::SameLine();
    if (ImGui::Button("add (...)", lButtonSize)) {
        insertTone(opl3::NONE_NOTE);
    }
    // if (ImGui::Button("Add: ...", lButtonSize)) {}
    ImGui::EndDisabled();

    ImGui::SameLine();
    if (ImGui::Button("Silence all.", lButtonSize)) {
        controller->silenceAll(false);
    }



    // ---- Scale ------
    // int lScaleCount = 12 * 8;
    // // int lOctaveAdd = -1;
    // int  currentOctave = 0;

    struct PianoKey { const char* name; int offset; bool isBlack; };
    PianoKey keys[] = {
        {"C-", 0, false}, {"C#", 1, true}, {"D-", 2, false}, {"D#", 3, true},
        {"E-", 4, false},  {"F-", 5, false}, {"F#", 6, true}, {"G-", 7, false},
        {"G#", 8, true}, {"A-", 9, false}, {"A#", 10, true}, {"B-", 11, false}
    };



    ImVec2 startPos = ImGui::GetCursorScreenPos();
    float whiteWidth = 35.0f, whiteHeight = 90.0f;
    float blackWidth = 26.0f, blackHeight = 50.0f;

    // float whiteWidth = 30.0f, whiteHeight = 70.0f;
    // float blackWidth = 26.0f, blackHeight = 40.0f;

    ImVec4 lWhileColor = ImColor4F(cl_LightGray);
    // ImVec4 lWhileColorHoover = ImColor4F(cl_SkyBlue);
    if (mInsertMode) {
        lWhileColor = ImColor4F(cl_White);
    }

    // 1. Draw White Keys (Allowing Overlap)
    int whiteKeyCount = 0;

    // bool isNull = !controller->songValid(mCurrentSong);
    int id = 0;

    // if (isNull) ImGui::BeginDisabled();

    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
    // PASS 1: WHITE KEYS
    for (int octave = startOctave; octave <= endOctave; octave++) {
        for (int key = 0; key < 12; key++) {
            if (!keys[key].isBlack) {
                float xPos = startPos.x + (whiteKeyCount * whiteWidth);
                ImGui::SetCursorScreenPos(ImVec2(xPos, startPos.y));

                ImGui::PushID((octave * 12) + key);
                ImGui::SetNextItemAllowOverlap();
                ImGui::PushStyleColor(ImGuiCol_Button, lWhileColor);
                ImGui::Button("##white", ImVec2(whiteWidth, whiteHeight));

                // --- Note Logic for White Keys ---
                if (ImGui::IsItemActivated()) {
                    uint8_t midiNote = (octave * 12) + keys[key].offset + 12;
                    if (chords::selectedTypeIdx == 0) {
                        // Single Note
                        SongStep step{midiNote, mCurrentInstrumentId};
                        getMain()->getController()->playNote(channel, step);
                    } else {
                        // Chord - using the pointer from our helper array
                        getMain()->getController()->playChord(channel, mCurrentInstrumentId, midiNote, *chords::chordOffsets[chords::selectedTypeIdx]);
                    }
                }

                if (ImGui::IsItemDeactivated()) {
                    for (uint8_t ch = channel; ch < channel+4 ; ch++) {
                        if (ch < SOFTWARE_CHANNEL_COUNT)
                            getMain()->getController()->stopNote(ch);
                    }
                }

                if (key == 0)
                {
                    ImGui::SetCursorScreenPos(ImVec2(startPos.x + 5 + (whiteKeyCount * whiteWidth), startPos.y + whiteHeight - 20.f));
                    ImGui::TextColored(ImColor4F(cl_Black), "%s%d",keys[key].name,  octave);
                }


                ImGui::PopStyleColor(1);
                ImGui::PopID();
                whiteKeyCount++;
            }
        }
    }

    // PASS 2: BLACK KEYS
    whiteKeyCount = 0;
    for (int octave = startOctave; octave <= endOctave; octave++) {
        for (int key = 0; key < 12; key++) {
            if (keys[key].isBlack) {
                float xPos = startPos.x + (whiteKeyCount * whiteWidth) - (blackWidth / 2.0f);
                ImGui::SetCursorScreenPos(ImVec2(xPos, startPos.y));

                ImGui::PushID((octave * 12) + key);
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.05f, 0.05f, 0.05f, 1.0f));
                ImGui::Button("##black", ImVec2(blackWidth, blackHeight));

                // --- Note Logic for Black Keys ---
                if (ImGui::IsItemActivated()) {
                    uint8_t midiNote = (octave * 12) + keys[key].offset + 12;
                    if (chords::selectedTypeIdx == 0) {
                        // Single Note
                        SongStep step{midiNote, mCurrentInstrumentId};
                        getMain()->getController()->playNote(channel, step);
                    } else {
                        // Chord - using the pointer from our helper array
                        getMain()->getController()->playChord(channel, mCurrentInstrumentId, midiNote, *chords::chordOffsets[chords::selectedTypeIdx]);
                    }
                }

                if (ImGui::IsItemDeactivated()) {
                    for (uint8_t ch = channel; ch < channel+4 ; ch++) {
                        if (ch < SOFTWARE_CHANNEL_COUNT)
                            getMain()->getController()->stopNote(ch);
                    }
                }


                ImGui::PopStyleColor(1);
                ImGui::PopID();
            } else {
                whiteKeyCount++;
            }
        }
    }
    ImGui::PopStyleVar(1);   // Pop FrameBorderSize

    // if (isNull) ImGui::EndDisabled();

    // Correctly extend window boundary
    ImVec2 finalPos = ImVec2(startPos.x + (whiteKeyCount * whiteWidth), startPos.y + whiteHeight);
    ImGui::SetCursorScreenPos(finalPos);
    ImGui::Dummy(ImVec2(0, 10));


    if (standAlone) ImGui::End();
}

