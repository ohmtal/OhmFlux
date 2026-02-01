#include "sequencerGui.h"
#include "sequencerMain.h"
#include <imgui_internal.h>

#include <algorithm>
#include <string>
#include <cctype>
#include <src/fonts/IconsFontAwesome6.h>
#include <OPL3Instruments.h>

//------------------------------------------------------------------------------
void SequencerGui::ShowSoundBankWindow()
{
    // if (!mGuiSettings.mShowSoundBankEditor) return;
    // ImGui::SetNextWindowSize(ImVec2(600, 600), ImGuiCond_FirstUseEver);
    // if (!ImGui::Begin("Sound Bank", &mGuiSettings.mShowSoundBankEditor))
    // {
    //     ImGui::End();
    //     return;
    // }
    //
    // if (ImGui::BeginPopupContextItem())
    // {
    //     if (ImGui::MenuItem("Close"))
    //         mGuiSettings.mShowSoundBankEditor = false;
    //     ImGui::EndPopup();
    // }
    //
    // ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 12.0f);
    //
    //
    // // Outer table with 2 columns
    // if (ImGui::BeginTable("InstrumentLayoutTable", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerV)) {
    //
    //     // --- COLUMN 1: Sidebar (List) ---
    //     ImGui::TableSetupColumn("Sidebar", ImGuiTableColumnFlags_WidthFixed, 150.0f);
    //     // --- COLUMN 2: Content (Editor & Scale Player) ---
    //     ImGui::TableSetupColumn("Content", ImGuiTableColumnFlags_WidthStretch);
    //
    //     ImGui::TableNextRow();
    //
    //     // Left Side
    //     ImGui::TableSetColumnIndex(0);
    //     if (ImGui::BeginChild("ListRegion")) {
    //         RenderInstrumentListUI();
    //     }
    //     ImGui::EndChild();
    //
    //     // Right Side
    //     ImGui::TableSetColumnIndex(1);
    //     if (ImGui::BeginChild("ContentRegion")) {
    //
    //         // Top: Instrument Editor
    //         RenderInstrumentEditorUI();
    //
    //         ImGui::Spacing();
    //         ImGui::Separator();
    //         ImGui::Spacing();
    //
    //         // Bottom: Scale Player
    //         // RenderScalePlayerUI();
    //     }
    //     ImGui::EndChild();
    //
    //     ImGui::EndTable();
    // }
    //
    // // RenderScalePlayerUI(true);
    //
    // ImGui::PopStyleVar();
    //
    // ImGui::End();

}
//------------------------------------------------------------------------------
void RenderInsListButtons(OPL3Controller* controller)
{
    ImFlux::ButtonParams bp = ImFlux::SLATE_BUTTON.WithSize(ImVec2(32.f,32.f));

    if (ImFlux::ButtonFancy(ICON_FA_ROTATE_LEFT "##Reset", bp )){
        controller->initDefaultBank();
    }
    ImFlux::Hint("Reset the default Sound Bank ");

    ImGui::SameLine();
    if (ImFlux::ButtonFancy(ICON_FA_PLUS "##AppendNewIns", bp )){
        //FIXME menu to select a default !!!!!
        opl3::Instrument newIns = OPL3InstrumentPresets::GetMelodicDefault(0);
        newIns.name = "New Instrument";
        controller->getSoundBank().push_back(newIns);
    }
    ImFlux::Hint("Append a new Instrument.");


    ImGui::SameLine();
    if (ImFlux::ButtonFancy(ICON_FA_FOLDER_OPEN "##Import Bank", bp)){
        g_FileDialog.setFileName("");
        g_FileDialog.mSaveMode = false;
        g_FileDialog.mSaveExt = "";
        g_FileDialog.mLabel = "Import Soundbank";
        g_FileDialog.mFilters = {".fms3", ".fms", ".op2", ".wopl"};
        g_FileDialog.mUserData = "ImportBank";
        g_FileDialog.mDirty  = true;

    }
    if (ImGui::IsItemHovered()) ImGui::SetItemTooltip("Import Bank");

    ImGui::SameLine();
    if (ImFlux::ButtonFancy(ICON_FA_FLOPPY_DISK "##SaveExportBank", bp)) {
        ImGui::OpenPopup("Popup_SaveBankMenu");
    }
    if (ImGui::IsItemHovered()) ImGui::SetItemTooltip("Save (fmb3) or Export (wopl) Bank ");

    if (ImGui::BeginPopup("Popup_SaveBankMenu")) {

        ImGui::TextDisabled("Save or Export Soundbank");
        ImGui::Separator();

        // 1. Save Soundbank (fmb3)
        if (ImGui::MenuItem("Save Soundbank (.fmb3)")) {
            g_FileDialog.setFileName("");
            g_FileDialog.mSaveMode = true;
            g_FileDialog.mSaveExt = ".fmb3";
            g_FileDialog.mLabel = "Save Soundbank";
            g_FileDialog.mFilters = {".fmb3"};
            g_FileDialog.mUserData = "SaveBankInternal";
            g_FileDialog.mDirty = true;
        }

        // 2. Export SoundBank (wopl)
        if (ImGui::MenuItem("Export Soundbank (.wopl)")) {
            g_FileDialog.setFileName("");
            g_FileDialog.mSaveMode = true;
            g_FileDialog.mSaveExt = ".wopl";
            g_FileDialog.mLabel = "Export Soundbank";
            g_FileDialog.mFilters = {".wopl"};
            g_FileDialog.mUserData = "ExportBankWOPL";
            g_FileDialog.mDirty = true;
        }

        ImGui::EndPopup();
    }



}

