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

    //--------------------------------------------------------------------------------------------------
    // Colors for your Dark/Gradient Theme
    struct KnobSettings {
        float radius       = 16.f;
        float speed        = 0.01f;

        // Background & Body
        ImU32 bg_outer     = IM_COL32(25, 25, 30, 255);  // Deepest part
        ImU32 bg_inner     = IM_COL32(50, 50, 55, 255);  // Surface of the knob

        // Effects
        ImU32 glare        = IM_COL32(255, 255, 255, 15); // Top shine
        ImU32 shadow       = IM_COL32(0, 0, 0, 150);      // Inner shadow/depth
        ImU32 bevel        = IM_COL32(255, 255, 255, 35); // Outer rim light

        // Active Elements
        ImU32 active       = IM_COL32(0, 255, 200, 255);  // Neon Cyan
        ImU32 arc_bg       = IM_COL32(0, 0, 0, 80);       // Grooved track for the arc
        ImU32 needle       = IM_COL32(255, 255, 255, 255); // Indicator color
    };
    constexpr KnobSettings DARK_KNOB;



    inline bool MiniKnobF(const char* label, float* v, float v_min, float v_max, KnobSettings ks = DARK_KNOB) {
        ImGui::PushID(label);
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems) { ImGui::PopID(); return false; }

        ImVec2 pos = ImGui::GetCursorScreenPos();
        ImVec2 size = ImVec2(ks.radius * 2, ks.radius * 2);
        ImRect bb(pos, pos + size);

        ImGui::InvisibleButton(label, {ks.radius*2.f, ks.radius*2.f});


        if (ImGui::GetItemFlags() & ImGuiItemFlags_Disabled) {
            ks.active = ImGui::GetColorU32(ImGuiCol_TextDisabled);
        }

        // ImGui::ItemSize(size);
        // if (!ImGui::ItemAdd(bb, ImGui::GetID("##knob"))) { ImGui::PopID(); return false; }

        bool value_changed = false;
        bool is_active = ImGui::IsItemActive();
        bool is_hovered = ImGui::IsItemHovered();
        ImGuiIO& io = ImGui::GetIO();

        // Interaction (Scroll & Drag)
        if (is_hovered && io.MouseWheel != 0) {
            float wheel_speed = (v_max - v_min) * 0.05f * (io.KeyShift ? 0.1f : 1.0f);
            *v = std::clamp(*v + (io.MouseWheel * wheel_speed), v_min, v_max);
            value_changed = true;
        }

        if (is_active && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
            float drag_delta = -io.MouseDelta.y;
            if (drag_delta != 0.0f) {
                float range = v_max - v_min;
                // Adjust speed based on range and Shift key
                float current_speed = ks.speed * range;
                if (io.KeyShift) current_speed *= 0.1f;

                *v = std::clamp(*v + (drag_delta * current_speed), v_min, v_max);
                value_changed = true;
            }
        }

        // --- DRAWING ---
        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 center = ImVec2(pos.x + ks.radius, pos.y + ks.radius);
        float fraction = (*v - v_min) / (v_max - v_min);
        float angle = (fraction * 1.5f * IM_PI) + (0.75f * IM_PI);

        // 1. Body & Depth
        dl->AddCircleFilled(center, ks.radius, ks.bg_outer);
        dl->AddCircleFilled(center, ks.radius - 2.0f, ks.bg_inner);
        dl->AddCircle(center, ks.radius, ks.shadow, 32, 1.5f);

        // 2. Glossy top glare
        dl->AddCircleFilled(center - ImVec2(0, ks.radius * 0.3f), ks.radius * 0.6f, ks.glare);

        // 3. Neon Value Arc
        ImU32 current_arc_col = is_active ? ks.active : (is_hovered ? ModifyRGB(ks.active, 1.2f) : ks.active);
        // Background track
        dl->PathArcTo(center, ks.radius - 3.0f, 0.75f * IM_PI, 2.25f * IM_PI, 20);
        dl->PathStroke(ks.arc_bg, 0, 3.5f);
        // Active fill
        dl->PathArcTo(center, ks.radius - 3.0f, 0.75f * IM_PI, angle, 20);
        dl->PathStroke(current_arc_col, 0, 3.0f);

        // 4. Indicator Needle
        ImVec2 n_start = center + ImVec2(cosf(angle) * (ks.radius * 0.45f), sinf(angle) * (ks.radius * 0.45f));
        ImVec2 n_end   = center + ImVec2(cosf(angle) * (ks.radius - 2.5f), sinf(angle) * (ks.radius - 2.5f));
        dl->AddLine(n_start, n_end, ks.needle, 2.0f);

        // 5. Final Outer Rim Light
        dl->AddCircle(center, ks.radius, ks.bevel, 32, 1.0f);

        if (is_hovered) ImGui::SetTooltip("%s: %.2f", label, *v);

        ImGui::PopID();
        return value_changed;
    }

    // ~~~~~~~~~~~~~~~  LEDMiniKnob

    inline bool LEDMiniKnob(const char* label, float* v, float v_min, float v_max, KnobSettings ks = DARK_KNOB) {
        ImGui::PushID(label);
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems) { ImGui::PopID(); return false; }

        ImVec2 pos = ImGui::GetCursorScreenPos();
        ImVec2 size = ImVec2(ks.radius * 2, ks.radius * 2);
        ImRect bb(pos, pos + size);

        ImGui::InvisibleButton(label, size);

        bool value_changed = false;
        bool is_active = ImGui::IsItemActive();
        bool is_hovered = ImGui::IsItemHovered();
        ImGuiIO& io = ImGui::GetIO();

        // Interaction logic
        if (is_hovered && io.MouseWheel != 0) {
            float wheel_speed = (v_max - v_min) * 0.05f * (io.KeyShift ? 0.1f : 1.0f);
            *v = std::clamp(*v + (io.MouseWheel * wheel_speed), v_min, v_max);
            value_changed = true;
        }

        if (is_active && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
            float range = v_max - v_min;
            float current_speed = ks.speed * range * (io.KeyShift ? 0.1f : 1.0f);
            *v = std::clamp(*v + (-io.MouseDelta.y * current_speed), v_min, v_max);
            value_changed = true;
        }

        // --- DRAWING ---
        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 center = ImVec2(pos.x + ks.radius, pos.y + ks.radius);
        float fraction = (*v - v_min) / (v_max - v_min);

        // 1. Knob Body (from MiniKnobF)
        dl->AddCircleFilled(center, ks.radius * 0.75f, ks.bg_outer);
        dl->AddCircleFilled(center, ks.radius * 0.70f, ks.bg_inner);
        dl->AddCircleFilled(center - ImVec2(0, ks.radius * 0.2f), ks.radius * 0.4f, ks.glare);

        // 2. LED Ring (Replacing the Arc)
        const int num_leds = 12;
        const float start_angle = 0.75f * IM_PI;
        const float end_angle   = 2.25f * IM_PI;
        const float led_radius  = ks.radius - 2.5f; // Positioned near the edge

        for (int i = 0; i < num_leds; i++) {
            // Calculate position along the 270-degree arc
            float t = (float)i / (float)(num_leds - 1);
            float ang = start_angle + t * (end_angle - start_angle);

            ImVec2 led_pos = center + ImVec2(cosf(ang) * led_radius, sinf(ang) * led_radius);

            // Determine LED color
            ImU32 led_col;
            if (t <= fraction && fraction > 0.0f) {
                led_col = is_active ? ks.active : (is_hovered ? ModifyRGB(ks.active, 1.3f) : ks.active);
            } else {
                led_col = ks.arc_bg; // Dim color for "off" LEDs
            }

            // Draw the LED (with a slight glow if active)
            dl->AddCircleFilled(led_pos, 2.0f, led_col);
            if (t <= fraction) {
                dl->AddCircle(led_pos, 2.5f, ModifyRGB(led_col, 0.6f), 12, 1.0f); // Subtle outer glow
            }
        }

        // 3. Indicator Needle (Mini version)
        float needle_ang = start_angle + fraction * (end_angle - start_angle);
        ImVec2 n_start = center + ImVec2(cosf(needle_ang) * (ks.radius * 0.35f), sinf(needle_ang) * (ks.radius * 0.35f));
        ImVec2 n_end   = center + ImVec2(cosf(needle_ang) * (ks.radius * 0.60f), sinf(needle_ang) * (ks.radius * 0.60f));
        dl->AddLine(n_start, n_end, ks.needle, 1.5f);

        if (is_hovered) ImGui::SetTooltip("%s: %.2f", label, *v);

        ImGui::PopID();
        return value_changed;
    }


    // --------------- LEDRingKnob
    inline bool LEDRingKnob(const char* label, float* v, float v_min, float v_max, float radius = 18.f, float speed = 0.01f) {
        ImGui::PushID(label);

        ImVec2 p = ImGui::GetCursorScreenPos();
        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImGui::InvisibleButton(label, {radius*2, radius*2});

        if (!ImGui::IsItemVisible()) { ImGui::PopID(); return false; }

        bool value_changed = false;
        bool is_active = ImGui::IsItemActive();
        bool is_hovered = ImGui::IsItemHovered();
        ImGuiIO& io = ImGui::GetIO();

        // 1. SCROLL WHEEL (Hold Shift for fine-tuning)
        if (is_hovered && io.MouseWheel != 0) {
            float wheel_speed = (v_max - v_min) * 0.05f; // 5% of range per notch
            if (io.KeyShift) wheel_speed *= 0.1f;        // 0.5% if Shift is held

            *v = std::clamp(*v + (io.MouseWheel * wheel_speed), v_min, v_max);
            value_changed = true;
        }

        // 2. DRAG (Hold Shift for fine-tuning)
        if (is_active && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
            float drag_delta = -io.MouseDelta.y;
            if (drag_delta != 0.0f) {
                float range = v_max - v_min;
                // Adjust speed based on range and Shift key
                float current_speed = speed * range;
                if (io.KeyShift) current_speed *= 0.1f;

                *v = std::clamp(*v + (drag_delta * current_speed), v_min, v_max);
                value_changed = true;
            }
        }

        // Drawing
        float fraction = (*v - v_min) / (v_max - v_min);
        int num_leds = 12;
        for (int i = 0; i < num_leds; i++) {
            float angle = (i / (float)num_leds) * 2.0f * 3.14159f;
            ImVec2 led_p = p + ImVec2(radius + cosf(angle)*radius, radius + sinf(angle)*radius);
            ImU32 led_col = (i / (float)num_leds <= fraction) ? IM_COL32(0, 255, 0, 255) : IM_COL32(40, 40, 40, 255);
            dl->AddCircleFilled(led_p, 2.5f, led_col);
        }
        // ImGui::SetCursorScreenPos(p + ImVec2(radius*2 + 5, radius/2));
        // ImGui::Text("%s", label);


        Hint(std::format("{:s} {:4.2f}", label, *v));

        ImGui::PopID(/*label*/);


        return value_changed;
    }

    //------------------------------------------------------------------------------
    //---------------- MiniKnobs with default colors
    inline bool MiniKnobFloat(const char* label, float* v, float v_min, float v_max, float radius = 12.f,  float speed = 0.01f) {
        ImGui::PushID(label);
        ImVec2 pos = ImGui::GetCursorScreenPos();
        ImVec2 size = ImVec2(radius * 2, radius * 2);
        ImGui::InvisibleButton("##knob", size);

        if (!ImGui::IsItemVisible()) { ImGui::PopID(); return false; }

        bool value_changed = false;
        bool is_active = ImGui::IsItemActive();
        bool is_hovered = ImGui::IsItemHovered();
        ImGuiIO& io = ImGui::GetIO();

        // 1. SCROLL WHEEL (Hold Shift for fine-tuning)
        if (is_hovered && io.MouseWheel != 0) {
            float wheel_speed = (v_max - v_min) * 0.05f; // 5% of range per notch
            if (io.KeyShift) wheel_speed *= 0.1f;        // 0.5% if Shift is held

            *v = std::clamp(*v + (io.MouseWheel * wheel_speed), v_min, v_max);
            value_changed = true;
        }

        // 2. DRAG (Hold Shift for fine-tuning)
        if (is_active && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
            float drag_delta = -io.MouseDelta.y;
            if (drag_delta != 0.0f) {
                float range = v_max - v_min;
                // Adjust speed based on range and Shift key
                float current_speed = speed * range;
                if (io.KeyShift) current_speed *= 0.1f;

                *v = std::clamp(*v + (drag_delta * current_speed), v_min, v_max);
                value_changed = true;
            }
        }

        // --- DRAWING ---
        ImDrawList* dl = ImGui::GetWindowDrawList();

        ImU32 col = is_active ? ImGui::GetColorU32(ImGuiCol_SliderGrabActive) :
        (is_hovered ? ImGui::GetColorU32(ImGuiCol_FrameBgHovered) : ImGui::GetColorU32(ImGuiCol_FrameBg));

        float fraction = (*v - v_min) / (v_max - v_min);
        float angle = (fraction * 1.5f * 3.14159f) + (0.75f * 3.14159f);

        ImVec2 center = ImVec2(pos.x + radius, pos.y + radius);
        dl->AddCircleFilled(center, (float)radius, col);
        // Draw "Value Arc"
        dl->PathArcTo(center, (float)(radius - 1.f), 0.75f * 3.14159f, angle, 10);

        dl->PathStroke(ImGui::GetColorU32(ImGuiCol_PlotLines), 0, 2.0f);

        // Draw Indicator Needle

        dl->AddLine(center,
                    ImVec2(center.x + cosf(angle) * (radius - 1.f), center.y + sinf(angle) * (radius - 1.f)),
                    ImGui::GetColorU32(ImGuiCol_Text), 2.0f);


        if (is_hovered) {
            std::string lLabel = GetLabelText(label);
            ImGui::SetTooltip("%s: %.2f", lLabel.c_str(), *v);
        }


        ImGui::PopID();
        return value_changed;
    }


    // --------------- MiniKnobInt
    inline bool MiniKnobInt(const char* label, int* v, int v_min, int v_max, float radius = 12.f, int step = 1, int defaultValue = -4711) {
        ImGui::PushID(label);
        ImVec2 pos = ImGui::GetCursorScreenPos();
        ImVec2 size = ImVec2(radius * 2, radius * 2);
        ImGui::InvisibleButton("##knob", size);

        if (!ImGui::IsItemVisible()) { ImGui::PopID(); return false; }


        bool value_changed = false;
        bool is_active = ImGui::IsItemActive();
        bool is_hovered = ImGui::IsItemHovered();
        bool is_clicked = ImGui::IsItemClicked();

        // SCROLL WHEEL support (fastest way to change values)
        if (is_hovered && ImGui::GetIO().MouseWheel != 0) {
            int new_v = std::clamp(*v + (int)ImGui::GetIO().MouseWheel * step, v_min, v_max);
            if (new_v != *v) {
                *v = new_v ;
                value_changed = true;
            }
        }

        // DRAG support (with higher sensitivity)
        if (is_active && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
            float delta = ImGui::GetIO().MouseDelta.y;
            if (std::abs(delta) > 0.0f) {
                // Sensitivity: every 5 pixels of drag is 1 step
                // We use a temporary static float to keep movement smooth
                static float accumulator = 0.0f;
                accumulator -= delta;

                if (std::abs(accumulator) >= 5.0f) {
                    int steps = (int)(accumulator / 5.0f) * step;
                    int new_v = std::clamp(*v + steps, v_min, v_max);
                    if (new_v != *v) {
                        *v = new_v ;
                        value_changed = true;
                    }
                    accumulator -= (float)steps * 5.0f; // keep the remainder
                }
            }
        } else {
            // Reset accumulator when not active
            // (Note: for a production app, use a per-item ID storage if possible)
        }

        // --- DRAWING ---
        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImU32 col = is_active ? ImGui::GetColorU32(ImGuiCol_SliderGrabActive) :
        (is_hovered ? ImGui::GetColorU32(ImGuiCol_FrameBgHovered) : ImGui::GetColorU32(ImGuiCol_FrameBg));

        float fraction = (float)(*v - v_min) / (float)(v_max - v_min);
        float angle = (fraction * 1.5f * 3.14159f) + (0.75f * 3.14159f);

        ImVec2 center = ImVec2(pos.x + radius, pos.y + radius);
        dl->AddCircleFilled(center, (float)radius, col);

        dl->PathArcTo(center, (float)(radius - 1.f), 0.75f * 3.14159f, angle, 10);
        dl->PathStroke(ImGui::GetColorU32(ImGuiCol_PlotLines), 0, 2.0f);

        dl->AddLine(center,
                    ImVec2(center.x + cosf(angle) * (radius - 1.f), center.y + sinf(angle) * (radius - 1.f)),
                    ImGui::GetColorU32(ImGuiCol_Text), 2.0f);


        if (is_hovered) {
            std::string lLabel = GetLabelText(label);
            ImGui::SetTooltip("%s: %d", lLabel.c_str(), *v);
        }

        if (is_clicked && defaultValue != -4711) {
            *v = defaultValue;
            value_changed = true;
        }

        ImGui::PopID();
        return value_changed;
    }


    // ~~~~~~~~~~~~~~~  Hackfest MiniKnobIntWithText
    inline bool MiniKnobIntWithText(const char* label, int* v, int v_min, int v_max, const char* format = "%d",  float radius = 12.f, int step = 1, int defaultValue = -4711) {
        ImVec2 pos = ImGui::GetCursorScreenPos();

        ImDrawList* dl = ImGui::GetWindowDrawList();
        dl->AddText(ImVec2(pos.x, pos.y), IM_COL32(200,200,200,200), label);
        float lY = 12.f;
        // ImVec2 size = ImGui::CalcTextSize("Your text here");
        ImGui::Dummy(ImVec2(radius*2.f, lY)); //FIXME get font height

        bool value_changed = MiniKnobInt(label, v,  v_min, v_max, radius, step, defaultValue);

        if (format == NULL)
            format = "%d";


        Log("");

        char buff[32];
        snprintf(buff, sizeof(buff), format, *v);
        lY += pos.y+radius * 2 +  4.f;
        dl->AddText(ImVec2(pos.x, lY), IM_COL32(128,128,128,255), buff);
        ImGui::Dummy(ImVec2(radius*2.f, 8.f)); //FIXME get font height

        return value_changed;
    }



}//namespace
