//-----------------------------------------------------------------------------
// Copyright (c) 2026 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#pragma once

#include <core/fluxBaseObject.h>
#include <utils/fluxSettingsManager.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <cmath>
#include <algorithm>

namespace ImFlux {


    //------------------------------------------------------------------------------
    inline void Hint(std::string tooltip)
    {
        if (!tooltip.empty() && ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) {
            ImGui::SetTooltip("%s", tooltip.c_str());
        }
    }

    //------------------------------------------------------------------------------
    //---------------- LED
    // Examples:
    // ImFlux::DrawLED("ALARM! Test only.", true, ImFlux::LED_RED_ALERT);
    // ImFlux::DrawLED("simply green", true, ImFlux::LED_GREEN);
    // ImFlux::DrawLED("green glow", true, ImFlux::LED_GREEN_GLOW);


    enum LedAnimationTypes {
        LED_Ani_Linear,
        LED_Ani_FADE,
        LED_Ani_PULSE
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

    };



    constexpr LedParams LED_GREEN               = { 8.f, false, false, ImColor(0, 255, 0)};
    constexpr LedParams LED_GREEN_GLOW          = { 8.f, false, true, ImColor(0, 255, 0), 4.f, 2.f};
    constexpr LedParams LED_GREEN_ANIMATED_GLOW = { 8.f, true, true, ImColor(0, 255, 0), 4.f, 2.f};

    constexpr LedParams LED_BLUE_GLOW          = { 8.f, false, true, ImColor(128, 128, 255), 4.f, 2.f}; //not really blue

    constexpr LedParams LED_RED                 = { 8.f, false, false, ImColor(255,0, 0)};
    constexpr LedParams LED_RED_ANIMATED_GLOW   = { 8.f, true, true, ImColor(255,0, 0)};

    constexpr LedParams LED_RED_ALERT = { 8.f, true, true, ImColor(255,0, 0), 8.f, 2.f, LED_Ani_PULSE};

    inline bool DrawLED(std::string tooltip, bool on, LedParams params) {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems) return false;

        // 1. Setup ID and Bounding Box
        const ImGuiID id = window->GetID(tooltip.c_str()); // Use tooltip or a unique string as ID
        ImVec2 pos = ImGui::GetCursorScreenPos();
        float maxRadius = params.radius + (params.animated ? params.aniRadius : 0.0f);
        ImVec2 size(maxRadius * 2.0f, maxRadius * 2.0f);
        ImRect bb(pos, pos + size);

        // 2. Register Item
        ImGui::ItemSize(bb);
        if (!ImGui::ItemAdd(bb, id)) return false;

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


    // inline void DrawLED(std::string tooltip, bool on, LedParams params) {
    //     ImGuiWindow* window = ImGui::GetCurrentWindow();
    //     if (window->SkipItems) return;
    //
    //
    //
    //     ImVec2 pos = ImGui::GetCursorScreenPos();
    //
    //     // Calculate max radius to keep layout stable
    //     float maxRadius = params.radius + (params.animated ? params.aniRadius : 0.0f);
    //     ImVec2 size(maxRadius * 2.0f, maxRadius * 2.0f);
    //     ImRect bb(pos, pos + size);
    //
    //     ImGui::ItemSize(bb);
    //     if (!ImGui::ItemAdd(bb, 0)) return;
    //
    //     ImDrawList* draw_list = ImGui::GetWindowDrawList();
    //     ImVec2 center = pos + ImVec2(maxRadius, maxRadius);
    //
    //     if (on) {
    //         float pulse = 1.0f;
    //         float t = (float)ImGui::GetTime() * params.aniSpeed;
    //
    //         if (params.animated) {
    //             switch (params.animationType) {
    //                 case LED_Ani_Linear:
    //                     // Constant smooth oscillation
    //                     pulse = (sinf(t) * 0.5f) + 0.5f;
    //                     break;
    //                 case LED_Ani_FADE:
    //                     // Exponential/Natural breathing (lingers at extremes)
    //                     pulse = expf(sinf(t)) / 2.71828f;
    //                     break;
    //                 case LED_Ani_PULSE:
    //                     // Sharp "Heartbeat" or Strobe effect
    //                     pulse = powf(sinf(t) > 0 ? sinf(t) : 0, 8.0f);
    //                     break;
    //             }
    //         }
    //
    //         float currentRadius = params.animated ?  params.radius + (pulse * params.aniRadius) : params.radius;
    //         // Alpha modulates based on pulse for all animated types
    //         float alphaMod = 0.3f + (pulse * 0.7f);
    //
    //         // 1. Glow Effect
    //         if (params.glow) {
    //             for (int i = 6; i > 0; i--) {
    //                 float layerAlpha = ((1.0f - (float)i / 6.0f) * 0.2f) * alphaMod;
    //                 ImU32 glow_col = ImGui::ColorConvertFloat4ToU32(ImVec4(
    //                     params.colorOn.Value.x, params.colorOn.Value.y,
    //                     params.colorOn.Value.z, layerAlpha));
    //                 draw_list->AddCircleFilled(center, currentRadius + (i * 2.0f), glow_col, 24);
    //             }
    //         }
    //
    //         // 2. Main LED Core
    //         ImU32 core_col = ImGui::ColorConvertFloat4ToU32(ImVec4(
    //             params.colorOn.Value.x, params.colorOn.Value.y,
    //             params.colorOn.Value.z, params.colorOn.Value.w * alphaMod));
    //         draw_list->AddCircleFilled(center, currentRadius, core_col, 24);
    //
    //         // 3. Hotspot
    //         draw_list->AddCircleFilled(center, currentRadius * 0.35f, IM_COL32(255, 255, 255, (int)(180 * alphaMod)), 24);
    //
    //     } else {
    //         // Off State: Flat dark color
    //         draw_list->AddCircleFilled(center, params.radius, IM_COL32(40, 40, 40, 255), 24);
    //         draw_list->AddCircle(center, params.radius, IM_COL32(70, 70, 70, 255), 24, 1.0f);
    //     }
    //
    //     if (!tooltip.empty() && ImGui::IsItemHovered()) {
    //         ImGui::SetTooltip("%s", tooltip.c_str());
    //     }
    // }



} //namespace

