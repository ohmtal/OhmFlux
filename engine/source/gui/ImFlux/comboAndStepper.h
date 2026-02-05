//-----------------------------------------------------------------------------
// Copyright (c) 2026 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#pragma once

#include <imgui.h>
#include <imgui_internal.h>
#include <cmath>
#include <algorithm>

#include "helper.h"

namespace ImFlux {

    //---------------------------------------------------------------------------
    // Helper to draw a small hardware-style triangle button
    inline bool StepperButton(const char* id, bool is_left, ImVec2 size) {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        ImGuiID imgui_id = window->GetID(id);
        ImRect bb(window->DC.CursorPos, window->DC.CursorPos + size);
        ImGui::ItemSize(bb);
        if (!ImGui::ItemAdd(bb, imgui_id)) return false;

        bool hovered, held;
        bool pressed = ImGui::ButtonBehavior(bb, imgui_id, &hovered, &held);

        // Visuals
        ImDrawList* dl = window->DrawList;
        ImU32 col = held ? IM_COL32(100, 100, 100, 255) : (hovered ? IM_COL32(70, 70, 70, 255) : IM_COL32(40, 40, 40, 255));

        // Button Shape
        dl->AddRectFilled(bb.Min, bb.Max, col, 2.0f);
        dl->AddRect(bb.Min, bb.Max, IM_COL32(90, 90, 90, 255), 2.0f);

        // Triangle Icon
        float pad = size.x * 0.3f;
        ImVec2 a, b, c;
        if (is_left) {
            a = {bb.Max.x - pad, bb.Min.y + pad};
            b = {bb.Max.x - pad, bb.Max.y - pad};
            c = {bb.Min.x + pad, bb.Min.y + size.y * 0.5f};
        } else {
            a = {bb.Min.x + pad, bb.Min.y + pad};
            b = {bb.Min.x + pad, bb.Max.y - pad};
            c = {bb.Max.x - pad, bb.Min.y + size.y * 0.5f};
        }

        ImU32 lTextCol = IM_COL32(0, 255, 200, 255);
        if (ImGui::GetItemFlags() & ImGuiItemFlags_Disabled) {
            lTextCol  = ImGui::GetColorU32(ImGuiCol_TextDisabled);
        }


        dl->AddTriangleFilled(a, b, c, lTextCol);

        return pressed;
    }


    template<typename T>
    inline bool ValueStepper(const char* label, int* current_idx, const T& items, int items_count, float width = 140.0f) {
        ImGui::PushID(label);
        bool changed = false;
        float h = ImGui::GetFrameHeight();
        ImVec2 btn_sz(h, h);

        if (StepperButton("##left", true, btn_sz)) {
            *current_idx = (*current_idx > 0) ? *current_idx - 1 : items_count - 1;
            changed = true;
        }

        ImGui::SameLine(0, 4);

        ImVec2 pos = ImGui::GetCursorScreenPos();
        ImVec2 size(width, h);

        if (ImGui::InvisibleButton("##display_btn", size)) {
            *current_idx = (*current_idx + 1) % items_count;
            changed = true;
        }

        if (ImGui::BeginPopupContextItem("##StepperPopupLocal")) {
            for (int i = 0; i < items_count; i++) {
                const char* item_name = "";
                if constexpr (std::is_pointer_v<std::decay_t<decltype(items[0])>>) item_name = items[i];
                else item_name = items[i].name.c_str();

                if (ImGui::Selectable(item_name, *current_idx == i)) {
                    *current_idx = i;
                    changed = true;
                    ImGui::CloseCurrentPopup();
                }
            }
            ImGui::EndPopup();
        }

        ImDrawList* dl = ImGui::GetWindowDrawList();
        dl->AddRectFilled(pos, pos + size, IM_COL32(10, 10, 12, 255), 2.0f);
        dl->AddRect(pos, pos + size, IM_COL32(60, 60, 70, 255), 2.0f);

        const char* text = "";
        if (*current_idx >= 0 && *current_idx < items_count) {
            if constexpr (std::is_pointer_v<std::decay_t<decltype(items[0])>>) text = items[*current_idx];
            else text = items[*current_idx].name.c_str();
        }

        ImVec2 t_size = ImGui::CalcTextSize(text);
        ImU32 lTextCol = (ImGui::GetItemFlags() & ImGuiItemFlags_Disabled)
        ? ImGui::GetColorU32(ImGuiCol_TextDisabled)
        : IM_COL32(0, 255, 180, 255);

        dl->AddText({pos.x + (size.x - t_size.x) * 0.5f, pos.y + (size.y - t_size.y) * 0.5f}, lTextCol, text);

        ImGui::SameLine(0, 4);

        // 3. Rechter Button
        if (StepperButton("##right", false, btn_sz)) {
            *current_idx = (*current_idx + 1) % items_count;
            changed = true;
        }

        ImGui::PopID();
        return changed;
    }

