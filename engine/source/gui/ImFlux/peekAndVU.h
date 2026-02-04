//-----------------------------------------------------------------------------
// Copyright (c) 2026 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#pragma once

#include <imgui.h>
#include <imgui_internal.h>
#include <cmath>
#include <algorithm>
#include "background.h"


namespace ImFlux {
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

    //------------------------------------------------------------------------------
    // old school VU Meter 70th
    inline void VUMeter70th(ImVec2 size, float value) { // value is 0.0 to 1.0
        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 pos = ImGui::GetCursorScreenPos();

        // Pivot point slightly below the bottom of the box for a natural swing
        ImVec2 center = ImVec2(pos.x + size.x * 0.5f, pos.y + size.y * 1.05f);

        // 1. Draw Background
        GradientParams grad_params = DEFAULT_GRADIENTBOX;
        grad_params.col_top = IM_COL32(82, 74, 60, 255);
        grad_params.col_bot = IM_COL32(63, 54, 35, 255);
        grad_params.pos = pos;
        grad_params.size = size;
        GradientBoxDL(grad_params, dl);

        // 2. Map 0..1 to Angle (-60 to +60 degrees approx)
        float t = std::max(0.0f, std::min(1.0f, value));
        float angle = -1.0f + (t * 2.0f); // -1.0 rad to +1.0 rad

        // 3. Draw Scale Ticks
        float radius = size.y * 0.85f;
        for (int i = 0; i <= 12; i++) {
            float st = (float)i / 12.0f;
            float s_angle = -1.0f + (st * 2.0f);

            // Vary tick length: longer ticks for major divisions
            float len = (i % 3 == 0) ? 0.85f : 0.92f;

            ImVec2 p1 = ImVec2(center.x + std::sin(s_angle) * radius, center.y - std::cos(s_angle) * radius);
            ImVec2 p2 = ImVec2(center.x + std::sin(s_angle) * (radius * len), center.y - std::cos(s_angle) * (radius * len));

            // Color transition
            ImU32 col = IM_COL32(220, 210, 190, 180);
            if (st > 0.9f) col = IM_COL32(255, 30, 30, 255); // Clip zone

            dl->AddLine(p1, p2, col, (i % 3 == 0) ? 2.0f : 1.0f);
        }

        // 4. Draw Needle with Shadow
        float needle_len = radius * 0.98f;
        ImVec2 needle_end = ImVec2(center.x + std::sin(angle) * needle_len,
                                   center.y - std::cos(angle) * needle_len);

        // Subtle shadow
        dl->AddLine(ImVec2(center.x + 1.5f, center.y), ImVec2(needle_end.x + 1.5f, needle_end.y), IM_COL32(0, 0, 0, 80), 2.0f);
        // Needle
        dl->AddLine(center, needle_end, IM_COL32(230, 40, 0, 255), 2.0f);

        // 5. Pivot cap
        dl->AddCircleFilled(center, size.y * 0.1f, IM_COL32(40, 35, 30, 255));

        // Advance Cursor
        ImGui::Dummy(size);
    }

