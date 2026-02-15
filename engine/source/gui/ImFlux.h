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
    //--------------------------------------------------------------------------
    inline bool PatternEditor16Bit(const char* label,
                                   uint16_t* bits,
                                   int8_t currentStep,
                                   ImU32 color_on = IM_COL32(255, 50, 50, 255),
                                   ImU32 color_step = IM_COL32(255, 255, 255, 180))
    {
        bool changed = false;
        const float labelWidth = 100.f;
        const ImU32 color_off = IM_COL32(45, 45, 45, 255);

        static bool isDragging = false;
        static bool dragTargetState = false;

        ImGui::PushID(label);
        ImFlux::TextColoredEllipsis(ImVec4(0.6f, 0.6f, 0.8f, 1.f), label, labelWidth);
        ImGui::SameLine(labelWidth);

        for (int i = 15; i >= 0; i--) {
            ImGui::PushID(i);
            bool isSet = (*bits >> i) & 1;
            bool isCurrent = (15 - i) == currentStep;
            ImU32 col = isSet ? color_on : color_off;
            if (isCurrent) {
                col = isSet ? ImFlux::ModifyRGB(color_on, 1.2f) : ImFlux::ModifyRGB(color_off, 1.4f);
            }
            ImVec2 p = ImGui::GetCursorScreenPos();
            ImGui::InvisibleButton("bit", {18, 18});
            if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
                dragTargetState = !isSet;
                isDragging = true;
                if (dragTargetState) *bits |= (1 << i);
                else *bits &= ~(1 << i);
                changed = true;
            }
            else if (isDragging && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem)) {
                if (isSet != dragTargetState) {
                    if (dragTargetState) *bits |= (1 << i);
                    else *bits &= ~(1 << i);
                    changed = true;
                }
            }
            auto drawList = ImGui::GetWindowDrawList();
            drawList->AddRectFilled(p, {p.x + 16, p.y + 16}, col, 2.0f);
            if (isCurrent) {
                drawList->AddRect(p, {p.x + 16, p.y + 16}, color_step, 2.0f, 0, 1.5f);
            }

            if (i > 0) {
                float spacing = (i % 4 == 0) ? 6.0f : 1.5f;
                ImGui::SameLine(0, spacing);
            }

            ImGui::PopID();
        }
        if (isDragging && !ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
            isDragging = false;
        }
        ImGui::SameLine();
        int bitsInt = (int) *bits;
        ImGui::SetNextItemWidth(60.f);
        if (ImGui::InputInt("##patValue", &bitsInt,0,0/*, ImGuiInputTextFlags_CharsHexadecimal*/)) { *bits = bitsInt; changed = true;}

        ImGui::PopID();
        return changed;
    }

    //--------------------------------------------------------------------------
    //........... stable version :




} //namespace

