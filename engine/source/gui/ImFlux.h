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
#include "ImFlux/music.h"

namespace ImFlux {


    //------------------------------------------------------------------------------
    //MISC:
    //------------------------------------------------------------------------------
    // ------------ BitEditor
    inline bool BitEditor(const char* label, uint8_t* bits, ImU32  color_on = IM_COL32(255, 0, 0, 255)) {
        bool changed = false;
        ImGui::PushID(label);

        const float labelWidth = 100.f;
        ImFlux::TextColoredEllipsis(ImVec4(0.6f,0.6f,0.8f,1.f), label, labelWidth );
        ImGui::SameLine(labelWidth);

        for (int i = 7; i >= 0; i--) {
            ImGui::PushID(i);
            bool val = (*bits >> i) & 1;
            ImU32 col = val ? color_on : IM_COL32(60, 20, 20, 255);

            ImVec2 p = ImGui::GetCursorScreenPos();
            if (ImGui::InvisibleButton("bit", {12, 12})) {
                *bits ^= (1 << i);
                changed = true;
            }
            ImGui::GetWindowDrawList()->AddRectFilled(p, {p.x + 10, p.y + 10}, col, 2.0f);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Bit %d", i);

            ImGui::SameLine();
            ImGui::PopID();
        }
        // ImGui::NewLine();
        ImGui::PopID();
        return changed;
    }




} //namespace

