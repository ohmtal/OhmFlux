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
        // Adding a tiny offset ensures black (0) becomes a value that can be scaled
        float r = (col.x + 0.05f) * factor;
        float g = (col.y + 0.05f) * factor;
        float b = (col.z + 0.05f) * factor;

        // Use ImGui::Saturate or clamp to keep values in [0, 1] range
        return ImVec4(ImSaturate(r), ImSaturate(g), ImSaturate(b), col.w);
    }

    inline ImVec4 ModifierColorTint(ImVec4 col, float factor) {
        if (factor > 1.0f) {
            // Blend from current color towards White (1.0)
            float t = factor - 1.0f;
            return ImVec4(
                col.x + (1.0f - col.x) * t,
                          col.y + (1.0f - col.y) * t,
                          col.z + (1.0f - col.z) * t,
                          col.w
            );
        } else {
            // Traditional darkening (scales towards 0)
            return ImVec4(col.x * factor, col.y * factor, col.z * factor, col.w);
        }
    }

    // -------- calculation for a font color when a background is painted
    inline ImVec4 GetContrastColor(ImU32 backgroundColor, bool isSelected = false)
    {
        ImVec4 bg = ImGui::ColorConvertU32ToFloat4(backgroundColor);
        float luminance = (0.299f * bg.x + 0.587f * bg.y + 0.114f * bg.z);
        if (isSelected) return (luminance > 0.5f) ? ImVec4(0,0,0,1) : ImVec4(1,1,1,1);
        else return (luminance > 0.5f) ? ImVec4(0.25f, 0.25f, 0.25f, 1.0f) : ImVec4(0.75f, 0.75f, 0.75f, 1.0f);
    }
    // as U32
    inline ImU32 GetContrastColorU32(ImU32 backgroundColor, bool isSelected = false)
    {
        ImVec4 bg = ImGui::ColorConvertU32ToFloat4(backgroundColor);
        float luminance = (0.299f * bg.x + 0.587f * bg.y + 0.114f * bg.z);
        if (isSelected) return (luminance > 0.5f) ? IM_COL32(0, 0, 0, 255) : IM_COL32(255, 255, 255, 255);
        else return (luminance > 0.5f) ? IM_COL32(64, 64, 64, 255) : IM_COL32(191, 191, 191, 255);
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

    inline void SeparatorVertical(float padding = 20.f, float spacing = 8.f) {
        float currentX = ImGui::GetCursorPosX();
        ImGui::SetCursorPosX(currentX + padding);
        ImGui::SameLine(0.0f, spacing);
        ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
        ImGui::SameLine(0.0f, spacing);
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + padding);
    }

    // inline void SeparatorVertical(float padding = 20.f)
    // {
    //     ImGui::SameLine();
    //     ImGui::Dummy(ImVec2(padding,0.f));
    //     ImGui::SameLine();
    //     ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
    //     ImGui::SameLine();
    //     ImGui::Dummy(ImVec2(padding,0.f));
    //     ImGui::SameLine();
    // }

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