//FIXME menu
// if (ImGui::MenuItem("Rename Instrument")) {
//     showMessageNotImplemented("Rename Instrument");
// }
// if (ImGui::MenuItem("Save Instrument")) {
//     showMessageNotImplemented("Insert Instrument");
// }
//
// if (ImGui::MenuItem("Insert Instrument")) {
//     showMessageNotImplemented("Insert Instrument");
// }
// if (ImGui::MenuItem("Replace Instrument")) {
//     //load ....
//     showMessageNotImplemented("Replace Instrument");
// }
// if (ImGui::MenuItem("Delete Instrument")) {
//     showMessageNotImplemented("Delete Instrument");
// }

//------------------------------------------------------------------------------
void SequencerGui::RenderInstrumentListUI(bool standAlone) {

    if (standAlone) {
        ImGui::SetNextWindowSize(ImVec2(200, 600), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSizeConstraints(ImVec2(100.0f, 100.0f), ImVec2(FLT_MAX, FLT_MAX));
        if (!ImGui::Begin("Sound Bank")) //, nullptr, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoScrollWithMouse))
        { ImGui::End(); return; }
    }


    // default size
    ImVec2 controlSize = {0,0};
    // magic pointer movement
    static int move_from = -1, move_to = -1;
    int delete_idx = -1, insert_idx = -1;
    // init pointers
    OPL3Controller* controller =  getMain()->getController();
    std::vector<Instrument>& soundBank = controller->getSoundBank();


    // --- Compact Header ---
    RenderInsListButtons(controller);
    ImGui::Separator();

    // Start a child region for scrolling if the list gets long
    ImGui::BeginChild("OrderListScroll", controlSize, false);

    for (int instIdx = 0; instIdx < (int)soundBank.size(); instIdx++) {
        const bool is_selected = (mCurrentInstrumentId == instIdx);
        ImGui::PushID(instIdx);

        // 1. Draw Index
        ImGui::AlignTextToFramePadding();
        // ImGui::TextDisabled("%02d", instIdx);
        ImGui::TextDisabled("%02X", instIdx);
        ImGui::SameLine();

        // 2. Button Dimensions & Interaction
        ImVec2 pos = ImGui::GetCursorScreenPos();
        ImVec2 size = ImVec2(ImGui::GetContentRegionAvail().x - 10.0f, ImGui::GetFrameHeight());


        float coloredWidth = 30.0f;
        ImVec2 coloredSize(coloredWidth, size.y);


        // InvisibleButton acts as the interaction hit-box for DragDrop and Clicks
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
        bool pressed = ImGui::InvisibleButton("inst_btn", size);
        if (pressed) mCurrentInstrumentId = instIdx;

        bool is_hovered = ImGui::IsItemHovered();
        bool is_active = ImGui::IsItemActive();
        ImDrawList* draw_list = ImGui::GetWindowDrawList();

        // ----------- rendering

        // 1. Logic for enhanced selection visibility
        ImU32 col32 = getInstrumentColor(instIdx);
        ImU32 colMiddle32 =  IM_COL32(20, 20, 20, 255);
        ImU32 border_col = IM_COL32_WHITE;
        float border_thickness = 1.0f;

        if (is_selected) {
            // Make the background much brighter or more saturated
            // We force Alpha to 255 to make it solid
            col32 = (col32 & 0x00FFFFFF) | 0xFF000000;

            // Use a thick, high-contrast border (e.g., Bright Yellow or Cyan)
            border_col = IM_COL32(255, 255, 0, 255); // Neon Yellow
            border_thickness = 2.0f;

            colMiddle32 = IM_COL32(40, 40, 40, 255);

        } else {
            // Dim unselected items significantly
            col32 = (col32 & 0x00FFFFFF) | 0x66000000;
        }

        // 2. Draw Background
        // draw_list->AddRectFilled(pos, pos + size, col32, 3.0f);

        // left
        draw_list->AddRectFilled(pos, pos + coloredSize, col32, 3.0f);

        // Middle part: Starts after left bar, width is (total - 2 * side bars)
        ImVec2 midPos = ImVec2(pos.x + coloredWidth, pos.y);
        ImVec2 midSize = ImVec2(size.x - (2.0f * coloredWidth), size.y);
        draw_list->AddRectFilled(midPos, midPos + midSize, colMiddle32, 0.0f); // No rounding for middle to avoid gaps

        // Right part: Starts at the end minus the side bar width
        ImVec2 rightPos = ImVec2(pos.x + size.x - coloredWidth, pos.y);
        draw_list->AddRectFilled(rightPos, rightPos + coloredSize, col32, 3.0f);


        // 3. Selection "Glow" / Outline
        if (is_selected) {
            // Outer Glow Effect: Draw a slightly larger, transparent rect behind/around
             // draw_list->AddRect(pos - ImVec2(2, 2), pos + size + ImVec2(2, 2),
             //                    IM_COL32(255, 255, 0, 100), 3.0f, 0, 4.0f);



            // Solid Inner Border
            draw_list->AddRect(pos, pos + size, border_col, 3.0f, 0, border_thickness);

            // OPTIONAL: Add a small white "active" indicator circle on the left
            draw_list->AddCircleFilled(ImVec2(pos.x - 5, pos.y + size.y * 0.5f), 3.0f, border_col);
        } else if (is_hovered) {
            draw_list->AddRect(pos, pos + size, IM_COL32(255, 255, 255, 180), 3.0f);
        }

        // 4. Text Contrast
        // For the selected item, use Black text if the background is very bright
        // (or stay with White+Shadow for consistency)

        const char* instName = soundBank[instIdx].name.c_str();
        ImVec2 text_size = ImGui::CalcTextSize(instName);
        ImVec2 text_pos = ImVec2(pos.x + (size.x - text_size.x) * 0.5f, pos.y + (size.y - text_size.y) * 0.5f);

        ImU32 text_col = is_selected ? IM_COL32_WHITE : IM_COL32(200, 200, 200, 255);
        draw_list->AddText(text_pos + ImVec2(1, 1), IM_COL32(0, 0, 0, 255), instName);
        draw_list->AddText(text_pos, text_col, instName);




        // 6. Context Menu
        if (ImGui::BeginPopupContextItem("row_menu")) {
            if (ImGui::MenuItem("Insert Above")) insert_idx = instIdx;
            if (ImGui::MenuItem("Remove")) delete_idx = instIdx;
            ImGui::EndPopup();
        }

        // 7. Drag and Drop (Attached to the InvisibleButton)
        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
            ImGui::SetDragDropPayload("DND_ORDER", &instIdx, sizeof(int));
        ImGui::Text("Moving %s", instName);
        ImGui::EndDragDropSource();
        }
    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_ORDER")) {
            move_from = *(const int*)payload->Data;
            move_to = instIdx;
        }
        ImGui::EndDragDropTarget();
    }

    ImGui::PopStyleVar();
    ImGui::PopID();
    }

    // for (int instIdx = 0; instIdx < (int)soundBank.size(); instIdx++) {
    //     const bool is_selected = (mCurrentInstrumentId == instIdx);
    //
    //     ImGui::PushID(instIdx);
    //
    //     // 1. Draw Index
    //     ImGui::AlignTextToFramePadding();
    //     ImGui::TextDisabled("%02d", instIdx);
    //     ImGui::SameLine();
    //
    //     // 2. Custom Colored Button (The "Fancy" part)
    //     ImVec2 pos = ImGui::GetCursorScreenPos();
    //     ImVec2 size = ImVec2(ImGui::GetContentRegionAvail().x - 10, ImGui::GetFrameHeight());
    //
    //     ImU32 col32 = getInstrumentColor(instIdx);
    //
    //     // Invisible button to handle interaction
    //     ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
    //     if (ImGui::InvisibleButton("inst_btn", size)) {
    //         // Left click action
    //     }
    //
    //     // Background Drawing
    //     bool is_hovered = ImGui::IsItemHovered();
    //     bool is_active = ImGui::IsItemActive();
    //     float alpha = is_active ? 1.0f : (is_hovered ? 0.8f : 0.6f);
    //
    //     ImDrawList* draw_list = ImGui::GetWindowDrawList();
    //     draw_list->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), col32, 3.0f);
    //     if (is_hovered)
    //         draw_list->AddRect(pos, ImVec2(pos.x + size.x, pos.y + size.y), IM_COL32_WHITE, 3.0f);
    //
    //     // Center Text in Button
    //     ImVec2 text_size = ImGui::CalcTextSize(soundBank[instIdx].name.c_str());
    //     draw_list->AddText(ImVec2(pos.x + (size.x - text_size.x) * 0.5f, pos.y + (size.y - text_size.y) * 0.5f),
    //                        IM_COL32_BLACK, soundBank[instIdx].name.c_str());
    //     //shadow
    //     draw_list->AddText(ImVec2(pos.x + 1.f + (size.x - text_size.x) * 0.5f, pos.y+ 1.f + (size.y - text_size.y) * 0.5f),
    //                        IM_COL32_WHITE, soundBank[instIdx].name.c_str());
    //
    //     // 3. Actions via Context Menu (Right Click)
    //     if (ImGui::BeginPopupContextItem("row_menu")) {
    //         if (ImGui::MenuItem("Insert Above")) insert_idx = instIdx;
    //         if (ImGui::MenuItem("Remove")) delete_idx = instIdx;
    //         ImGui::Separator();
    //         ImGui::EndPopup();
    //     }
    //
    //     // 4. Drag and Drop Logic
    //     if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
    //         ImGui::SetDragDropPayload("DND_ORDER", &instIdx, sizeof(int));
    //         ImGui::Text("Moving %s", soundBank[instIdx].name.c_str());
    //         ImGui::EndDragDropSource();
    //     }
    //     if (ImGui::BeginDragDropTarget()) {
    //         if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_ORDER")) {
    //             move_from = *(const int*)payload->Data;
    //             move_to = instIdx;
    //         }
    //         ImGui::EndDragDropTarget();
    //     }
    //
    //     ImGui::PopStyleVar();
    //     ImGui::PopID();
    // } //FOR


    ImGui::EndChild();



    // Logic Execution (Deferred)
    if (delete_idx != -1) soundBank.erase(soundBank.begin() + delete_idx);
    if (insert_idx != -1) soundBank.insert(soundBank.begin() + insert_idx, soundBank[insert_idx]);
    if (move_from != -1 && move_to != -1) {
        auto it_f = soundBank.begin() + move_from;
        auto it_t = soundBank.begin() + move_to;
        if (move_from < move_to) std::rotate(it_f, it_f + 1, it_t + 1);
        else std::rotate(it_t, it_f, it_f + 1);
        move_from = move_to = -1;
    }


    if (standAlone) ImGui::End();

}



