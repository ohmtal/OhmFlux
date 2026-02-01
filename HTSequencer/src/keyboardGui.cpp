#include "sequencerGui.h"
#include "sequencerMain.h"
#include <imgui_internal.h>


#include <algorithm>
#include <string>
#include <cctype>



//------------------------------------------------------------------------------
// FIXME
// - move to header ...
// - channel !!
// - save / restore : start/end octave
//------------------------------------------------------------------------------

static int mCurrentStartOctave = 3;
int guiChannelToNote[12] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };
uint64_t mChannelLastUsed[12] = { 0 };
uint64_t mGlobalCounter = 0;
uint8_t getCurrentStartOctave() { return mCurrentStartOctave;}
int mScancodeToChannel[SDL_SCANCODE_COUNT]; //FIXME INIT with -1 !!!

const char* channelNames[] = {
    "Channel  1 (OP4)",
    "Channel  2 (OP4)",
    "Channel  3 (OP4)",
    "Channel  4 (OP4)",
    "Channel  5 (OP4)",
    "Channel  6 (OP4)",
    "Channel  7",
    "Channel  8",
    "Channel  9",
    "Channel 10",
    "Channel 11",
    "Channel 12",
};


int /*SequencerGui::*/getPianoMapOffset(SDL_Scancode scancode) {
    switch (scancode) {

        // --- Lower Octave (Physical Bottom Row) ---
        case SDL_SCANCODE_Z:        return 0;  // C
        case SDL_SCANCODE_S:        return 1;  // C#
        case SDL_SCANCODE_X:        return 2;  // D
        case SDL_SCANCODE_D:        return 3;  // D#
        case SDL_SCANCODE_C:        return 4;  // E
        case SDL_SCANCODE_V:        return 5;  // F
        case SDL_SCANCODE_G:        return 6;  // F#
        case SDL_SCANCODE_B:        return 7;  // G
        case SDL_SCANCODE_H:        return 8;  // G#
        case SDL_SCANCODE_N:        return 9;  // A
        case SDL_SCANCODE_J:        return 10; // A#
        case SDL_SCANCODE_M:        return 11; // B
        case SDL_SCANCODE_COMMA:    return 12; // C (Next Octave)

        // --- Upper Octave (Physical Middle Row) ---
        case SDL_SCANCODE_Q:        return 12; // C
        case SDL_SCANCODE_2:        return 13; // C#
        case SDL_SCANCODE_W:        return 14; // D
        case SDL_SCANCODE_3:        return 15; // D#
        case SDL_SCANCODE_E:        return 16; // E
        case SDL_SCANCODE_R:        return 17; // F
        case SDL_SCANCODE_5:        return 18; // F#
        case SDL_SCANCODE_T:        return 19; // G
        case SDL_SCANCODE_6:        return 20; // G#
        case SDL_SCANCODE_Y:        return 21; // A (Physical 'Y' key position)
        case SDL_SCANCODE_7:        return 22; // A#
        case SDL_SCANCODE_U:        return 23; // B
        case SDL_SCANCODE_I:        return 24; // C (Next Octave)

        default: return -1;
    }
}
//------------------------------------------------------------------------------
uint8_t SequencerGui::getCurrentChannel(){


    if ( getCurrentPattern() )
        return mPatternEditorState.cursorCol;

    // maybe a other ?!
    return 0;
}
//------------------------------------------------------------------------------
bool SequencerGui::setCurrentChannel(uint8_t channel){
    if (channel >= SOFTWARE_CHANNEL_COUNT)
        return false;

    if ( !getCurrentPattern() )
        return false;

    mPatternEditorState.cursorCol = channel;

    return true;
}
//------------------------------------------------------------------------------
bool SequencerGui::stopPlayedNotes( )
{
    for (uint8_t i = 0; i < SOFTWARE_CHANNEL_COUNT; i++)
    {
        if (guiChannelToNote[i] >= 0)
            stopNote(i);
    }
    return true;
}


bool SequencerGui::stopNote(uint8_t softwareChannel)
{
    if (softwareChannel >= SOFTWARE_CHANNEL_COUNT)
        return false;

    guiChannelToNote[softwareChannel] = -1;

    // PatternEditorState& state = mPatternEditorState;

    // only when playing!
    if ( mSettings.InsertMode  && isPlaying() ) {
        Pattern* lPat = this->getCurrentPattern();
        if (lPat)
        {
            uint16_t row =  getPlayingRow();
            SongStep& tmpStep = lPat->getStep(row, softwareChannel);
            tmpStep.note = STOP_NOTE;
        } else {
            dLog("unable to get current pattern while mInsertMode is active, that can be ok!");
            return false;
        }
    }

    return getMain()->getController()->stopNote(softwareChannel);
}
//------------------------------------------------------------------------------
int moveSchedId = -1;

