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
    inline void GradientBoxDL(ImDrawList* dl, ImRect bb, float rounding, bool inset = true) {
        if (inset) {
            // 1. "Floor" Gradient: Darker at the top, slightly lighter at the bottom
            // This ensures visibility even on a pitch-black window background
            ImU32 col_top = IM_COL32(25, 25, 30, 255);
            ImU32 col_bot = IM_COL32(45, 45, 55, 255);
            dl->AddRectFilledMultiColor(bb.Min, bb.Max, col_top, col_top, col_bot, col_bot);

            // 2. Inner Shadow (Top/Left) - The "Cavity" effect
            dl->AddRect(bb.Min, bb.Max, IM_COL32(0, 0, 0, 200), rounding);

            // 3. The "Rim Light" (Bottom/Right) - Makes it pop on dark UI
            // We draw two lines to create the edge that catches the ambient light
            dl->AddLine(ImVec2(bb.Min.x + rounding, bb.Max.y),
                        ImVec2(bb.Max.x - rounding, bb.Max.y), IM_COL32(255, 255, 255, 50));
            dl->AddLine(ImVec2(bb.Max.x, bb.Min.y + rounding),
                        ImVec2(bb.Max.x, bb.Max.y - rounding), IM_COL32(255, 255, 255, 50));

        } else {
            // --- Raised State ---
            // Base fill
            dl->AddRectFilled(bb.Min, bb.Max, IM_COL32(55, 55, 65, 255), rounding);

            // Glass Glare on the top half
            float glare_inset = (rounding > 0.0f) ? rounding * 0.2f : 2.0f;
            ImVec2 glare_min = bb.Min + ImVec2(glare_inset, 1);
            ImVec2 glare_max = ImVec2(bb.Max.x - glare_inset, bb.Min.y + bb.GetSize().y * 0.45f);

            dl->AddRectFilledMultiColor(
                glare_min, glare_max,
                IM_COL32(255, 255, 255, 65), IM_COL32(255, 255, 255, 65),
                                        IM_COL32(255, 255, 255, 0),  IM_COL32(255, 255, 255, 0)
            );

            // Subtle Outer Bevel
            dl->AddRect(bb.Min, bb.Max, IM_COL32(255, 255, 255, 45), rounding);
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

        ImRect bb(pos, pos + lSize);

        GradientBoxDL(ImGui::GetWindowDrawList(), bb, rounding, inset);
    }

//------------------------------------------------------------------------------
};
