#include "sequencerGui.h"
#include "sequencerMain.h"
#include <imgui_internal.h>

#include <algorithm>
#include <string>
#include <cctype>
#include <src/fonts/IconsFontAwesome6.h>

//------------------------------------------------------------------------------
// TODO:
// [ ] current:
//  [ ] Volume and panning should be not overwritten all the time ... not sure how (effects)

//  [X] Insertmode
//    [X] to keyboard gui ==> NEW LED :D and namespace ImFlux!
//    [X] only when window is visible

//  [ ] Instrument Colors: Do not render the name !! and center the text again
//      (better a small rect with color)
//      Color should be also shown in Instrument List

//  [ ] CellPopup:
//      check insert and shift makes no sense there ?!
//      - delete and shift is wrong call copy&paste i guess

//  [ ] Paste is broken if copied from a longer pattern and paste into
//      example copy column of 256 row pattern into 63 row pattern
//      I thougth it's fixed but still paste still sucks sometimes.

//  [ ] Pattern: delete (also update order list)
//  [ ] Pattern editor follow
//      [ ] check mCurrentSong is playing (via pointer? )
//      [ ] not when exporting !
//  [X] * reset bank icon
//  [ ] * rename instrument
//  [ ] * export bank .. Which format(s) ....wopl+own i guess
//  [ ]
//  [ ]
//  [ ]
//
// [ ] Limiter settings (maybe only Threshold )
//  [ ] jsON def and save/load
//  [ ] Gui Slider 0.1..0.99 + default
//
// [ ] OPL Effects
//  [ ] how to editor then the best way / context menu

// [ ] change fms3 format again :P better now than later
//    the data should be optional check EOF !
//    [ ] save / load ALL the DSP effects
//          i dont care the filesize (some bytes)
//          so i store the settings also if it's off.
// [ ] Save Instruments format
//   [ ] Single 4OP Instrument
//   [ ] Wopl Export
//
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
// [ ] Undo
//
// [ ] Make it nice with buttons (icons)
//
// [X] current 2026-01-27
//  [X] new pattern added as 02 but is 01!
//  [X] reverse selected  does paste reverse ? => sort was broken
//  [X] play pattern  plays single (selected)
//  [X] right mouse always use selected cell ==> fixed for paste (useContextPoint)
//  [X] Added Icons Font mIconFont ++ ICON_FA_...
//  [X] colored step cell rendering
//  [X] need change instrument !! for channel or better selection
//
// [X] Add Soundrender to DSP window and also to effects (not in Controller directly)
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
// [X] ctrl+pageup transpose octave up
// [X] ctrl+pagedown octave transpose down
constexpr float CellHeight = 24.f;
ImVec2 cellSize = {50, CellHeight};

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

    const OPL3Controller::SequencerState& lSeqState = getMain()->getController()->getSequencerState();


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

        ImGui::PushFont(mIconFont);

        if (ImGui::Button(ICON_FA_STOP "##Stop",lButtonSize))
        {
            stopSong();
        }
        ImGui::PopFont();
        ImGui::PopStyleColor(4);
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, ImColor4F(cl_Black));
        ImGui::PushStyleColor(ImGuiCol_Button, ImColor4F(cl_Gold));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImColor4F(cl_Yellow));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImColor4F(cl_Orange));

