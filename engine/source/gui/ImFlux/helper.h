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

    //---------------- GetLabelText extraxt ## stuff
    inline std::string GetLabelText(std::string label) {
        const char* begin = label.c_str();
        const char* end = ImGui::FindRenderedTextEnd(begin);
        return std::string(begin, end);
    }

    // -------- we alter the color
    inline ImVec4 ModifierColor(ImVec4 col, float factor) {
        return ImVec4(col.x * factor, col.y * factor, col.z * factor, col.w);
    }
    //---------------- Hint
    inline void Hint(std::string tooltip )
    {

        if (!tooltip.empty() && ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) {
            ImGui::PushFont(ImGui::GetDefaultFont());

            // ImGui::SetTooltip("%s", tooltip.c_str());
            ImGui::BeginTooltip();
            ImGui::TextUnformatted(tooltip.c_str());
            ImGui::EndTooltip();

            ImGui::PopFont();
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

    //---------------- Separator horizontal but in a group
    inline void GroupSeparator(float width ) {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems) return;

        if (width <= 0.0f)
            width = ImGui::GetContentRegionAvail().x;
        ImVec2 pos = ImGui::GetCursorScreenPos();
        window->DrawList->AddLine(
            pos,
            ImVec2(pos.x + width, pos.y),
                                  ImGui::GetColorU32(ImGuiCol_Separator)
        );
        ImGui::ItemSize(ImVec2(width, 1.0f));
    }


}
