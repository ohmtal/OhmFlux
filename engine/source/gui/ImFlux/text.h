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

    //--------------------------------------------------------------------------
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

    //--------------------------------------------------------------------------

    inline void ShadowText(const char* label, ImU32 textColor = IM_COL32(200,200,200,255), ImU32 shadowColor = IM_COL32(30, 30, 30, 200)) {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems) return;

        if (ImGui::GetItemFlags() & ImGuiItemFlags_Disabled) {
            textColor = ImGui::GetColorU32(ImGuiCol_TextDisabled);
            ImU32 alpha = (shadowColor >> IM_COL32_A_SHIFT) & 0xFF;
            shadowColor = (shadowColor & ~IM_COL32_A_MASK) | ((alpha / 2) << IM_COL32_A_SHIFT);
        }

        std::string lLabelStr = ImFlux::GetLabelText(label);
        const char* text_begin = lLabelStr.c_str();
        const char* text_end = text_begin + lLabelStr.size();

        ImVec2 textSize = ImGui::CalcTextSize(text_begin, text_end);

        ImVec2 textPos = ImGui::GetCursorScreenPos();
        ImRect bb(textPos, textPos + textSize);
        ImGui::ItemSize(textSize);
        if (!ImGui::ItemAdd(bb, 0)) return;
        ImDrawList* dl = ImGui::GetWindowDrawList();
        dl->AddText(textPos + ImVec2(1.0f, 1.0f), shadowColor, text_begin, text_end);
        dl->AddText(textPos, textColor, text_begin, text_end);
    }

    //--------------------------------------------------------------------------
    // add a text on a position via DrawList
    enum class TextAlign {
        Left, Center, Right
    };

    inline void AddCenteredText(ImDrawList* dl, ImVec2 pos, ImVec2 size, const char* label, TextAlign align = TextAlign::Center, ImU32 color = IM_COL32_WHITE) {
        if (!label || label[0] == '\0') return;

        ImVec2 textSize = ImGui::CalcTextSize(label);
        ImVec2 textPos = pos;


        if (align == TextAlign::Center) {
            textPos.x += (size.x - textSize.x) * 0.5f;
        } else if (align == TextAlign::Right) {
            textPos.x += (size.x - textSize.x);
        }

        textPos.y += (size.y - textSize.y) * 0.5f;

        dl->AddText(textPos, color, label);
    }

    inline void AddCenteredShadowText(ImDrawList* dl, ImVec2 pos, ImVec2 size, const char* label, TextAlign align = TextAlign::Center,
                            ImU32 textColor = IM_COL32(200,200,200,255), ImU32 shadowColor = IM_COL32(0, 0, 0, 200))
    {
        AddCenteredText(dl, pos + ImVec2(1, 1), size, label, align, shadowColor);
        AddCenteredText(dl, pos, size, label, align, textColor);
    }



};
