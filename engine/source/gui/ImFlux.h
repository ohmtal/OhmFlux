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

