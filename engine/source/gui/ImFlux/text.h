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

    inline void ShadowText(const char* label, ImU32 textColor = IM_COL32(200,200,200,255), ImU32 shadowColor = IM_COL32_BLACK) {
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


//------------------------------------------------------------------------------
};
