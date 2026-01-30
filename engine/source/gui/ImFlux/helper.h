//-----------------------------------------------------------------------------
// Copyright (c) 2026 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#pragma once

#include <imgui.h>
#include <imgui_internal.h>
#include <cmath>
#include <algorithm>


namespace ImFlux {

    // -------- we alter the color
    inline ImVec4 ModifierColor(ImVec4 col, float factor) {
        return ImVec4(col.x * factor, col.y * factor, col.z * factor, col.w);
    }
    //---------------- Hint
    inline void Hint(std::string tooltip)
    {
        if (!tooltip.empty() && ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) {
            ImGui::SetTooltip("%s", tooltip.c_str());
        }
    }

    //---------------- SeparatorVertical
    inline void SeparatorVertical(float padding = 20.f)
    {
        ImGui::SameLine();
        ImGui::Dummy(ImVec2(padding,0.f));
        ImGui::SameLine();
        ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
        ImGui::SameLine();
        ImGui::Dummy(ImVec2(padding,0.f));
        ImGui::SameLine();
    }



}