bool SequencerGui::playNote(uint8_t softwareChannel,  SongStep step)
{
    if (softwareChannel >= SOFTWARE_CHANNEL_COUNT)
        return false;

    if (step.note < LAST_NOTE)
        guiChannelToNote[softwareChannel] = step.note;

    PatternEditorState& state = this->mPatternEditorState;



    if ( mSettings.InsertMode && state.visible ) {
        Pattern* lPat = this->getCurrentPattern();
        if (lPat)
        {
            // lPat.getStep(state.cursorRow, state.cursorCol).note = note;
            // i need this when i play cords!
            // FIXME EFFECTS or simple setStep!

            uint16_t row =  isPlaying() ? getPlayingRow() : state.cursorRow;

            SongStep& tmpStep = lPat->getStep(row, softwareChannel);

            dLog("set step row:%d, col:%d, instrument:%d note:%d",
                 state.cursorRow, softwareChannel, step.instrument, step.note);


            tmpStep.instrument = step.instrument;
            tmpStep.note = step.note;


            //Lol when playing cords this is really funny
            //FIXME but how ? i can schedule it but which step to take since i
            //      have it for each channel


            if (!isPlaying())
            {
                if (!FluxSchedule.isPending(moveSchedId)) {
                    int stepval = mCurrentSong.getStepByChannel(softwareChannel);
                    moveSchedId = FluxSchedule.add(1.0, nullptr, [&state, stepval]() mutable {
                        state.moveCursorPosition(stepval, 0);
                    });
                    dLog("Schedule step %d", stepval);
                } else {
                    dLog("Schedule is pending ...");
                }
            }

            // state.moveCursorPosition(mCurrentSong.getStepByChannel(softwareChannel), 0);



        } else {
            dLog("unable to get current pattern while mInsertMode is active, that can be ok!");
            // mInsertMode = false;
            return false;

        }
    }

    return getMain()->getController()->playNote(softwareChannel, step);
}
//------------------------------------------------------------------------------


