//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#pragma once

#include <vector>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <atomic>
#include <fstream>
#include <cstdlib>

#ifdef FLUX_ENGINE
#include <imgui.h>
#include <imgui_internal.h>
#include <gui/ImFlux.h>
#endif

#include "../DSP_Math.h"

namespace DSP {
namespace MonoProcessors {

    class BasicVisualizer {
        static constexpr int scope_size = 512;
        float scope_buffer[scope_size] = {0};
        std::atomic<int> scope_pos{0};
    public:
        float mScopeZoom = 3.f; //for ui

        void process(float input) {
            int pos = scope_pos.load();
            scope_buffer[pos] = input;
            scope_pos = (pos + 1) % scope_size;
        }

#ifdef FLUX_ENGINE

        void DrawOsci () {
            // input Osci::
            ImGui::BeginChild("InputOsci", ImVec2(0.f,140.f));
            ImFlux::FaderHWithText("Zoom", &mScopeZoom, 0.1f, 10.0f, "%.2f");

            ImGui::PlotLines("##Scope", scope_buffer,
                             scope_size,
                             scope_pos.load(),
                             NULL,
                             -1.0f/mScopeZoom,
                             1.0f/mScopeZoom,
                             ImVec2(-1, -1));
            ImGui::EndChild();
        }
#endif

    };




};};
