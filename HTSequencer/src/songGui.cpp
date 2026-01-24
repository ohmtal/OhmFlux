#include "sequencerGui.h"
#include "sequencerMain.h"
#include <imgui_internal.h>

#include <algorithm>
#include <string>
#include <cctype>

//------------------------------------------------------------------------------
// TODO:
// [ ] New Song add a default Pattern
// [ ] OPL3Controller => play pattern WITH active channel only  -> ticktrigger i guess
// [ ] live playing << MUST have
//   [ ] row cursor must react to the playing or not ?! << in FluxEditor it stucks when it followed in edit mode
//   [ ] FIXME first: add a custom stop note (so a STOP_NOTE is added to the pattern )
//       ONLY IN LIVE PLAYING
//   [ ] play pattern starts when first note is pressed
//
// [ ] reset pattern - i need this for sure :)
// [ ] set default instrument / step for each channel / also an octave ?!
//   [ ] update fms import with default Instrument
//
        // in songdata:
        // channelInstrument.fill(0);
        // channelOctave.fill(4);
        // channelStep.fill(1);

        // std::array<int8_t, CHANNELS> channelInstrument = {};
        // std::array<int16_t, CHANNELS> channelOctave = {};
        // std::array<int8_t, CHANNELS> channelStep = {};

// [ ] select a rect with mouse or shift cursor : shift up/down = row select and ctrl shift cell select
//     ==> ImGui::BeginMultiSelect ....
// [ ] del should reset the selected (one cell or rect cells)
// [ ] ctrl-c copy to a mWorkPattern for pasting somewhere
// [ ] ctrl + del delete rows -> a check that full rows selected
// [ ] ctrl + ins insert a row (see also InsertRow)
// -------- FUTURE
// [ ] OrderList Editor