void SequencerGui::RenderInstrumentListUI_OLD(bool standAlone)
{
    if (standAlone)
    {
        ImGui::SetNextWindowSize(ImVec2(200, 600), ImGuiCond_FirstUseEver);
        ImGui::Begin("Instruments [OLD]");
    }

    if (ImGui::BeginChild("INSTRUMENT_Box", ImVec2(0, 0), ImGuiChildFlags_Borders)) {
        std::vector<opl3::Instrument>& bank = getMain()->getController()->getSoundBank();

        //Buttons
        RenderInsListButtons(getMain()->getController());
        ImGui::Separator();


        // List
        std::string instrumentCaption;

        if (ImGui::BeginListBox("##InstList", ImVec2(-FLT_MIN, -FLT_MIN))) {
            for (int n = 0; n < (int)bank.size(); n++) {
                const bool is_selected = (mCurrentInstrumentId == n);

                instrumentCaption = std::format("{:02X} {}", n, bank[n].name);

                if (ImGui::Selectable(instrumentCaption.c_str(), is_selected)) {
                    mCurrentInstrumentId = n;
                }

                if (ImGui::IsItemHovered()) {

                    if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                        Log("Using Instrument %d", mCurrentInstrumentId);
                        mTempSong = mOpl3Tests->createScaleSong(mCurrentInstrumentId);
                        getMain()->getController()->playSong(mTempSong);
                    }
                    ImGui::SetTooltip("#%d %s\nFour-OP:%s", n, instrumentCaption.c_str(), bank[n].isFourOp ? "Yes" : "No");
                }

                if (is_selected) {
                    ImGui::SetItemDefaultFocus();

                    if (ImGui::BeginPopupContextItem("##InstListInstrumentPopup")) {
                        ImGui::TextColored(Color4FIm(cl_SkyBlue), "%s", instrumentCaption.c_str());
                        ImGui::Separator();
                        ImGui::EndPopup();
                    }
                }



            }
            ImGui::EndListBox();
        }
    }
    ImGui::EndChild();

    if (standAlone) ImGui::End();
}
//------------------------------------------------------------------------------
uint8_t* GetValPtr(opl3::Instrument::OpPair::OpParams& op, int metaIdx)
{
        // Mapping metadata index to struct members
        switch (metaIdx) {
            case 0:  return  &op.multi;   break;
            case 1:  return  &op.tl;      break;
            case 2:  return  &op.attack;  break;
            case 3:  return  &op.decay;   break;
            case 4:  return  &op.sustain; break;
            case 5:  return  &op.release; break;
            case 6:  return  &op.wave;    break;
            case 7:  return  &op.ksr;     break;
            case 8:  return  &op.egTyp;   break;
            case 9:  return  &op.vib;     break;
            case 10: return  &op.am;      break;
            case 11: return  &op.ksl;     break;
        }
        return nullptr;
}

