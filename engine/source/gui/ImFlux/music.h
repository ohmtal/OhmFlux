//-----------------------------------------------------------------------------
// Copyright (c) 2026 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Music related
//-----------------------------------------------------------------------------
#pragma once

#include <imgui.h>
#include <imgui_internal.h>
#include <cmath>
#include <algorithm>

#include "helper.h"

namespace ImFlux {
    //--------------------------------------------------------------------------
    // Basic 16Bit 4/4 one bank pattern
    //--------------------------------------------------------------------------
    inline bool PatternEditor16Bit(const char* label,
                                   uint16_t* bits,
                                   int8_t currentStep,
                                   ImU32 color_on = IM_COL32(255, 50, 50, 255),
                                   ImU32 color_step = IM_COL32(255, 255, 255, 180))
    {
        bool changed = false;
        const float labelWidth = 100.f;
        const ImU32 color_off = IM_COL32(45, 45, 45, 255);

        static bool isDragging = false;
        static bool dragTargetState = false;

        ImGui::PushID(label);
        ImFlux::TextColoredEllipsis(ImVec4(0.6f, 0.6f, 0.8f, 1.f), label, labelWidth);
        ImGui::SameLine(labelWidth);

        for (int i = 15; i >= 0; i--) {
            ImGui::PushID(i);
            bool isSet = (*bits >> i) & 1;
            bool isCurrent = (15 - i) == currentStep;
            ImU32 col = isSet ? color_on : color_off;
            if (isCurrent) {
                col = isSet ? ImFlux::ModifyRGB(color_on, 1.2f) : ImFlux::ModifyRGB(color_off, 1.4f);
            }
            ImVec2 p = ImGui::GetCursorScreenPos();
            ImGui::InvisibleButton("bit", {18, 18});
            if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
                dragTargetState = !isSet;
                isDragging = true;
                if (dragTargetState) *bits |= (1 << i);
                else *bits &= ~(1 << i);
                changed = true;
            }
            else if (isDragging && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem)) {
                if (isSet != dragTargetState) {
                    if (dragTargetState) *bits |= (1 << i);
                    else *bits &= ~(1 << i);
                    changed = true;
                }
            }
            auto drawList = ImGui::GetWindowDrawList();
            drawList->AddRectFilled(p, {p.x + 16, p.y + 16}, col, 2.0f);
            if (isCurrent) {
                drawList->AddRect(p, {p.x + 16, p.y + 16}, color_step, 2.0f, 0, 1.5f);
            }

            if (i > 0) {
                float spacing = (i % 4 == 0) ? 6.0f : 1.5f;
                ImGui::SameLine(0, spacing);
            }

