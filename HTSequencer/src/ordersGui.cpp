#include "sequencerGui.h"
#include "sequencerMain.h"
#include <imgui_internal.h>

#include <algorithm>
#include <string>
#include <cctype>
#include <src/fonts/IconsFontAwesome6.h>

//-----------------------------------------------------------------------------
/*
 *  Like DrawFancyOrderList but vertical, no append button or header caption,
 *  small colored pattern with index only name via tooltip
 */
void SequencerGui::DrawMiniOrderList(SongData& song, int currentIndex, float buttonSize, ImVec2 controlSize) {
    static int move_from = -1, move_to = -1;
    int delete_idx = -1, insert_idx = -1;

    ImVec2 btnSize = ImVec2(buttonSize, buttonSize);
    bool isPlaying = (currentIndex >= 0);
    ImDrawList* dl = ImGui::GetWindowDrawList();


    // Style for tight horizontal layout
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2, 0));
    ImGui::BeginChild("MiniOrderScroll", controlSize, false, ImGuiWindowFlags_HorizontalScrollbar);

    for (int n = 0; n < (int)song.orderList.size(); n++) {
        uint8_t& patIdx = song.orderList[n];
        Pattern& pat = song.patterns[patIdx];
        bool isCurrent = (currentIndex == n);

        ImGui::PushID(n);
        ImVec2 pos = ImGui::GetCursorScreenPos();

        // 1. Interaction (Disabled if playing, except for tooltips/menus)
        ImGui::BeginDisabled(isPlaying);
        ImGui::InvisibleButton("tile", btnSize);
        ImGui::EndDisabled();

        // 2. Tooltip (Always active)
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
            ImGui::SetTooltip("Order: %02d\nPattern: [%02d] %s", n, patIdx, pat.mName.c_str());
        }


        // Base Color
        dl->AddRectFilled(pos, ImVec2(pos.x + btnSize.x, pos.y + btnSize.y), pat.mColor, 2.0f);

        if (isCurrent) {
            // "Playing" Visual Effect: Pulsing border
            // float pulse = (sinf((float)ImGui::GetTime() * 10.0f) * 0.5f) + 0.5f;
            // ImU32 pulseCol = ImGui::GetColorU32(ImVec4(1, 1, 1, 0.5f + pulse * 0.5f));

            ImVec4 baseCol = ImGui::ColorConvertU32ToFloat4(ImFlux::GetContrastColorU32(pat.mColor));
            float pulse = (sinf((float)ImGui::GetTime() * 10.0f) * 0.5f) + 0.5f;
            ImVec4 finalCol;
            finalCol.x = ImLerp(baseCol.x, 1.0f, pulse * 0.6f); // 0.6f begrenzt die Helligkeit
            finalCol.y = ImLerp(baseCol.y, 1.0f, pulse * 0.6f);
            finalCol.z = ImLerp(baseCol.z, 1.0f, pulse * 0.6f);
            finalCol.w = 1.0f;
            ImU32 pulseCol = ImGui::GetColorU32(finalCol);


            dl->AddRect(pos, ImVec2(pos.x + btnSize.x, pos.y + btnSize.y), pulseCol, 3.0f, 0, 2.0f);
        } else if (ImGui::IsItemHovered() && !isPlaying) {
            dl->AddRect(pos, ImVec2(pos.x + btnSize.x, pos.y + btnSize.y), IM_COL32_WHITE, 1.0f);
        }

        // Display Pattern Index (patIdx) instead of Order Index (n)
        char buf[4]; snprintf(buf, 4, "%02d", patIdx);
        ImVec2 ts = ImGui::CalcTextSize(buf);


        dl->AddText(ImVec2(pos.x + (btnSize.x - ts.x) * 0.5f, pos.y + (btnSize.y - ts.y) * 0.5f),
                    ImFlux::GetContrastColorU32(pat.mColor), buf);

        // 4. Full Context Menu (Insert / Change / Remove)
        if (ImGui::BeginPopupContextItem("mini_row_menu")) {
            if (ImGui::MenuItem("Insert before")) insert_idx = n;
            if (ImGui::MenuItem("Remove")) delete_idx = n;
            ImGui::Separator();
            if (ImGui::BeginMenu("Change Pattern")) {
                for (int p = 0; p < (int)song.patterns.size(); p++) {
                    if (ImGui::Selectable(song.patterns[p].mName.c_str(), patIdx == p)) patIdx = p;
                }
                ImGui::EndMenu();
            }
            ImGui::EndPopup();
        }

        // 5. Drag & Drop (Disabled if playing)
        if (!isPlaying) {
            if (ImGui::BeginDragDropSource()) {
                ImGui::SetDragDropPayload("DND_ORDER", &n, sizeof(int));
                ImGui::Text("Move Pattern %02d", patIdx);
                ImGui::EndDragDropSource();
            }
            if (ImGui::BeginDragDropTarget()) {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_ORDER")) {
                    move_from = *(const int*)payload->Data;
                    move_to = n;
                }
                ImGui::EndDragDropTarget();
            }
        }

        ImGui::SameLine();
        ImGui::PopID();
    }

    ImGui::EndChild();
    ImGui::PopStyleVar();

    // Deferred Logic execution
    if (delete_idx != -1) song.orderList.erase(song.orderList.begin() + delete_idx);
    if (insert_idx != -1) song.orderList.insert(song.orderList.begin() + insert_idx, song.orderList[insert_idx]);
    if (move_from != -1 && move_to != -1) {
        auto it_f = song.orderList.begin() + move_from;
        auto it_t = song.orderList.begin() + move_to;
        if (move_from < move_to) std::rotate(it_f, it_f + 1, it_t + 1);
        else std::rotate(it_t, it_f, it_f + 1);
        move_from = move_to = -1;
    }
}
//------------------------------------------------------------------------------
// DrawFancyOrderList
//------------------------------------------------------------------------------
void SequencerGui::DrawFancyOrderList(SongData& song, bool standAlone, ImVec2 controlSize) {

    if (standAlone) {
        ImGui::SetNextWindowSize(ImVec2(250, 200), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSizeConstraints(ImVec2(100.0f, 100.0f), ImVec2(FLT_MAX, FLT_MAX));
        //NOTE added flags (table madness ) ==
        if (!ImGui::Begin("Playlist (ORDERS)")) //, nullptr, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoScrollWithMouse))
        { ImGui::End(); return; }
    }

    static int move_from = -1, move_to = -1;
    int delete_idx = -1, insert_idx = -1;





    // --- Compact Header ---
    ImGui::TextDisabled("ORDERS");
    ImGui::SameLine();

    ImFlux::ButtonParams lbp = ImFlux::SLATE_BUTTON.WithSize(ImVec2(50.f,18.f)).WithRounding(2.f);
    ImGui::BeginDisabled(isPlaying());
    if (ImFlux::ButtonFancy("append",lbp)) song.orderList.push_back(mPatternEditorState.currentPatternIdx);
    ImGui::EndDisabled(/*isPlaying()*/);

    ImGui::Separator();

    // Start a child region for scrolling if the list gets long
    ImGui::BeginChild("OrderListScroll", controlSize, false);

    bool  isCurrent = false;

    for (int n = 0; n < (int)song.orderList.size(); n++) {
        uint8_t& patIdx = song.orderList[n];
        Pattern& pat = song.patterns[patIdx];

        isCurrent = isPlaying() && n == getMain()->getController()->getSequencerState().orderIdx ;

        ImGui::PushID(n);

        // 1. Draw Index
        ImGui::AlignTextToFramePadding();
        ImGui::TextDisabled("%02d", n);
        ImGui::SameLine();

        // 2. Custom Colored Button (The "Fancy" part)
        ImVec2 pos = ImGui::GetCursorScreenPos();
        ImVec2 size = ImVec2(ImGui::GetContentRegionAvail().x - 10, ImGui::GetFrameHeight());

        // Convert ABGR to ImGui's expected U32 (usually RGBA/RGBA depending on platform)
        // If colors look wrong, you might need: ((pat.mColor << 8) | (pat.mColor >> 24))
        ImU32 col32 = pat.mColor;

        // Invisible button to handle interaction
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);

        ImGui::BeginDisabled(isPlaying());
        if (ImGui::InvisibleButton("pat_btn", size)) {
        }
        ImGui::EndDisabled(/*isPlaying()*/);

        // Background Drawing
        bool is_hovered = ImGui::IsItemHovered();
        bool is_active = ImGui::IsItemActive();
        float alpha = is_active ? 1.0f : (is_hovered ? 0.8f : 0.6f);

        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        draw_list->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), col32, 3.0f);


        if (isCurrent) {
            // // "Playing" Visual Effect: Pulsing  border
            // float pulse = (sinf((float)ImGui::GetTime() * 10.0f) * 0.5f) + 0.5f;
            // //ImU32 pulseCol = ImGui::GetColorU32(ImVec4(1, 1, 1, 0.5f + pulse * 0.5f));
            // ImU32 pulseCol = ImGui::GetColorU32(ImFlux::ModifierColor(ImFlux::GetContrastColor(col32), pulse));
            //
            // draw_list->AddRect(pos, ImVec2(pos.x + size.x, pos.y + size.y), pulseCol, 5.0f);
            ImVec4 baseCol = ImGui::ColorConvertU32ToFloat4(ImFlux::GetContrastColorU32(col32));
            float pulse = (sinf((float)ImGui::GetTime() * 10.0f) * 0.5f) + 0.5f;
            ImVec4 finalCol;
            finalCol.x = ImLerp(baseCol.x, 1.0f, pulse * 0.6f); // 0.6f begrenzt die Helligkeit
            finalCol.y = ImLerp(baseCol.y, 1.0f, pulse * 0.6f);
            finalCol.z = ImLerp(baseCol.z, 1.0f, pulse * 0.6f);
            finalCol.w = 1.0f;

            ImU32 pulseCol = ImGui::GetColorU32(finalCol);
            draw_list->AddRect(pos, ImVec2(pos.x + size.x, pos.y + size.y), pulseCol, 3.0f, 0, 3.f);

        } else if (is_hovered && !isPlaying()) {
            draw_list->AddRect(pos, ImVec2(pos.x + size.x, pos.y + size.y), ImFlux::GetContrastColorU32(col32), 3.0f);
        }


        // if (is_hovered)
        //     draw_list->AddRect(pos, ImVec2(pos.x + size.x, pos.y + size.y), IM_COL32_WHITE, 3.0f);
        //
        // Center Text in Button
        ImVec2 text_size = ImGui::CalcTextSize(pat.mName.c_str());
        draw_list->AddText(ImVec2(pos.x + (size.x - text_size.x) * 0.5f, pos.y + (size.y - text_size.y) * 0.5f),
                           ImFlux::GetContrastColorU32(pat.mColor), pat.mName.c_str());

        // 3. Actions via Context Menu (Right Click)
        if (ImGui::BeginPopupContextItem("row_menu")) {
            if (ImGui::MenuItem("Insert Above")) insert_idx = n;
            if (ImGui::MenuItem("Remove")) delete_idx = n;
            ImGui::Separator();
            if (ImGui::BeginMenu("Change Pattern")) {
                for (int p = 0; p < (int)song.patterns.size(); p++) {
                    if (ImGui::Selectable(song.patterns[p].mName.c_str(), patIdx == p)) patIdx = p;
                }
                ImGui::EndMenu();
            }
            ImGui::EndPopup();
        }

        // 4. Drag and Drop Logic
        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
            ImGui::SetDragDropPayload("DND_ORDER", &n, sizeof(int));
            ImGui::Text("Moving %s", pat.mName.c_str());
            ImGui::EndDragDropSource();
        }
        if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_ORDER")) {
                move_from = *(const int*)payload->Data;
                move_to = n;
            }
            ImGui::EndDragDropTarget();
        }

        ImGui::PopStyleVar();
        ImGui::PopID();
    }
    ImGui::EndChild();



    // Logic Execution (Deferred)
    if (delete_idx != -1) song.orderList.erase(song.orderList.begin() + delete_idx);
    if (insert_idx != -1) song.orderList.insert(song.orderList.begin() + insert_idx, song.orderList[insert_idx]);
    if (move_from != -1 && move_to != -1) {
        auto it_f = song.orderList.begin() + move_from;
        auto it_t = song.orderList.begin() + move_to;
        if (move_from < move_to) std::rotate(it_f, it_f + 1, it_t + 1);
        else std::rotate(it_t, it_f, it_f + 1);
        move_from = move_to = -1;
    }


    if (standAlone) ImGui::End();

}
//------------------------------------------------------------------------------
void SequencerGui::DrawOrderListEditor(SongData& song) {
    // Action trackers to prevent modifying vector during iteration
    int delete_idx = -1;
    int insert_idx = -1;
    static int move_from = -1, move_to = -1;

    if (ImGui::Button("Append Pattern") && !song.patterns.empty()) {
        song.orderList.push_back(0);
    }

    if (ImGui::BeginTable("OrdersTable", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        ImGui::TableSetupColumn("Pos", ImGuiTableColumnFlags_WidthFixed, 30.0f);
        ImGui::TableSetupColumn("Pattern", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Color", ImGuiTableColumnFlags_WidthFixed, 40.0f);
        ImGui::TableSetupColumn("Actions", ImGuiTableColumnFlags_WidthFixed, 100.0f);
        ImGui::TableHeadersRow();

        for (int n = 0; n < (int)song.orderList.size(); n++) {
            ImGui::PushID(n);
            ImGui::TableNextRow();

            // Safety check for pattern index
            uint8_t& patIdx = song.orderList[n];
            if (patIdx >= song.patterns.size()) patIdx = 0;
            Pattern& pat = song.patterns[patIdx];


            // --- COL 0: Drag Handle ---
            ImGui::TableSetColumnIndex(0);

            // 1. Ensure the ID is unique for this specific row
            ImGui::PushID(n);

            // 2. Use a unique label or "##row" + AllowOverlap
            if (ImGui::Selectable("##row", move_from == n, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap)) {
                // Row selection logic
            }

            // // Overlay position text over the selectable
            ImGui::SameLine();
            ImGui::Text("%03d", n);


            // 3. FIX: Add ImGuiDragDropFlags_SourceAllowNullID to prevent the crash
            if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
                ImGui::SetDragDropPayload("DND_ORDER", &n, sizeof(int));
                ImGui::Text("Moving Position %d", n);
                ImGui::EndDragDropSource();
            }

            // ... DragDropTarget ...
            ImGui::PopID(); // Match the PushID(n)

            // --- COL 1: Selector ---
            ImGui::TableSetColumnIndex(1);
            ImGui::SetNextItemWidth(-FLT_MIN);
            if (ImGui::BeginCombo("##combo", pat.mName.c_str())) {
                for (int i = 0; i < (int)song.patterns.size(); i++) {
                    if (ImGui::Selectable(song.patterns[i].mName.c_str(), patIdx == i)) patIdx = (uint8_t)i;
                }
                ImGui::EndCombo();
            }

            // --- COL 2: Color ---
            ImGui::TableSetColumnIndex(2);
            ImVec4 col = ImGui::ColorConvertU32ToFloat4(pat.mColor);
            ImGui::ColorButton("##c", col, ImGuiColorEditFlags_NoInputs);

            // --- COL 3: Logic ---
            ImGui::TableSetColumnIndex(3);
            if (ImGui::SmallButton("Ins")) insert_idx = n;
            ImGui::SameLine();
            if (ImGui::SmallButton("Del")) delete_idx = n;

            ImGui::PopID();
        }
        ImGui::EndTable();
    }

    // --- DEFERRED MODIFICATIONS ---
    // Handle delete
    if (delete_idx != -1) {
        song.orderList.erase(song.orderList.begin() + delete_idx);
    }
    // Handle insert (inserts a copy of the pattern at current position)
    if (insert_idx != -1) {
        song.orderList.insert(song.orderList.begin() + insert_idx, song.orderList[insert_idx]);
    }
    // Handle move
    if (move_from != -1 && move_to != -1) {
        if (move_from != move_to) {
            auto it_from = song.orderList.begin() + move_from;
            auto it_to = song.orderList.begin() + move_to;
            if (move_from < move_to) std::rotate(it_from, it_from + 1, it_to + 1);
            else std::rotate(it_to, it_from, it_from + 1);
        }
        move_from = move_to = -1;
    }
}

// void SequencerGui::DrawOrderListEditor(SongData& song) {
//     static int move_from = -1, move_to = -1;
//
//     // 1. Efficient Header / Append
//     if (ImGui::Button("+ Append Pattern") && !song.patterns.empty()) {
//         song.orderList.push_back(0);
//     }
//
//     // 2. Table with Fixed Height for Scroll
//     const float row_height = ImGui::GetTextLineHeightWithSpacing();
//     if (ImGui::BeginTable("OrdersTable", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY, ImVec2(0, 400))) {
//
//         ImGui::TableSetupColumn("Pos", ImGuiTableColumnFlags_WidthFixed, 30.0f);
//         ImGui::TableSetupColumn("Pattern", ImGuiTableColumnFlags_WidthStretch);
//         ImGui::TableSetupColumn("Color", ImGuiTableColumnFlags_WidthFixed, 40.0f);
//         ImGui::TableSetupColumn("Actions", ImGuiTableColumnFlags_WidthFixed, 110.0f);
//         ImGui::TableHeadersRow();
//
//         // 3. Performance: Use ListClipper for large lists
//         ImGuiListClipper clipper;
//         clipper.Begin((int)song.orderList.size());
//
//         while (clipper.Step()) {
//             for (int n = clipper.DisplayStart; n < clipper.DisplayEnd; n++) {
//                 ImGui::PushID(n);
//                 ImGui::TableNextRow(ImGuiTableRowFlags_None, row_height);
//
//                 uint8_t& patIdx = song.orderList[n];
//                 if (patIdx >= song.patterns.size()) patIdx = 0;
//
//                 // --- COLUMN 0: DRAG HANDLE ---
//                 ImGui::TableSetColumnIndex(0);
//                 char label[16];
//                 ImFormatString(label, IM_ARRAYSIZE(label), "%03d", n);
//
//                 // Use Selectable as a background for the Row, but AllowOverlap
//                 // so widgets in other columns remain clickable.
//                 ImGuiSelectableFlags sel_flags = ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap;
//                 if (ImGui::Selectable(label, move_from == n, sel_flags, ImVec2(0, row_height))) {
//                     // Row selection logic if needed
//                 }
//
//                 if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
//                     ImGui::SetDragDropPayload("DND_ORDER", &n, sizeof(int));
//                     ImGui::Text("Moving Position %d", n);
//                     ImGui::EndDragDropSource();
//                 }
//                 if (ImGui::BeginDragDropTarget()) {
//                     if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_ORDER")) {
//                         move_from = *(const int*)payload->Data;
//                         move_to = n;
//                     }
//                     ImGui::EndDragDropTarget();
//                 }
//
//                 // --- COLUMN 1: WIDGETS (Now clickable thanks to AllowOverlap) ---
//                 ImGui::TableSetColumnIndex(1);
//                 ImGui::SetNextItemWidth(-FLT_MIN); // Fill column width
//                 if (ImGui::BeginCombo("##combo", song.patterns[patIdx].mName.c_str())) {
//                     for (int i = 0; i < (int)song.patterns.size(); i++) {
//                         if (ImGui::Selectable(song.patterns[i].mName.c_str(), patIdx == i)) patIdx = i;
//                     }
//                     ImGui::EndCombo();
//                 }
//
//                 // --- COLUMN 2: COLOR ---
//                 ImGui::TableSetColumnIndex(2);
//                 ImVec4 c = ImGui::ColorConvertU32ToFloat4(song.patterns[patIdx].mColor);
//                 ImGui::ColorButton("##c", c, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip);
//
//                 // --- COLUMN 3: BUTTONS ---
//                 ImGui::TableSetColumnIndex(3);
//                 if (ImGui::Button("Ins")) {
//                     song.orderList.insert(song.orderList.begin() + n, patIdx);
//                 }
//                 ImGui::SameLine();
//                 if (ImGui::Button("Del")) {
//                     song.orderList.erase(song.orderList.begin() + n);
//                 }
//
//                 ImGui::PopID();
//             }
//         }
//         ImGui::EndTable();
//     }
//
//     // 4. Efficient Reordering
//     if (move_from != -1 && move_to != -1) {
//         if (move_from != move_to) {
//             auto it_from = song.orderList.begin() + move_from;
//             auto it_to = song.orderList.begin() + move_to;
//             if (move_from < move_to) std::rotate(it_from, it_from + 1, it_to + 1);
//             else std::rotate(it_to, it_from, it_from + 1);
//         }
//         move_from = move_to = -1;
//     }
// }
// void SequencerGui::DrawOrderListEditor(opl3::SongData& song) {
//     ImGui::SeparatorText("Orders List (Playlist)");
//
//     // Add new pattern to the end of the list
//     if (ImGui::Button("Append Pattern") && !song.patterns.empty()) {
//         song.orderList.push_back(0);
//     }
//
//     static int move_from = -1, move_to = -1;
//
//     if (ImGui::BeginTable("OrdersTable", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY, ImVec2(0, 300))) {
//         ImGui::TableSetupColumn("Pos", ImGuiTableColumnFlags_WidthFixed, 30.0f);
//         ImGui::TableSetupColumn("Pattern Name", ImGuiTableColumnFlags_WidthStretch);
//         ImGui::TableSetupColumn("Color", ImGuiTableColumnFlags_WidthFixed, 40.0f);
//         ImGui::TableSetupColumn("Actions", ImGuiTableColumnFlags_WidthFixed, 120.0f);
//         ImGui::TableHeadersRow();
//
//         for (int n = 0; n < (int)song.orderList.size(); n++) {
//             uint8_t& patternIdx = song.orderList[n];
//
//             // Safety check: ensure index is within song.patterns range
//             if (patternIdx >= song.patterns.size()) patternIdx = 0;
//             Pattern& pat = song.patterns[patternIdx];
//
//             ImGui::TableNextRow();
//             ImGui::PushID(n);
//
//             // Column 0: Index + Drag & Drop Source
//             ImGui::TableSetColumnIndex(0);
//             ImGui::Selectable(std::to_string(n).c_str(), false, ImGuiSelectableFlags_SpanAllColumns);
//
//             if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
//                 ImGui::SetDragDropPayload("DND_ORDER_IDX", &n, sizeof(int));
//                 ImGui::Text("Moving Pattern %d", n);
//                 ImGui::EndDragDropSource();
//             }
//             if (ImGui::BeginDragDropTarget()) {
//                 if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_ORDER_IDX")) {
//                     move_from = *(const int*)payload->Data;
//                     move_to = n;
//                 }
//                 ImGui::EndDragDropTarget();
//             }
//
//             // Column 1: Pattern Selector
//             ImGui::TableSetColumnIndex(1);
//             if (ImGui::BeginCombo("##pat_select", pat.mName.c_str())) {
//                 for (int p = 0; p < (int)song.patterns.size(); p++) {
//                     if (ImGui::Selectable(song.patterns[p].mName.c_str(), patternIdx == p)) {
//                         patternIdx = (uint8_t)p;
//                     }
//                 }
//                 ImGui::EndCombo();
//             }
//
//             // Column 2: Color Preview (ABGR format)
//             ImGui::TableSetColumnIndex(2);
//             ImVec4 col = ImGui::ColorConvertU32ToFloat4(pat.mColor);
//             ImGui::ColorButton("##col", col, ImGuiColorEditFlags_NoTooltip);
//
//             // Column 3: Buttons
//             ImGui::TableSetColumnIndex(3);
//             if (ImGui::SmallButton("Ins")) { // Insert copy
//                 song.orderList.insert(song.orderList.begin() + n, patternIdx);
//             }
//             ImGui::SameLine();
//             if (ImGui::SmallButton("Del")) { // Remove
//                 song.orderList.erase(song.orderList.begin() + n);
//             }
//
//             ImGui::PopID();
//         }
//         ImGui::EndTable();
//     }
//
//     // Handle Drag & Drop reordering
//     if (move_from != -1 && move_to != -1) {
//         uint8_t item = song.orderList[move_from];
//         song.orderList.erase(song.orderList.begin() + move_from);
//         song.orderList.insert(song.orderList.begin() + move_to, item);
//         move_from = move_to = -1;
//     }
// }