void SequencerGui::RenderOpParam(const opl3::ParamMeta& meta, opl3::Instrument::OpPair::OpParams& op, int metaIdx) {
    uint8_t* val = GetValPtr(op, metaIdx);
    if (!val) return;

    ImGui::PushID(metaIdx);

    // UI Logic: Differentiate between Switches and Values
    if (meta.maxValue == 1) {
        bool bVal = (*val != 0);
        // Use a SmallButton-style toggle or Checkbox
        if (ImGui::Checkbox(meta.name.c_str(), &bVal)) {
            *val = bVal ? 1 : 0;
        }
    } else {
        // --- The Slider Logic ---
        ImGui::BeginGroup();
        ImGui::TextDisabled("%s", meta.name.c_str()); // Smaller label above
        ImGui::SetNextItemWidth(-FLT_MIN);

        // Use a DragScalar instead of a Slider for a cleaner, "knob-like" feel
        // This is more space-efficient in tight grids
        static const uint8_t zero = 0;
        // ImGui::DragScalar("##v", ImGuiDataType_U8, val, 0.2f, &zero, &meta.maxValue, "%d");
        // ImGui::SliderScalar("##Feedback", ImGuiDataType_U8, &pair.feedback, &minFeed, &maxFeed);
        ImGui::SliderScalar("##v", ImGuiDataType_U8, val, &zero, &meta.maxValue, "%d");


        // Tooltip showing the raw value + description
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("%s: %d / %d", meta.name.c_str(), *val, meta.maxValue);
        }
        ImGui::EndGroup();
    }

    ImGui::PopID();
}

