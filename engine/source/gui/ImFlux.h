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

    // -------- we alter the color
    inline ImVec4 ModifierColor(ImVec4 col, float factor) {
        return ImVec4(col.x * factor, col.y * factor, col.z * factor, col.w);
    }
    //---------------- Hint
    inline void Hint(std::string tooltip)
    {
        if (!tooltip.empty() && ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) {
            ImGui::SetTooltip("%s", tooltip.c_str());
        }
    }

    //---------------- SeparatorVertical
    inline void SeparatorVertical(float padding = 20.f)
    {
        ImGui::SameLine();
        ImGui::Dummy(ImVec2(padding,0.f));
        ImGui::SameLine();
        ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
        ImGui::SameLine();
        ImGui::Dummy(ImVec2(padding,0.f));
        ImGui::SameLine();
    }

    //------------------------------------------------------------------------------
    enum ButtonMouseOverEffects {
        BUTTON_MO_HIGHLIGHT,  // Static brightness boost
        BUTTON_MO_PULSE,      // Brightness waves
        BUTTON_MO_GLOW,       // Outer glow/bloom
        BUTTON_MO_SHAKE,      // Subtle wiggle (good for errors/warnings)
        BUTTON_MO_BOUNCE,     // Slight vertical offset
        BUTTON_MO_GLOW_PULSE  // Glow and Pulse
    };

    struct ButtonParams {
        ImU32 color = IM_COL32(64, 64, 64, 255);
        ImVec2 size = { 24.f, 24.f };
        bool   bevel = true;
        bool   gloss = true;
        bool   shadowText = true;
        float  rounding = -1.0f; // -1 use global style
        float  animationSpeed = 1.0f; //NOTE: unused
        ButtonMouseOverEffects mouseOverEffect = BUTTON_MO_HIGHLIGHT;
    };

    constexpr ButtonParams DEFAULT_BUTTON;


    inline bool ButtonFancy(std::string label, ButtonParams params = DEFAULT_BUTTON)
    {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems) return false;

        const ImGuiID id = window->GetID(label.c_str());
        const ImRect bb(window->DC.CursorPos, window->DC.CursorPos + params.size);
        ImGui::ItemSize(params.size);
        if (!ImGui::ItemAdd(bb, id)) return false;

        bool hovered, held;
        bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);

        ImDrawList* dl = window->DrawList;
        const float rounding = params.rounding < 0 ? ImGui::GetStyle().FrameRounding : params.rounding;

        // Use double for time calculation to keep it smooth during long sessions
        double time = ImGui::GetTime();

        ImVec4 colFactor = ImVec4(1, 1, 1, 1);
        ImVec2 renderOffset = ImVec2(0, 0);
        float glowAlpha = 0.0f;

        if (held) {
            colFactor = ImVec4(0.7f, 0.7f, 0.7f, 1.0f);
        }
        else if (hovered) {
            switch (params.mouseOverEffect) {
                case BUTTON_MO_HIGHLIGHT:
                    colFactor = ImVec4(1.2f, 1.2f, 1.2f, 1.0f);
                    break;
                case BUTTON_MO_PULSE: {
                    float p = (sinf((float)(time * 10.0)) * 0.5f) + 0.5f;
                    float s = 1.0f + (p * 0.3f);
                    colFactor = ImVec4(s, s, s, 1.0f);
                    break;
                }
                case BUTTON_MO_GLOW:
                    glowAlpha = 1.0f; // Static glow on hover
                    break;
                case BUTTON_MO_GLOW_PULSE:
                    glowAlpha = (sinf((float)(time * 10.0)) * 0.5f) + 0.5f;
                    break;
                case BUTTON_MO_SHAKE:
                    renderOffset.x = sinf((float)(time * 30.0)) * 1.5f;
                    break;
                case BUTTON_MO_BOUNCE:
                    renderOffset.y = -fabsf(sinf((float)(time * 10.0))) * 3.0f;
                    break;
            }
        }

        ImRect rbb = ImRect(bb.Min + renderOffset, bb.Max + renderOffset);
        ImU32 finalCol = ImGui::ColorConvertFloat4ToU32(ImGui::ColorConvertU32ToFloat4(params.color) * colFactor);

        // --- Render Glow ---
        // if (glowAlpha > 0.0f) {
        //     ImVec4 bc = ImGui::ColorConvertU32ToFloat4(params.color);
        //     ImVec4 gc = ImVec4(ImMin(bc.x * 1.5f, 1.0f), ImMin(bc.y * 1.5f, 1.0f), ImMin(bc.z * 1.5f, 1.0f), 1.0f);
        //     for (int i = 1; i <= 8; i++) {
        //         float spread = (float)i * 1.2f;
        //         float alpha = (0.35f * glowAlpha) / (float)(i * i);
        //         ImU32 layerCol = ImGui::ColorConvertFloat4ToU32(ImVec4(gc.x, gc.y, gc.z, alpha));
        //         dl->AddRect(rbb.Min - ImVec2(spread, spread), rbb.Max + ImVec2(spread, spread), layerCol, rounding + spread, 0, 1.5f);
        //     }
        // }
        if (glowAlpha > 0.001f) {
            ImVec4 bc = ImGui::ColorConvertU32ToFloat4(params.color);
            // Brighten the core of the glow significantly
            ImVec4 gc = ImVec4(ImMin(bc.x * 1.8f, 1.0f), ImMin(bc.y * 1.8f, 1.0f), ImMin(bc.z * 1.8f, 1.0f), 1.0f);

            for (int i = 1; i <= 10; i++) { // More layers for smoother spread
                float spread = (float)i * 2.0f; // Wider spread (was 1.2f)

                // Linear-ish falloff is much more visible than quadratic (i*i)
                // We use glowAlpha to modulate the whole effect
                float alpha = (0.5f * glowAlpha) / (float)i;

                ImU32 layerCol = ImGui::ColorConvertFloat4ToU32(ImVec4(gc.x, gc.y, gc.z, alpha));

                // Thicker lines (2.5f) create a solid "cloud" of light
                dl->AddRect(rbb.Min - ImVec2(spread, spread),
                            rbb.Max + ImVec2(spread, spread),
                            layerCol, rounding + spread, 0, 2.5f);
            }

            // Optional: One extra "Hard Glow" line right at the edge
            dl->AddRect(rbb.Min - ImVec2(1,1), rbb.Max + ImVec2(1,1),
                        ImGui::ColorConvertFloat4ToU32(ImVec4(gc.x, gc.y, gc.z, glowAlpha * 0.6f)), rounding + 1.0f, 0, 1.0f);
        }


        // --- Render Body, Bevel, Gloss & Text ---
        dl->AddRectFilled(rbb.Min, rbb.Max, finalCol, rounding);

        if (params.bevel) {
            dl->AddRect(rbb.Min, rbb.Max, IM_COL32(255, 255, 255, 40), rounding);
            dl->AddRect(rbb.Min + ImVec2(1,1), rbb.Max - ImVec2(1,1), IM_COL32(0, 0, 0, 40), rounding);
        }

        if (params.gloss) {
            dl->AddRectFilledMultiColor(
                rbb.Min + ImVec2(rounding * 0.2f, 0), ImVec2(rbb.Max.x - rounding * 0.2f, rbb.Min.y + params.size.y * 0.5f),
                                        IM_COL32(255, 255, 255, 50), IM_COL32(255, 255, 255, 50),
                                        IM_COL32(255, 255, 255, 0),  IM_COL32(255, 255, 255, 0)
            );
        }

        ImVec2 textSize = ImGui::CalcTextSize(label.c_str());
        ImVec2 textPos = rbb.Min + (params.size - textSize) * 0.5f;
        if (params.shadowText) dl->AddText(textPos + ImVec2(1, 1), IM_COL32(0, 0, 0, 200), label.c_str());
        dl->AddText(textPos, IM_COL32_WHITE, label.c_str());

        return pressed;
    }

    // ----------------- Fancy !! ButtonFancy ....
    // inline bool ButtonFancy(std::string label, ButtonParams params = DEFAULT_BUTTON)
    // {
    //     ImGuiWindow* window = ImGui::GetCurrentWindow();
    //     if (window->SkipItems) return false;
    //
    //     const ImGuiID id = window->GetID(label.c_str());
    //     const ImRect bb(window->DC.CursorPos, window->DC.CursorPos + params.size);
    //     ImGui::ItemSize(params.size);
    //     if (!ImGui::ItemAdd(bb, id)) return false;
    //
    //     bool hovered, held;
    //     bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);
    //
    //     ImDrawList* dl = window->DrawList;
    //     const float rounding = params.rounding < 0 ? ImGui::GetStyle().FrameRounding : params.rounding;
    //
    //     // --- 1. Handle Mouse Over Effects & Color Logic ---
    //     ImVec4 colFactor = ImVec4(1, 1, 1, 1);
    //     ImVec2 renderOffset = ImVec2(0, 0);
    //     float glowAlpha = 0.0f;
    //
    //
    //     // Use double for the base time to prevent jittering/stuttering
    //     // after the application has been running for a long time.
    //     double baseTime = ImGui::GetTime();
    //
    //     if (held) {
    //         colFactor = ImVec4(0.7f, 0.7f, 0.7f, 1.0f);
    //     }
    //     else if (hovered) {
    //         float speed = params.animationSpeed;
    //         switch (params.mouseOverEffect) {
    //             case BUTTON_MO_HIGHLIGHT:
    //                 colFactor = ImVec4(1.2f, 1.2f, 1.2f, 1.0f);
    //                 break;
    //             case BUTTON_MO_PULSE: {
    //                 float p = (sinf((float)(baseTime * 10.0 * speed)) * 0.5f) + 0.5f;
    //                 float s = 1.0f + (p * 0.3f);
    //                 colFactor = ImVec4(s, s, s, 1.0f);
    //                 break;
    //             }
    //             case BUTTON_MO_GLOW:
    //             case BUTTON_MO_GLOW_PULSE: {
    //                 float p = (params.mouseOverEffect == BUTTON_MO_GLOW) ? 1.0f : (sinf((float)(baseTime * 10.0 * speed)) * 0.5f) + 0.5f;
    //                 glowAlpha = p;
    //                 break;
    //             }
    //             case BUTTON_MO_SHAKE: {
    //                 renderOffset.x = sinf((float)(baseTime * 30.0 * speed)) * 1.5f;
    //                 break;
    //             }
    //             case BUTTON_MO_BOUNCE: {
    //                 renderOffset.y = -fabsf(sinf((float)(baseTime * 10.0 * speed))) * 3.0f;
    //                 break;
    //             }
    //         }
    //     }
    //
    //
    //     ImRect rbb = ImRect(bb.Min + renderOffset, bb.Max + renderOffset);
    //     ImU32 finalCol = ImGui::ColorConvertFloat4ToU32(ImGui::ColorConvertU32ToFloat4(params.color) * colFactor);
    //
    //
    //
    //     // --- 2. Render Background & Glow ---
    //     // if (glowAlpha > 0.0f) {
    //     //     // Create a brighter version of the button color for the glow
    //     //     ImVec4 bc = ImGui::ColorConvertU32ToFloat4(params.color);
    //     //     ImVec4 gc = ImVec4(ImMin(bc.x * 1.5f, 1.0f), ImMin(bc.y * 1.5f, 1.0f), ImMin(bc.z * 1.5f, 1.0f), 1.0f);
    //     //
    //     //     for (int i = 1; i <= 8; i++) {
    //     //         float spread = (float)i * 1.2f;
    //     //         // Use glowAlpha with a quadratic falloff for a soft bloom
    //     //         float alpha = (0.4f * glowAlpha) / (float)(i * i);
    //     //         ImU32 layerCol = ImGui::ColorConvertFloat4ToU32(ImVec4(gc.x, gc.y, gc.z, alpha));
    //     //
    //     //         dl->AddRect(rbb.Min - ImVec2(spread, spread),
    //     //                     rbb.Max + ImVec2(spread, spread),
    //     //                     layerCol, rounding + spread, 0, 2.0f);
    //     //     }
    //     // }
    //     // --- 2. Render Background & Glow ---
    //     if (glowAlpha > 0.0f) {
    //         ImVec4 bc = ImGui::ColorConvertU32ToFloat4(params.color);
    //         // Tint the glow slightly towards white for more "light"
    //         ImVec4 gc = ImVec4(ImMin(bc.x + 0.2f, 1.0f), ImMin(bc.y + 0.2f, 1.0f), ImMin(bc.z + 0.2f, 1.0f), 1.0f);
    //
    //         for (int i = 1; i <= 8; i++) {
    //             float spread = (float)i * 1.2f;
    //             // Quadratic falloff for soft edges
    //             float alpha = (0.35f * glowAlpha) / (float)(i * i);
    //             ImU32 layerCol = ImGui::ColorConvertFloat4ToU32(ImVec4(gc.x, gc.y, gc.z, alpha));
    //
    //             dl->AddRect(rbb.Min - ImVec2(spread, spread),
    //                         rbb.Max + ImVec2(spread, spread),
    //                         layerCol, rounding + spread, 0, 1.5f);
    //         }
    //     }
    //
    //     // Draw body after glow so the button sits "on top" of the light
    //     dl->AddRectFilled(rbb.Min, rbb.Max, finalCol, rounding);
    //
    //     // --- 3. Render Bevel ---
    //     if (params.bevel) {
    //         dl->AddRect(rbb.Min, rbb.Max, IM_COL32(255, 255, 255, 40), rounding); // Light edge
    //         dl->AddRect(rbb.Min + ImVec2(1,1), rbb.Max - ImVec2(1,1), IM_COL32(0, 0, 0, 40), rounding); // Shadow edge
    //     }
    //
    //     // --- 4. Render Gloss ---
    //     if (params.gloss) {
    //         dl->AddRectFilledMultiColor(
    //             rbb.Min + ImVec2(rounding * 0.2f, 0), ImVec2(rbb.Max.x - rounding * 0.2f, rbb.Min.y + params.size.y * 0.5f),
    //                                     IM_COL32(255, 255, 255, 50), IM_COL32(255, 255, 255, 50),
    //                                     IM_COL32(255, 255, 255, 0),  IM_COL32(255, 255, 255, 0)
    //         );
    //     }
    //
    //     // --- 5. Render Text ---
    //     ImVec2 textSize = ImGui::CalcTextSize(label.c_str());
    //     ImVec2 textPos = rbb.Min + (params.size - textSize) * 0.5f;
    //     if (params.shadowText) {
    //         dl->AddText(textPos + ImVec2(1, 1), IM_COL32(0, 0, 0, 200), label.c_str());
    //     }
    //     dl->AddText(textPos, IM_COL32_WHITE, label.c_str());
    //
    //     return pressed;
    // }

    //-------------- Demo ShowButtonFancyGallery
    inline void ShowButtonFancyGallery() {
        ImGui::Begin("Fancy Button Gallery");

        // Standard styling for the whole gallery
        static ButtonParams p;
        p.size = ImVec2(120, 40);
        p.rounding = 4.0f;

        // --- ROW 1: Mouse Over Effects ---
        ImGui::Text("Mouse Over Effects:");

        p.color = IM_COL32(50, 100, 200, 255); // Blueish
        p.mouseOverEffect = BUTTON_MO_HIGHLIGHT;
        ButtonFancy("Highlight", p); ImGui::SameLine();

        p.mouseOverEffect = BUTTON_MO_PULSE;
        ButtonFancy("Pulse", p); ImGui::SameLine();

        p.mouseOverEffect = BUTTON_MO_GLOW;
        ButtonFancy("Glow", p); ImGui::SameLine();

        p.mouseOverEffect = BUTTON_MO_GLOW_PULSE;
        ButtonFancy("Glow and Pulse", p); ImGui::SameLine();

        p.mouseOverEffect = BUTTON_MO_SHAKE;
        ButtonFancy("Shake", p); ImGui::SameLine();

        p.mouseOverEffect = BUTTON_MO_BOUNCE;
        ButtonFancy("Bounce", p);

        ImGui::Separator();

        // ImGui::Text("Animation Speed");
        // p.mouseOverEffect = BUTTON_MO_GLOW_PULSE;
        // p.color = IM_COL32(200, 40, 40, 255);
        //
        // p.animationSpeed = 5.0f;
        // ButtonFancy("Slow Pulse", p); ImGui::SameLine();
        //
        // p.animationSpeed = 25.0f;
        // ButtonFancy("URGENT", p);


        ImGui::Separator();

        // --- ROW 2: Shapes & Rounding ---
        ImGui::Text("Shapes & Rounding:");

        p.mouseOverEffect = BUTTON_MO_HIGHLIGHT;
        p.color = IM_COL32(100, 100, 100, 255); // Grey

        p.rounding = 0.0f;
        ButtonFancy("Square", p); ImGui::SameLine();

        p.rounding = 8.0f;
        ButtonFancy("Rounded", p); ImGui::SameLine();

        p.rounding = p.size.y * 0.5f; // Perfect Capsule
        ButtonFancy("Capsule", p);

        ImGui::Separator();

        // --- ROW 3: Toggle Features ---
        ImGui::Text("Feature Toggles:");

        p.rounding = 4.0f;
        p.color = IM_COL32(40, 150, 40, 255); // Green

        p.bevel = true; p.gloss = true;
        ButtonFancy("Bevel+Gloss", p); ImGui::SameLine();

        p.bevel = false; p.gloss = false;
        ButtonFancy("Flat Style", p); ImGui::SameLine();

        p.shadowText = false;
        ButtonFancy("No Shadow", p);

        ImGui::End();
    }



    //------------------------------------------------------------------------------
    //------------------------------------------------------------------------------
    inline bool ColoredButton(const char* label, ImU32 color, ImVec2 size) {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems) return false;

        const ImGuiID id = window->GetID(label);
        const ImRect bb(window->DC.CursorPos, window->DC.CursorPos + size);
        ImGui::ItemSize(size);
        if (!ImGui::ItemAdd(bb, id)) return false;

        bool hovered, held;
        bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);

        ImDrawList* dl = window->DrawList;
        const float rounding = ImGui::GetStyle().FrameRounding;

        // 1. Interaction & Pulse Logic
        ImU32 baseCol = color;
        if (held) {
            // Darken when clicked
            baseCol = ImGui::ColorConvertFloat4ToU32(ImGui::ColorConvertU32ToFloat4(color) * ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
        } else if (hovered) {
            // Pulse brightness on hover
            float pulse = (sinf((float)ImGui::GetTime() * 10.0f) * 0.5f) + 0.5f;
            float boost = 1.0f + (pulse * 0.25f);
            ImVec4 c = ImGui::ColorConvertU32ToFloat4(color);
            baseCol = ImGui::ColorConvertFloat4ToU32(ImVec4(c.x * boost, c.y * boost, c.z * boost, c.w));
        }

        // 2. Main Body (Respects FrameRounding)
        dl->AddRectFilled(bb.Min, bb.Max, baseCol, rounding);

        // 3. Hardware Bevel Logic
        // We draw two concentric rounded rectangles to simulate the 3D edge without line-bleeding
        if (rounding > 0.0f) {
            // Highlight top-left arc
            dl->AddRect(bb.Min, bb.Max, IM_COL32(255, 255, 255, 45), rounding, 0, 1.0f);
            // Inner shadow to give depth
            dl->AddRect(bb.Min + ImVec2(1,1), bb.Max - ImVec2(1,1), IM_COL32(0, 0, 0, 40), rounding, 0, 1.0f);
        }

        // 4. Glossy Overlay
        // Note: AddRectFilledMultiColor is always rectangular.
        // We draw it slightly inset or on top half to simulate a glass reflection.
        dl->AddRectFilledMultiColor(
            bb.Min + ImVec2(rounding * 0.2f, 0), ImVec2(bb.Max.x - rounding * 0.2f, bb.Min.y + size.y * 0.5f),
                                    IM_COL32(255, 255, 255, 50), IM_COL32(255, 255, 255, 50),
                                    IM_COL32(255, 255, 255, 0),  IM_COL32(255, 255, 255, 0)
        );

        // 5. Centered Shadowed Text
        ImVec2 textSize = ImGui::CalcTextSize(label);
        ImVec2 textPos = bb.Min + (size - textSize) * 0.5f;

        // Draw Shadow
        dl->AddText(textPos + ImVec2(1.0f, 1.0f), IM_COL32(0, 0, 0, 200), label);
        // Draw Main Text
        dl->AddText(textPos, IM_COL32_WHITE, label);

        // 6. External Pulse Border (Only on Hover)
        if (hovered && !held) {
            float p = (sinf((float)ImGui::GetTime() * 10.0f) * 0.5f) + 0.5f;
            dl->AddRect(bb.Min - ImVec2(0.5f, 0.5f), bb.Max + ImVec2(0.5f, 0.5f),
                        IM_COL32(255, 255, 255, (int)(p * 180)), rounding, 0, 1.0f);
        }

        return pressed;
    }
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

    //------------------------------------------------------------------------------
    //---------------- MiniKnobs
    inline bool MiniKnobFloat(const char* label, float* v, float v_min, float v_max, float radius = 12.f,  float speed = 0.01f) {
        ImGui::PushID(label);
        ImVec2 pos = ImGui::GetCursorScreenPos();
        ImVec2 size = ImVec2(radius * 2, radius * 2);
        ImGui::InvisibleButton("##knob", size);

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
        // dl->AddCircleFilled(center, 10.0f, col);
        dl->AddCircleFilled(center, (float)radius, col);
        // Draw "Value Arc" (optional fancy addition)
        // dl->PathArcTo(center, 8.0f, 0.75f * 3.14159f, angle, 10);
        dl->PathArcTo(center, (float)(radius - 1.f), 0.75f * 3.14159f, angle, 10);

        dl->PathStroke(ImGui::GetColorU32(ImGuiCol_PlotLines), 0, 2.0f);

        // Draw Indicator Needle
        // dl->AddLine(center,
        //             ImVec2(center.x + cosf(angle) * 8, center.y + sinf(angle) * 8),
        //             ImGui::GetColorU32(ImGuiCol_Text), 2.0f);

        dl->AddLine(center,
                    ImVec2(center.x + cosf(angle) * (radius - 1.f), center.y + sinf(angle) * (radius - 1.f)),
                    ImGui::GetColorU32(ImGuiCol_Text), 2.0f);


        if (is_hovered)
            ImGui::SetTooltip("%s: %.2f", label, *v);

        ImGui::PopID();
        return value_changed;
    }

    // --------------- LEDRingKnob
    inline bool LEDRingKnob(const char* label, float* v, float v_min, float v_max, float radius = 18.f, float speed = 0.01f) {
        ImVec2 p = ImGui::GetCursorScreenPos();
        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImGui::InvisibleButton(label, {radius*2, radius*2});

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

        return value_changed;
    }


    // --------------- MiniKnobInt
    inline bool MiniKnobInt(const char* label, int* v, int v_min, int v_max, float radius = 12.f) {
        ImGui::PushID(label);
        ImVec2 pos = ImGui::GetCursorScreenPos();
        ImVec2 size = ImVec2(radius * 2, radius * 2);
        ImGui::InvisibleButton("##knob", size);

        bool value_changed = false;
        bool is_active = ImGui::IsItemActive();
        bool is_hovered = ImGui::IsItemHovered();

        // 1. SCROLL WHEEL support (fastest way to change values)
        if (is_hovered && ImGui::GetIO().MouseWheel != 0) {
            int new_v = std::clamp(*v + (int)ImGui::GetIO().MouseWheel, v_min, v_max);
            if (new_v != *v) {
                *v = new_v;
                value_changed = true;
            }
        }

        // 2. DRAG support (with higher sensitivity)
        if (is_active && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
            float delta = ImGui::GetIO().MouseDelta.y;
            if (std::abs(delta) > 0.0f) {
                // Sensitivity: every 5 pixels of drag is 1 step
                // We use a temporary static float to keep movement smooth
                static float accumulator = 0.0f;
                accumulator -= delta;

                if (std::abs(accumulator) >= 5.0f) {
                    int steps = (int)(accumulator / 5.0f);
                    int new_v = std::clamp(*v + steps, v_min, v_max);
                    if (new_v != *v) {
                        *v = new_v;
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
        // dl->AddCircleFilled(center, 10.0f, col);
        // dl->AddLine(center,
        //             ImVec2(center.x + cosf(angle) * 8, center.y + sinf(angle) * 8),
        //             ImGui::GetColorU32(ImGuiCol_Text), 2.0f);
        dl->AddCircleFilled(center, (float)radius, col);
        dl->AddLine(center,
                    ImVec2(center.x + cosf(angle) * (radius - 1.f), center.y + sinf(angle) * (radius - 1.f)),
                    ImGui::GetColorU32(ImGuiCol_Text), 2.0f);


        if (is_hovered)
            ImGui::SetTooltip("%s: %d", label, *v);

        ImGui::PopID();
        return value_changed;
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

    //---------------- LEDCheckBox (default glow)
    // inline bool LEDCheckBox(std::string caption, bool* on, const ImVec4 color) {
    //     LedParams params = LED_GREEN_GLOW;
    //     params.colorOn = color;
    //
    //     bool pressed = DrawLED("", *on, params);
    //     if (pressed)  *on = !(*on);
    //
    //     ImGui::SameLine();
    //     ImGui::TextColored(color, "%s", caption.c_str());
    //     return pressed;
    // }
    // better with click on text:
    inline bool LEDCheckBox(const std::string& caption, bool* on, const ImVec4 color) {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems) return false;
        ImGui::PushID(caption.c_str());

        float height = ImGui::GetFrameHeight();
        ImVec2 text_size = ImGui::CalcTextSize(caption.c_str(), nullptr, true);
        ImVec2 size(height + 4.0f + text_size.x, height);

        ImRect bb(window->DC.CursorPos, window->DC.CursorPos + size);
        ImGui::ItemSize(bb);
        if (!ImGui::ItemAdd(bb, window->GetID("##hitbox"))) {
            ImGui::PopID();
            return false;
        }
        bool hovered, held;
        bool pressed = ImGui::ButtonBehavior(bb, window->GetID("##hitbox"), &hovered, &held);
        if (pressed) {
            *on = !(*on); // Toggle the actual boolean value
        }
        LedParams params = LED_GREEN_GLOW;
        params.colorOn = color;
        ImVec2 led_pos = bb.Min;
        ImGui::SetCursorScreenPos(led_pos);
        DrawLED("##led_internal", *on, params);

        // ImU32 text_col = *on ? ImGui::ColorConvertFloat4ToU32(color) : ImGui::GetColorU32(ImGuiCol_TextDisabled);
        ImU32 text_col = ImGui::ColorConvertFloat4ToU32(color);
        if (*on) {
            window->DrawList->AddText(ImVec2(bb.Min.x + height + 4.0f, bb.Min.y + (height - text_size.y) * 0.5f), text_col, caption.c_str());
        } else {
            window->DrawList->AddText(ImVec2(bb.Min.x + height + 4.0f, bb.Min.y + (height - text_size.y) * 0.5f), text_col, caption.c_str());
        }
        ImGui::PopID();
        return pressed;
    }
    //------------------------------------------------------------------------------

    //---------------- LCD Segments
    inline void DrawLCDDigitDelayed(ImDrawList* draw_list, ImVec2 pos, int digit, float height, ImU32 color_on) {
        float width = height * 0.5f;
        float thickness = height * 0.1f;
        float pad = thickness * 0.5f;
        // Speed of the "ghosting" effect
        float fade_out_speed = ImGui::GetIO().DeltaTime * 4.0f;

        static const uint8_t segment_masks[11] = {
            0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F, 0x00
        };

        uint8_t target_mask = (digit >= 0 && digit <= 9) ? segment_masks[digit] : 0x00;

        // Unique ID for this specific digit position
        // ImGuiID id = ImGui::GetID(&pos.x + &pos.y);
        ImGuiID id = ImGui::GetID("segments");
        ImGuiStorage* storage = ImGui::GetStateStorage();

        auto draw_seg = [&](ImVec2 p1, ImVec2 p2, int bit) {
            // Retrieve persistent alpha for this segment
            float current_alpha = storage->GetFloat(id + bit, 0.0f);
            float target_alpha = (target_mask & (1 << bit)) ? 1.0f : 0.0f;

            // Logic: Turn on instantly, fade out slowly
            if (target_alpha > current_alpha) current_alpha = target_alpha;
            else current_alpha = ImMax(0.0f, current_alpha - fade_out_speed);

            storage->SetFloat(id + bit, current_alpha);

            // Mix "Off" color (dim/transparent) with "On" color
            ImVec4 off_v = ImGui::ColorConvertU32ToFloat4(IM_COL32(30, 30, 30, 255));
            ImVec4 on_v = ImGui::ColorConvertU32ToFloat4(color_on);

            // Linear interpolation for the fading effect
            ImU32 seg_col = ImGui::ColorConvertFloat4ToU32({
                ImLerp(off_v.x, on_v.x, current_alpha),
                                                            ImLerp(off_v.y, on_v.y, current_alpha),
                                                            ImLerp(off_v.z, on_v.z, current_alpha),
                                                            1.0f
            });

            draw_list->AddLine(p1, p2, seg_col, thickness);
        };

        float h2 = height * 0.5f;
        draw_seg({pos.x + pad, pos.y}, {pos.x + width - pad, pos.y}, 0);             // a
        draw_seg({pos.x + width, pos.y + pad}, {pos.x + width, pos.y + h2 - pad}, 1); // b
        draw_seg({pos.x + width, pos.y + h2 + pad}, {pos.x + width, pos.y + height - pad}, 2); // c
        draw_seg({pos.x + pad, pos.y + height}, {pos.x + width - pad, pos.y + height}, 3); // d
        draw_seg({pos.x, pos.y + h2 + pad}, {pos.x, pos.y + height - pad}, 4);       // e
        draw_seg({pos.x, pos.y + pad}, {pos.x, pos.y + h2 - pad}, 5);                // f
        draw_seg({pos.x + pad, pos.y + h2}, {pos.x + width - pad, pos.y + h2}, 6);   // g
    }

  // ------------- LCDDisplay
  // Like LCDNumber but with delayed (more oldschool)
  inline void LCDDisplay(const char* label, float value, int digits, int precision, float height, ImU32 color) {
      ImGui::PushID(label);
      ImVec2 pos = ImGui::GetCursorScreenPos();
      float digit_w = height * 0.5f;
      float spacing = height * 0.2f;

      char buf[32];
      ImFormatString(buf, 32, "%0*.*f", digits, precision, value);

      ImDrawList* dl = ImGui::GetWindowDrawList();


      for (int i = 0; buf[i] != '\0'; i++) {
          if (buf[i] == '.') {
              // Tiny fading dot
              dl->AddCircleFilled({pos.x - spacing * 0.4f, pos.y + height}, height * 0.06f, color);
              continue;
          }
          ImGui::PushID(i); // This ensures "segments" gets a unique ID per loop iteration
          DrawLCDDigitDelayed(dl, pos, buf[i] - '0', height, color);
          ImGui::PopID();
          pos.x += digit_w + spacing;
      }

      // for (int i = 0; buf[i] != '\0'; i++) {
      //     if (buf[i] == '.') {
      //         // Tiny fading dot
      //         dl->AddCircleFilled({pos.x - spacing * 0.4f, pos.y + height}, height * 0.06f, color);
      //         continue;
      //     }
      //     DrawLCDDigitDelayed(dl, pos, buf[i] - '0', height, color);
      //     pos.x += digit_w + spacing;
      // }

      ImGui::Dummy(ImVec2((digit_w + spacing) * strlen(buf), height));

      ImFlux::Hint(label);
      ImGui::PopID();
  }

  //------------------------------------------------------------------------------
  // ------------- LCD simpler direct Version:
    inline void DrawLCDDigit(ImDrawList* draw_list, ImVec2 pos, int digit, float height, ImU32 color_on, ImU32 color_off) {
        float width = height * 0.5f;
        float thickness = height * 0.1f;
        float pad = thickness * 0.5f;

        // Segment definitions (a-g)
        //    -a-
        //  f|   |b
        //    -g-
        //  e|   |c
        //    -d-
        static const uint8_t segments[11] = {
            0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F, 0x00 // 0-9 and blank
        };

        uint8_t mask = (digit >= 0 && digit <= 9) ? segments[digit] : segments[10];

        auto draw_seg = [&](ImVec2 p1, ImVec2 p2, int bit) {
            draw_list->AddLine(p1, p2, (mask & (1 << bit)) ? color_on : color_off, thickness);
        };

        float h2 = height * 0.5f;
        // Horizontal segments
        draw_seg({pos.x + pad, pos.y}, {pos.x + width - pad, pos.y}, 0);             // a
        draw_seg({pos.x + pad, pos.y + h2}, {pos.x + width - pad, pos.y + h2}, 6);   // g
        draw_seg({pos.x + pad, pos.y + height}, {pos.x + width - pad, pos.y + height}, 3); // d

        // Vertical segments
        draw_seg({pos.x + width, pos.y + pad}, {pos.x + width, pos.y + h2 - pad}, 1); // b
        draw_seg({pos.x + width, pos.y + h2 + pad}, {pos.x + width, pos.y + height - pad}, 2); // c
        draw_seg({pos.x, pos.y + h2 + pad}, {pos.x, pos.y + height - pad}, 4);       // e
        draw_seg({pos.x, pos.y + pad}, {pos.x, pos.y + h2 - pad}, 5);                // f
    }

    // -------- LCDNumber
    inline void LCDNumber(float value, int num_digits, int decimal_precision, float height, ImU32 color_on) {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems) return;

        ImDrawList* draw_list = window->DrawList;
        ImVec2 pos = ImGui::GetCursorScreenPos();

        float digit_width = height * 0.5f;
        float spacing = height * 0.15f;
        ImU32 color_off = (color_on & 0x00FFFFFF) | 0x15000000; // Dim version of same color

        // Convert value to string to process digits
        char buf[32];
        if (decimal_precision > 0)
            ImFormatString(buf, 32, "%0*.*f", num_digits + (decimal_precision ? 1 : 0), decimal_precision, value);
        else
            ImFormatString(buf, 32, "%0*d", num_digits, (int)value);

        for (int i = 0; buf[i] != '\0'; i++) {
            if (buf[i] == '.') {
                // Draw decimal point
                draw_list->AddCircleFilled({pos.x - spacing * 0.5f, pos.y + height}, height * 0.05f, color_on);
                continue;
            }

            int digit = buf[i] - '0';
            DrawLCDDigit(draw_list, pos, digit, height, color_on, color_off);
            pos.x += digit_width + spacing;
        }

        // Advance ImGui cursor
        ImGui::Dummy(ImVec2((digit_width + spacing) * strlen(buf), height));
    }


    //---------------------------------------------------------------------------
    inline bool FaderVertical(const char* label, ImVec2 size, float* v, float v_min, float v_max) {
        ImGui::PushID(label);
        ImVec2 pos = ImGui::GetCursorScreenPos();
        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImGuiIO& io = ImGui::GetIO();

        // 1. Interaction
        ImGui::InvisibleButton("##fader", size);
        bool is_active = ImGui::IsItemActive();
        bool is_hovered = ImGui::IsItemHovered();

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

        // 1. Interaction (Horizontal logic)
        ImGui::InvisibleButton("##fader", size);
        bool is_active = ImGui::IsItemActive();
        bool is_hovered = ImGui::IsItemHovered();

        if (is_active && io.MouseDown[0]) {
            float mouse_x = io.MousePos.x - pos.x;
            float fraction = std::clamp(mouse_x / size.x, 0.0f, 1.0f);
            *v = v_min + fraction * (v_max - v_min);
        }

        // 2. Drawing the "Track" (Horizontal slot)
        float mid_y = pos.y + size.y * 0.5f;
        dl->AddRectFilled({pos.x, mid_y - 2}, {pos.x + size.x, mid_y + 2}, IM_COL32(20, 20, 20, 255), 2.0f);

        // 3. Drawing Tick Marks (Every 25%)
        for (int i = 0; i <= 4; i++) {
            float tx = pos.x + (size.x * i * 0.25f);
            dl->AddLine({tx, mid_y - 8}, {tx, mid_y - 4}, IM_COL32(80, 80, 80, 255));
        }

        // 4. Drawing the "Cap" (Vertical handle moving horizontally)
        float cap_w = 20.0f; // Width of the handle
        float fraction = (*v - v_min) / (v_max - v_min);
        float cap_x = pos.x + fraction * (size.x - cap_w);

        ImU32 cap_col = is_active ? IM_COL32(180, 180, 180, 255) : (is_hovered ? IM_COL32(140, 140, 140, 255) : IM_COL32(110, 110, 110, 255));

        // Cap matches the vertical handle style but oriented for horizontal travel
        dl->AddRectFilled({cap_x, pos.y}, {cap_x + cap_w, pos.y + size.y}, cap_col, 2.0f);
        dl->AddLine({cap_x + cap_w * 0.5f, pos.y + 2}, {cap_x + cap_w * 0.5f, pos.y + size.y - 2}, IM_COL32(0, 0, 0, 200), 2.0f);

        Hint(label);

        ImGui::PopID();
        return is_active;
    }
    // -------------- FaderHWithText
    inline bool FaderHWithText(const char* label, float* v, float v_min, float v_max, const char* format) {
        ImGui::BeginGroup();
        bool active = FaderHorizontal(label, ImVec2(120, 18), v, v_min, v_max);
        ImGui::SameLine();
        ImGui::TextDisabled(format, *v); // Displays "Hz", "s", or "Wet" next to the fader
        ImGui::EndGroup();
        return active;
    }


    // --------- not a fader but a button is this style
    inline bool FaderButton(const char* label, ImVec2 size) {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems) return false;

        ImGui::PushID(label);
        ImGuiID id = window->GetID("##cap");

        // 1. Setup Hitbox
        const ImRect bb(window->DC.CursorPos, window->DC.CursorPos + size);
        ImGui::ItemSize(bb);
        if (!ImGui::ItemAdd(bb, id)) {
            ImGui::PopID();
            return false;
        }

        // 2. Interaction
        bool hovered, held;
        bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);

        // 3. Drawing (The Hardware Look)
        ImDrawList* dl = window->DrawList;

        // Shadow for depth
        dl->AddRectFilled(bb.Min + ImVec2(2, 2), bb.Max + ImVec2(2, 2), IM_COL32(0, 0, 0, 100), 2.0f);

        // Dynamic Colors based on state
        ImU32 col_top = held ? IM_COL32(100, 100, 100, 255) : (hovered ? IM_COL32(160, 160, 160, 255) : IM_COL32(130, 130, 130, 255));
        ImU32 col_brdr = held ? IM_COL32(50, 50, 50, 255) : IM_COL32(80, 80, 80, 255);

        // Main Plastic Body
        ImVec2 offset = held ? ImVec2(1, 1) : ImVec2(0, 0); // Depress effect
        dl->AddRectFilled(bb.Min + offset, bb.Max + offset, col_top, 2.0f);
        dl->AddRect(bb.Min + offset, bb.Max + offset, col_brdr, 2.0f);

        // The center "Grip" line (like your faders)
        // float line_thickness = 2.0f;
        // if (size.x > size.y) {
        //     // Horizontal Cap style (Vertical line)
        //     dl->AddLine({bb.Min.x + size.x * 0.5f + offset.x, bb.Min.y + 2 + offset.y},
        //                 {bb.Min.x + size.x * 0.5f + offset.x, bb.Max.y - 2 + offset.y},
        //                 IM_COL32(0, 0, 0, 150), line_thickness);
        // } else {
        //     // Vertical Cap style (Horizontal line)
        //     dl->AddLine({bb.Min.x + 2 + offset.x, bb.Min.y + size.y * 0.5f + offset.y},
        //                 {bb.Max.x - 2 + offset.x, bb.Min.y + size.y * 0.5f + offset.y},
        //                 IM_COL32(0, 0, 0, 150), line_thickness);
        // }

        // Optional Label
        if (label[0] != '#' && label[1] != '#') {
            ImVec2 t_size = ImGui::CalcTextSize(label);
            dl->AddText(bb.Min + (size - t_size) * 0.5f + offset, IM_COL32_BLACK, label);
            offset.x+=1.f; offset.y+=1.f;
            dl->AddText(bb.Min + (size - t_size) * 0.5f + offset, IM_COL32_WHITE, label);
        }

        ImGui::PopID();
        return pressed;
    }

    //---------------------------------------------------------------------------
    // Helper to draw a small hardware-style triangle button
    inline bool StepperButton(const char* id, bool is_left, ImVec2 size) {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        ImGuiID imgui_id = window->GetID(id);
        ImRect bb(window->DC.CursorPos, window->DC.CursorPos + size);
        ImGui::ItemSize(bb);
        if (!ImGui::ItemAdd(bb, imgui_id)) return false;

        bool hovered, held;
        bool pressed = ImGui::ButtonBehavior(bb, imgui_id, &hovered, &held);

        // Visuals
        ImDrawList* dl = window->DrawList;
        ImU32 col = held ? IM_COL32(100, 100, 100, 255) : (hovered ? IM_COL32(70, 70, 70, 255) : IM_COL32(40, 40, 40, 255));

        // Button Shape
        dl->AddRectFilled(bb.Min, bb.Max, col, 2.0f);
        dl->AddRect(bb.Min, bb.Max, IM_COL32(90, 90, 90, 255), 2.0f);

        // Triangle Icon
        float pad = size.x * 0.3f;
        ImVec2 a, b, c;
        if (is_left) {
            a = {bb.Max.x - pad, bb.Min.y + pad};
            b = {bb.Max.x - pad, bb.Max.y - pad};
            c = {bb.Min.x + pad, bb.Min.y + size.y * 0.5f};
        } else {
            a = {bb.Min.x + pad, bb.Min.y + pad};
            b = {bb.Min.x + pad, bb.Max.y - pad};
            c = {bb.Max.x - pad, bb.Min.y + size.y * 0.5f};
        }
        dl->AddTriangleFilled(a, b, c, IM_COL32(0, 255, 200, 255));

        return pressed;
    }

    template<typename T>
    inline bool ValueStepper(const char* label, int* current_idx, const T& items, int items_count, float width = 140.0f) {
        ImGui::PushID(label);
        bool changed = false;
        float h = ImGui::GetFrameHeight();
        ImVec2 btn_sz(h, h);

        // 1. Custom Left Button
        if (StepperButton("##left", true, btn_sz)) {
            *current_idx = (*current_idx > 0) ? *current_idx - 1 : items_count - 1;
            changed = true;
        }

        ImGui::SameLine(0, 4);

        // 2. LCD Display Area
        ImVec2 pos = ImGui::GetCursorScreenPos();
        ImVec2 size(width, h);

        if (ImGui::InvisibleButton("##display", size)) {
            *current_idx = (*current_idx + 1) % items_count;
            changed = true;
        }

        // Popup logic remains for Right-Click
        if (ImGui::BeginPopupContextItem("StepperPopup")) {
            for (int i = 0; i < items_count; i++) {
                // Determine text for selectable
                const char* item_name = "";
                if constexpr (std::is_pointer_v<std::decay_t<decltype(items[0])>>) item_name = items[i];
                else item_name = items[i].name.c_str(); // Handles your Instrument struct

                if (ImGui::Selectable(item_name, *current_idx == i)) {
                    *current_idx = i;
                    changed = true;
                }
            }
            ImGui::EndPopup();
        }

        // DRAW DISPLAY
        ImDrawList* dl = ImGui::GetWindowDrawList();
        dl->AddRectFilled(pos, pos + size, IM_COL32(10, 10, 12, 255), 2.0f);
        dl->AddRect(pos, pos + size, IM_COL32(60, 60, 70, 255), 2.0f);

        const char* text = "";
        if (*current_idx >= 0 && *current_idx < items_count) {
            if constexpr (std::is_pointer_v<std::decay_t<decltype(items[0])>>) text = items[*current_idx];
            else text = items[*current_idx].name.c_str();
        }

        ImVec2 t_size = ImGui::CalcTextSize(text);
        dl->AddText({pos.x + (size.x - t_size.x) * 0.5f, pos.y + (size.y - t_size.y) * 0.5f},
                    IM_COL32(0, 255, 180, 255), text);

        ImGui::SameLine(0, 4);

        // 3. Custom Right Button
        if (StepperButton("##right", false, btn_sz)) {
            *current_idx = (*current_idx + 1) % items_count;
            changed = true;
        }

        ImGui::PopID();
        return changed;
    }


} //namespace

