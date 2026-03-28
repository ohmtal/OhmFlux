//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// ImFlux VirtualTapePlayer
//-----------------------------------------------------------------------------
#pragma once
#include "imgui.h"
#include "imgui_internal.h"

#include <string>
#include <cmath>
#include <cstring>


namespace ImFlux {

    enum class CassetteType { Standard, Transparent, Chrome };
    enum class CassetteMode { Stop, Play, Record };

    struct VirtualTapePlayer {

    private:
        float rotation = 0.0f;
        float head_pos = 0.0f; // 0.0 to 1.0 for head movement

    public:

        std::string label = "My Mix Tape";
        ImVec2 size = ImVec2(300.0f, 0.0f);
        CassetteType type = CassetteType::Standard;
        CassetteMode mode = CassetteMode::Stop;
        bool  scaleFont = true;
        float progress = 0.0f; //left rotator speed normalized modifier

        //--------------------------------------------------------------------------
        void Draw() {
            if (size.x < 150.0f) size.x = 150.0f;

            float cassette_w = size.x;
            float cassette_h = size.x / 1.58f;
            size.y = cassette_h ;

            // Update head position (animation)
            float target_head = (mode != CassetteMode::Stop) ? 1.0f : 0.0f;
            head_pos = ImLerp(head_pos, target_head, ImGui::GetIO().DeltaTime * 10.0f);

            ImGui::BeginChild("TapePlayer", size, ImGuiChildFlags_None, ImGuiWindowFlags_NoScrollbar);

            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            ImVec2 p0 = ImGui::GetCursorScreenPos();
            ImVec2 p1 = ImVec2(p0.x + cassette_w, p0.y + cassette_h);
            float rounding = cassette_w * 0.033f;

            // --- Theme Colors ---
            ImU32 body_col, outline_col, label_col, text_col;
            if (type == CassetteType::Transparent) {
                body_col = IM_COL32(60, 60, 70, 180);
                outline_col = IM_COL32(100, 100, 120, 200);
                label_col = IM_COL32(255, 255, 255, 100);
                text_col = IM_COL32(30, 30, 60, 255);
            } else if (type == CassetteType::Chrome) {
                body_col = IM_COL32(15, 15, 15, 255);
                outline_col = IM_COL32(180, 160, 100, 255); // Gold touch
                label_col = IM_COL32(40, 40, 40, 255);
                text_col = IM_COL32(200, 180, 120, 255);
            } else { // Standard
                body_col = IM_COL32(40, 40, 40, 255);
                outline_col = IM_COL32(80, 80, 80, 255);
                label_col = IM_COL32(220, 220, 220, 255);
                text_col = IM_COL32(20, 20, 100, 255);
            }

            // --- 1. Cassette Main Body ---
            draw_list->AddRectFilled(p0, p1, body_col, rounding);
            draw_list->AddRect(p0, p1, outline_col, rounding, 0, cassette_w * 0.003f + 1.0f);

            // --- 2. Bottom "Head" Trapezoid ---
            float trap_w_top = cassette_w * 0.7f;
            float trap_w_bot = cassette_w * 0.6f;
            float trap_h = cassette_h * 0.15f;
            float trap_y_off = cassette_w * 0.016f;
            ImVec2 t0 = ImVec2(p0.x + (cassette_w - trap_w_top) * 0.5f, p1.y - trap_h - trap_y_off);
            ImVec2 t1 = ImVec2(p0.x + (cassette_w + trap_w_top) * 0.5f, p1.y - trap_h - trap_y_off);
            ImVec2 t2 = ImVec2(p0.x + (cassette_w + trap_w_bot) * 0.5f, p1.y - trap_y_off);
            ImVec2 t3 = ImVec2(p0.x + (cassette_w - trap_w_bot) * 0.5f, p1.y - trap_y_off);
            draw_list->AddQuadFilled(t0, t1, t2, t3, IM_COL32(30, 30, 30, 255));
            draw_list->AddQuad(t0, t1, t2, t3, outline_col, 1.5f);

            // --- 3. Mechanical Parts (Head & Rollers) ---
            float mech_y_max = t0.y + 5.0f;
            float mech_y_min = t0.y + trap_h * 0.5f;
            float current_mech_y = ImLerp(mech_y_min, mech_y_max, head_pos);

            // Center Head
            ImVec2 head_size = ImVec2(cassette_w * 0.1f, trap_h * 0.6f);
            ImVec2 head_pos_v = ImVec2(p0.x + cassette_w * 0.5f - head_size.x * 0.5f, current_mech_y);
            draw_list->AddRectFilled(head_pos_v, ImVec2(head_pos_v.x + head_size.x, head_pos_v.y + head_size.y),
                                    mode == CassetteMode::Record ? IM_COL32(200, 10, 10, 255) : IM_COL32(120, 120, 130, 255)
                                    , 2.0f);
            draw_list->AddRect(head_pos_v, ImVec2(head_pos_v.x + head_size.x, head_pos_v.y + head_size.y), IM_COL32(180, 180, 190, 255), 2.0f);

            // Pinch Rollers
            float roller_rad = cassette_w * 0.025f;
            draw_list->AddCircleFilled(ImVec2(t0.x + cassette_w * 0.1f, current_mech_y + roller_rad), roller_rad, IM_COL32(20, 20, 20, 255));
            draw_list->AddCircleFilled(ImVec2(t1.x - cassette_w * 0.1f, current_mech_y + roller_rad), roller_rad, IM_COL32(20, 20, 20, 255));

            // --- 4. Label Area ---
            float label_margin = cassette_w * 0.05f;
            ImVec2 l0 = ImVec2(p0.x + label_margin, p0.y + label_margin);
            ImVec2 l1 = ImVec2(p1.x - label_margin, p1.y - trap_h - label_margin * 0.6f);
            draw_list->AddRectFilled(l0, l1, label_col, rounding * 0.25f);

            ImGui::SetCursorScreenPos(ImVec2(l0.x + label_margin * 0.6f, l0.y + label_margin * 0.3f));
            ImGui::PushStyleColor(ImGuiCol_Text, text_col);
            if (scaleFont) ImGui::SetWindowFontScale(cassette_w / 300.0f);
            ImGui::Text("%s", label.c_str());
            if (scaleFont) ImGui::SetWindowFontScale(1.0f);
            ImGui::PopStyleColor();

            // --- 5. Central Window ---
            float win_w = cassette_w * 0.45f;
            float win_h = cassette_h * 0.35f;
            ImVec2 w0 = ImVec2(p0.x + (cassette_w - win_w) * 0.5f, p0.y + (cassette_h - win_h) * 0.5f - cassette_h * 0.02f);
            ImVec2 w1 = ImVec2(w0.x + win_w, w0.y + win_h);
            draw_list->AddRectFilled(w0, w1, IM_COL32(10, 10, 10, 255), 3.0f);

            // Reels & Tape (inside window)
            float reel_radius = cassette_w * 0.075f;
            float reel_inner_radius = reel_radius * 0.33f;
            ImVec2 r_left = ImVec2(w0.x + win_w * 0.25f, w0.y + win_h * 0.5f);
            ImVec2 r_right = ImVec2(w0.x + win_w * 0.75f, w0.y + win_h * 0.5f);
            if (mode != CassetteMode::Stop) rotation += ImGui::GetIO().DeltaTime * 3.0f;

            auto draw_reel = [&](ImVec2 center, float rot) {
                constexpr float reelMulti = 2.0f * 3.14159f / 6.0f;
                draw_list->AddCircleFilled(center, reel_radius, IM_COL32(60, 60, 60, 255), 32);
                draw_list->AddCircleFilled(center, reel_inner_radius, IM_COL32(20, 20, 20, 255), 16);
                for (int i = 0; i < 6; i++) {
                    float angle = rot + (float)i * reelMulti;
                    ImVec2 tp1 = ImVec2(center.x + cosf(angle) * reel_inner_radius, center.y + sinf(angle) * reel_inner_radius);
                    ImVec2 tp2 = ImVec2(center.x + cosf(angle) * (reel_inner_radius + reel_radius * 0.22f), center.y + sinf(angle) * (reel_inner_radius + reel_radius * 0.22f));
                    draw_list->AddLine(tp1, tp2, IM_COL32(200, 200, 200, 255), cassette_w * 0.005f + 1.0f);
                }
            };
            float speed_factor_left = 1.0f / (1.5f - progress);
            draw_reel(r_left, rotation * speed_factor_left);
            draw_reel(r_right, rotation);
            draw_list->AddLine(ImVec2(w0.x, w1.y - win_h * 0.1f), ImVec2(w1.x, w1.y - win_h * 0.1f), IM_COL32(80, 50, 30, 255), cassette_w * 0.006f + 1.0f);

            // --- 6. Gloss / Reflection Effect ---
            ImVec2 g0 = ImVec2(w0.x + win_w * 0.1f, w0.y);
            ImVec2 g1 = ImVec2(w0.x + win_w * 0.4f, w0.y);
            ImVec2 g2 = ImVec2(w0.x + win_w * 0.2f, w1.y);
            ImVec2 g3 = ImVec2(w0.x + win_w * 0.05f, w1.y);
            draw_list->AddQuadFilled(g0, g1, g2, g3, IM_COL32(255, 255, 255, 30));

            draw_list->AddRect(w0, w1, IM_COL32(150, 150, 150, 255), 3.0f, 0, 1.5f);

            // --- 7. Screws ---
            float sc_d = cassette_w * 0.025f;
            float sc_r = cassette_w * 0.006f + 1.0f;
            ImU32 sc_c = IM_COL32(100, 100, 100, 255);
            draw_list->AddCircleFilled(ImVec2(p0.x + sc_d, p0.y + sc_d), sc_r, sc_c);
            draw_list->AddCircleFilled(ImVec2(p1.x - sc_d, p0.y + sc_d), sc_r, sc_c);
            draw_list->AddCircleFilled(ImVec2(p0.x + sc_d, p1.y - sc_d), sc_r, sc_c);
            draw_list->AddCircleFilled(ImVec2(p1.x - sc_d, p1.y - sc_d), sc_r, sc_c);


            ImGui::EndChild();
        }
        //--------------------------------------------------------------------------

    };
}; //namespace