//------------------------------------------------------------------------------
// TODO unused ...
void DrawPanningMeter(ImVec2 size, uint8_t panValue) {
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 p = ImGui::GetCursorScreenPos();

    // 1. Draw Background Track
    float trackHeight = 4.0f;
    ImVec2 trackStart = {p.x, p.y + (size.y - trackHeight) * 0.5f};
    ImVec2 trackEnd = {p.x + size.x, trackStart.y + trackHeight};
    drawList->AddRectFilled(trackStart, trackEnd, ImColor(40, 40, 40, 255), 2.0f);

    // 2. Calculate Position (Map 0-64 to 0.0-1.0)
    float panPos = std::clamp(panValue / 64.0f, 0.0f, 1.0f);
    float indicatorX = p.x + (panPos * size.x);

    // 3. Draw Indicator "Dot"
    // Center is 32, so we can color-code the dot
    ImU32 dotColor = (panValue == 32) ? ImColor(255, 255, 255) : ImColor(0, 255, 200);
    drawList->AddCircleFilled({indicatorX, p.y + size.y * 0.5f}, 5.0f, dotColor);

    // 4. Labels L | R
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
    drawList->AddText({p.x, p.y}, ImGui::GetColorU32(ImGuiCol_Text), "L");
    drawList->AddText({p.x + size.x - 10, p.y}, ImGui::GetColorU32(ImGuiCol_Text), "R");
    ImGui::PopStyleColor();

    ImGui::Dummy(size);
}