//FONT TEST
        ImGui::PushFont(mIconFont);
        if (ImGui::Button(ICON_FA_PLAY "##Play",lButtonSize))
        {
            playSong(); //autodetect
        }
       ImGui::PopFont();
        ImGui::PopStyleColor(4);
    }
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) {
        ImGui::SetTooltip("To play or stop press [F1]");
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


    // std::string lSeqOrder = "Orders:";
    // for ( int i = 0 ; i < mCurrentSong.orderList.size(); i++ )
    // {
    //     lSeqOrder += std::format("{:02} ", mCurrentSong.orderList[i]);
    // }
    // ImGui::SameLine();
    // ImGui::TextColored(ImColor4F(cl_Magenta), "%s", lSeqOrder.c_str());


    DrawMiniOrderList(mCurrentSong, controller->isPlayingSong() ? lSeqState.orderIdx : -1 ,  20.f, ImVec2(300,30.f));
    // DrawMiniOrderList1(mCurrentSong, false, ImVec2(300.f,30.f));



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
            mPatternEditorState.visible = true;

        }



    } //PATTERN_Box
    ImGui::EndChild();




    // if (isPlaying())
    {
        std::string lChannelToNoteStates = "";
        for (int i = 0; i < SOFTWARE_CHANNEL_COUNT; i++)
            lChannelToNoteStates += std::format(" {:03}", getMain()->getController()->mChannelToNote[i]);
        // ImGui::SameLine();

        if (lSeqState.orderIdx < mCurrentSong.orderList.size())
        {
            ImGui::TextColored(ImColor4F(cl_AcidGreen),
                               "SEQ: order:%d pat:%d row:%d ChannelNoteStates:%s"
                               ,lSeqState.orderIdx
                               ,mCurrentSong.orderList[lSeqState.orderIdx]
                               ,lSeqState.rowIdx
                               ,lChannelToNoteStates.c_str()
            );
        }

    }




    if (standAlone) ImGui::End();
}
// //------------------------------------------------------------------------------
void SequencerGui::DrawStepCellPopup(PatternEditorState& state) {
    if (state.showContextRequest) {
        ImGui::OpenPopup("PatternCellContext");
        state.showContextRequest = false; // Reset the flag immediately

        state.selection.sort();

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


        if (isPlaying()) {
            if (ImGui::MenuItem(ICON_FA_STOP " Stop")) {
                stopSong();
            }
        } else  {
            if (ImGui::MenuItem(ICON_FA_PLAY " Play selected")) {
                playSelected(state);
            }
        }
        ImGui::Separator();
        if (ImGui::BeginMenu(ICON_FA_ARROWS_UP_DOWN " Transpose")) {
            if (ImGui::MenuItem(ICON_FA_SORT_UP   " Octave Up", "Ctrl+PageUp")) { transposeSelection(state, 12); }
            if (ImGui::MenuItem(ICON_FA_SORT_DOWN " Octave Down", "Ctrl+PageDown")) { transposeSelection(state, -12); }
            ImGui::Separator();
            if (ImGui::MenuItem(ICON_FA_ARROW_UP   " Semitone Up", "Ctrl+Up")) { transposeSelection(state, 1); }
            if (ImGui::MenuItem(ICON_FA_ARROW_DOWN " Semitone Down", "Ctrl+Down")) { transposeSelection(state, -1); }
            ImGui::EndMenu(); // End Transpose
        }
        if (ImGui::BeginMenu(ICON_FA_MUSIC " Change Instrument")) {
            char buff[256];

            if ( mCurrentInstrumentId <= getMain()->getController()->getSoundBank().size())
            {
                snprintf(buff, sizeof(buff), "Selected: %02X %s", mCurrentInstrumentId, getMain()->getController()->getSoundBank()[mCurrentInstrumentId].name.c_str());
                if (ImGui::MenuItem(buff, "")) { setInstrumentSelection(state, mCurrentInstrumentId); }
            }


            if (ImGui::BeginMenu("Instruments")) {
                for ( int i = 0; i < getMain()->getController()->getSoundBank().size(); i++ )
                {
                    snprintf(buff, sizeof(buff), "%02X %s", i, getMain()->getController()->getSoundBank()[i].name.c_str());
                    if (ImGui::MenuItem(buff, "")) { setInstrumentSelection(state, i); }
                }
                ImGui::EndMenu(); // Instruments
            }


            ImGui::EndMenu(); // Instrument
        }


        ImGui::Separator();
        if (ImGui::MenuItem(ICON_FA_COPY " Copy", "Ctrl+C | Ctrl+INSERT")) {
            copyStepsToClipboard(state, mPatternClipBoard);
        }
        if (ImGui::MenuItem(ICON_FA_PASTE " Paste", "Ctrl+V | Shift+INSERT")) {
            pasteStepsFromClipboard(state, mPatternClipBoard, true);
        }

        ImGui::Separator();

        if (ImGui::MenuItem(ICON_FA_DELETE_LEFT " Delete (clear steps)", "Del")) {
            clearSelectedSteps(state);
        }


        if (ImGui::MenuItem(ICON_FA_HAND_SCISSORS " Cut", "Ctrl+X")) {
            copyStepsToClipboard(state, mPatternClipBoard);
            clearSelectedSteps(state);
            state.selection.init(); // Clear selection after cut

        }

        if (ImGui::MenuItem(ICON_FA_ARROW_UP " Delete and shift up", "Ctrl+Delete")) {
            deleteAndShiftDataUp(state);
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
        bool altHeld = ImGui::GetIO().KeyAlt;


        // 2. Determine if we are moving the cursor this frame
        int oldRow = state.cursorRow;
        int oldCol = state.cursorCol;
        bool moved = false;
        if (!ctrlHeld && !altHeld)
        {
            if (ImGui::IsKeyPressed(ImGuiKey_UpArrow))    { state.moveCursorPosition(-1, 0);  moved = true; }
            if (ImGui::IsKeyPressed(ImGuiKey_DownArrow))  { state.moveCursorPosition( 1, 0);  moved = true; }
            if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow))  { state.moveCursorPosition( 0,-1);  moved = true; }
            if (ImGui::IsKeyPressed(ImGuiKey_RightArrow)) { state.moveCursorPosition( 0, 1);  moved = true; }
            if (ImGui::IsKeyPressed(ImGuiKey_PageUp))     { state.moveCursorPosition(-16, 0); moved = true; }
            if (ImGui::IsKeyPressed(ImGuiKey_PageDown))   { state.moveCursorPosition( 16, 0); moved = true; }
            if (ImGui::IsKeyPressed(ImGuiKey_Home))       { state.moveCursorPosition(-10000, 0); moved = true; }
            if (ImGui::IsKeyPressed(ImGuiKey_End))        { state.moveCursorPosition( 10000, 0); moved = true; }
        }

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
                //state.selection.init();
                // i set a single selected
                state.selection.active = true;
                state.selection.startPoint[0] = state.selection.endPoint[0] = state.cursorRow;
                state.selection.startPoint[1] = state.selection.endPoint[1] = state.cursorCol;

            }
        }

        // Other actions...
        if (ImGui::IsKeyPressed(ImGuiKey_Space)) state.pattern->getStep(state.cursorRow, state.cursorCol).note = opl3::STOP_NOTE;
        if (ImGui::IsKeyPressed(ImGuiKey_Delete)) clearSelectedSteps(state);

        if (ctrlHeld && ImGui::IsKeyPressed(ImGuiKey_UpArrow)) transposeSelection(state, +1);
        else
        if (ctrlHeld && ImGui::IsKeyPressed(ImGuiKey_DownArrow)) transposeSelection(state, -1);
        else
        if (ctrlHeld && ImGui::IsKeyPressed(ImGuiKey_PageUp)) transposeSelection(state, +12);
        else
        if (ctrlHeld && ImGui::IsKeyPressed(ImGuiKey_PageDown)) transposeSelection(state, -12);
        else
        if (ctrlHeld && ImGui::IsKeyPressed(ImGuiKey_C)) copyStepsToClipboard(state, mPatternClipBoard);
        else
        if (ctrlHeld && ImGui::IsKeyPressed(ImGuiKey_Insert)) copyStepsToClipboard(state, mPatternClipBoard);
        else
        if (ctrlHeld && ImGui::IsKeyPressed(ImGuiKey_V)) pasteStepsFromClipboard(state, mPatternClipBoard);
        else
        if (shiftHeld && ImGui::IsKeyPressed(ImGuiKey_Insert)) pasteStepsFromClipboard(state, mPatternClipBoard);
        else
        if (ctrlHeld && ImGui::IsKeyPressed(ImGuiKey_X)) {
            copyStepsToClipboard(state, mPatternClipBoard);
            clearSelectedSteps(state);
            state.selection.init(); // Clear selection after cut
        }
        else
        if (ctrlHeld && ImGui::IsKeyPressed(ImGuiKey_A)) selectPatternAll(state);
        else
        if (ImGui::IsKeyPressed(ImGuiKey_Insert)) insertBlanksAndshiftDataDown(state);
        else
        if (ctrlHeld && ImGui::IsKeyPressed(ImGuiKey_Delete)) deleteAndShiftDataUp(state);

        //-------- other actions
        else
        if (ImGui::IsKeyPressed(ImGuiKey_F1))  {if (isPlaying()) stopSong(); else playSong();}

    }
}