    // ------- with labels
    inline void VUMeter70thLabeled(ImVec2 size, float value) { // value is 0.0 to 1.0 (normalized)
        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 pos = ImGui::GetCursorScreenPos();

        // Pivot point slightly below the bottom
        ImVec2 center = ImVec2(pos.x + size.x * 0.5f, pos.y + size.y * 1.1f);

        // 1. Draw Background
        GradientParams grad_params = DEFAULT_GRADIENTBOX;
        grad_params.col_top = IM_COL32(82, 74, 60, 255);
        grad_params.col_bot = IM_COL32(63, 54, 35, 255);
        grad_params.pos = pos;
        grad_params.size = size;
        GradientBoxDL(grad_params, dl);

        // 2. Constants for Mapping
        float min_angle = -1.1f; // Left side
        float max_angle =  1.1f; // Right side
        float radius = size.y * 0.85f;

        // 3. Draw dB Labels and Ticks
        // Define the points where we want text labels
        struct Mark { float db; const char* label; };
        Mark marks[] = {
            { -60.0f, "-60" }, { -40.0f, "-40" }, { -20.0f, "-20" },
            { -10.0f, "-10" }, { 0.0f, "0" }
        };

        for (const auto& mark : marks) {
            // Map dB to 0..1 range (Matching your mapDB lambda logic)
            float t = (mark.db - (-60.0f)) / (0.0f - (-60.0f));
            float s_angle = min_angle + (t * (max_angle - min_angle));

            // Position for tick marks
            ImVec2 p1 = ImVec2(center.x + std::sin(s_angle) * radius, center.y - std::cos(s_angle) * radius);
            ImVec2 p2 = ImVec2(center.x + std::sin(s_angle) * (radius * 0.88f), center.y - std::cos(s_angle) * (radius * 0.88f));

            // Position for Text (Slightly further in than the ticks)
            ImVec2 text_p = ImVec2(center.x + std::sin(s_angle) * (radius * 0.72f), center.y - std::cos(s_angle) * (radius * 0.72f));

            // Draw Ticks
            ImU32 col = (mark.db >= 0.0f) ? IM_COL32(255, 50, 50, 255) : IM_COL32(220, 210, 190, 200);
            dl->AddLine(p1, p2, col, 2.0f);

            // Draw Text Labels (Centered)
            ImVec2 text_size = ImGui::CalcTextSize(mark.label);
            dl->AddText(ImVec2(text_p.x - text_size.x * 0.5f, text_p.y - text_size.y * 0.5f), col, mark.label);
        }

        // 4. Draw Needle
        float angle = min_angle + (std::max(0.0f, std::min(1.0f, value)) * (max_angle - min_angle));
        float needle_len = radius * 0.98f;
        ImVec2 needle_end = ImVec2(center.x + std::sin(angle) * needle_len, center.y - std::cos(angle) * needle_len);

        // Shadow & Needle line
        dl->AddLine(ImVec2(center.x + 1.5f, center.y), ImVec2(needle_end.x + 1.5f, needle_end.y), IM_COL32(0, 0, 0, 80), 2.0f);
        dl->AddLine(center, needle_end, IM_COL32(230, 40, 0, 255), 2.0f);

        // 5. Pivot cap
        dl->AddCircleFilled(center, size.y * 0.12f, IM_COL32(30, 25, 20, 255));
        dl->AddCircle(center, size.y * 0.12f, IM_COL32(100, 100, 100, 100));

        // Advance Cursor
        ImGui::Dummy(size);
    }

    //------------------------------------------------------------------------------
    // Old school LED Ladder Meter (80s Style)
    inline void VUMeter80th(float value, int ledCount = 12, ImVec2 ledSquare = ImVec2(22.f, 8.f)) {
        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 pos = ImGui::GetCursorScreenPos();
        float spacing = 2.0f;

        for (int i = 0; i < ledCount; i++) {
            // 1. Calculate the threshold for this specific LED
            float ledThreshold = (float)i / (float)ledCount;
            bool isLit = (value > ledThreshold);

            // 2. Determine Color based on position in the ladder
            ImU32 led_col;
            float positionFactor = (float)i / (float)ledCount;

            if (positionFactor < 0.6f) {
                // First 60%: Green
                led_col = isLit ? IM_COL32(40, 255, 40, 255) : IM_COL32(10, 60, 10, 255);
            }
            else if (positionFactor < 0.8f) {
                // 60% to 80%: Yellow/Orange
                led_col = isLit ? IM_COL32(255, 200, 40, 255) : IM_COL32(60, 50, 10, 255);
            }
            else {
                // 80% to 100%: Red
                led_col = isLit ? IM_COL32(255, 40, 40, 255) : IM_COL32(60, 10, 10, 255);
            }

            // 3. Draw the LED segment
            ImVec2 p_min = ImVec2(pos.x + i * (ledSquare.x + spacing), pos.y);
            ImVec2 p_max = ImVec2(p_min.x + ledSquare.x, p_min.y + ledSquare.y);

            // Subtle glow effect for lit LEDs
            if (isLit) {
                dl->AddRectFilled(p_min, p_max, led_col, 1.0f);
                // Add a small "inner light" highlight to make it look like a physical LED
                dl->AddLine(p_min, ImVec2(p_max.x, p_min.y), IM_COL32(255, 255, 255, 100));
            } else {
                dl->AddRectFilled(p_min, p_max, led_col, 1.0f);
            }
        }

        // 4. Advance Cursor correctly
        float totalWidth = ledCount * (ledSquare.x + spacing);
        ImGui::Dummy(ImVec2(totalWidth, ledSquare.y));
    }
    //------------------------------------------------------------------------------
    // LED Ladder Meter with peak cot i call it 90th
    struct VUMeterState {
        float peak = 0.0f;
        float holdTimer = 0.0f; // NEW: Timer for the "freeze" moment