//------------------------------------------------------------------------------
void SequencerGui::DrawOperatorGrid(opl3::Instrument::OpPair::OpParams& op) {
    const auto& metas = OPL_OP_METADATA;

    ImGui::PushID(&op);

    if (ImGui::BeginTable("OpParams", 2, ImGuiTableFlags_SizingStretchSame)) {

        // --- Row 1: Volume & Freq ---
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0); RenderOpParam(metas[1], op, 1); // TL (Volume)
        ImGui::TableSetColumnIndex(1); RenderOpParam(metas[0], op, 0); // MULTI (Freq)

        // --- Row 2: ADSR ---
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        RenderOpParam(metas[2], op, 2); // Attack
        RenderOpParam(metas[3], op, 3); // Decay
        ImGui::TableSetColumnIndex(1);
        RenderOpParam(metas[4], op, 4); // Sustain
        RenderOpParam(metas[5], op, 5); // Release


        ImGui::EndTable();
        // <<<<<<<<< table ends here ....

        // -------- FLAGS in a new Table : ----------
        if (ImGui::BeginTable("FlagsTable", 3, ImGuiTableFlags_NoSavedSettings)) {
            ImGui::TableSetupColumn("Vib", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("AM", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("KSR", ImGuiTableColumnFlags_WidthStretch);

            ImGui::TableNextRow();

            ImGui::TableSetColumnIndex(0);
            RenderOpParam(metas[9], op, 9);  // VIB

            ImGui::TableSetColumnIndex(1);
            RenderOpParam(metas[10], op, 10); // AM

            ImGui::TableSetColumnIndex(2);
            RenderOpParam(metas[7], op, 7);  // KSR

            ImGui::EndTable();
        }


        // waaaaaaaaaavvvvve form :D


        ImGui::TextDisabled("WAVEFORM");
        float iconSize = 20.0f;
        float padding = 3.0f;
        float btnSize = iconSize + (padding * 2);

        for (int i = 0; i < 8; i++) {
            ImGui::PushID(i);

            bool isActive = (op.wave == i);
            if (isActive) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyle().Colors[ImGuiCol_ButtonActive]);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetStyle().Colors[ImGuiCol_ButtonActive]);
            } else {
                ImGui::PushStyleColor(ImGuiCol_Button, ImColor(30, 30, 30, 255).Value);
            }

            if (ImGui::Button("##w", ImVec2(btnSize, btnSize))) {
                op.wave = i;
            }

            // Draw the Icon Overlay
            ImVec2 m = ImGui::GetItemRectMin();
            DrawWaveformIcon(ImGui::GetWindowDrawList(), ImVec2(m.x + padding, m.y + padding), i, iconSize, ImColor(255, 255, 255));

            if (ImGui::IsItemHovered()) ImGui::SetTooltip("OPL3 Waveform %d", i);

            ImGui::PopStyleColor(isActive ? 2 : 1);

            // Wrap to 2 rows of 4 icons to fit perfectly in your table cell
            if (/*i != 3 &&*/ i != 7)ImGui::SameLine();
            ImGui::PopID();
        }


        static int lMidiNote = 60;
        float lVisualPulseVolume = 0.0f;

        SongStep lastStep = getMain()->getController()->getSequencerState().last_steps[0];
        if (getMain()->getController()->getSequencerState().ui_dirty)
        {
            if (lastStep.note < LAST_NOTE)
            {
                lMidiNote = lastStep.note;
                // dLog("[info] lMidiNote is %d", lMidiNote);
            }
            if ( getMain()->getController()->isPlaying() && lastStep.volume <= MAX_VOLUME) {
                float newVol = (float)lastStep.volume / (float)MAX_VOLUME;

                // Only "jump" if the new volume is higher (a new hit)
                if (newVol > lVisualPulseVolume) {
                    lVisualPulseVolume = newVol;
                }
            }



        }
        DrawADSRGraphBezier(ImVec2(ImGui::GetContentRegionAvail().x, 35), op, lMidiNote , lVisualPulseVolume);



    } //if MAIN TABLE ....
    ImGui::PopID();

}
//------------------------------------------------------------------------------
void SequencerGui::DrawWaveformIcon(ImDrawList* drawList, ImVec2 pos, int waveIdx, float size, ImU32 color) {
    float h = size * 0.5f;
    float midY = pos.y + h;
    float thickness = 1.5f;
    float s = size;

    switch (waveIdx) {
        case 0: // Sine
            drawList->AddBezierQuadratic(pos + ImVec2(0, h), pos + ImVec2(s*0.25f, 0), pos + ImVec2(s*0.5f, h), color, thickness);
            drawList->AddBezierQuadratic(pos + ImVec2(s*0.5f, h), pos + ImVec2(s*0.75f, s), pos + ImVec2(s, h), color, thickness);
            break;
        case 1: // Half-Sine
            drawList->AddBezierQuadratic(pos + ImVec2(0, h), pos + ImVec2(s*0.25f, 0), pos + ImVec2(s*0.5f, h), color, thickness);
            drawList->AddLine(pos + ImVec2(s*0.5f, h), pos + ImVec2(s, h), color, thickness);
            break;
        case 2: // Abs-Sine
            drawList->AddBezierQuadratic(pos + ImVec2(0, h), pos + ImVec2(s*0.25f, 0), pos + ImVec2(s*0.5f, h), color, thickness);
            drawList->AddBezierQuadratic(pos + ImVec2(s*0.5f, h), pos + ImVec2(s*0.75f, 0), pos + ImVec2(s, h), color, thickness);
            break;
        case 3: // Pulse-Sine
            drawList->AddBezierQuadratic(pos + ImVec2(0, h), pos + ImVec2(s*0.25f, 0), pos + ImVec2(s*0.35f, h), color, thickness);
            drawList->AddLine(pos + ImVec2(s*0.35f, h), pos + ImVec2(s, h), color, thickness);
            break;
        case 4: // Sine - even periods only (OPL3)
            drawList->AddBezierQuadratic(pos + ImVec2(0, h), pos + ImVec2(s*0.12f, 0), pos + ImVec2(s*0.25f, h), color, thickness);
            drawList->AddBezierQuadratic(pos + ImVec2(s*0.25f, h), pos + ImVec2(s*0.37f, s), pos + ImVec2(s*0.5f, h), color, thickness);
            drawList->AddLine(pos + ImVec2(s*0.5f, h), pos + ImVec2(s, h), color, thickness);
            break;
        case 5: // Abs-Sine - even periods only (OPL3)
            drawList->AddBezierQuadratic(pos + ImVec2(0, h), pos + ImVec2(s*0.12f, 0), pos + ImVec2(s*0.25f, h), color, thickness);
            drawList->AddBezierQuadratic(pos + ImVec2(s*0.25f, h), pos + ImVec2(s*0.37f, 0), pos + ImVec2(s*0.5f, h), color, thickness);
            drawList->AddLine(pos + ImVec2(s*0.5f, h), pos + ImVec2(s, h), color, thickness);
            break;
        case 6: // Square
            drawList->AddLine(pos + ImVec2(0, 0), pos + ImVec2(s*0.5f, 0), color, thickness);
            drawList->AddLine(pos + ImVec2(s*0.5f, 0), pos + ImVec2(s*0.5f, s), color, thickness);
            drawList->AddLine(pos + ImVec2(s*0.5f, s), pos + ImVec2(s, s), color, thickness);
            break;
        case 7: // Derived Square (Jagged)
            drawList->AddLine(pos + ImVec2(0, 0), pos + ImVec2(s*0.25f, 0), color, thickness);
            drawList->AddLine(pos + ImVec2(s*0.25f, 0), pos + ImVec2(s*0.35f, h), color, thickness);
            drawList->AddLine(pos + ImVec2(s*0.35f, h), pos + ImVec2(s*0.5f, s), color, thickness);
            drawList->AddLine(pos + ImVec2(s*0.5f, s), pos + ImVec2(s, s), color, thickness);
            break;
    }
}
//------------------------------------------------------------------------------
void SequencerGui::RenderWaveformSelector(uint8_t& waveVal) {
    ImGui::TextDisabled("Waveform");
    float iconSize = 24.0f;
    float padding = 4.0f;

    ImDrawList* drawList = ImGui::GetWindowDrawList();

    for (int i = 0; i < 8; i++) {
        ImGui::PushID(i);

        // Highlight the active waveform
        bool isActive = (waveVal == i);
        if (isActive) ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyle().Colors[ImGuiCol_ButtonActive]);
        else ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyle().Colors[ImGuiCol_Button]);

        if (ImGui::Button("##wave", ImVec2(iconSize + padding*2, iconSize + padding*2))) {
            waveVal = i;
        }

        // Draw the icon on top of the button
        ImVec2 p = ImGui::GetItemRectMin() + ImVec2(padding, padding);

        DrawWaveformIcon(drawList, p, i, iconSize, ImColor(255, 255, 255));

        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Waveform %d", i);

        ImGui::PopStyleColor();

        // Wrap after 4 icons to keep it compact in your operator grid
        if (i != 3 && i != 7) ImGui::SameLine();
        ImGui::PopID();
    }
}

