//-----------------------------------------------------------------------------
// Copyright (c) 2026 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#pragma once

#include <imgui.h>
#include <imgui_internal.h>
#include <cmath>
#include <algorithm>

#include "ImFlux/buttons.h"
#include "ImFlux/comboAndStepper.h"
#include "ImFlux/helper.h"
#include "ImFlux/lcd.h"
#include "ImFlux/led.h"
#include "ImFlux/sliderAndKnobs.h"

namespace ImFlux {


    //------------------------------------------------------------------------------
    //MISC:
    //------------------------------------------------------------------------------
    // TextColoredEllipsis a text with a limited width
    inline void TextColoredEllipsis(ImVec4 color, std::string text, float maxWidth) {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems) return;

        const char* text_begin = text.c_str();
        const char* text_end = text_begin + text.size();

        ImVec2 pos = window->DC.CursorPos;
        ImRect bb(pos, ImVec2(pos.x + maxWidth, pos.y + ImGui::GetTextLineHeight()));

        ImGui::ItemSize(bb);
        if (!ImGui::ItemAdd(bb, 0)) return;

        ImGui::PushStyleColor(ImGuiCol_Text, color);
        // RenderTextEllipsis(ImDrawList* draw_list, const ImVec2& pos_min, const ImVec2& pos_max, float ellipsis_max_x, const char* text, const char* text_end, const ImVec2* text_size_if_known);
        ImGui::RenderTextEllipsis(
            ImGui::GetWindowDrawList(),
            bb.Min,             // pos_min
            bb.Max,             // pos_max
            bb.Max.x,           // ellipsis_max_x
            text_begin,         // text
            text_end,           // text_end
            NULL                // text_size_if_known
        );
        ImGui::PopStyleColor();
    }



    // ------------ BitEditor
    inline void BitEditor(const char* label, uint8_t* bits, ImU32  color_on = IM_COL32(255, 0, 0, 255)) {
        ImGui::TextDisabled("%s", label); ImGui::SameLine();
        for (int i = 7; i >= 0; i--) {
            ImGui::PushID(i);
            bool val = (*bits >> i) & 1;
            ImU32 col = val ? color_on : IM_COL32(60, 20, 20, 255);

            ImVec2 p = ImGui::GetCursorScreenPos();
            if (ImGui::InvisibleButton("bit", {12, 12})) {
                *bits ^= (1 << i);
            }
            ImGui::GetWindowDrawList()->AddRectFilled(p, {p.x + 10, p.y + 10}, col, 2.0f);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Bit %d", i);

            ImGui::SameLine();
            ImGui::PopID();
        }
        ImGui::NewLine();
    }


    //------------------------------------------------------------------------------
    // Simple PeakMeter
    inline void PeakMeter(float level) {
        ImVec2 p = ImGui::GetCursorScreenPos();
        ImVec2 size = ImVec2(100, 6);
        ImDrawList* dl = ImGui::GetWindowDrawList();

        dl->AddRectFilled(p, {p.x + size.x, p.y + size.y}, IM_COL32(40, 40, 40, 255));
        float fill = std::clamp(level, 0.0f, 1.0f) * size.x;
        ImU32 col = level > 0.9f ? IM_COL32(255, 50, 50, 255) : IM_COL32(50, 255, 50, 255);
        dl->AddRectFilled(p, {p.x + fill, p.y + size.y}, col);

        ImGui::Dummy(size);
    }







} //namespace

