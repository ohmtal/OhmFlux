#include "sequencerGui.h"
#include "sequencerMain.h"
#include <imgui_internal.h>

#include <algorithm>
#include <string>
#include <cctype>

//------------------------------------------------------------------------------
// TODO:
// [X] Add Soundrender to DSP window and also to effects (not in Controller directly)
// [ ] change fms3 format again :P better now than later
//    [ ] save / load ALL the DSP effects
//          i dont care the filesize (some bytes)
//          so i store the settings also if it's off.
// [ ] Bank editor
//   [ ] save
//   [ ] load
//   [ ] add
//   [ ] replace
// [ ] live playing << MUST have ~~ works a bit ;)
//   [ ] row cursor must react to the playing or not ?! << in FluxEditor it stucks when it followed in edit mode
//   [ ] FIXME first: add a custom stop note (so a STOP_NOTE is added to the pattern )
//       ONLY IN LIVE PLAYING
//   [ ] play pattern starts when first note is pressed
//
// [ ] Channel Menu => set instrument (or do i use selection ?!) need a menu for seletion too
//
// [ ] OrderList Editor
//
//
// [X] table drives me crazy lol >> look at IMGui demo of AssetBrowser  => it's ok for me now
// [X] Pattern with mColCount instead of fixed for copy paste / template
// [X] instrument select combo => Widget_InstrumentCombo
// [X] New Song add a default Pattern
// [X] OPL3Controller => play pattern WITH active channel only  -> ticktrigger i guess
// [~] reset pattern - i need this for sure :) => ctrl+a :P
// [~] set default instrument / step for each channel / also an octave ?! << bullshit
// [X] select a rect with mouse or shift cursor : shift up/down = row select and ctrl shift cell select
// [X] del should reset the selected (one cell or rect cells)
// [X] ctrl+x
// [X] ctrl+a
// [X] ctrl+c copy mPatternClipBoard
// [X] ctrl+v paste
// [X] ctrl + del delete rows using selection
// [X] ctrl + ins insert using selection
// [X] ctrl+up transpose up
// [X] ctrl+down transpose down
// [X] ctrl+up transpose octave up
// [X] ctrl+pagedown octave transpose down
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
    DrawStepCellPopup(mPatternEditorState);


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

            playSong(); //autodetect
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




    //  not sure if i want to change the ticks ...... Yes i want ! :P
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "Ticks per Row");ImGui::SameLine();
    int tempSpeed = mCurrentSong.ticksPerRow;
    ImGui::SetNextItemWidth(80);
    if (ImGui::InputInt("##Ticks per Row", &tempSpeed, 1, 1)) {
        mCurrentSong.ticksPerRow = static_cast<uint8_t>(std::clamp(tempSpeed, 1, 32));
    }



    ImGui::SameLine();
    ImGui::Checkbox("Insert Mode",&mSettings.InsertMode);

    ImGui::SameLine();
    ImGui::Checkbox("E.View",&mSettings.EnhancedStepView);

    ImGui::Separator();
    ImGui::Text("ROW:%d, COL:%d, currentChannel:%d", mPatternEditorState.cursorRow, mPatternEditorState.cursorCol, getCurrentChannel());


    std::string lSeqOrder = "Orders:";
    for ( int i = 0 ; i < mCurrentSong.orderList.size(); i++ )
    {
        lSeqOrder += std::format("{:02} ", mCurrentSong.orderList[i]);
    }
    ImGui::SameLine();
    ImGui::TextColored(ImColor4F(cl_Magenta), "%s", lSeqOrder.c_str());



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

            if (isPlaying()) {
                if ( getMain()->getController()->getSequencerStateMutable()->playRange.active)
                {
                    mPatternEditorState.currentPatternIdx = getMain()->getController()->getSequencerStateMutable()->playRange.patternIdx;
                } else {
                    int orderIdx = getMain()->getController()->getSequencerState().orderIdx;
                    mPatternEditorState.currentPatternIdx = mCurrentSong.orderList[orderIdx];
                }
            }

            Pattern* lCurrentPattern = &mCurrentSong.patterns[mPatternEditorState.currentPatternIdx];
            mPatternEditorState.pattern = lCurrentPattern;
            DrawPatternEditor(mPatternEditorState);

        }



    } //PATTERN_Box
    ImGui::EndChild();




    // if (isPlaying())
    {
        std::string lChannelToNoteStates = "";
        for (int i = 0; i < SOFTWARE_CHANNEL_COUNT; i++)
            lChannelToNoteStates += std::format(" {:03}", getMain()->getController()->mChannelToNote[i]);
        // ImGui::SameLine();
        const OPL3Controller::SequencerState& lSeqState = getMain()->getController()->getSequencerState();

        ImGui::TextColored(ImColor4F(cl_AcidGreen),
                           "SEQ: order:%d pat:%d row:%d ChannelNoteStates:%s"
                           ,lSeqState.orderIdx
                           ,mCurrentSong.orderList[lSeqState.orderIdx]
                           ,lSeqState.rowIdx
                           ,lChannelToNoteStates.c_str()
        );
    }




    if (standAlone) ImGui::End();
}
// //------------------------------------------------------------------------------
void SequencerGui::DrawStepCellPopup(PatternEditorState& state) {
    if (state.showContextRequest) {
        ImGui::OpenPopup("PatternCellContext");
        state.showContextRequest = false; // Reset the flag immediately


        dLog("[info] Selection: count:%d, rowcount=%d, colcount=%d, startpoint=%d,%d, endPoint=%d,%d ",
             state.selection.getCount()
             ,state.selection.getRowCount()
             ,state.selection.getColCount()
             ,state.selection.startPoint[0],state.selection.startPoint[1]
             ,state.selection.endPoint[0],state.selection.endPoint[1]

        );

    }

    if (ImGui::BeginPopup("PatternCellContext"))
    {
        // 1. Context Information
        if (state.selection.active) {
            ImGui::TextDisabled("Selection: R%d:C%d to R%d:C%d",
                                state.selection.startPoint[0], state.selection.startPoint[1],
                                state.selection.endPoint[0], state.selection.endPoint[1]);
            ImGui::Separator();
        }


        if (ImGui::MenuItem("Play selected")) {
            playSelected(state);
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Copy", "Ctrl+C")) {
            copyStepsToClipboard(state, mPatternClipBoard);
        }
        if (ImGui::MenuItem("Paste", "Ctrl+V")) {
            pasteStepsFromClipboard(state, mPatternClipBoard);
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Cut", "Ctrl+X")) {
            copyStepsToClipboard(state, mPatternClipBoard);
            clearSelectedSteps(state);
            state.selection.init(); // Clear selection after cut
        }

        if (ImGui::MenuItem("Clear", "Del")) {
            clearSelectedSteps(state);
        }


        ImGui::Separator();

        if (ImGui::MenuItem("Select All", "Ctrl+A")) {
            selectPatternAll(state);
        }

        if (ImGui::MenuItem("Select Col (channel)", "")) {
            selectPatternCol(state);
        }

        if (ImGui::MenuItem("Select Row", "")) {
            selectPatternRow(state);
        }

        if (ImGui::MenuItem("Deselect", "")) {
            state.selection.init();
        }
        ImGui::EndPopup();
    }
}
//------------------------------------------------------------------------------
void SequencerGui::ActionPatternEditor(PatternEditorState& state)
{
    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) {
        // 1. Capture modifier state
        bool shiftHeld = ImGui::GetIO().KeyShift;
        bool ctrlHeld = ImGui::GetIO().KeyCtrl;


        // 2. Determine if we are moving the cursor this frame
        int oldRow = state.cursorRow;
        int oldCol = state.cursorCol;
        bool moved = false;
        if (ImGui::IsKeyPressed(ImGuiKey_UpArrow))    { state.moveCursorPosition(-1, 0);  moved = true; }
        if (ImGui::IsKeyPressed(ImGuiKey_DownArrow))  { state.moveCursorPosition( 1, 0);  moved = true; }
        if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow))  { state.moveCursorPosition( 0,-1);  moved = true; }
        if (ImGui::IsKeyPressed(ImGuiKey_RightArrow)) { state.moveCursorPosition( 0, 1);  moved = true; }
        if (ImGui::IsKeyPressed(ImGuiKey_PageUp))     { state.moveCursorPosition(-16, 0); moved = true; }
        if (ImGui::IsKeyPressed(ImGuiKey_PageDown))   { state.moveCursorPosition( 16, 0); moved = true; }
        if (ImGui::IsKeyPressed(ImGuiKey_Home))       { state.moveCursorPosition(-10000, 0); moved = true; }
        if (ImGui::IsKeyPressed(ImGuiKey_End))        { state.moveCursorPosition( 10000, 0); moved = true; }

        // 3. Selection Logic
        if (moved) {
            if (shiftHeld) {
                if (!state.selection.active) {
                    // First movement with shift: Anchor the start where we WERE
                    // (Assuming moveCursorPosition already updated cursorRow/Col)
                    state.selection.active = true;
                    // You may need to store 'previous' cursor pos if you want
                    // the selection to start from the exact original click point.
                    state.selection.startPoint[0] = oldRow;
                    state.selection.startPoint[1] = oldCol;
                }
                // Always update the end point to the current cursor position
                state.selection.endPoint[0] = state.cursorRow;
                state.selection.endPoint[1] = state.cursorCol;
            } else {
                // Moved without shift: Clear selection
                state.selection.init();
            }
        }

        // Other actions...
        if (ImGui::IsKeyPressed(ImGuiKey_Space)) state.pattern->getStep(state.cursorRow, state.cursorCol).note = opl3::STOP_NOTE;
        if (ImGui::IsKeyPressed(ImGuiKey_Delete)) clearSelectedSteps(state);

        if (ctrlHeld && ImGui::IsKeyPressed(ImGuiKey_UpArrow)) transposeSelection(state, +1);
        if (ctrlHeld && ImGui::IsKeyPressed(ImGuiKey_DownArrow)) transposeSelection(state, -1);
        if (ctrlHeld && ImGui::IsKeyPressed(ImGuiKey_PageUp)) transposeSelection(state, +12);
        if (ctrlHeld && ImGui::IsKeyPressed(ImGuiKey_PageDown)) transposeSelection(state, -12);



        if (ctrlHeld && ImGui::IsKeyPressed(ImGuiKey_C)) copyStepsToClipboard(state, mPatternClipBoard);
        if (ctrlHeld && ImGui::IsKeyPressed(ImGuiKey_V)) pasteStepsFromClipboard(state, mPatternClipBoard);

        if (ctrlHeld && ImGui::IsKeyPressed(ImGuiKey_X)) {
            copyStepsToClipboard(state, mPatternClipBoard);
            clearSelectedSteps(state);
            state.selection.init(); // Clear selection after cut
        }

        if (ctrlHeld && ImGui::IsKeyPressed(ImGuiKey_A)) selectPatternAll(state);
        if (ctrlHeld && ImGui::IsKeyPressed(ImGuiKey_Insert)) insertAndshiftDataDown(state);
        if (ctrlHeld && ImGui::IsKeyPressed(ImGuiKey_Delete)) deleteAndShiftDataUp(state);

    }
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
        ImGui::TableSetupScrollFreeze(1, 1); // Freeze header row
        ImGui::TableSetupColumn("##Row", ImGuiTableColumnFlags_WidthFixed, 35.0f);


        for (int col = 0; col < lChannelCount; col++) {

            char tmpbuf[128];
            // int lIdx = mCurrentSong.channelInstrument[col];

            snprintf(tmpbuf,sizeof(tmpbuf)
                    , "CH:%d##%d"
                    , col
                    // , getMain()->getController()->getInstrumentName(lIdx).c_str()
                    , col
                    );



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

            if (col==0)
            {
                ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, state.pattern->mColor);
            }


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

                    // instrument NOT!
                    // ImGui::Text("Instrument:");
                    // ImGui::SetNextItemWidth(140.0f); // Often needed as menus are narrow by default
                    // int newInst = Widget_InstrumentCombo(mCurrentSong.channelInstrument[channel], getMain()->getController()->getSoundBank() );
                    // if ( newInst >= 0)
                    // {
                    //     mCurrentSong.channelInstrument[channel] = newInst;
                    // }
                    // ImGui::SameLine();
                    // if (ImGui::SmallButton("U")) {
                    //     // add function to update all Step!
                    //     Log("[warning] update steps with new instrument not implemented");
                    // }

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
                    if (row % 4 == 0)
                        ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, Color4FIm(cl_Slate));
                    else
                        ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, Color4FIm(cl_Black));

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

                    // moved to DrawStepCell ImGui::PushID(row * opl3::SOFTWARE_CHANNEL_COUNT + col);
                    // Pass current row/column and state to the cell renderer
                    bool isSelected = (state.cursorRow == row && state.cursorCol == col)
                        || state.selection.isSelected(row, col);

                    DrawStepCell(step, isSelected, row, col, state);

                    // moved to DrawStepCell ImGui::PopID();
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
//------------------------------------------------------------------------------
void SequencerGui::DrawStepCell(opl3::SongStep& step, bool isSelected, int row, int col, PatternEditorState& state) {

    ImGui::PushID(row * 1000 + col);

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
        if (step.note < LAST_NOTE)
            snprintf(buf, sizeof(buf), "%s %d",noteStr.c_str(), step.instrument);
        else
            snprintf(buf, sizeof(buf), "%s ",noteStr.c_str());
    }


    // Highlight if this is the active selection/cursor
    if (isSelected /*&& !state.scrollToSelected*/) {
        // ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImGui::GetColorU32(ImGuiCol_HeaderActive));
        ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, Color4FIm(cl_Blue));

    }

    // A. Render the Selectable purely for the visual feedback/hitbox
    ImGui::Selectable(buf, isSelected, ImGuiSelectableFlags_AllowOverlap);

    // B. SNAPPY CURSOR: Use IsItemClicked(0) for immediate response on mouse-down
    if (ImGui::IsItemClicked(0)) {

        if (!ImGui::GetIO().KeyShift) {
            state.selection.init();
            state.selectionAnchorRow = row;
            state.selectionAnchorCol = col;
        }
        state.cursorRow = row;
        state.cursorCol = col;
    }

    // C. DRAG LOGIC (Activated triggers on the very first frame of mouse-down)
    if (ImGui::IsItemActivated()) {
        if (!ImGui::GetIO().KeyShift) {
            state.selection.active = true;
            state.selection.startPoint[0] = (uint16_t)row;
            state.selection.startPoint[1] = (uint16_t)col;
        } else {
            // If Shift is held, we are extending an existing selection
            // starting from our pivot/anchor
            state.selection.active = true;
            state.selection.startPoint[0] = (uint16_t)state.selectionAnchorRow;
            state.selection.startPoint[1] = (uint16_t)state.selectionAnchorCol;
        }
    }

    // D. LIVE HOVER UPDATE (End-point and Cursor follow the mouse during drag)
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem) && ImGui::IsMouseDown(0)) {
        state.selection.endPoint[0] = (uint16_t)row;
        state.selection.endPoint[1] = (uint16_t)col;

        // This makes the cursor follow the selection tip live
        state.cursorRow = row;
        state.cursorCol = col;
    }


    // ----- tooltip
    if (step.note < LAST_NOTE && ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) {
        ImGui::SetTooltip("%s", hintBuffer);
    }

    // ----- popup
    if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
        state.contextRow = row;
        state.contextCol = col;
        state.showContextRequest = true;
    }

    ImGui::PopID();
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
            for (int lPatternIndex = 0; lPatternIndex < (int)song.patterns.size(); lPatternIndex++) {
                Pattern& lPat = song.patterns[lPatternIndex];

                // 1. Convert color and Push Styles BEFORE BeginTabItem
                ImVec4 tabColor = ImGui::ColorConvertU32ToFloat4(lPat.mColor);
                ImVec4 inactiveColor = tabColor;
                inactiveColor.w *= 0.6f;

                ImGui::PushStyleColor(ImGuiCol_Tab, inactiveColor);
                ImGui::PushStyleColor(ImGuiCol_TabActive, tabColor);
                ImGui::PushStyleColor(ImGuiCol_TabHovered, tabColor);

                char label[64];
                snprintf(label, sizeof(label), "%s###Tab%d",lPat.mName.c_str(), lPatternIndex);

                // 2. Begin the Tab
                bool tabOpen = ImGui::BeginTabItem(label);

                // Context menu logic (associated with the last item, the tab)
                char popupId[32];
                snprintf(popupId, sizeof(popupId), "TabCtx%d", lPatternIndex);
                if (ImGui::BeginPopupContextItem(popupId)) {
                    ImGui::TextColored(ImColor4F(cl_Emerald), "Pattern settings");
                    char patName[64];
                    strncpy(patName, lPat.mName.c_str(), sizeof(patName));
                    ImGui::TextDisabled("Pattern Name");
                    if (ImGui::InputText("##Pattern Name", patName, 64))
                    {
                        lPat.mName = patName;
                    }
                    ImVec4 tempCol = ImGui::ColorConvertU32ToFloat4(lPat.mColor);
                    ImGui::TextDisabled("Pattern Color");
                    if (ImGui::ColorEdit4("##Pattern Color", (float*)&tempCol)) {
                        lPat.mColor = ImGui::ColorConvertFloat4ToU32(tempCol);
                    }


                    //FIXME func ?!
                    if (ImGui::Button("Play", ImVec2(120, 0))) {
                        playSelected(state);
                    }

                    //FIXME temp ?!
                    if (ImGui::Button("Append to Orders", ImVec2(120, 0))) {
                        mCurrentSong.orderList.push_back(lPatternIndex);
                    }

                    //FIXME as func
                    if (ImGui::Button("Clone", ImVec2(120, 0))) {
                        Pattern clonePat = lPat;
                        clonePat.mName += " (Copy)";
                        song.patterns.push_back(std::move(clonePat));
                    }


                    ImGui::SetCursorPosX((ImGui::GetWindowSize().x - 60) * 0.5f);
                    if (ImGui::Button("Close", ImVec2(60, 0))) { ImGui::CloseCurrentPopup(); }



                    ImGui::EndPopup();

                }

                // 3. Handle Tab Content if Open
                if (tabOpen) {
                    state.currentPatternIdx = lPatternIndex; // Mark as selected
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
            uint8_t newPatternIdx = (uint8_t)song.patterns.size();
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
