#include "sequencerGui.h"
#include "sequencerMain.h"
#include <imgui_internal.h>

#include <algorithm>
#include <string>
#include <cctype>

//------------------------------------------------------------------------------
// TODO:
// [ ] table drives me crazy lol >> look at IMGui demo of AssetBrowser !!
// [X] Pattern with mColCount instead of fixed for copy paste / template
// [X] instrument select combo => Widget_InstrumentCombo
// [ ] Bank editor
//   [ ] save
//   [ ] load
//   [ ] add
//   [ ] replace
// [X] New Song add a default Pattern
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
constexpr float CellHeight = 20.f;

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
        // ImGui::SetNextWindowSize(ImVec2(1100, 600), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSizeConstraints(ImVec2(400.0f, 300.0f), ImVec2(FLT_MAX, FLT_MAX));
        //NOTE added flags (table madness ) ==
        if (!ImGui::Begin("Sequencer")) //, nullptr, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoScrollWithMouse))
            { ImGui::End(); return; }
    }



    char nameBuf[256];
    strncpy(nameBuf, mCurrentSong.title.c_str(), sizeof(nameBuf));

    // ImGui::TextColored(ImVec4(0.0f, 0.8f, 1.0f, 1.0f), "Sequencer");

    ImGui::BeginGroup();
    if (controller->isPlaying())
    {
        ImGui::PushStyleColor(ImGuiCol_Text, ImColor4F(cl_Black));
        ImGui::PushStyleColor(ImGuiCol_Button, ImColor4F(cl_Orange));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImColor4F(cl_Yellow));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImColor4F(cl_Gold));

        if (ImGui::Button("Stop",lButtonSize))
        {
            stopSong();
        }
        ImGui::PopStyleColor(4);
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, ImColor4F(cl_Black));
        ImGui::PushStyleColor(ImGuiCol_Button, ImColor4F(cl_Gold));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImColor4F(cl_Yellow));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImColor4F(cl_Orange));

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

    ImGui::Separator();

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

    ImGui::SameLine();
    ImGui::Checkbox("E.View",&mSettings.EnhancedStepView);

    ImGui::SameLine();
    ImGui::Text("ROW:%d, COL:%d, currentChannel:%d", mPatternEditorState.cursorRow, mPatternEditorState.cursorCol, getCurrentChannel());
    // if (isPlaying())
    {
        std::string lChannelToNoteStates = "";
        for (int i = 0; i < SOFTWARE_CHANNEL_COUNT; i++)
            lChannelToNoteStates += std::format(" {:03}", getMain()->getController()->mChannelToNote[i]);
        // ImGui::SameLine();
        const OPL3Controller::SequencerState& lSeqState = getMain()->getController()->getSequencerState();
        ImGui::TextColored(ImColor4F(cl_AcidGreen),
                "SEQ: seq:%d row:%d ChannelNoteStates: %s"
                ,lSeqState.orderIdx
                ,lSeqState.rowIdx
                ,lChannelToNoteStates.c_str()
                );
    }