//------------------------------------------------------------------------------
void SequencerGui::DrawADSRGraphBezier(ImVec2 size, const opl3::Instrument::OpPair::OpParams& op, int virtualNote, float pulseVol) {
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 startPos = ImGui::GetCursorScreenPos();

    // Background & Border
    drawList->AddRectFilled(startPos, {startPos.x + size.x, startPos.y + size.y}, ImColor(10, 10, 15), 4.0f);
    drawList->AddRect(startPos, {startPos.x + size.x, startPos.y + size.y}, ImColor(60, 60, 70), 4.0f);

    // KSR Calculation
    float ksrCompression = 1.0f;
    if (op.ksr != 0) {
        // High notes compress the time segments
        ksrCompression = 1.0f - (std::max(0, virtualNote - 24) / 160.0f);
    }

    auto applyKSR = [&](float val) {
        return ((15.0f - val) / 15.0f) * ksrCompression;
    };

    float attack  = applyKSR((float)op.attack);
    float decay   = applyKSR((float)op.decay);
    float sustain = (float)op.sustain / 15.0f;
    float release = applyKSR((float)op.release);

    float segmentW = size.x / 4.0f;
    float baseline = startPos.y + size.y - 5.0f;

    float topLimit = startPos.y + 5.0f;
    float bottomLimit = startPos.y + size.y - 5.0f;
    float peakY = topLimit + (10.0f * (1.0f - pulseVol));

    // --- Points ---
    ImVec2 pStart   = {startPos.x + 5.0f, baseline};

    float sustainY = peakY + (1.0f - sustain) * (bottomLimit - peakY);
    sustainY = std::clamp(sustainY, peakY, bottomLimit);

    ImVec2 pAttack  = {pStart.x + (attack * segmentW), peakY};
    ImVec2 pDecay   = {pStart.x + segmentW + (decay * segmentW), sustainY};
    ImVec2 pSusHold = {pStart.x + segmentW * 2.5f, sustainY};
    ImVec2 pEnd     = {std::min(startPos.x + size.x - 5.0f, pSusHold.x + (release * segmentW)), bottomLimit};


    // ImU32 color = ImColor(0, 255, 200, 255); // Neon OPL Green
    ImU32 color = ImColor(0, 255, 200, (int)(100 + (155 * pulseVol)));
    float thick = 1.0f + (2.0f * pulseVol);

    // 1. Attack (Linear)
    drawList->AddLine(pStart, pAttack, color, thick);

    // 2. Decay (Exponential Bezier)
    ImVec2 cpDecay = {pAttack.x + (pDecay.x - pAttack.x) * 0.2f, pDecay.y};
    drawList->AddBezierQuadratic(pAttack, cpDecay, pDecay, color, thick);

    // 3. Sustain Hold
    drawList->AddLine(pDecay, pSusHold, color, thick);

    // 4. Release (Exponential Bezier)
    ImVec2 cpRelease = {pSusHold.x + (pEnd.x - pSusHold.x) * 0.2f, pEnd.y};
    drawList->AddBezierQuadratic(pSusHold, cpRelease, pEnd, color, thick);

    // If KSR is active, draw a ghost line of the "Original" speed for comparison
    if (op.ksr != 0) {
        ImGui::SetCursorScreenPos({startPos.x + 5, startPos.y + 5});
        ImGui::TextDisabled("KSR ACTIVE (%d)", virtualNote);

    }

    ImGui::Dummy(size);
}

//------------------------------------------------------------------------------