        // settings:
        int ledCount = 30;
        ImVec2 ledSquare = ImVec2(4.f, 16.f);
        bool vertical = false;

        void update(float current, float deltaTime) {
            if (current >= peak) {
                peak = current;
                holdTimer = 0.8f; //0.8 Stay at the top for 800ms
            } else {
                if (holdTimer > 0.0f) {
                    holdTimer -= deltaTime; // Just wait
                } else {
                    peak -= deltaTime * 0.5f; // Now fall slowly
                }
            }
            if (peak < 0) peak = 0;
        }
    };

    constexpr VUMeterState VUMETER_DEFAULT = {0.f, 0.f, 40, {4.f, 16.f}, false};

    // 90s Style Vertical Studio Ladder
    inline void VUMeter90th(float value, VUMeterState& state) {
        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 pos = ImGui::GetCursorScreenPos();
        float spacing = 2.0f;
        float dt = ImGui::GetIO().DeltaTime;

        // 1. Update Ballistics
        state.update(value, dt);

        // 2. Calculate Total Dimensions
        float totalW = state.vertical ? state.ledSquare.x : (state.ledCount * (state.ledSquare.x + spacing));
        float totalH = state.vertical ? (state.ledCount * (state.ledSquare.y + spacing)) : state.ledSquare.y;

        for (int i = 0; i < state.ledCount; i++) {
            // Threshold for this LED (0.0 to 1.0)
            float ledThreshold = (float)(i + 1) / (float)state.ledCount;
            bool isLit = (value >= ledThreshold);
            bool isPeak = (state.peak >= ledThreshold) && !isLit;

            // 3. 90s Color Grading
            ImU32 led_col;
            float posFactor = (float)i / (float)state.ledCount;
            if (posFactor < 0.7f)      led_col = isLit ? IM_COL32(0, 255, 100, 255) : IM_COL32(0, 45, 20, 255);
            else if (posFactor < 0.9f) led_col = isLit ? IM_COL32(255, 255, 0, 255) : IM_COL32(45, 45, 0, 255);
            else                       led_col = isLit ? IM_COL32(255, 0, 50, 255) : IM_COL32(45, 0, 10, 255);

            if (isPeak) led_col = (posFactor < 0.7f) ? IM_COL32(0, 255, 100, 255) :
                (posFactor < 0.9f) ? IM_COL32(255, 255, 0, 255) : IM_COL32(255, 0, 50, 255);

            // 4. Position Logic (Vertical vs Horizontal)
            ImVec2 p_min, p_max;
            if (state.vertical) {
                // Vertical: Stack from bottom to top (Y decreases)
                p_min = ImVec2(pos.x, pos.y + totalH - (i + 1) * (state.ledSquare.y + spacing));
                p_max = ImVec2(p_min.x + state.ledSquare.x, p_min.y + state.ledSquare.y);
            } else {
                // Horizontal: Stack from left to right (X increases)
                p_min = ImVec2(pos.x + i * (state.ledSquare.x + spacing), pos.y);
                p_max = ImVec2(p_min.x + state.ledSquare.x, p_min.y + state.ledSquare.y);
            }

            dl->AddRectFilled(p_min, p_max, led_col, 0.0f);

            // Subtle gloss for active segments
            if (isLit || isPeak) {
                dl->AddLine(p_min, ImVec2(state.vertical ? p_max.x : p_min.x, state.vertical ? p_min.y : p_max.y), IM_COL32(255, 255, 255, 60));
            }
        }

        // 5. Advance ImGui Cursor
        ImGui::Dummy(ImVec2(totalW, totalH));
    }


} //namespace