void SequencerGui::onKeyEventKeyBoard(SDL_KeyboardEvent event) {
    // Ignore OS key repeats to prevent re-triggering FM envelopes
    if (event.repeat) return;



    ImGuiIO& io = ImGui::GetIO();
    if (io.WantTextInput /*FIXME console does ? || io.WantCaptureKeyboard*/) {
        return;
    }
    if (ImGui::IsAnyItemActive()) {
        return;
    }



    // NOTE: global input
    if (event.down /*&& event.mod & SDL_KMOD_CTRL*/) {
        // --- OCTAVE CONTROL ---
        // plus minus mapped keys
        if (event.key == SDLK_PLUS) {
            if (mCurrentStartOctave < 8) mCurrentStartOctave++;
            return;
        }
        if (event.key == SDLK_MINUS) {
            if (mCurrentStartOctave > 0) mCurrentStartOctave--;
            return;
        }

        // F8 INSERT MODE
        if (event.key == SDLK_F8) {
            mSettings.InsertMode = !mSettings.InsertMode;
            return;
        }


    }



    // Exit if any of these functional modifiers are pressed
    const SDL_Keymod activeMods = SDL_KMOD_SHIFT | SDL_KMOD_CTRL | SDL_KMOD_ALT | SDL_KMOD_GUI;
    if (event.mod & activeMods) {
        return;
    }

    // --- 2. MUSICAL MAPPING ---
    // Convert physical key location to a semi-tone offset (0-24)
    int offset = getPianoMapOffset(event.scancode);
    if (offset == -1) return; // Not a musical key


    if (event.down) {
        // --- NOTE ON ---

        // Calculate MIDI note based on the octave at the moment of pressing
        uint8_t midiNote = (mCurrentStartOctave * 12) + offset +  12;

        // Step A: Find an available software channel (0-11)

        int targetSwChan = -1;
        for (int i = getCurrentChannel(); i < SOFTWARE_CHANNEL_COUNT; ++i) {
            if (guiChannelToNote[i] == -1) {
                targetSwChan = i;
                break;
            }
        }

        // Step B: VOICE STEALING
        // If all 12 channels are busy, find the one used longest ago
        if (targetSwChan == -1) {
            uint64_t oldest = UINT64_MAX;
            for (int i = 0; i < 12; ++i) {
                if (mChannelLastUsed[i] < oldest) {
                    oldest = mChannelLastUsed[i];
                    targetSwChan = i;
                }
            }
            // Stop the oldest note immediately to free the channel
            this->stopNote(targetSwChan);
        }

        // Step C: TRACKING & EXECUTION
        // Map the physical scancode to the channel so NoteOff works correctly
        // even if the octave is changed while the key is held.
        mScancodeToChannel[event.scancode] = targetSwChan;
        // mChannelToNote[targetSwChan] = midiNote;
        mChannelLastUsed[targetSwChan] = ++mGlobalCounter;


        SongStep step{midiNote, mCurrentInstrumentId};
        this->playNote(targetSwChan, step);
    }
    else {
        // --- NOTE OFF ---

        // Retrieve which software channel was assigned to this physical key
        int chan = mScancodeToChannel[event.scancode];
        if (chan != -1) {
            // Tell the OPL3 controller to enter the Release phase for this slot
            this->stopNote(chan);

            // Mark the tracking slots as free/empty
            // mChannelToNote[chan] = -1;
            mScancodeToChannel[event.scancode] = -1;
        }
    }
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
                        getMain()->getController()->playNote(channel, step); //scale player

                    } else {
                        // Chord - using the pointer from our helper array
                        getMain()->getController()->playChord(channel, mCurrentInstrumentId, midiNote, *chords::chordOffsets[chords::selectedTypeIdx]);
                    }
                }

                if (ImGui::IsItemDeactivated()) {
                    // i play on last channels !
                    getMain()->getController()->stopPlayedNotes();//scale player
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
void SequencerGui::RenderPianoUI(bool standAlone)
{


    OPL3Controller* controller = getMain()->getController();
    if (!controller) return;

    uint8_t channel = getCurrentChannel();

    if (standAlone) {
        ImGui::SetNextWindowSize(ImVec2(1100, 250), ImGuiCond_FirstUseEver);
        if (!ImGui::Begin("Piano")) { ImGui::End(); return; }
    }


    // --- Octave Range  ---
    static int visibleOctaves = 5;

    static float lHeaderWidth = 0.f;
    static float lPianoWidth = 0.f;

    float lWindowWidth = ImGui::GetContentRegionAvail().x;


    // from dry run
    if (lWindowWidth > lHeaderWidth) ImGui::SetCursorPosX((lWindowWidth - lHeaderWidth) * 0.5f);


    ImGui::BeginGroup(); // top controls
    ImFlux::LCDNumber((float)mCurrentStartOctave, 1, 0, 12.0f, Color4FIm(cl_Yellow));
    ImGui::SameLine();
    ImFlux::MiniKnobInt("Octaves", &mCurrentStartOctave, 0,7, 12.f );

    ImFlux::SeparatorVertical(1.f);

    ImFlux::LCDNumber((float)visibleOctaves, 1, 0, 12.0f, Color4FIm(cl_Yellow));
    ImGui::SameLine();
    ImFlux::MiniKnobInt("maximum visible Octaves", &visibleOctaves, 1,7, 12.f );
    ImGui::SameLine();


    ImFlux::SeparatorVertical(1.f);

    ImGui::SameLine();
    ImFlux::LEDCheckBox("Insert Mode", &mSettings.InsertMode,mPatternEditorState.visible ? Color4FIm(cl_Lime) : Color4FIm(cl_Blue));

    ImGui::SameLine();
    ImGui::SetNextItemWidth(150.f);
    Widget_InstrumentCombo(mCurrentInstrumentId, getMain()->getController()->getSoundBank());

    ImGui::SameLine();
    int channelIdx = getCurrentChannel();
    if (ImFlux::ValueStepper("##Channel", &channelIdx, channelNames, IM_ARRAYSIZE(channelNames))) {
        setCurrentChannel(channelIdx);
    }

    ImGui::SameLine();
    ImFlux::LCDDisplay("pattern row", (float)mPatternEditorState.cursorRow, 3,0,16.f);

    ImGui::EndGroup(); // top controls

    lHeaderWidth = ImGui::GetItemRectSize().x;



    //-----------

    if (lWindowWidth > lPianoWidth) ImGui::SetCursorPosX((lWindowWidth - lPianoWidth) * 0.5f);

    ImGui::BeginGroup(); //Piano

    int endOctave = std::min(7, (int)mCurrentStartOctave + visibleOctaves - 1);

    // ... [Keep Header/PlayMode code same] ...

    struct PianoKey { const char* name; int offset; bool isBlack; };
    PianoKey keys[] = {
        {"C-", 0, false}, {"C#", 1, true}, {"D-", 2, false}, {"D#", 3, true},
        {"E-", 4, false}, {"F-", 5, false}, {"F#", 6, true}, {"G-", 7, false},
        {"G#", 8, true}, {"A-", 9, false}, {"A#", 10, true}, {"B-", 11, false}
    };

    ImVec2 startPos = ImGui::GetCursorScreenPos();
    float whiteWidth = 35.0f, whiteHeight = 90.0f;
    float blackWidth = 26.0f, blackHeight = 50.0f;

    auto checkNoteActive = [&](int midiNote) -> bool {
        for (int i = 0; i < 12; i++) {
            if (guiChannelToNote[i] == midiNote) return true;
        }
        return false;
    };

    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);

    // --- PASS 1: WHITE KEYS ---
    int whiteKeyCount = 0;
    for (int octave = mCurrentStartOctave; octave <= endOctave; octave++) {
        for (int key = 0; key < 12; key++) {
            if (keys[key].isBlack) continue;

            uint8_t midiNote = (octave * 12) + keys[key].offset  + 12;
            bool isActive = checkNoteActive(midiNote);

            float xPos = startPos.x + (whiteKeyCount * whiteWidth);
            ImGui::SetCursorScreenPos(ImVec2(xPos, startPos.y));

            ImGui::PushID(midiNote);


            // ImVec4 color = isActive ? ImColor4F(cl_SkyBlue) : (mSettings.InsertMode ? ImColor4F(cl_White) : ImColor4F(cl_LightGray));
            ImVec4 color = isActive ? ImColor4F(cl_SkyBlue) :  ImColor4F(cl_LightGray);

            ImGui::PushStyleColor(ImGuiCol_Button, color);

            ImGui::SetNextItemAllowOverlap();
            ImGui::Button("##white", ImVec2(whiteWidth, whiteHeight));

            if (ImGui::IsItemActivated()) {
                SongStep step{midiNote, mCurrentInstrumentId};
                this->playNote(channel, step);

                // if (chords::selectedTypeIdx == 0 || controller->isPlaying()) {
                //     // Single Note
                //     SongStep step{midiNote, mCurrentInstrumentId};
                //     this->playNote(channel, step);
                // } else {
                //     // Chord - using the pointer from our helper array
                //     getMain()->getController()->playChord(channel, mCurrentInstrumentId, midiNote, *chords::chordOffsets[chords::selectedTypeIdx]);
                // }
            }

            if (ImGui::IsItemDeactivated()) {
                if (controller->isPlaying()) this->stopNote(channel);
                else this->stopPlayedNotes();
            }


            if (key == 0) {
                ImGui::SetCursorScreenPos(ImVec2(xPos + 5, startPos.y + whiteHeight - 20.f));
                ImGui::TextColored(ImColor4F(cl_Black), "C%d", octave);
            }

            ImGui::PopStyleColor();
            ImGui::PopID();
            whiteKeyCount++;
        }
    }

    // --- PASS 2: BLACK KEYS ---
    whiteKeyCount = 0;
    for (int octave = mCurrentStartOctave; octave <= endOctave; octave++) {
        for (int key = 0; key < 12; key++) {
            if (keys[key].isBlack) {
                uint8_t midiNote = (octave * 12) + keys[key].offset  + 12;
                bool isActive = checkNoteActive(midiNote);

                float xPos = startPos.x + (whiteKeyCount * whiteWidth) - (blackWidth / 2.0f);
                ImGui::SetCursorScreenPos(ImVec2(xPos, startPos.y));

                ImGui::PushID(midiNote);

                // Color Logic: Highlight black keys if active
                ImVec4 color = isActive ? ImColor4F(cl_SkyBlue) : ImVec4(0.05f, 0.05f, 0.05f, 1.0f);
                ImGui::PushStyleColor(ImGuiCol_Button, color);

                ImGui::Button("##black", ImVec2(blackWidth, blackHeight));


                if (ImGui::IsItemActivated()) {
                    SongStep step{midiNote, mCurrentInstrumentId};
                    this->playNote(channel, step);
                    // if (chords::selectedTypeIdx == 0 || controller->isPlaying()) {
                    //     // Single Note
                    //     SongStep step{midiNote, mCurrentInstrumentId};
                    //     getMain()->getController()->playNote(channel, step);
                    // } else {
                    //     // Chord - using the pointer from our helper array
                    //     getMain()->getController()->playChord(channel, mCurrentInstrumentId, midiNote, *chords::chordOffsets[chords::selectedTypeIdx]);
                    // }
                }

                if (ImGui::IsItemDeactivated()) {
                    if (controller->isPlaying()) this->stopNote(channel);
                    else this->stopPlayedNotes();
                }



                ImGui::PopStyleColor();
                ImGui::PopID();
            } else {
                whiteKeyCount++;
            }
        }
    }

    ImGui::PopStyleVar(1);

    // Correctly extend window boundary
    ImVec2 finalPos = ImVec2(startPos.x + (whiteKeyCount * whiteWidth), startPos.y + whiteHeight);
    ImGui::SetCursorScreenPos(finalPos);
    ImGui::Dummy(ImVec2(0, 10));

    ImGui::EndGroup(); //Piano

    lPianoWidth = ImGui::GetItemRectSize().x;


    if (standAlone) ImGui::End();

}


