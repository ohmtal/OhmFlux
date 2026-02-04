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
    inline bool FaderVertical(const char* label, ImVec2 size, float* v, float v_min, float v_max) {
        ImGui::PushID(label);
        ImVec2 pos = ImGui::GetCursorScreenPos();
        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImGuiIO& io = ImGui::GetIO();

        // 1. Interaction
        ImGui::InvisibleButton("##fader", size);

        if (!ImGui::IsItemVisible()) { ImGui::PopID(); return false; }


        bool is_active = ImGui::IsItemActive();
        bool is_hovered = ImGui::IsItemHovered();

        // ScrollWheel
        if (is_hovered && io.MouseWheel != 0) {
            float wheel_speed = (v_max - v_min) * 0.05f; // 5% of range per notch
            if (io.KeyShift) wheel_speed *= 0.1f;        // 0.5% if Shift is held

            *v = std::clamp(*v + (io.MouseWheel * wheel_speed), v_min, v_max);
            is_active = true;
        }


        if (is_active && io.MouseDown[0]) {
            float mouse_y = io.MousePos.y - pos.y;
            float fraction = 1.0f - std::clamp(mouse_y / size.y, 0.0f, 1.0f);
            *v = v_min + fraction * (v_max - v_min);
        }

        // 2. Drawing the "Track" (The slot)
        float mid_x = pos.x + size.x * 0.5f;
        dl->AddRectFilled({mid_x - 2, pos.y}, {mid_x + 2, pos.y + size.y}, IM_COL32(20, 20, 20, 255), 2.0f);

        // 3. Drawing Tick Marks (Every 25%)
        for (int i = 0; i <= 4; i++) {
            float ty = pos.y + (size.y * i * 0.25f);
            dl->AddLine({mid_x - 8, ty}, {mid_x - 4, ty}, IM_COL32(80, 80, 80, 255));
        }

        // 4. Drawing the "Cap" (The plastic handle)
        float cap_h = 20.0f;
        float fraction = (*v - v_min) / (v_max - v_min);
        float cap_y = pos.y + (1.0f - fraction) * (size.y - cap_h);

        ImU32 cap_col = is_active ? IM_COL32(180, 180, 180, 255) : (is_hovered ? IM_COL32(140, 140, 140, 255) : IM_COL32(110, 110, 110, 255));
        dl->AddRectFilled({pos.x, cap_y}, {pos.x + size.x, cap_y + cap_h}, cap_col, 2.0f);
        dl->AddLine({pos.x + 2, cap_y + cap_h * 0.5f}, {pos.x + size.x - 2, cap_y + cap_h * 0.5f}, IM_COL32(0, 0, 0, 200), 2.0f); // Center line on cap

        ImGui::PopID();
        return is_active;
    }


    // ------------- vertical FaderVertical2
    inline bool FaderVertical2(const char* label, ImVec2 size, float* v, float v_min, float v_max, const char* format = "%.2f") {
        ImGui::PushID(label);
        ImVec2 pos = ImGui::GetCursorScreenPos();
        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImGuiIO& io = ImGui::GetIO();

        // 1. Interaction Hitbox
        ImGui::InvisibleButton("##fader", size);

        if (!ImGui::IsItemVisible()) { ImGui::PopID(); return false; }

        // ImGui::GetWindowDrawList()->AddRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), IM_COL32(255,0,0,255));


        bool is_active = ImGui::IsItemActive();
        bool is_hovered = ImGui::IsItemHovered();
        bool changed = false;

        if (is_active && io.MouseDown[0]) {
            float mouse_y = io.MousePos.y - pos.y;
            float fraction = 1.0f - std::clamp(mouse_y / size.y, 0.0f, 1.0f);
            float new_v = v_min + fraction * (v_max - v_min);
            if (new_v != *v) {
                *v = new_v;
                changed = true;
            }
        }

        // 2. Draw Track (Slot)
        float mid_x = pos.x + size.x * 0.5f;
        dl->AddRectFilled({mid_x - 2, pos.y}, {mid_x + 2, pos.y + size.y}, IM_COL32(10, 10, 10, 255), 2.0f);
        dl->AddRect({mid_x - 2, pos.y}, {mid_x + 2, pos.y + size.y}, IM_COL32(60, 60, 60, 255), 2.0f);

        // 3. Draw Hardware Ticks (Scale)
        for (int i = 0; i <= 10; i++) {
            float ty = pos.y + (size.y * i * 0.1f);
            float length = (i % 5 == 0) ? 8.0f : 4.0f; // Longer lines for 0%, 50%, 100%
            dl->AddLine({mid_x - length - 4, ty}, {mid_x - 4, ty}, IM_COL32(90, 90, 90, 255));
        }

        // 4. Draw Handle (Cap)
        float cap_h = 18.0f;
        float fraction = (*v - v_min) / (v_max - v_min);
        float cap_y = pos.y + (1.0f - fraction) * (size.y - cap_h);

        ImRect cap_bb = {{pos.x, cap_y}, {pos.x + size.x, cap_y + cap_h}};
        ImU32 cap_col = is_active ? IM_COL32(150, 150, 150, 255) : (is_hovered ? IM_COL32(120, 120, 120, 255) : IM_COL32(90, 90, 90, 255));

        dl->AddRectFilled(cap_bb.Min, cap_bb.Max, cap_col, 1.0f);
        dl->AddRect(cap_bb.Min, cap_bb.Max, IM_COL32(30, 30, 30, 255), 1.0f);
        // Center "grip" line
        dl->AddLine({cap_bb.Min.x + 2, cap_bb.Min.y + cap_h*0.5f}, {cap_bb.Max.x - 2, cap_bb.Min.y + cap_h*0.5f}, IM_COL32(255, 255, 255, 180));

        // 5. Value Tooltip (Formatting using your format string)
        if (is_hovered || is_active) {
            char val_buf[64];
            ImFormatString(val_buf, IM_ARRAYSIZE(val_buf), format, *v);
            ImGui::SetTooltip("%s: %s", label, val_buf);
        }

        ImGui::PopID();
        return changed;
    }


    // -------------- FaderHorizontal
    inline bool FaderHorizontal(const char* label, ImVec2 size, float* v, float v_min, float v_max) {
        ImGui::PushID(label);
        ImVec2 pos = ImGui::GetCursorScreenPos();
        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImGuiIO& io = ImGui::GetIO();

        // Interaction (Horizontal logic)
        ImGui::InvisibleButton("##fader", size);

        if (!ImGui::IsItemVisible()) { ImGui::PopID(); return false; }

        // bounds check: ImGui::GetWindowDrawList()->AddRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), IM_COL32(255,0,0,255));


        bool is_active = ImGui::IsItemActive();
        bool is_hovered = ImGui::IsItemHovered();


        // ScrollWheel
        if (is_hovered && io.MouseWheel != 0) {
            float wheel_speed = (v_max - v_min) * 0.05f; // 5% of range per notch
            if (io.KeyShift) wheel_speed *= 0.1f;        // 0.5% if Shift is held

            *v = std::clamp(*v + (io.MouseWheel * wheel_speed), v_min, v_max);
            is_active = true;
        }




        if (is_active && io.MouseDown[0]) {
            float mouse_x = io.MousePos.x - pos.x;
            float fraction = std::clamp(mouse_x / size.x, 0.0f, 1.0f);
            *v = v_min + fraction * (v_max - v_min);
        }

        // Drawing the "Track" (Horizontal slot)
        float mid_y = pos.y + size.y * 0.5f;
        dl->AddRectFilled({pos.x, mid_y - 2}, {pos.x + size.x, mid_y + 2}, IM_COL32(20, 20, 20, 255), 2.0f);

        // Drawing Tick Marks (Every 25%)
        for (int i = 0; i <= 4; i++) {
            float tx = pos.x + (size.x * i * 0.25f);
            dl->AddLine({tx, mid_y - 8}, {tx, mid_y - 4}, IM_COL32(80, 80, 80, 255));
        }

        // adding label
        std::string lbStr = std::string(label);
        if (!lbStr.empty())
        {
            Hint(label);
            // TEST: only on short text max 8!
            // if (lbStr.length() < 9)
            dl->AddText(ImVec2(pos.x, pos.y+8.f), IM_COL32(128,128,128,128), label);
        }



        // Drawing the "Cap" (Vertical handle moving horizontally)
        float cap_w = 20.0f; // Width of the handle
        float fraction = (*v - v_min) / (v_max - v_min);
        float cap_x = pos.x + fraction * (size.x - cap_w);

        ImU32 cap_col = is_active ? IM_COL32(180, 180, 180, 255) : (is_hovered ? IM_COL32(140, 140, 140, 255) : IM_COL32(110, 110, 110, 255));

        // Cap matches the vertical handle style but oriented for horizontal travel
        dl->AddRectFilled({cap_x, pos.y}, {cap_x + cap_w, pos.y + size.y}, cap_col, 2.0f);
        dl->AddLine({cap_x + cap_w * 0.5f, pos.y + 2}, {cap_x + cap_w * 0.5f, pos.y + size.y - 2}, IM_COL32(0, 0, 0, 200), 2.0f);


        ImGui::PopID();
        return is_active;
    }
    // -------------- FaderHWithText
    inline bool FaderHWithText(const char* label, float* v, float v_min, float v_max, const char* format,
                               ImVec2 size = ImVec2(150, 18)) {
        ImGui::BeginGroup();
        bool active = FaderHorizontal(label, size, v, v_min, v_max);
        ImGui::SameLine();
        ImGui::TextDisabled(format, *v); // Displays "Hz", "s", or "Wet" next to the fader
        ImGui::EndGroup();
        return active;
    }



}//namespace
