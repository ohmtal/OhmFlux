//-----------------------------------------------------------------------------
// Copyright (c) 2026 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#pragma once

#include <imgui.h>
#include <imgui_internal.h>
#include <cmath>
#include <algorithm>
#include <string>


namespace ImFlux {

    // ----------------- AutoLineBreak
    inline void SameLineBreak(float nextItemWidth) {
        float windowVisibleX2 = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;
        float lastItemX2 = ImGui::GetItemRectMax().x;
        float nextItemX2 = lastItemX2 + ImGui::GetStyle().ItemSpacing.x + nextItemWidth;

        if (nextItemX2 < windowVisibleX2) {
            ImGui::SameLine();
        }
    }
    // ----------------- shift cursor
    inline void ShiftCursor(ImVec2 offset) {
        ImGui::SetCursorPos(ImGui::GetCursorPos() + offset);
    }

    // ------- for lists where i want different colors per index
    inline ImU32 getColorByIndex(uint16_t idx, float seed = 0.f) {
        const float golden_ratio_conjugate = 0.618033988749895f;
        float hue = fmodf((float)idx * golden_ratio_conjugate + seed, 1.0f);
        ImVec4 colRGB;
        ImGui::ColorConvertHSVtoRGB(hue, 0.7f, 0.9f, colRGB.x, colRGB.y, colRGB.z);
        colRGB.w = 1.0f;
        return ImGui::GetColorU32(colRGB);
    }




    // ---------------- Some Neon Colors
    constexpr ImU32 COL32_NEON_CYAN    = IM_COL32(0, 255, 255, 255);
    constexpr ImU32 COL32_NEON_GREEN   = IM_COL32(57, 255, 20, 255);  //
    constexpr ImU32 COL32_NEON_PINK    = IM_COL32(255, 0, 255, 255);  //
    constexpr ImU32 COL32_NEON_YELLOW  = IM_COL32(255, 255, 0, 255);  //
    constexpr ImU32 COL32_NEON_RED     = IM_COL32(255, 49, 49, 255);
    constexpr ImU32 COL32_NEON_PURPLE  = IM_COL32(160, 0, 208, 255);  //
    constexpr ImU32 COL32_NEON_ORANGE  = IM_COL32(255, 173, 0, 255);
    constexpr ImU32 COL32_NEON_ELECTRIC= IM_COL32(125, 249, 255, 255);

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

    inline ImU32 ModifyRGB(ImU32 col, float factor) {
        // Extract channels using ImGui's packing format (usually RGBA)
        float r = ((col >> IM_COL32_R_SHIFT) & 0xFF) / 255.0f;
        float g = ((col >> IM_COL32_G_SHIFT) & 0xFF) / 255.0f;
        float b = ((col >> IM_COL32_B_SHIFT) & 0xFF) / 255.0f;
        ImU32 a = (col >> IM_COL32_A_SHIFT) & 0xFF;

        // Apply factor with your offset logic
        r = ImSaturate((r + 0.05f) * factor);
        g = ImSaturate((g + 0.05f) * factor);
        b = ImSaturate((b + 0.05f) * factor);

        // Reassemble to ImU32 keeping the original Alpha
        return IM_COL32((int)(r * 255), (int)(g * 255), (int)(b * 255), a);
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

    //---------------- SameLineCentered very helpfull :D
    inline void SameLineCentered(float targetHeight) {
        float currentHeight = ImGui::GetItemRectSize().y;
        ImGui::SameLine();
        // Move cursor down if the next item is smaller than the previous one
        if (currentHeight > targetHeight) {
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (currentHeight - targetHeight) * 0.5f);
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


    // EierlegendeWollMilchSau Separator
    inline void SeparatorFancy(ImGuiSeparatorFlags flags = ImGuiSeparatorFlags_Horizontal, float thickness = 1.0f, float padding = 8.f, float spacing = 8.f, bool inset = true) {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems) return;

        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 pos = ImGui::GetCursorScreenPos();
        ImVec2 avail = ImGui::GetContentRegionAvail();

        if (flags & ImGuiSeparatorFlags_Vertical) {
            // --- VERTICAL ---
            // 1. Spacing before (Horizontal space)
            ImGui::SameLine(0, spacing);
            pos = ImGui::GetCursorScreenPos(); // Update pos after SameLine

            // 2. Calculate vertical line points with padding
            ImVec2 p1 = pos + ImVec2(0, padding);
            ImVec2 p2 = pos + ImVec2(0, avail.y - padding);

            // 3. Draw Inset Style
            dl->AddLine(p1, p2, IM_COL32(0, 0, 0, 150), thickness); // Shadow
            if (inset) dl->AddLine(p1 + ImVec2(1, 0), p2 + ImVec2(1, 0), IM_COL32(255, 255, 255, 30), thickness); // Rim

            // 4. Advance layout
            ImGui::Dummy(ImVec2(thickness, avail.y));
            ImGui::SameLine(0, spacing);

        } else {
            // --- HORIZONTAL (Default) ---
            // 1. Spacing before (Vertical space)
            ImGui::Dummy(ImVec2(0, spacing));
            pos = ImGui::GetCursorScreenPos();

            // 2. Calculate horizontal line points with padding
            ImVec2 p1 = pos + ImVec2(padding, 0);
            ImVec2 p2 = pos + ImVec2(avail.x - padding, 0);

            // 3. Draw Inset Style
            dl->AddLine(p1, p2, IM_COL32(0, 0, 0, 150), thickness); // Shadow
            if (inset) dl->AddLine(p1 + ImVec2(0, 1), p2 + ImVec2(0, 1), IM_COL32(255, 255, 255, 30), thickness); // Rim

            // 4. Advance layout
            ImGui::Dummy(ImVec2(avail.x, spacing));
        }
    }

    inline void SeparatorHInset() {
        SeparatorFancy(ImGuiSeparatorFlags_Horizontal, 1.f, 8.f,8.f,true);
    }
    inline void SeparatorVInset() {
        SeparatorFancy(ImGuiSeparatorFlags_Vertical, 1.f, 8.f,8.f,true);
    }


    //---------------- Separator horizontal but in a group
    inline void GroupSeparator(float width ) {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems) return;

        if (width <= 0.0f)
            width = ImGui::GetContentRegionAvail().x;
        ImVec2 pos = ImGui::GetCursorScreenPos();
        window->DrawList->AddLine(
            pos, ImVec2(pos.x + width, pos.y),
            ImGui::GetColorU32(ImGuiCol_Separator)
        );
        ImGui::ItemSize(ImVec2(width, 1.0f));
    }


}