//------------------------------------------------------------------------------
void SequencerGui::DrawExportStatus() {
    // Check if the thread task exists
    if (mCurrentExport == nullptr) return;

    //  Force the modal to open
    if (!ImGui::IsPopupOpen("Exporting...")) {
        ImGui::OpenPopup("Exporting...");
    }

    // Set the window position to the center of the screen
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    //  Draw the Modal (This disables keyboard/mouse for everything else)
    if (ImGui::BeginPopupModal("Exporting...", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove)) {

        ImGui::Text("Generating FM Audio: %s", mCurrentExport->filename.c_str());
        ImGui::Separator();

        // Draw the Progress Bar
        ImGui::ProgressBar(mCurrentExport->progress, ImVec2(300, 0));

        // Auto-close when the thread finishes
        if (mCurrentExport->isFinished) {
            ImGui::CloseCurrentPopup();

            // Clean up the task memory here
            delete mCurrentExport;
            mCurrentExport = nullptr;
        }

        ImGui::EndPopup();
    }
}
//------------------------------------------------------------------------------
void SequencerGui::RenderSequencerUI(bool standAlone)
{

    const ImVec2 lButtonSize = ImVec2(70,32);

    OPL3Controller*  controller = getMain()->getController();
    if (!controller)
        return;

    DrawExportStatus();

    if (standAlone) {
        ImGui::SetNextWindowSize(ImVec2(1100, 600), ImGuiCond_FirstUseEver);
        if (!ImGui::Begin("Sequencer")) { ImGui::End(); return; }
    }



    char nameBuf[256];
    strncpy(nameBuf, mCurrentSong.title.c_str(), sizeof(nameBuf));

    // ImGui::TextColored(ImVec4(0.0f, 0.8f, 1.0f, 1.0f), "Sequencer");

    ImGui::BeginGroup();
    if (controller->isPlaying())
    {
        ImGui::PushStyleColor(ImGuiCol_Text, ImColor4F(cl_Black));
        ImGui::PushStyleColor(ImGuiCol_Button, ImColor4F(cl_Orange));        // Normal
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImColor4F(cl_Yellow)); // Hover
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImColor4F(cl_Gold));  // Geklickt

        if (ImGui::Button("Stop",lButtonSize))
        {
            stopSong();
        }
        ImGui::PopStyleColor(4);
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, ImColor4F(cl_Black));
        ImGui::PushStyleColor(ImGuiCol_Button, ImColor4F(cl_Gold));        // Normal
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImColor4F(cl_Yellow)); // Hover
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImColor4F(cl_Orange));  // Geklickt

        if (ImGui::Button("Play",lButtonSize))
        {

            playSong(3); //autodetect
        }
        ImGui::PopStyleColor(4);
    }

    if (ImGui::Checkbox("Loop", &mLoopSong))
    {
        controller->setLoop( mLoopSong );
    }
    ImGui::EndGroup();

    ImGui::SameLine();
    ImGui::Dummy(ImVec2(6.f, 0.f));
    ImGui::SameLine();
    ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
    ImGui::SameLine();
    ImGui::Dummy(ImVec2(6.f, 0.f));

    ImGui::SameLine();
    if (ImGui::Button("New",lButtonSize))
    {
        newSong();
    }
    ImGui::SameLine();
    ImGui::Dummy(ImVec2(6.f, 0.f));
    ImGui::SameLine();
    ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
    ImGui::SameLine();
    ImGui::Dummy(ImVec2(6.f, 0.f));
    ImGui::SameLine();
    static bool sShowNewPatternPopup = false;
    if (ImGui::Button("Pattern +",lButtonSize))
    {
        sShowNewPatternPopup = true;
        ImGui::OpenPopup("New Pattern Configuration");
    }
    if (DrawNewPatternModal(mCurrentSong, mNewPatternSettings)) {
        sShowNewPatternPopup = "false";
    }


    ImGui::SameLine();
    if (ImGui::Button("Save",lButtonSize))
    {
        callSaveSong();

    }
    ImGui::SameLine();
    ImGui::BeginGroup();
    if (ImGui::Button("Export",lButtonSize))
    {
        callExportSong();
    }
    if (ImGui::Checkbox("DSP on", &mExportWithEffects))
    {
        controller->setLoop( mExportWithEffects );
    }
    ImGui::EndGroup();



    if (ImGui::BeginChild("BC_Box", ImVec2(0, 0), ImGuiChildFlags_Borders)) {
        ImGui::AlignTextToFramePadding();

        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "Title");ImGui::SameLine();
        ImGui::SetNextItemWidth(120);
        if (ImGui::InputText("##Song Title", nameBuf, sizeof(nameBuf))) {
            mCurrentSong.title = nameBuf;
        }

        ImGui::SameLine();
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "BPM");ImGui::SameLine();
        ImGui::SetNextItemWidth(80);
        ImGui::InputFloat("##BPM", &mCurrentSong.bpm,  1.0f, 10.0f, "%.0f");

        // NOTE: not sure if i want to change the ticks ......
        // ImGui::SameLine();
        // ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "Ticks per Row");ImGui::SameLine();
        // int tempSpeed = mCurrentSong.ticksPerRow;
        // ImGui::SetNextItemWidth(80);
        // if (ImGui::InputInt("##Ticks per Row", &tempSpeed, 1, 1)) {
        //     mCurrentSong.ticksPerRow = static_cast<uint8_t>(std::clamp(tempSpeed, 1, 32));
        // }

        ImGui::SameLine();
        ImGui::Checkbox("Insert Mode",&mSettings.InsertMode);
        // ImGui::Dummy(ImVec2(0.f, 5.f)); ImGui::Separator();


        DrawPatternSelector(mCurrentSong, mPatternEditorState);
        if (mPatternEditorState.currentPatternIdx >= 0) {
            Pattern* lCurrentPattern = &mCurrentSong.patterns[mPatternEditorState.currentPatternIdx];

            // Apply pattern-specific color to the editor background if desired
            ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImGui::ColorConvertU32ToFloat4(lCurrentPattern->mColor));

            DrawPatternEditor(*lCurrentPattern, mPatternEditorState);

            ImGui::PopStyleColor();
        }
    }


    ImGui::EndChild();

    if (standAlone) ImGui::End();
}