//------------------------------------------------------------------------------
void SequencerGui::DrawPatternEditor( PatternEditorState& state) {
    if (!state.pattern)
        return;

    const int numRows = (int)state.pattern->getRowCount();

    cellSize.x = mSettings.EnhancedStepView ? 105.f : 50.f;

    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && !ImGui::IsAnyItemActive()) {
        ImGui::SetKeyboardFocusHere();
    }

    // Style
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1));
    // InvisibleButton add some space set cellPadding to 0!
    ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(0, 0));

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
                    , col+1
                    // , getMain()->getController()->getInstrumentName(lIdx).c_str()
                    , col
                    );



            ImGui::TableSetupColumn(tmpbuf, ImGuiTableColumnFlags_WidthFixed, cellSize.x);

        }
        // ImGui::TableHeadersRow();

        // ------------- custom draw header
        ImGui::TableNextRow(ImGuiTableRowFlags_Headers, 20.f);
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


            ImGui::TableHeader(lColCaption.c_str());


            // Pulse selected
            if (channel == state.cursorCol) {
                ImDrawList* dl = ImGui::GetWindowDrawList();
                ImVec2 posMin = ImGui::GetItemRectMin();
                ImVec2 posMax = ImGui::GetItemRectMax();
                float pulse = (sinf((float)ImGui::GetTime() * 10.0f) * 0.5f) + 0.5f;
                // ImU32 pulseCol = ImGui::GetColorU32(ImVec4(1, 1, 0, 0.4f + pulse * 0.4f)); // Yellowish pulse
                ImU32 pulseCol = ImGui::GetColorU32(ImVec4(0.4, 0.4, 0.9, 0.4f + pulse * 0.4f));
                dl->AddRect(posMin, posMax, pulseCol, 0.0f, 0, 2.5f);
            }


            // ImGui::PopStyleColor();

            // --------------- Header Popup // CLICK
            if (channel >= 0)
            {
                if (ImGui::IsItemClicked()) {
                    state.cursorCol = channel;
                    selectPatternCol(state);
                }

                if (ImGui::BeginPopupContextItem())
                {
                    ImGui::TextColored(ImColor4F(cl_Crimson), "Channel %d", channel + 1);
                    ImGui::Separator();
                    if (channel < 6) {
                        ImGui::TextColored(ImColor4F(cl_Blue), "Four Operator channel");
                    } else {
                        ImGui::TextColored(ImColor4F(cl_Yellow), "Two Operator channel");
                    }

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
                if ( lDoScroll
                    && row == lScrolltoRow
                    //&& lScrolltoRow > clipper.DisplayEnd - 3
                    )
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
                        ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, Color4FIm({0.12f,0.12f,0.12f,1.f}));
                    else
                        ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, Color4FIm(cl_Black));

                    // ImGui::TextDisabled("%03d", row);
                    // if (ImGui::IsItemClicked()) {
                    //     state.cursorRow = row;
                    //     selectPatternRow(state);
                    // }

                    char buf[4];
                    snprintf(buf, sizeof(buf), "%03d", row);
                    // Get cell dimensions
                    ImVec2 pos = ImGui::GetCursorScreenPos();
                    ImVec2 size = ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetFrameHeight());
                    if (ImGui::InvisibleButton("##cell", size)) {
                        state.cursorRow = row;
                        selectPatternRow(state);
                    }
                    ImVec2 textSize = ImGui::CalcTextSize(buf);
                    ImVec2 textPos = ImVec2(pos.x + (size.x - textSize.x) * 0.5f, pos.y + (size.y - textSize.y) * 0.5f);
                    ImGui::GetWindowDrawList()->AddText(textPos, ImGui::GetColorU32(ImGuiCol_TextDisabled), buf);

                    // std::string tmpStr = std::format( "{:03d}", row);
                    // // ImGui::Button(tmpStr.c_str(), ImVec2(0,cellSize.y));
                    // // if (ImFlux::FaderButton(tmpStr.c_str(), ImVec2(30,cellSize.y - 2))) {
                    // if (ImGui::InvisibleButton(tmpStr.c_str(), ImVec2(0,cellSize.y))) {
                    //     state.cursorRow = row;
                    //     selectPatternRow(state);
                    // }

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
    ImGui::PopStyleVar(/*ImGuiStyleVar_CellPadding, ImVec2(0, 0)*/);
    ImGui::PopStyleVar();
}
//------------------------------------------------------------------------------
//FIXME TO HEADER
// FIXME unfinished !!!
std::string GetStepText(SongStep& step, bool enhanced)
{


    std::string result ="";
    //1. Note
    result += opl3::ValueToNote(step.note);

    if ( step.note <= LAST_NOTE )
        result += std::format(" {:02X}", step.instrument);
    else
        result += "   ";

    if ( enhanced ) {
        if ( step.volume > 63 )
            result += "  ";
        else
            result += std::format("{:02d}", step.volume);


    }


    return result;

}