            ImGui::PopID();
        }
        if (isDragging && !ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
            isDragging = false;
        }
        ImGui::SameLine();
        int bitsInt = (int) *bits;
        ImGui::SetNextItemWidth(60.f);
        if (ImGui::InputInt("##patValue", &bitsInt,0,0/*, ImGuiInputTextFlags_CharsHexadecimal*/)) { *bits = bitsInt; changed = true;}

        ImGui::PopID();
        return changed;
    }
    //--------------------------------------------------------------------------
    // VelocityPad used for Drums :)
    // Normalized Velocity from bottom to top
    //--------------------------------------------------------------------------
    inline bool VelocityPad(const char* label, ImVec2 size, ImU32 color, float* out_velocity) {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems) return false;

        const ImGuiID id = window->GetID(label);
        const ImVec2 pos = ImGui::GetCursorScreenPos();
        const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));

        ImGui::ItemSize(bb);
        if (!ImGui::ItemAdd(bb, id)) return false;

        bool hovered, held;
        bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);

        // --- Rendering ---
        ImDrawList* draw_list = ImGui::GetWindowDrawList();

        // Calculate highlight color (mix with white if held)
        ImU32 pad_col = color;
        if (held) pad_col = IM_COL32(200, 200, 200, 255);

        // Glow
        if (hovered) {
            ImU32 glow_col = (color & 0x00FFFFFF) | 0x44000000; // Low alpha glow
            for (int i = 1; i <= 3; i++) {
                draw_list->AddRect(bb.Min - ImVec2(i,i), bb.Max + ImVec2(i,i), glow_col, 4.0f, 0, 1.5f);
            }
        }

        // Main Pad Body
        draw_list->AddRectFilled(bb.Min, bb.Max, pad_col, 4.0f);

        // 3. Draw Centered Text
        ImVec2 text_size = ImGui::CalcTextSize(label);
        ImVec2 text_pos = bb.Min + (size - text_size) * 0.5f;
        draw_list->AddText(text_pos, ImFlux::GetContrastColorU32(pad_col) , label);

        // --- Velocity Calculation (Normalized 0.0 - 1.0) ---
        if (pressed) {
            float mouse_y = ImGui::GetMousePos().y;
            // Y-Inversion: Clicking the TOP gives 1.0, BOTTOM gives 0.0
            float rel_y = 1.0f - ((mouse_y - bb.Min.y) / size.y);
            *out_velocity = ImClamp(rel_y, 0.0f, 1.0f);
        }

        return pressed;
    }

    //--------------------------------------------------------------------------
    // Enhanced VelocityPad used for Drums :)
    // Normalized Velocity from bottom to top
    // Normalized Pitch from left to right
    //--------------------------------------------------------------------------
    inline bool DrumPad(const char* label, ImVec2 size, ImU32 color, float* out_velocity, float* out_pitch) {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems) return false;

        const ImGuiID id = window->GetID(label);
        const ImVec2 pos = ImGui::GetCursorScreenPos();
        const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));

        ImGui::ItemSize(bb);
        if (!ImGui::ItemAdd(bb, id)) return false;

        bool hovered, held;
        bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);

        // --- Rendering ---
        ImDrawList* draw_list = ImGui::GetWindowDrawList();

        // Calculate highlight color (mix with white if held)
        ImU32 pad_col = color;
        if (held) pad_col = IM_COL32(200, 200, 200, 255);

        // Glow
        if (hovered) {
            ImU32 glow_col = (color & 0x00FFFFFF) | 0x44000000; // Low alpha glow
            for (int i = 1; i <= 3; i++) {
                draw_list->AddRect(bb.Min - ImVec2(i,i), bb.Max + ImVec2(i,i), glow_col, 4.0f, 0, 1.5f);
            }
        }

        // Main Pad Body
        ImU32 col_left = ModifyRGB(pad_col, 0.5f);
        ImU32 col_right = ModifyRGB(pad_col, 1.5f);
        //IMGUI_API void  AddRectFilledMultiColor(const ImVec2& p_min, const ImVec2& p_max,
        // ImU32 col_upr_left, ImU32 col_upr_right, ImU32 col_bot_right, ImU32 col_bot_left);
        draw_list->AddRectFilledMultiColor(
            bb.Min, bb.Max,
            col_left, col_right, col_right, col_left
        );


        // draw_list->AddRectFilled(bb.Min, bb.Max, pad_col, 4.0f);

        // 3. Draw Centered Text
        ImVec2 text_size = ImGui::CalcTextSize(label);
        ImVec2 text_pos = bb.Min + (size - text_size) * 0.5f;
        draw_list->AddText(text_pos, ImFlux::GetContrastColorU32(pad_col) , label);

        // --- Velocity Calculation (Normalized 0.0 - 1.0) ---
        if (pressed) {
            if (out_velocity) {
                float mouse_y = ImGui::GetMousePos().y;
                // Y-Inversion: Clicking the TOP gives 1.0, BOTTOM gives 0.0
                float rel_y = 1.0f - ((mouse_y - bb.Min.y) / size.y);
                *out_velocity = ImClamp(rel_y, 0.0f, 1.0f);
            }
            if (out_pitch) {
                float mouse_x = ImGui::GetMousePos().x;
                float rel_x = (mouse_x - bb.Min.x) / size.x;
                *out_pitch = ImClamp(rel_x, 0.0f, 1.0f);
            }
        }
        return pressed;
    }


}
