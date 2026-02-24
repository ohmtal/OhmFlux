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


    //------------------------------------------------------------------------------
    //---------------- LED
    // Examples:
    // ImFlux::DrawLED("ALARM! Test only.", true, ImFlux::LED_RED_ALERT);
    // ImFlux::DrawLED("simply green", true, ImFlux::LED_GREEN);
    // ImFlux::DrawLED("green glow", true, ImFlux::LED_GREEN_GLOW);


    enum LedAnimationTypes {
        LED_Ani_Linear,
        LED_Ani_FADE,
        LED_Ani_PULSE,
        // params.aniPhase must be used !
        // can also be uses for other things with normalized 0.f .. 1.f
        LED_Ani_BEAT,
        // also fro anyPhase but uses the value directly !
        LED_Ani_PHASE,
    };

    struct LedParams {
        float   radius   = 8.f;
        bool    animated = true;
        bool    glow     = true;
        ImColor colorOn  = ImColor(0, 255, 0);
        float aniSpeed   = 4.f;
        float aniRadius  = 2.f;
        LedAnimationTypes animationType = LED_Ani_FADE;
        bool renderMouseEvents  = false;
        float aniPhase   = 0.f; //for LED_Ani_BEAT to 0..1f to sync the beat or other normalized data

        LedParams WithAniType(LedAnimationTypes at) const { LedParams p = *this; p.animationType = at; return p; }
        LedParams WithAniPhase(float phase) const { LedParams p = *this; p.aniPhase = phase; return p; }
        LedParams WithColorOn(ImU32 c) const { LedParams p = *this; p.colorOn = c; return p; }
        LedParams WithRadius(float r) const { LedParams p = *this; p.radius = r; return p; }

    };


    constexpr LedParams LED_WHITE               = { 8.f, false, false, ImColor(200,200,200)};

    constexpr LedParams LED_GREEN               = { 8.f, false, false, ImColor(0, 200, 0)};
    constexpr LedParams LED_GREEN_GLOW          = { 8.f, false, true, ImColor(0, 200, 0), 4.f, 2.f};
    constexpr LedParams LED_GREEN_ANIMATED_GLOW = { 8.f, true, true, ImColor(0, 200, 0), 4.f, 2.f};

    constexpr LedParams LED_YELLOW               = { 8.f, false, false, ImColor(200, 200, 0)};
    constexpr LedParams LED_YELLOW_GLOW          = { 8.f, false, true, ImColor(200, 200, 0), 4.f, 2.f};
    constexpr LedParams LED_YELLOW_ANIMATED_GLOW = { 8.f, true, true, ImColor(200, 200, 0), 4.f, 2.f};


    constexpr LedParams LED_BLUE_GLOW          = { 8.f, false, true, ImColor(128, 128, 255), 4.f, 2.f}; //not really blue
    constexpr LedParams LED_BLUE_ANIMATED_GLOW = { 8.f, true, true, ImColor(128, 128, 255), 4.f, 2.f}; //not really blue

    constexpr LedParams LED_RED                 = { 8.f, false, false, ImColor(255,0, 0)};
    constexpr LedParams LED_RED_ANIMATED_GLOW   = { 8.f, true, true, ImColor(255,0, 0)};

    constexpr LedParams LED_RED_ALERT = { 8.f, true, true, ImColor(255,0, 0), 8.f, 2.f, LED_Ani_PULSE};
    constexpr LedParams LED_YELLOW_ALERT = { 8.f, true, true, ImColor(255,255, 0), 8.f, 2.f, LED_Ani_PULSE};

    constexpr ImFlux::LedParams LED_RED_BEAT = { 8.f, true, true, ImColor(255,0, 0), 8.f, 2.f, ImFlux::LED_Ani_BEAT};
    constexpr ImFlux::LedParams LED_YELLOW_BEAT = { 8.f, true, true, ImColor(255,255,0), 8.f, 2.f, ImFlux::LED_Ani_BEAT};
    constexpr ImFlux::LedParams LED_BLUE_BEAT = { 8.f, true, true, ImColor(0,0, 255), 8.f, 2.f, ImFlux::LED_Ani_BEAT};

    constexpr ImFlux::LedParams LED_RED_PHASE = { 8.f, true, true, ImColor(255,0, 0), 8.f, 2.f, ImFlux::LED_Ani_PHASE};



    inline bool DrawLED(std::string tooltip, bool on, LedParams params, bool embed = false) {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems) return false;

        // 1. Setup ID and Bounding Box

        const ImGuiID id = window->GetID((std::string("##Tooltip") + tooltip).c_str()); // Use tooltip or a unique string as ID
        ImVec2 pos = ImGui::GetCursorScreenPos();
        // looks stupid if animated and non animated in a row, but ....
        float maxRadius = params.radius + (params.animated ? params.aniRadius : 0.0f);
        ImVec2 size(maxRadius * 2.0f, maxRadius * 2.0f);
        ImRect bb(pos, pos + size);

        // 2. Register Item
        if (!embed) {
            ImGui::ItemSize(bb);
            if (!ImGui::ItemAdd(bb, id)) return false;
        }

        // 3. Handle Interactions (Click)
        bool hovered, held;
        bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);

        // 4. Drawing Logic (Same as before)
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImVec2 center = pos + ImVec2(maxRadius, maxRadius);

        if (on) {
            float pulse = 1.0f;
            float t = (float)ImGui::GetTime() * params.aniSpeed;

            if (params.animated) {
                switch (params.animationType) {
                    case LED_Ani_Linear: pulse = (sinf(t) * 0.5f) + 0.5f; break;
                    case LED_Ani_FADE:   pulse = expf(sinf(t)) / 2.71828f; break;
                    case LED_Ani_PULSE:  pulse = powf(std::max(0.0f, sinf(t)), 8.0f); break;
                    case LED_Ani_BEAT:
                        //too blinky pulse = powf(std::max(0.0f, cosf(params.aniPhase * M_2PI)), 8.0f);
                        //still too blinky pulse = powf(std::max(0.0f, cosf(params.aniPhase * M_2PI)), 3.0f);
                        // best so far
                        pulse = (cosf(params.aniPhase * M_2PI) * 0.4f) + 0.6f;

                        // also good but more to calculate
                        // float c = cosf(params.aniPhase * M_2PI);pulse = expf(c - 1.0f);
                        break;
                    case LED_Ani_PHASE: pulse = std::clamp(params.aniPhase, 0.0f, 1.0f); break;
                }
            }

            // If 'held' (clicking), we can shrink it slightly for visual feedback
            float currentRadius = params.animated ?  params.radius + (pulse * params.aniRadius) : params.radius;
            if ( held && params.renderMouseEvents ) currentRadius *= 0.9f;

            float alphaMod = 0.3f + (pulse * 0.7f);
            if (hovered) alphaMod = std::min(1.0f, alphaMod + 0.2f); // Glow brighter on hover

            // Draw Glow
            if (params.glow) {
                for (int i = 6; i > 0; i--) {
                    float layerAlpha = ((1.0f - (float)i / 6.0f) * 0.2f) * alphaMod;
                    ImU32 glow_col = ImGui::ColorConvertFloat4ToU32(ImVec4(params.colorOn.Value.x, params.colorOn.Value.y, params.colorOn.Value.z, layerAlpha));
                    draw_list->AddCircleFilled(center, currentRadius + (i * 2.0f), glow_col, 24);
                }
            }

            // Draw Core
            ImU32 core_col = ImGui::ColorConvertFloat4ToU32(ImVec4(params.colorOn.Value.x, params.colorOn.Value.y, params.colorOn.Value.z, params.colorOn.Value.w * alphaMod));
            draw_list->AddCircleFilled(center, currentRadius, core_col, 24);
            draw_list->AddCircleFilled(center, currentRadius * 0.35f, IM_COL32(255, 255, 255, (int)(180 * alphaMod)), 24);

        } else {
            // Off State: Subtle highlight on hover
            ImU32 ring_col = (hovered && params.renderMouseEvents) ? IM_COL32(120, 120, 120, 255) : IM_COL32(80, 80, 80, 255);
            ImU32 core_col = (hovered && params.renderMouseEvents) ? IM_COL32(60, 60, 60, 255) : IM_COL32(40, 40, 40, 255);

            draw_list->AddCircleFilled(center, params.radius, core_col, 24);
            draw_list->AddCircle(center, params.radius, ring_col, 24, 1.0f);
        }

        // 5. Tooltip
        if (!tooltip.empty() && hovered) {
            ImGui::SetTooltip("%s", tooltip.c_str());
        }

        return pressed; // Returns true on the frame it was clicked
    }

    //---------------- LEDCheckBox (default glow)

    inline bool LEDCheckBox(const std::string& caption, bool* on, const ImVec4 color, const ImVec4 fontColor = {-1.f, -1.f, -1.f, -1.f}) {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems) return false;

        //  Generate ID: Handle empty captions to avoid collision with window ID
        const ImGuiID id = window->GetID(caption.empty() ? "##led_checkbox_hidden" : caption.c_str());

        LedParams params = LED_GREEN_GLOW;
        params.colorOn = color;

        //  Calculate Dimensions
        const ImGuiStyle& style = ImGui::GetStyle();
        float maxRadius = params.radius + (params.animated ? params.aniRadius : 0.0f);
        float ledSize = maxRadius * 2.0f;
        ImVec2 text_size = ImGui::CalcTextSize(caption.c_str(), nullptr, true);

        float spacing = 4.0f;
        // Total size: LED width + spacing + text width
        ImVec2 size(
            ledSize + (text_size.x > 0.0f ? spacing + text_size.x : 0.0f),
                    ImMax(ledSize, ImGui::GetFrameHeight()) // Ensure it respects standard frame height
        );

        const ImRect bb(window->DC.CursorPos, window->DC.CursorPos + size);

        // Register Item: This advances the CursorPos for the NEXT widget
        ImGui::ItemSize(size, style.FramePadding.y);
        if (!ImGui::ItemAdd(bb, id)) return false;

        // Interaction
        bool hovered, held;
        bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);
        if (pressed) {
            *on = !(*on);
        }

        // Render LED
        ImVec2 backup_pos = window->DC.CursorPos;
        window->DC.CursorPos = bb.Min;
        DrawLED("##led_internal", *on, params, true);

        // Render Text
        if (text_size.x > 0.0f) {
            ImVec2 text_pos = ImVec2(
                bb.Min.x + ledSize + spacing,
                bb.Min.y + (size.y - text_size.y) * 0.5f
            );

            ImU32 fcol = fontColor.x==-1.f ? ImGui::ColorConvertFloat4ToU32(color) : ImGui::ColorConvertFloat4ToU32(fontColor);
            window->DrawList->AddText(text_pos + ImVec2(1.f,1.f),IM_COL32(32,32,32,255), caption.c_str());
            window->DrawList->AddText(text_pos,fcol, caption.c_str());
        }

        window->DC.CursorPos = backup_pos;

        return pressed;
    }

//------------------------------------------------------------------------------
};
