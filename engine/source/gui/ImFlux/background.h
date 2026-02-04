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
    struct GradientParams {
        ImVec2 pos  = ImVec2(0, 0);
        ImVec2 size = ImVec2(0, 0);
        float  rounding = 0.f;
        bool   inset    = true;

        // Background Colors
        ImU32 col_top   = IM_COL32(25, 25, 30, 255);
        ImU32 col_bot   = IM_COL32(45, 45, 55, 255);

        // Decoration Colors
        ImU32 col_shadow = IM_COL32(0, 0, 0, 200);      // Inner shadow
        ImU32 col_rim    = IM_COL32(255, 255, 255, 50); // Light edge
        ImU32 col_raised = IM_COL32(55, 55, 65, 255);   // Base for !inset

        // Fluent interface helpers
        GradientParams WithPosSize(ImVec2 p, ImVec2 s) const { GradientParams gp = *this; gp.pos = p; gp.size = s; return gp; }
        GradientParams WithRounding(float r) const { GradientParams gp = *this; gp.rounding = r; return gp; }
        GradientParams WithInset(bool i) const { GradientParams gp = *this; gp.inset = i; return gp; }
    };

    constexpr GradientParams DEFAULT_GRADIENTBOX;

    inline void GradientBoxDL(GradientParams gp, ImDrawList* dl = nullptr) {
        // 1. Fallback for DrawList (Get current window if none provided)
        if (dl == nullptr) dl = ImGui::GetWindowDrawList();

        // 2. Fallback for Position (Use current cursor screen position if 0,0)
        ImVec2 start_p = gp.pos;
        if (start_p.x == 0.0f && start_p.y == 0.0f) {
            start_p = ImGui::GetCursorScreenPos();
        }

        // 3. Fallback for Size (Fill available region if size is 0 or negative)
        ImVec2 size = gp.size;
        if (size.x <= 0.0f) size.x = ImGui::GetContentRegionAvail().x;
        if (size.y <= 0.0f) size.y = ImGui::GetContentRegionAvail().y;

        ImVec2 max_p = ImVec2(start_p.x + size.x, start_p.y + size.y);

        if (gp.inset) {
            // --- Inset State (Hole/Cavity effect) ---
            // Main gradient background
            dl->AddRectFilledMultiColor(start_p, max_p, gp.col_top, gp.col_top, gp.col_bot, gp.col_bot);

            // Inner shadow stroke
            dl->AddRect(start_p, max_p, gp.col_shadow, gp.rounding);

            // Bottom-Right Rim Light (Ambient occlusion)
            dl->AddLine(ImVec2(start_p.x + gp.rounding, max_p.y),
                        ImVec2(max_p.x - gp.rounding, max_p.y), gp.col_rim);
            dl->AddLine(ImVec2(max_p.x, start_p.y + gp.rounding),
                        ImVec2(max_p.x, max_p.y - gp.rounding), gp.col_rim);
        }
        else {
            // --- Raised State (Button/Elevated effect) ---
            // Base solid fill
            dl->AddRectFilled(start_p, max_p, gp.col_raised, gp.rounding);

            // Glass Glare (Upper half shine)
            float glare_h = size.y * 0.45f;
            dl->AddRectFilledMultiColor(
                ImVec2(start_p.x + 2, start_p.y + 1),
                                        ImVec2(max_p.x - 2, start_p.y + glare_h),
                                        IM_COL32(255, 255, 255, 45), IM_COL32(255, 255, 255, 45), // Top bright
                                        IM_COL32(255, 255, 255, 0),  IM_COL32(255, 255, 255, 0)   // Bottom transparent
            );

            // Subtle outer bevel stroke
            dl->AddRect(start_p, max_p, gp.col_rim, gp.rounding);
        }
    }
    //--------------------------------------------------------------------------
    inline void GradientBox(ImVec2 size, float rounding, bool inset = true)
    {
        ImVec2 pos = ImGui::GetCursorScreenPos();
        ImVec2 avail = ImGui::GetContentRegionAvail();

        ImVec2 lSize = size;

        // Calculate width: if -FLT_MIN or 0, use full available width
        if (lSize.x <= 0.0f) lSize.x = avail.x + lSize.x;

        // Calculate height: if -FLT_MIN or 0, use full available height (this was the bug)
        if (lSize.y <= 0.0f) lSize.y = avail.y + lSize.y;


        GradientParams gp =  DEFAULT_GRADIENTBOX.WithPosSize(pos,lSize);
        gp.rounding = rounding;
        gp.inset = inset;

        GradientBoxDL(gp);
    }
    //--------------------------------------------------------------------------
    inline void GradientBox(ImVec2 size, GradientParams gp = DEFAULT_GRADIENTBOX)
    {
        ImVec2 pos = ImGui::GetCursorScreenPos();
        ImVec2 avail = ImGui::GetContentRegionAvail();

        ImVec2 lSize = size;

        // Calculate width: if -FLT_MIN or 0, use full available width
        if (lSize.x <= 0.0f) lSize.x = avail.x + lSize.x;

        // Calculate height: if -FLT_MIN or 0, use full available height (this was the bug)
        if (lSize.y <= 0.0f) lSize.y = avail.y + lSize.y;


        gp.pos = pos;
        gp.size = size;

        GradientBoxDL(gp);
    }

//------------------------------------------------------------------------------
};