//
    // ImGui::Dummy(ImVec2(0.f, 5.f)); ImGui::Separator();

    DrawPatternSelector(mCurrentSong, mPatternEditorState);

    // ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoScrollWithMouse
    if (ImGui::BeginChild("PATTERN_Box",
        ImVec2(0, -ImGui::GetTextLineHeightWithSpacing()), //ImVec2(0, 0),
        ImGuiChildFlags_Borders,
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse)) {
        // ImGui::AlignTextToFramePadding();
        if (mPatternEditorState.currentPatternIdx >= 0) {


            // ~~~~~~~~~ scroll attempt 1001 ~~~~~~~~~~~~~~+
            // int lScrolltoRow = isPlaying() ? getPlayingRow() : mPatternEditorState.cursorRow;
            // bool lDoScroll =  isPlaying() || mPatternEditorState.scrollToSelected;
            // if (lDoScroll) {
            //     // Use GetContentRegionAvail() to get the exact inner visible height
            //     float visibleHeight = ImGui::GetContentRegionAvail().y;
            //
            //     // Formula: (Target Row * Height) - (Half of Visible Area) + (Half of Item Height)
            //     float scrollY = (lScrolltoRow * CellHeight)
            //     - (visibleHeight * 0.5f)
            //     + (CellHeight * 0.5f);
            //
            //     ImGui::SetScrollY(scrollY);
            //     mPatternEditorState.scrollToSelected = false;
            // }
            //

            // <<< scroll

            Pattern* lCurrentPattern = &mCurrentSong.patterns[mPatternEditorState.currentPatternIdx];
            //FIXME ?!
            mPatternEditorState.pattern = lCurrentPattern;
            DrawPatternEditor(mPatternEditorState);
        }

    }
    ImGui::EndChild();



    if (standAlone) ImGui::End();
}
//------------------------------------------------------------------------------
void SequencerGui::ActionPatternEditor(PatternEditorState& state)
{
    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) {
        if (ImGui::IsKeyPressed(ImGuiKey_UpArrow))    state.moveCursorPosition(-1, 0);
        if (ImGui::IsKeyPressed(ImGuiKey_DownArrow))  state.moveCursorPosition( 1, 0);
        if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow))  state.moveCursorPosition( 0,-1);
        if (ImGui::IsKeyPressed(ImGuiKey_RightArrow)) state.moveCursorPosition( 0, 1);
        if (ImGui::IsKeyPressed(ImGuiKey_PageUp))     state.moveCursorPosition(-16, 0);
        if (ImGui::IsKeyPressed(ImGuiKey_PageDown))   state.moveCursorPosition( 16, 0);
        if (ImGui::IsKeyPressed(ImGuiKey_Home))       state.moveCursorPosition(-10000, 0);
        if (ImGui::IsKeyPressed(ImGuiKey_End))        state.moveCursorPosition( 10000, 0);

        if ( ImGui::IsKeyPressed(ImGuiKey_Space)) state.pattern->getStep(state.cursorRow, state.cursorCol).note = opl3::STOP_NOTE;
        // //FIXME delete selected func
        if ( ImGui::IsKeyPressed(ImGuiKey_Delete)) state.pattern->getStep(state.cursorRow, state.cursorCol).init();
    } // is focused
}
//------------------------------------------------------------------------------
void SequencerGui::DrawPatternEditor( PatternEditorState& state) {
    if (!state.pattern)
        return;

    const int numRows = (int)state.pattern->getRowCount();


    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && !ImGui::IsAnyItemActive()) {
        ImGui::SetKeyboardFocusHere();
    }

    // Style
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1));
    // NOTE: IMPORTANT Push transparent colors to hide the highlight
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_NavHighlight, ImVec4(0, 0, 0, 0));

    static ImGuiTableFlags flags =
                ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersOuterH
                | ImGuiTableFlags_ScrollY  | ImGuiTableFlags_ScrollX
                | ImGuiTableFlags_RowBg
                /*| ImGuiTableFlags_Resizable*/ //NOTE: Resizable ? or not
                ;



    int lChannelCount = opl3::SOFTWARE_CHANNEL_COUNT;
    int lColCount =  lChannelCount + 1;


    if (ImGui::BeginTable("PatternTable", lColCount, flags, ImVec2(0, 0))) {

        // Setup Row Index Column
        ImGui::TableSetupScrollFreeze(0, 1); // Freeze header row
        ImGui::TableSetupColumn("##Row", ImGuiTableColumnFlags_WidthFixed, 35.0f);


        for (int col = 0; col < lChannelCount; col++) {

            char tmpbuf[128];
            int lIdx = mCurrentSong.channelInstrument[col];

            snprintf(tmpbuf,sizeof(tmpbuf)
                    , "[%d]%s##%d"
                    , col
                    , getMain()->getController()->getInstrumentName(lIdx).c_str()
                    , col
                    );


            // if (col < 6) {
            //     snprintf(name, sizeof(name), "[%02d]##channel", col + 1);
            // } else {
            //     snprintf(name, sizeof(name), "[%02d]##channel", col + 1);
            // }

            if ( mSettings.EnhancedStepView )
                ImGui::TableSetupColumn(tmpbuf, ImGuiTableColumnFlags_WidthFixed, 120.0f);
            else
                ImGui::TableSetupColumn(tmpbuf, ImGuiTableColumnFlags_WidthFixed, 50.0f);

        }
        // ImGui::TableHeadersRow();
        // ------------- custom draw header
        ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
        int channel = 0;
        for (int col = 0; col <= lChannelCount; col++) {
            // Maybe we need the top left too


            channel = col -1 ;
            if (!ImGui::TableSetColumnIndex(col)) continue;

            static std::string lColCaption;
            lColCaption = ImGui::TableGetColumnName(col);
            // TODO header states
            // ImGui::PushStyleColor(ImGuiCol_Text, ImColor4F(cl_Lime));
            // ImGui::PushStyleColor(ImGuiCol_Text, ImColor4F(cl_Gray));

            if (channel ==  state.cursorCol)
                ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, Color4FIm(cl_DeepSea));

            ImGui::TableHeader(lColCaption.c_str());

            // ImGui::PopStyleColor();

            // --------------- Header Popup
            if (channel >= 0)
            {
                if (ImGui::BeginPopupContextItem())
                {
                    ImGui::TextColored(ImColor4F(cl_Crimson), "Channel %d", channel + 1);
                    ImGui::Separator();
                    if (channel < 6) {
                        ImGui::TextColored(ImColor4F(cl_Blue), "Four Operator channel");
                    } else {
                        ImGui::TextColored(ImColor4F(cl_Yellow), "Two Operator channel");
                    }

                    // instrument
                    ImGui::Text("Instrument:");
                    ImGui::SetNextItemWidth(140.0f); // Often needed as menus are narrow by default
                    int newInst = Widget_InstrumentCombo(mCurrentSong.channelInstrument[channel], getMain()->getController()->getSoundBank() );
                    if ( newInst >= 0)
                    {
                        mCurrentSong.channelStep[channel] = newInst;
                    }
                    ImGui::SameLine();
                    if (ImGui::SmallButton("U")) {
                        //FIXME add function to update all Step!
                        Log("[warning] update steps with new instrument not implemented");
                    }

                    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Update all Steps with this instrument");



                    ImGui::Separator();

                    // Step ..
                    int lStep = mCurrentSong.channelStep[channel];
                    ImGui::Text("Step:");
                    ImGui::SetNextItemWidth(100.0f); // Often needed as menus are narrow by default
                    if (ImGui::InputInt("##step", &lStep)) {
                        lStep = std::clamp(lStep, 0, 127);
                        mCurrentSong.channelStep[channel] = lStep;
                    }
                    //<<< Step

                    ImGui::Separator();
                    ImGui::SetCursorPosX((ImGui::GetWindowSize().x - 60) * 0.5f);
                    if (ImGui::Button("Close", ImVec2(60, 0))) { ImGui::CloseCurrentPopup(); }

                    //-----
                    ImGui::EndPopup();
                }
                // --------------- Header Popup
            } // channel >= 0
        } // Header stuff

        //--------------------- TABLE CONTENT ------------------------------
        // High-performance clipping

        int lScrolltoRow = isPlaying() ? getPlayingRow() : state.cursorRow;
        bool lDoScroll =  isPlaying() || state.scrollToSelected;

        ImGuiListClipper clipper;
        clipper.Begin(numRows, CellHeight);

        // NOTE: Version ... not soo bad but bad
        if (lDoScroll) {
            clipper.IncludeItemByIndex(lScrolltoRow); // NOTE: scrolling
            state.scrollToSelected = false;
        }

        while (clipper.Step()) {

            for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++) {

                ImGui::TableNextRow(ImGuiTableRowFlags_None, CellHeight);

                // ~~~~~~~~~~~ scrolling madness ~~~~~~~~~~~~~~~~~~~
                // NOTE: Version ... not soo bad but bad
                if ( lDoScroll && lScrolltoRow > clipper.DisplayEnd - 3 )
                {

                    ImGui::ScrollToItem(ImGuiScrollFlags_AlwaysCenterY );
                    //ImGui::SetScrollHereY(0.5f);
                }

                ImGui::PushID(row);

                // Column 0: Row Number
                ImGui::TableSetColumnIndex(0);
                if ( isPlaying() && getPlayingRow() == row )
                {
                   // ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, Color4FIm(cl_Coral));

                   ImGui::TextColored(Color4FIm(cl_Black), "%03d", row);

                   ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, Color4FIm(cl_Coral));
                } else {
                    ImGui::TextDisabled("%03d", row);
                }

                // Columns 1-12: Channel Steps
                for (int col = 0; col < opl3::SOFTWARE_CHANNEL_COUNT; col++) {


                    if ( lDoScroll && col == state.cursorCol) {
                        ImGui::SetScrollHereX(0.0f);
                        // ImGui::ScrollToItem(ImGuiScrollFlags_AlwaysCenterX );
                        // lDoScroll = false;
                    }


                    ImGui::TableSetColumnIndex(col + 1);
                    SongStep& step = state.pattern->getStep(row, col);
                    ImGui::PushID(row * opl3::SOFTWARE_CHANNEL_COUNT + col);
                    // Pass current row/column and state to the cell renderer
                    bool isCursorPos = (state.cursorRow == row && state.cursorCol == col);
                    RenderStepCell(step, isCursorPos, row, col, state);


                    ImGui::PopID();
                } //columns
                // -----------


                ImGui::PopID(); //row


            } // for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++) ...
        } //while
        ImGui::EndTable();

        ActionPatternEditor(state);

    } //PatternTable
    ImGui::PopStyleColor(4);
    ImGui::PopStyleVar();
}
// //------------------------------------------------------------------------------
// void SequencerGui::state.moveCursorPosition(int rowAdd, int colAdd) {
//     PatternEditorState& state = mPatternEditorState;
//     setCursorPosition(state.cursorRow + rowAdd, state.cursorCol+colAdd);
// }
// void SequencerGui::setCursorPosition(int row, int col) {
//     PatternEditorState& state = mPatternEditorState;
//     Pattern* pattern = getCurrentPattern();
//     if (!pattern) return;
//
//     row = std::clamp(row, 0, pattern->getRowCount() -1 );
//     col = std::clamp(col, 0, pattern->getColCount() -1 );
//
//     if ( row == state.cursorRow && col == state.cursorCol )
//         return ;
//
//
//     state.cursorRow = row;
//     state.cursorCol = col;
//
//     state.scrollToSelected = true;
//
// }
//------------------------------------------------------------------------------
void SequencerGui::RenderStepCell(opl3::SongStep& step, bool isSelected, int r, int c, PatternEditorState& state) {
    // Construct the tracker-style string: "C-4 01 v63 A0F"
    std::string noteStr = opl3::ValueToNote(step.note);

    char buf[32];
    char hintBuffer[256];


    //TODO nicer hint:
    snprintf(hintBuffer, sizeof(hintBuffer), "%s\n%s (%d)\nVol:%02d\nEff:%c%02X",
             noteStr.c_str(),
             getMain()->getController()->getInstrumentName(step.instrument).c_str(),step.instrument,
             step.volume,
             (step.effectType == 0 ? '.' : (char)step.effectType),
             step.effectVal);



    if (mSettings.EnhancedStepView)
    {
        snprintf(buf, sizeof(buf), "%s %02X %02d %c%02X",
                 noteStr.c_str(),
                 step.instrument,
                 step.volume,
                 (step.effectType == 0 ? '.' : (char)step.effectType),
                 step.effectVal);
    } else {
        snprintf(buf, sizeof(buf), "%s",noteStr.c_str());
    }


    // Highlight if this is the active selection/cursor
    if (isSelected /*&& !state.scrollToSelected*/) {
        // ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImGui::GetColorU32(ImGuiCol_HeaderActive));
        ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, Color4FIm(cl_Blue));

    }

    // 3. Make the cell clickable to set the cursor
    // Use "##" to create a unique ID for the Selectable without showing the label twice
    if (ImGui::Selectable(buf, isSelected, ImGuiSelectableFlags_AllowOverlap)) {
        state.cursorRow = r;
        state.cursorCol = c;
    }

    if (step.note < LAST_NOTE && ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) {
        ImGui::SetTooltip("%s", hintBuffer);
    }


}
//------------------------------------------------------------------------------
void SequencerGui::DrawPatternSelector(opl3::SongData& song, PatternEditorState& state) {

    static bool sShowNewPatternPopup = false;
    if (ImGui::SmallButton("[+]"))
    {
        sShowNewPatternPopup = true;
        ImGui::OpenPopup("New Pattern Configuration");
    }
    if (DrawNewPatternModal(mCurrentSong, mNewPatternSettings)) {
        sShowNewPatternPopup = "false";
    }
    ImGui::SameLine();


    if (song.patterns.empty()) {
        ImGui::Text("No patterns created.");
        state.currentPatternIdx = -1;
        return;
    }

    if (state.currentPatternIdx < 0) state.currentPatternIdx = 0;
    else
        if (state.currentPatternIdx >= (int)song.patterns.size()) {
            state.currentPatternIdx = (int)song.patterns.size() - 1;
        }
        //-----------

        if (ImGui::BeginTabBar("PatternTabs", ImGuiTabBarFlags_AutoSelectNewTabs | ImGuiTabBarFlags_Reorderable)) {
            for (int i = 0; i < (int)song.patterns.size(); i++) {
                Pattern& p = song.patterns[i];

                // 1. Convert color and Push Styles BEFORE BeginTabItem
                ImVec4 tabColor = ImGui::ColorConvertU32ToFloat4(p.mColor);
                ImVec4 inactiveColor = tabColor;
                inactiveColor.w *= 0.6f;

                ImGui::PushStyleColor(ImGuiCol_Tab, inactiveColor);
                ImGui::PushStyleColor(ImGuiCol_TabActive, tabColor);
                ImGui::PushStyleColor(ImGuiCol_TabHovered, tabColor);

                char label[64];
                snprintf(label, sizeof(label), "%s###Tab%d",p.mName.c_str(), i);

                // 2. Begin the Tab
                bool tabOpen = ImGui::BeginTabItem(label);

                // Context menu logic (associated with the last item, the tab)
                char popupId[32];
                snprintf(popupId, sizeof(popupId), "TabCtx%d", i);
                if (ImGui::BeginPopupContextItem(popupId)) {
                    ImGui::TextColored(ImColor4F(cl_Emerald), "Pattern settings");
                    char patName[64];
                    strncpy(patName, p.mName.c_str(), sizeof(patName));
                    ImGui::TextDisabled("Pattern Name");
                    if (ImGui::InputText("##Pattern Name", patName, 64))
                    {
                        p.mName = patName;
                    }
                    ImVec4 tempCol = ImGui::ColorConvertU32ToFloat4(p.mColor);
                    ImGui::TextDisabled("Pattern Color");
                    if (ImGui::ColorEdit4("##Pattern Color", (float*)&tempCol)) {
                        p.mColor = ImGui::ColorConvertFloat4ToU32(tempCol);
                    }

                    ImGui::SetCursorPosX((ImGui::GetWindowSize().x - 60) * 0.5f);
                    if (ImGui::Button("Close", ImVec2(60, 0))) { ImGui::CloseCurrentPopup(); }

                    ImGui::EndPopup();
                }

                // 3. Handle Tab Content if Open
                if (tabOpen) {
                    state.currentPatternIdx = i; // Mark as selected
                    ImGui::EndTabItem();
                }

                // 4. IMPORTANT: Always Pop here, outside the 'if (tabOpen)' block
                // This ensures every push is matched by a pop every frame
                ImGui::PopStyleColor(3);
            }
            ImGui::EndTabBar();
        }

}
//------------------------------------------------------------------------------
bool SequencerGui::DrawNewPatternModal(opl3::SongData& song, NewPatternSettings& settings) {
    bool result = false;
    uint8_t patCount =  (uint8_t)song.patterns.size();


    if (ImGui::BeginPopupModal("New Pattern Configuration", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::InputText("Pattern Name", settings.name, 64);
        ImGui::ColorEdit4("Pattern Color", (float*)&settings.color);

        ImGui::SliderInt("Rows", &settings.rowCount, 1, 1024);
        ImGui::SameLine();
        if(ImGui::Button("256")) settings.rowCount = 256;

        if (ImGui::Button("OK", ImVec2(120, 0))) {
            Pattern p;
            p.mName = settings.name;
            p.mColor = ImGui::ColorConvertFloat4ToU32(settings.color);

            // Allocation: Rows * 12 channels
            p.getStepsMutable().resize(settings.rowCount * opl3::SOFTWARE_CHANNEL_COUNT);

            // Initialize with "None" notes
            for(auto& s : p.getStepsMutable()) {
                s.note = 255;
                s.volume = 63;
                s.panning = 32;
            }

            song.patterns.push_back(std::move(p));
            // automatic add to orderlist ! you can edit it later
            uint8_t newPatternIdx = patCount - 1;
            song.orderList.push_back(newPatternIdx);
            ImGui::CloseCurrentPopup();
            result = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
            result = true;
        }

        ImGui::EndPopup();
    }
    return result;
}
//------------------------------------------------------------------------------