//------------------------------------------------------------------------------
void SequencerGui::DrawStepCell(opl3::SongStep& step, bool isSelected, int row, int col, PatternEditorState& state) {

    ImGui::PushID(row * 1000 + col);

    // Construct the tracker-style string: "C-4 01 v63 A0F"
    std::string noteStr = opl3::ValueToNote(step.note);

    char hintBuffer[256];

    std::string insName = getMain()->getController()->getInstrumentName(step.instrument);


    //TODO nicer hint:
    snprintf(hintBuffer, sizeof(hintBuffer), "%s\n%s (%02X)\nVol:%02d\nEffect Type:%d\nEffectValue:%02X",
             noteStr.c_str(),
             insName.c_str(),step.instrument,
             step.volume,
             step.effectType,
             step.effectVal);

    //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 pos = ImGui::GetCursorScreenPos();

    //FIXME
    ImGui::InvisibleButton("##hitbox", cellSize);

    if (isSelected) {
        // drawList->AddRectFilled(pos, ImVec2(pos.x + cellSize.x, pos.y + cellSize.y)
        const float rectadd = 0.f;
        drawList->AddRectFilled(
            ImVec2(pos.x - rectadd, pos.y - rectadd)
          , ImVec2(pos.x + cellSize.x + rectadd, pos.y + cellSize.y + rectadd)
          , Color4FIm(cl_Blue));
    }

    // Note
    float offsetX = 4.0f; // Padding
    float centerY;

    if (mSettings.EnhancedStepView) {
        centerY = (cellSize.y - ImGui::GetFontSize()) * 0.5f + 3.f; //add Y for instrument display
    } else {
        centerY = (cellSize.y - ImGui::GetFontSize()) * 0.5f;
    }

    drawList->AddText(ImVec2(pos.x + offsetX, pos.y + centerY),
                        Color4FIm(cl_White), noteStr.c_str());

    offsetX += ImGui::CalcTextSize(noteStr.c_str()).x;

    // instrument
    if ( step.note <= LAST_NOTE )
    {
        if (mSettings.EnhancedStepView)
        {
            ImGui::PushFont(mTinyFont);
            drawList->AddText(ImVec2(pos.x + 4.f/*+ offsetX*/, pos.y - 1.f /*+ centerY*/),
                              getInstrumentColor(step.instrument),
                              std::format("{}",insName.substr(0, std::min<size_t>(insName.size(), 20))).c_str()
            );


            ImGui::PopFont(/*mTinyFont*/);
        }

        drawList->AddText(ImVec2(pos.x + offsetX, pos.y + centerY),
                          Color4FIm(cl_Sand),
                          std::format(" {:02X}", step.instrument).c_str());

    }
    offsetX += ImGui::CalcTextSize("   ").x;

    if (mSettings.EnhancedStepView)
    {
        std::string tmpStr = "";
        if ( step.volume > 63)
            tmpStr = "    ";
        else if ( step.volume == 63 )
            tmpStr = " .. ";
        else
            tmpStr = std::format(" {:02d} ", step.volume);

        drawList->AddText(ImVec2(pos.x + offsetX, pos.y + centerY),
                          Color4FIm({0.8f,0.5f,0.5f}),
                          tmpStr.c_str());

        offsetX += ImGui::CalcTextSize(tmpStr.c_str()).x;

        //effects
        if (step.effectType > 0)
        {
            tmpStr = std::format("{:X} ",step.effectType);
            drawList->AddText(ImVec2(pos.x + offsetX, pos.y + centerY),
                              Color4FIm({0.5f,0.5f,1.f}),
                              tmpStr.c_str());
            offsetX += ImGui::CalcTextSize(tmpStr.c_str()).x;
            tmpStr = std::format("{:02X} ",step.effectVal);
            drawList->AddText(ImVec2(pos.x + offsetX, pos.y + centerY),
                              Color4FIm({0.8f,0.4f,0.8f}),
                              tmpStr.c_str());

        }
    }






    //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
    // char buf[32];
    // if (mSettings.EnhancedStepView)
    // {
    //
    //     if (step.note >= LAST_NOTE && step.effectVal == 0 && step.volume >= 63)
    //     {
    //         snprintf(buf, sizeof(buf), "...");
    //
    //     } else {
    //         snprintf(buf, sizeof(buf), "%s %02X %02d %dv%02X",
    //                  noteStr.c_str(),
    //                  step.instrument,
    //                  step.volume,
    //                  step.effectType,
    //                  step.effectVal);
    //     }
    //
    //     snprintf(buf, sizeof(buf),"%s", GetStepText(step,true).c_str());
    //
    // } else {
    //     if (step.note < LAST_NOTE)
    //         snprintf(buf, sizeof(buf), "%s %d",noteStr.c_str(), step.instrument);
    //     else
    //         snprintf(buf, sizeof(buf), "%s ",noteStr.c_str());
    // }
    // // Highlight if this is the active selection/cursor
    // if (isSelected /*&& !state.scrollToSelected*/) {
    //     // ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImGui::GetColorU32(ImGuiCol_HeaderActive));
    //     ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, Color4FIm(cl_Blue));
    //
    // }
    // // A. Render the Selectable purely for the visual feedback/hitbox
    // ImGui::Selectable(buf, isSelected, ImGuiSelectableFlags_AllowOverlap, cellSize);

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


    // + button >>>>>>>>>>

    // if (ImGui::SmallButton("[+]"))
    // if (ImFlux::FaderButton(ICON_FA_PLUS, ImVec2(24.f, 24.f)))

    // ImGui::PushFont(mIconFont);
    ImFlux::ButtonParams bparams = ImFlux::DEFAULT_BUTTON;
    bparams.color = Color4FImU32(cl_Slate);
    bparams.rounding = 4.f;
    bparams.size = ImVec2(24,24);
    bparams.mouseOverEffect = ImFlux::BUTTON_MO_GLOW;
    // if (ImFlux::ColoredButton(ICON_FA_PLUS,Color4FImU32(cl_Brown), ImVec2(124.f, 124.f)))
    if (ImFlux::ButtonFancy(ICON_FA_PLUS, bparams))
    {
        sShowNewPatternPopup = true;
        ImGui::OpenPopup("New Pattern Configuration");
    }
    // ImGui::PopFont();

    //<<<< fancy button :P


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

        if (ImGui::BeginTabBar("PatternTabs", ImGuiTabBarFlags_AutoSelectNewTabs /*| ImGuiTabBarFlags_Reorderable*/)) {
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

                    //FIXME USE ICONS !
                    const int tmpWidth = 180;
                    ImGui::Spacing();
                    ImGui::SetCursorPosX((ImGui::GetWindowSize().x - tmpWidth) * 0.5f);
                    if (isPlaying())
                    {
                        if (ImGui::Button("Stop", ImVec2(tmpWidth, 0))) {
                            stopSong();
                        }
                    } else {
                        if (ImGui::Button("Play Pattern", ImVec2(tmpWidth, 0))) {
                            playSelected(state, true);
                        }
                    }
                    ImGui::Spacing();
                    ImGui::SetCursorPosX((ImGui::GetWindowSize().x - tmpWidth) * 0.5f);
                    //FIXME temp ?!
                    if (ImGui::Button("Append to Orders", ImVec2(tmpWidth, 0))) {
                        mCurrentSong.orderList.push_back(lPatternIndex);
                    }

                    ImGui::Spacing();
                    ImGui::SetCursorPosX((ImGui::GetWindowSize().x - 180) * 0.5f);
                    if (ImGui::Button("Clone", ImVec2(tmpWidth, 0))) {
                        Pattern clonePat = lPat;
                        clonePat.mName += " (Copy)";
                        song.patterns.push_back(std::move(clonePat));
                    }

                    ImGui::Spacing();
                    ImGui::Separator();

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

            uint8_t newPatternIdx = (uint8_t)song.patterns.size();
            song.patterns.push_back(std::move(p));
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
