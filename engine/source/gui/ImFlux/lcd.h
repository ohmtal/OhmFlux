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
    //--------------------------------------------------------------------------
    inline void DrawLCD14Segment(ImDrawList* dl, ImVec2 pos, char c, float h, ImU32 colOn, ImU32 colOff) {
        float w = h * 0.5f;
        float thickness = h * 0.08f;
        float pad = h * 0.03f;

        // Standard C++ approach: Initialize with 0 and fill
        static uint16_t charTable[128] = { 0 };
        static bool initialized = false;
        if (!initialized) {
            // Numbers (0-9) - Standard 7-segment logic applied to 14-seg
            charTable['0'] = 0x003F; charTable['1'] = 0x0006; charTable['2'] = 0x00DB;
            charTable['3'] = 0x00CF; charTable['4'] = 0x00E6; charTable['5'] = 0x00ED;
            charTable['6'] = 0x00FD; charTable['7'] = 0x0007; charTable['8'] = 0x00FF;
            charTable['9'] = 0x00EF;

            // Alphabet - Re-verified based on your corrections
            charTable['A'] = 0x00F7;
            charTable['B'] = 0x038F; // Top(0), TR(1), BR(2), Bot(3), MidR(7), TC(8), BC(9)
            charTable['C'] = 0x0039;
            charTable['D'] = 0x030F; // Vertical center + Top/Bot/TR/BR
            charTable['E'] = 0x0079;
            charTable['F'] = 0x0071;
            charTable['G'] = 0x00BD;
            charTable['H'] = 0x00F6;
            charTable['I'] = 0x0309; // Top/Bot + Vertical Center
            charTable['J'] = 0x001E;
            charTable['K'] = 0x2870; // (1<<5|1<<4) + (1<<6) + (1<<11|1<<13)
            charTable['L'] = 0x0038;
            charTable['M'] = 0x0C36; // TL/BL/TR/BR + Diags TL/TR
            charTable['N'] = 0x2436; // TL/BL/TR/BR + Diags TL/BR
            charTable['O'] = 0x003F;
            charTable['P'] = 0x00F3;
            charTable['Q'] = 0x203F; // O + Diag BR
            charTable['R'] = 0x20F3; // P + Diag BR
            charTable['S'] = 0x00ED;
            charTable['T'] = 0x0301; // Top + Vertical Center
            charTable['U'] = 0x003E;

            // V not possible with 14 digits :P
            // charTable['V'] = 0x0C14; // TL-Diag(10), TR-Diag(11) + BL-Side(4), BR-Side(2)
            charTable['V'] = 0x3022; // TL-Diag(10), TR-Diag(11) + BL-Side(4), BR-Side(2)

            charTable['W'] = 0x3036; // TL/BL/TR/BR + Diags BL/BR
            charTable['X'] = 0x3C00; // All 4 Diagonals
            charTable['Y'] = 0x0E00; // DiagTL(10), DiagTR(11), BC(9)
            charTable['Z'] = 0x1809; // Top(0), Bot(3), DiagTR(11), DiagBL(12)

            // Punctuation & Symbols
            charTable['-'] = 0x00C0;
            charTable['_'] = 0x0008;
            charTable['+'] = 0x03C0; // Mid cross + Vertical center
            charTable['/'] = 0x1200; // Diags BL to TR (12+11) -> 0x1800 if 11,12
            charTable['/'] = 0x1800; // CORRECTED: Diags BL(12) + TR(11)
            charTable['\\'] = 0x2400; // CORRECTED: Diags TL(10) + BR(13)
            charTable['*'] = 0x3FC0;
            charTable['|'] = 0x0300; // Vertical center (8+9)
            charTable['<'] = 0x1400; // Diag TR(11) + Diag BL(12)
            charTable['>'] = 0x2800; // Diag TL(10) + Diag BR(13)
            charTable['['] = 0x0039;
            charTable[']'] = 0x000F;
            charTable['='] = 0x00C8; // Mid-cross + Bot
            charTable['!'] = 0x0200; // Top-center (8)

            initialized = true;
        }

        unsigned char lookupIdx = (unsigned char)c;
        uint16_t mask = (lookupIdx < 128) ? charTable[toupper(lookupIdx)] : 0;


        auto DrawSeg = [&](int bit, ImVec2 p1, ImVec2 p2) {
            dl->AddLine(pos + p1, pos + p2, (mask & (1 << bit)) ? colOn : colOff, thickness);
        };

        // Frame (0-5)
        DrawSeg(0, {pad, 0}, {w-pad, 0});               // Top
        DrawSeg(1, {w, pad}, {w, h/2-pad});             // Top-Right
        DrawSeg(2, {w, h/2+pad}, {w, h-pad});           // Bottom-Right
        DrawSeg(3, {pad, h}, {w-pad, h});               // Bottom
        DrawSeg(4, {0, h/2+pad}, {0, h-pad});           // Bottom-Left
        DrawSeg(5, {0, pad}, {0, h/2-pad});             // Top-Left

        // Crossbar (6-7) - For H, A, F, E
        DrawSeg(6, {pad, h/2}, {w/2-pad, h/2});         // Mid-Left
        DrawSeg(7, {w/2+pad, h/2}, {w-pad, h/2});       // Mid-Right

        // Vertical Center (8-9) - For T, I
        DrawSeg(8, {w/2, pad}, {w/2, h/2-pad});         // Top-Center
        DrawSeg(9, {w/2, h/2+pad}, {w/2, h-pad});       // Bottom-Center

        // Diagonals (10-13) - For M, X, W, R, K
        DrawSeg(10, {pad, pad}, {w/2-pad, h/2-pad});    // Diag Top-Left
        DrawSeg(11, {w-pad, pad}, {w/2+pad, h/2-pad});  // Diag Top-Right
        DrawSeg(12, {pad, h-pad}, {w/2-pad, h/2+pad});  // Diag Bottom-Left
        DrawSeg(13, {w-pad, h-pad}, {w/2+pad, h/2+pad});// Diag Bottom-Right
    }
    //--------------------------------------------------------------------------
    inline void LCDText(std::string text, int display_chars, float height, ImU32 color_on, bool scroll = true, float scroll_speed = 2.0f) {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems) return;

        ImDrawList* draw_list = window->DrawList;
        ImVec2 pos = ImGui::GetCursorScreenPos();

        float digit_width = height * 0.5f;
        float spacing = height * 0.15f;
        ImU32 color_off = (color_on & 0x00FFFFFF) | 0x15000000;

        // 1. Convert the entire input to Upper Case for the LCD Table
        std::string processedText = text;
        for (auto & c: processedText) c = (char)toupper((unsigned char)c);

        int offset = 0;
        if (scroll && (int)processedText.length() > display_chars) {
            // Add 4 spaces of padding for a cleaner loop
            std::string loopedText = processedText + "    ";

            // Use double/fmod for smooth, long-running scrolling
            double time = ImGui::GetTime() * scroll_speed;
            offset = (int)fmod(time, (double)loopedText.length());

            // Render the visible window
            for (int i = 0; i < display_chars; i++) {
                char c = loopedText[(offset + i) % loopedText.length()];
                DrawLCD14Segment(draw_list, pos, c, height, color_on, color_off);
                pos.x += digit_width + spacing;
            }
        } else {
            // Static rendering (centered or left-aligned)
            for (int i = 0; i < display_chars; i++) {
                char c = (i < (int)processedText.length()) ? processedText[i] : ' ';
                DrawLCD14Segment(draw_list, pos, c, height, color_on, color_off);
                pos.x += digit_width + spacing;
            }
        }

        // Advance Cursor
        ImGui::Dummy(ImVec2((digit_width + spacing) * display_chars, height));
    }



} //namespace