void SequencerGui::RenderInstrumentEditorUI(bool standAlone) {

    if (getMain()->getController()->getSoundBank().size() < mCurrentInstrumentId+1)
    {
        Log("[error] Current Instrument out of bounds!");
        mCurrentInstrumentId = 0;
        return;
    }

    if (standAlone)
    {
        ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
        ImGui::Begin("Instrument Editor");
    }


    Instrument& inst = getMain()->getController()->getSoundBank()[mCurrentInstrumentId];

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
    if (ImGui::Checkbox("4-OP Mode", &inst.isFourOp)) {
        if (inst.isFourOp) {
            ImGui::OpenPopup("Init4OP");
        } else {
            inst.setFourOpMode(false);
        }
    }
    DrawAlgorithmHoverFunc(inst);

    // Handling the popup results
    if (ImGui::BeginPopup("Init4OP")) {
        if (ImGui::Selectable("Copy Ops 1&2 -> 3&4 (Chorus/Fat)")) {
            inst.setFourOpMode(true, true);
        }
        if (ImGui::Selectable("Clean Pair 2")) {
            inst.setFourOpMode(true, false);
        }
        ImGui::EndPopup();
    }

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
        int opIdMod = (p * 2) + 1;
        int opIdCar = (p * 2) + 2;
        auto& pair = inst.pairs[p];

        ImGui::PushID(p); // Important: Unique ID per pair row

        // Create a table with 3 main sections: Modulator | Pair Controls | Carrier
        if (ImGui::BeginTable("WidePairTable", 3, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_NoSavedSettings)) {

            // Setup column widths: Operators take more space, middle column is compact
            ImGui::TableSetupColumn("Mod", ImGuiTableColumnFlags_WidthStretch, 0.4f);
            ImGui::TableSetupColumn("Controls", ImGuiTableColumnFlags_WidthFixed, 160.0f);
            ImGui::TableSetupColumn("Car", ImGuiTableColumnFlags_WidthStretch, 0.4f);

            ImGui::TableNextRow();

            // --- SECTION 1: MODULATOR ---
            ImGui::TableSetColumnIndex(0);
            ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "MODULATOR (Op %d)", opIdMod);
            ImGui::Separator();
            DrawOperatorGrid(pair.ops[0]); // Using the helper from the previous step

            // --- SECTION 2: PAIR GLOBALS (Feedback, Connection, Pan) ---
            ImGui::TableSetColumnIndex(1);
            ImGui::TextColored(ImVec4(0.4f, 0.4f, 1.0f, 1.0f), "PAIR %d CONFIG", p + 1);
            ImGui::Separator();

            // Use a nested table to match the row-height of the Modulator/Carrier grids
            if (ImGui::BeginTable("PairAligner", 1, ImGuiTableFlags_None)) {

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::BeginGroup();
                ImGui::TextDisabled("Feedback");
                ImGui::SetNextItemWidth(-FLT_MIN);
                ImGui::SliderScalar("##Feedback", ImGuiDataType_U8, &pair.feedback, &minFeed, &maxFeed);
                ImGui::EndGroup();

                // We add a little extra padding here so the Connection combo centers against the 2-row ADSR
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Dummy(ImVec2(0, 1.0f));
                ImGui::BeginGroup();
                ImGui::TextDisabled("Connection");
                ImGui::SetNextItemWidth(-FLT_MIN);
                const char* connTypes[] = { "FM (Serial)", "Additive (Parallel)" };
                int connIdx = (int)pair.connection;
                if (ImGui::Combo("##Connection", &connIdx, connTypes, 2)) {
                    pair.connection = (uint8_t)connIdx;
                }
                DrawAlgorithmHoverFunc(inst);
                ImGui::EndGroup();


                // --- MATCH ROW 3: Panning (Aligned with Flags/Wave) ---
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                // ImGui::Dummy(ImVec2(0, 1.0f));
                ImGui::TextDisabled("Panning");
                ImGui::SetNextItemWidth(-FLT_MIN);
                const char* panTypes[] = { "Mute", "Left", "Right", "Center" };
                int panIdx = (int)pair.panning;
                if (ImGui::Combo("##Panning", &panIdx, panTypes, 4)) {
                    pair.panning = (uint8_t)panIdx;
                }

                // make no sense :P
                // SongStep lastStep = getMain()->getController()->getSequencerState().last_steps[0];
                // static uint8_t lPanning = 32; // Center default
                // if (getMain()->getController()->getSequencerState().ui_dirty)
                // {
                //     lPanning = lastStep.panning;
                // }
                // DrawPanningMeter(ImVec2(ImGui::GetContentRegionAvail().x, 20), lPanning);

                ImGui::EndTable();
            }


            // --- SECTION 3: CARRIER ---
            ImGui::TableSetColumnIndex(2);
            ImGui::TextColored(ImVec4(1.0f, 0.7f, 0.4f, 1.0f), "CARRIER (Op %d)", opIdCar);
            ImGui::Separator();
            DrawOperatorGrid(pair.ops[1]);

            ImGui::EndTable();
        }
        ImGui::PopID();

        if (p < numPairs - 1) ImGui::Dummy(ImVec2(0, 10)); // Vertical gap between 4-OP pairs
    }


    ImGui::PopID();
    if (standAlone) ImGui::End();
}
