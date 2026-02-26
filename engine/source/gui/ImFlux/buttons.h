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
        ImVec2 size = { 60.f, 24.f };
        bool   bevel = true;
        bool   selected = false;
        bool   gloss = true;
        bool   shadowText = true;
        float  rounding = -1.0f; // -1 use global style
        float  animationSpeed = 1.0f; //NOTE: unused
        ImU32  textColor = IM_COL32(240,240,240, 255);
        ButtonMouseOverEffects mouseOverEffect = BUTTON_MO_HIGHLIGHT;
        // love this lazy way : example ImFlux::RED_BUTTON.WithSize(ImVec2(tmpWidth, 24.f )).WithRounding(12.f))
        ButtonParams WithSize(ImVec2 s) const { ButtonParams p = *this; p.size = s; return p; }
        ButtonParams WithRounding(float r) const { ButtonParams p = *this; p.rounding = r; return p; }

        ButtonParams WithColor(ImU32 c) const { ButtonParams p = *this; p.color = c; return p; }
    };

    constexpr ButtonParams DEFAULT_BUTTON;
    constexpr ButtonParams RED_BUTTON { .color = IM_COL32(180,0,0,255)};
    constexpr ButtonParams GREEN_BUTTON { .color = IM_COL32(0,180,0,255)};
    constexpr ButtonParams BLUE_BUTTON { .color = IM_COL32(0,0,180,255)};
    constexpr ButtonParams SLATE_BUTTON { .color = IM_COL32(46, 61, 79, 255) };
    constexpr ButtonParams SLATEDARK_BUTTON { .color = IM_COL32(23, 30, 40, 255) };

    constexpr ButtonParams YELLOW_BUTTON { .color = IM_COL32(180,180,0,255)};



    inline bool ButtonFancy(std::string label, ButtonParams params = DEFAULT_BUTTON)
    {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems) return false;

        // 1. Calculate actual size if -FLT_MIN or 0.0f is passed
        ImVec2 lSize = params.size;
        if (lSize.x <= 0.0f) lSize.x = ImGui::GetContentRegionAvail().x + lSize.x; // Handles -FLT_MIN (full width)
        if (lSize.y <= 0.0f) lSize.y = ImGui::GetFrameHeight(); // Fallback for height if not specified


        const ImGuiID id = window->GetID(label.c_str());
        const ImRect bb(window->DC.CursorPos, window->DC.CursorPos + lSize);
        ImGui::ItemSize(lSize);
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

        if (glowAlpha > 0.001f) {
            ImVec4 bc = ImGui::ColorConvertU32ToFloat4(params.color);
            // Brighten the core of the glow significantly
            ImVec4 gc = ImVec4(ImMin(bc.x * 1.8f, 1.0f), ImMin(bc.y * 1.8f, 1.0f), ImMin(bc.z * 1.8f, 1.0f), 1.0f);

            for (int i = 1; i <= 24; i++) { // More layers for smoother spread (was 8)
                float spread = (float)i * 0.4f; //  spread (was 1.2f)

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

        if (params.selected) {
            dl->AddRect(rbb.Min, rbb.Max, IM_COL32(0, 0, 0, 60), rounding);
            dl->AddRect(rbb.Min + ImVec2(1,1), rbb.Max - ImVec2(1,1), IM_COL32(0, 0, 0, 30), rounding);
            dl->AddRect(rbb.Min + ImVec2(2,2), rbb.Max - ImVec2(1,1), IM_COL32(255, 255, 255, 20), rounding);

        } else
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


        ImU32 lTextColor = params.textColor;
        ImU32 lShadowColor = IM_COL32_BLACK;
        if (ImGui::GetItemFlags() & ImGuiItemFlags_Disabled) {
            lTextColor = ImGui::GetColorU32(ImGuiCol_TextDisabled);
            // Usually, we dim or remove the shadow when disabled
            lShadowColor = (lShadowColor & IM_COL32_A_MASK) >> 1;
        }


        std::string lLabelStr = GetLabelText(label.c_str());
        ImVec2 textSize = ImGui::CalcTextSize(lLabelStr.c_str());
        ImVec2 textPos = rbb.Min + (lSize - textSize) * 0.5f;
        if (params.shadowText) dl->AddText(textPos + ImVec2(1.f, 1.f), lShadowColor, lLabelStr.c_str());
        dl->AddText(textPos, lTextColor, lLabelStr.c_str());

        return pressed;
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



//------------------------------------------------------------------------------
} //namespace
