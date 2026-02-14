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
#include "ImFlux/slider.h"
#include "ImFlux/knobs.h"
#include "ImFlux/background.h"
#include "ImFlux/text.h"
#include "ImFlux/peekAndVU.h"
#include "ImFlux/window.h"

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


    inline void PatternEditor16Bit(const char* label,
                                   uint16_t* bits,
                                   uint8_t currentStep, // 0 .. 15
                                   ImU32 color_on = IM_COL32(255, 50, 50, 255),
                                   ImU32 color_step = IM_COL32(255, 255, 255, 180))
    {
        currentStep = currentStep % 16; //fail safe
        ImGui::TextDisabled("%s", label);
        ImGui::SameLine(80.0f);
        for (int i = 15; i >= 0; i--) {
            ImGui::PushID(i);
            bool isSet = (*bits >> i) & 1;
            bool isCurrent = (15 - i) == currentStep;
            ImU32 col = isSet ? color_on : IM_COL32(45, 45, 45, 255);
            if (isCurrent) col = ImGui::GetColorU32(ImGuiCol_CheckMark);
            ImVec2 p = ImGui::GetCursorScreenPos();
            if (ImGui::InvisibleButton("bit", {18, 18})) {
                *bits ^= (1 << i);
            }
            auto drawList = ImGui::GetWindowDrawList();
            drawList->AddRectFilled(p, {p.x + 16, p.y + 16}, col, 2.0f);
            if (isCurrent) {
                drawList->AddRect(p, {p.x + 16, p.y + 16}, color_step, 2.0f, 0, 1.5f);
            }
            if (i > 0) {
                float spacing = (i % 4 == 0) ? 8.0f : 2.0f;
                ImGui::SameLine(0, spacing);
            }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Step %d", 15 - i);
            ImGui::PopID();
        }
        ImGui::NewLine();
    }




} //namespace

