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
    //---------------- MiniKnobs
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

        ImGui::ItemSize(size);
        if (!ImGui::ItemAdd(bb, ImGui::GetID("##knob"))) { ImGui::PopID(); return false; }

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
            float drag_speed = ks.speed * (v_max - v_min) * (io.KeyShift ? 0.1f : 1.0f);
            *v = std::clamp(*v - (io.MouseDelta.y * drag_speed), v_min, v_max);
            value_changed = true;
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

    //---------------------------------------------------------------------------
    inline bool FaderVertical(const char* label, ImVec2 size, float* v, float v_min, float v_max) {
        ImGui::PushID(label);
        ImVec2 pos = ImGui::GetCursorScreenPos();
        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImGuiIO& io = ImGui::GetIO();

        // 1. Interaction
        ImGui::InvisibleButton("##fader", size);

        if (!ImGui::IsItemVisible()) { ImGui::PopID(); return false; }


        bool is_active = ImGui::IsItemActive();
        bool is_hovered = ImGui::IsItemHovered();

        // ScrollWheel
        if (is_hovered && io.MouseWheel != 0) {
            float wheel_speed = (v_max - v_min) * 0.05f; // 5% of range per notch
            if (io.KeyShift) wheel_speed *= 0.1f;        // 0.5% if Shift is held

            *v = std::clamp(*v + (io.MouseWheel * wheel_speed), v_min, v_max);
            is_active = true;
        }


        if (is_active && io.MouseDown[0]) {
            float mouse_y = io.MousePos.y - pos.y;
            float fraction = 1.0f - std::clamp(mouse_y / size.y, 0.0f, 1.0f);
            *v = v_min + fraction * (v_max - v_min);
        }

        // 2. Drawing the "Track" (The slot)
        float mid_x = pos.x + size.x * 0.5f;
        dl->AddRectFilled({mid_x - 2, pos.y}, {mid_x + 2, pos.y + size.y}, IM_COL32(20, 20, 20, 255), 2.0f);

        // 3. Drawing Tick Marks (Every 25%)
        for (int i = 0; i <= 4; i++) {
            float ty = pos.y + (size.y * i * 0.25f);
            dl->AddLine({mid_x - 8, ty}, {mid_x - 4, ty}, IM_COL32(80, 80, 80, 255));
        }

        // 4. Drawing the "Cap" (The plastic handle)
        float cap_h = 20.0f;
        float fraction = (*v - v_min) / (v_max - v_min);
        float cap_y = pos.y + (1.0f - fraction) * (size.y - cap_h);

        ImU32 cap_col = is_active ? IM_COL32(180, 180, 180, 255) : (is_hovered ? IM_COL32(140, 140, 140, 255) : IM_COL32(110, 110, 110, 255));
        dl->AddRectFilled({pos.x, cap_y}, {pos.x + size.x, cap_y + cap_h}, cap_col, 2.0f);
        dl->AddLine({pos.x + 2, cap_y + cap_h * 0.5f}, {pos.x + size.x - 2, cap_y + cap_h * 0.5f}, IM_COL32(0, 0, 0, 200), 2.0f); // Center line on cap

        ImGui::PopID();
        return is_active;
    }


    // ------------- vertical FaderVertical2
    inline bool FaderVertical2(const char* label, ImVec2 size, float* v, float v_min, float v_max, const char* format = "%.2f") {
        ImGui::PushID(label);
        ImVec2 pos = ImGui::GetCursorScreenPos();
        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImGuiIO& io = ImGui::GetIO();

        // 1. Interaction Hitbox
        ImGui::InvisibleButton("##fader", size);

        if (!ImGui::IsItemVisible()) { ImGui::PopID(); return false; }

        // ImGui::GetWindowDrawList()->AddRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), IM_COL32(255,0,0,255));


        bool is_active = ImGui::IsItemActive();
        bool is_hovered = ImGui::IsItemHovered();
        bool changed = false;

        if (is_active && io.MouseDown[0]) {
            float mouse_y = io.MousePos.y - pos.y;
            float fraction = 1.0f - std::clamp(mouse_y / size.y, 0.0f, 1.0f);
            float new_v = v_min + fraction * (v_max - v_min);
            if (new_v != *v) {
                *v = new_v;
                changed = true;
            }
        }

        // 2. Draw Track (Slot)
        float mid_x = pos.x + size.x * 0.5f;
        dl->AddRectFilled({mid_x - 2, pos.y}, {mid_x + 2, pos.y + size.y}, IM_COL32(10, 10, 10, 255), 2.0f);
        dl->AddRect({mid_x - 2, pos.y}, {mid_x + 2, pos.y + size.y}, IM_COL32(60, 60, 60, 255), 2.0f);

        // 3. Draw Hardware Ticks (Scale)
        for (int i = 0; i <= 10; i++) {
            float ty = pos.y + (size.y * i * 0.1f);
            float length = (i % 5 == 0) ? 8.0f : 4.0f; // Longer lines for 0%, 50%, 100%
            dl->AddLine({mid_x - length - 4, ty}, {mid_x - 4, ty}, IM_COL32(90, 90, 90, 255));
        }

        // 4. Draw Handle (Cap)
        float cap_h = 18.0f;
        float fraction = (*v - v_min) / (v_max - v_min);
        float cap_y = pos.y + (1.0f - fraction) * (size.y - cap_h);

        ImRect cap_bb = {{pos.x, cap_y}, {pos.x + size.x, cap_y + cap_h}};
        ImU32 cap_col = is_active ? IM_COL32(150, 150, 150, 255) : (is_hovered ? IM_COL32(120, 120, 120, 255) : IM_COL32(90, 90, 90, 255));

        dl->AddRectFilled(cap_bb.Min, cap_bb.Max, cap_col, 1.0f);
        dl->AddRect(cap_bb.Min, cap_bb.Max, IM_COL32(30, 30, 30, 255), 1.0f);
        // Center "grip" line
        dl->AddLine({cap_bb.Min.x + 2, cap_bb.Min.y + cap_h*0.5f}, {cap_bb.Max.x - 2, cap_bb.Min.y + cap_h*0.5f}, IM_COL32(255, 255, 255, 180));

        // 5. Value Tooltip (Formatting using your format string)
        if (is_hovered || is_active) {
            char val_buf[64];
            ImFormatString(val_buf, IM_ARRAYSIZE(val_buf), format, *v);
            ImGui::SetTooltip("%s: %s", label, val_buf);
        }

        ImGui::PopID();
        return changed;
    }


    // -------------- FaderHorizontal
    inline bool FaderHorizontal(const char* label, ImVec2 size, float* v, float v_min, float v_max) {
        ImGui::PushID(label);
        ImVec2 pos = ImGui::GetCursorScreenPos();
        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImGuiIO& io = ImGui::GetIO();

        // Interaction (Horizontal logic)
        ImGui::InvisibleButton("##fader", size);

        if (!ImGui::IsItemVisible()) { ImGui::PopID(); return false; }

        // bounds check: ImGui::GetWindowDrawList()->AddRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), IM_COL32(255,0,0,255));


        bool is_active = ImGui::IsItemActive();
        bool is_hovered = ImGui::IsItemHovered();


        // ScrollWheel
        if (is_hovered && io.MouseWheel != 0) {
            float wheel_speed = (v_max - v_min) * 0.05f; // 5% of range per notch
            if (io.KeyShift) wheel_speed *= 0.1f;        // 0.5% if Shift is held

            *v = std::clamp(*v + (io.MouseWheel * wheel_speed), v_min, v_max);
            is_active = true;
        }




        if (is_active && io.MouseDown[0]) {
            float mouse_x = io.MousePos.x - pos.x;
            float fraction = std::clamp(mouse_x / size.x, 0.0f, 1.0f);
            *v = v_min + fraction * (v_max - v_min);
        }

        // Drawing the "Track" (Horizontal slot)
        float mid_y = pos.y + size.y * 0.5f;
        dl->AddRectFilled({pos.x, mid_y - 2}, {pos.x + size.x, mid_y + 2}, IM_COL32(20, 20, 20, 255), 2.0f);

        // Drawing Tick Marks (Every 25%)
        for (int i = 0; i <= 4; i++) {
            float tx = pos.x + (size.x * i * 0.25f);
            dl->AddLine({tx, mid_y - 8}, {tx, mid_y - 4}, IM_COL32(80, 80, 80, 255));
        }

        // adding label
        std::string lbStr = std::string(label);
        if (!lbStr.empty())
        {
            Hint(label);
            // TEST: only on short text max 8!
            // if (lbStr.length() < 9)
            dl->AddText(ImVec2(pos.x, pos.y+8.f), IM_COL32(128,128,128,128), label);
        }



        // Drawing the "Cap" (Vertical handle moving horizontally)
        float cap_w = 20.0f; // Width of the handle
        float fraction = (*v - v_min) / (v_max - v_min);
        float cap_x = pos.x + fraction * (size.x - cap_w);

        ImU32 cap_col = is_active ? IM_COL32(180, 180, 180, 255) : (is_hovered ? IM_COL32(140, 140, 140, 255) : IM_COL32(110, 110, 110, 255));

        // Cap matches the vertical handle style but oriented for horizontal travel
        dl->AddRectFilled({cap_x, pos.y}, {cap_x + cap_w, pos.y + size.y}, cap_col, 2.0f);
        dl->AddLine({cap_x + cap_w * 0.5f, pos.y + 2}, {cap_x + cap_w * 0.5f, pos.y + size.y - 2}, IM_COL32(0, 0, 0, 200), 2.0f);


        ImGui::PopID();
        return is_active;
    }
    // -------------- FaderHWithText
    inline bool FaderHWithText(const char* label, float* v, float v_min, float v_max, const char* format,
                               ImVec2 size = ImVec2(150, 18)) {
        ImGui::BeginGroup();
        bool active = FaderHorizontal(label, size, v, v_min, v_max);
        ImGui::SameLine();
        ImGui::TextDisabled(format, *v); // Displays "Hz", "s", or "Wet" next to the fader
        ImGui::EndGroup();
        return active;
    }



}//namespace
