//-----------------------------------------------------------------------------
// Copyright (c) 2026 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Digital Sound Processing : VisualAnalyzer
//-----------------------------------------------------------------------------
#pragma once

#include <vector>
#include <cstdint>
#include <algorithm>
#include <cstring>
#include <atomic>
#include <mutex>

#ifdef FLUX_ENGINE
#include <imgui.h>
#include <imgui_internal.h>
#include <gui/ImFlux.h>
#endif


#include "DSP_Effect.h"

namespace DSP {
    class VisualAnalyzer : public Effect {
    private:
        std::vector<float> mMirrorBuffer;
        std::mutex mDataMutex;

    public:
        VisualAnalyzer( bool switchOn = false) : Effect(switchOn) {
            mMirrorBuffer.resize(2048, 0.0f); // Size for the oscilloscope display
        }

        // The process method just copies data, it doesn't change it
        void process(float* buffer, int numSamples) override {
            if (!mEnabled) return;

            std::lock_guard<std::mutex> lock(mDataMutex);
            // We store the last 'n' samples for the GUI to pick up
            int toCopy = std::min(numSamples, (int)mMirrorBuffer.size());

            // Shift old data and add new (or use a ring buffer)
            std::move(mMirrorBuffer.begin() + toCopy, mMirrorBuffer.end(), mMirrorBuffer.begin());
            std::copy(buffer + (numSamples - toCopy), buffer + numSamples, mMirrorBuffer.end() - toCopy);
        }

        // GUI calls this to get the data
        void getLatestSamples(std::vector<float>& outBuffer) {
            std::lock_guard<std::mutex> lock(mDataMutex);
            outBuffer = mMirrorBuffer;
        }

        DSP::EffectType getType() const override { return DSP::EffectType::VisualAnalyzer; }
    };


#ifdef FLUX_ENGINE
    inline void DrawVisualAnalyzerOszi(DSP::VisualAnalyzer* analyzer, ImVec2 size) {
        if (!analyzer || !analyzer->isEnabled()) return;

        static std::vector<float> samples;
        analyzer->getLatestSamples(samples); // Get interleaved L/R data

        if (samples.empty()) return;

        // Use ImGui Child Window for a contained drawing area
        if (ImGui::BeginChild("OsziArea", size, true, ImGuiWindowFlags_NoScrollbar)) {
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            ImVec2 canvas_pos = ImGui::GetCursorScreenPos();
            ImVec2 canvas_size = ImGui::GetContentRegionAvail();

            // 1. Draw Grid (Retro Style)
            draw_list->AddRectFilled(canvas_pos, ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y), IM_COL32(10, 15, 10, 255));

            float mid_y = canvas_pos.y + canvas_size.y * 0.5f;
            draw_list->AddLine(ImVec2(canvas_pos.x, mid_y), ImVec2(canvas_pos.x + canvas_size.x, mid_y), IM_COL32(50, 100, 50, 150));

            // 2. Plot Channels
            // We iterate with i+=2 because data is [L, R, L, R...]
            int num_frames = (int)samples.size() / 2;
            float x_step = canvas_size.x / (float)num_frames;

            for (int i = 0; i < num_frames - 1; ++i) {
                // Left Channel (Cyan)
                float l_y1 = mid_y - (samples[i * 2] * canvas_size.y * 0.45f);
                float l_y2 = mid_y - (samples[(i + 1) * 2] * canvas_size.y * 0.45f);

                // Right Channel (Yellow or Red)
                float r_y1 = mid_y - (samples[i * 2 + 1] * canvas_size.y * 0.45f);
                float r_y2 = mid_y - (samples[(i + 1) * 2 + 1] * canvas_size.y * 0.45f);

                float x1 = canvas_pos.x + i * x_step;
                float x2 = canvas_pos.x + (i + 1) * x_step;

                draw_list->AddLine(ImVec2(x1, l_y1), ImVec2(x2, l_y2), IM_COL32(0, 255, 255, 200), 1.5f);
                draw_list->AddLine(ImVec2(x1, r_y1), ImVec2(x2, r_y2), IM_COL32(255, 255, 0, 150), 1.5f);
            }
        }
        ImGui::EndChild();
    }
#endif


} //namespace