    // template<typename T>
    // inline bool ValueStepper(const char* label, int* current_idx, const T& items, int items_count, float width = 140.0f) {
    //     ImGui::PushID(label);
    //     bool changed = false;
    //     float h = ImGui::GetFrameHeight();
    //     ImVec2 btn_sz(h, h);
    //
    //     // 1. Custom Left Button
    //     if (StepperButton("##left", true, btn_sz)) {
    //         *current_idx = (*current_idx > 0) ? *current_idx - 1 : items_count - 1;
    //         changed = true;
    //     }
    //
    //     ImGui::SameLine(0, 4);
    //
    //     // 2. LCD Display Area
    //     ImVec2 pos = ImGui::GetCursorScreenPos();
    //     ImVec2 size(width, h);
    //
    //     if (ImGui::InvisibleButton("##display", size)) {
    //         *current_idx = (*current_idx + 1) % items_count;
    //         changed = true;
    //     }
    //
    //     // Popup logic remains for Right-Click
    //     if (ImGui::BeginPopupContextItem("StepperPopup")) {
    //         for (int i = 0; i < items_count; i++) {
    //             // Determine text for selectable
    //             const char* item_name = "";
    //             if constexpr (std::is_pointer_v<std::decay_t<decltype(items[0])>>) item_name = items[i];
    //             else item_name = items[i].name.c_str(); // Handles your Instrument struct
    //
    //             if (ImGui::Selectable(item_name, *current_idx == i)) {
    //                 *current_idx = i;
    //                 changed = true;
    //             }
    //         }
    //         ImGui::EndPopup();
    //     }
    //
    //     // DRAW DISPLAY
    //     ImDrawList* dl = ImGui::GetWindowDrawList();
    //     dl->AddRectFilled(pos, pos + size, IM_COL32(10, 10, 12, 255), 2.0f);
    //     dl->AddRect(pos, pos + size, IM_COL32(60, 60, 70, 255), 2.0f);
    //
    //     const char* text = "";
    //     if (*current_idx >= 0 && *current_idx < items_count) {
    //         if constexpr (std::is_pointer_v<std::decay_t<decltype(items[0])>>) text = items[*current_idx];
    //         else text = items[*current_idx].name.c_str();
    //     }
    //
    //     ImVec2 t_size = ImGui::CalcTextSize(text);
    //
    //     ImU32 lTextCol = IM_COL32(0, 255, 180, 255);
    //
    //     if (ImGui::GetItemFlags() & ImGuiItemFlags_Disabled) {
    //         lTextCol  = ImGui::GetColorU32(ImGuiCol_TextDisabled);
    //     }
    //
    //
    //     dl->AddText({pos.x + (size.x - t_size.x) * 0.5f, pos.y + (size.y - t_size.y) * 0.5f}
    //                 , lTextCol
    //                 , text);
    //
    //     ImGui::SameLine(0, 4);
    //
    //     // 3. Custom Right Button
    //     if (StepperButton("##right", false, btn_sz)) {
    //         *current_idx = (*current_idx + 1) % items_count;
    //         changed = true;
    //     }
    //
    //     ImGui::PopID();
    //     return changed;
    // }


};
