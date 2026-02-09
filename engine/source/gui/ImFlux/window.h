//-----------------------------------------------------------------------------
// Copyright (c) 2026 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#pragma once

#include <imgui.h>
#include <imgui_internal.h>
#include <cmath>
#include <algorithm>
#include <vector>

#include "helper.h"

namespace ImFlux {

    //------------------------ Window handling :D
    inline bool getWindows(std::vector<ImGuiWindow*> & windowList) {
        ImGuiContext& g = *GImGui;
        const char* prefix = "WindowOverViewport_";

        for (int i = 0; i < g.Windows.Size; i++) {
            ImGuiWindow* window = g.Windows[i];
            if (window->WasActive && !window->Hidden && !(window->Flags & ImGuiWindowFlags_ChildWindow)
                && !(window->Flags & ImGuiWindowFlags_Tooltip)) {
                if (strstr(window->Name, "MainDockSpace") != nullptr) continue;
                if (strstr(window->Name, "Window") != nullptr) continue;
                if (strncmp(window->Name, prefix, strlen(prefix)) == 0) continue;

                if (!window->Name || window->Name[0] == '\0') continue; //empty names
                if (window->Name[0] == '#' && window->Name[1] == '#') continue; //names with id only
                windowList.push_back(window);
                }
        }
        return !windowList.empty();
    }

    inline void CascadeWindows() {
        std::vector<ImGuiWindow*> windowList;
        if (!getWindows(windowList)) return;
        ImVec2 displayPos = ImGui::GetMainViewport()->Pos;
        ImVec2 displaySize = ImGui::GetMainViewport()->Size;
        float menuBarHeight = 20.0f;
        displayPos.y += menuBarHeight;
        displaySize.y -= menuBarHeight;
        float offsetStep = 30.0f; // Versatz pro Fenster in Pixeln
        ImVec2 baseSize = ImVec2(displaySize.x * 0.7f, displaySize.y * 0.7f); // 70% der Fläche

        for (int i = 0; i < (int)windowList.size(); i++) {
            float currentOffset = i * offsetStep;
            if (displayPos.x + currentOffset + baseSize.x > displayPos.x + displaySize.x ||
                displayPos.y + currentOffset + baseSize.y > displayPos.y + displaySize.y) {
                currentOffset = 0; // Optional: Hier könnte man i % max_fits nutzen
                }
                ImVec2 pos = ImVec2(displayPos.x + currentOffset, displayPos.y + currentOffset);

            ImGui::SetWindowPos(windowList[i], pos, ImGuiCond_Always);
            ImGui::SetWindowSize(windowList[i], baseSize, ImGuiCond_Always);
        }
        ImGui::FocusWindow(windowList[(int)windowList.size()-1]);
    }

    inline void TileWindows() {
        std::vector<ImGuiWindow*> windowList;
        if (!getWindows(windowList)) return;
        ImVec2 displayPos = ImGui::GetMainViewport()->Pos;
        ImVec2 displaySize = ImGui::GetMainViewport()->Size;
        displayPos.y +=20;  displaySize.y -= 20; //space for the menubar
        int count = (int)windowList.size();
        int cols = (int)std::ceil(std::sqrt(count));
        int rows = (int)std::ceil((float)count / cols);
        ImVec2 winSize = ImVec2(displaySize.x / cols, displaySize.y / rows);
        for (int i = 0; i < count; i++) {
            ImVec2 pos = ImVec2(displayPos.x + (i % cols) * winSize.x,
                                displayPos.y + (i / cols) * winSize.y);
            ImGui::SetWindowPos(windowList[i], pos, ImGuiCond_Always);
            ImGui::SetWindowSize(windowList[i], winSize, ImGuiCond_Always);
        }
    }

    inline void drawWindowMenu() {
        if (ImGui::BeginMenu("Window")) {
            if (ImGui::MenuItem("Tile All")) TileWindows();
            if (ImGui::MenuItem("Cascade All")) CascadeWindows();
            if (ImGui::BeginMenu("Focus floating Window...")) {
                std::vector<ImGuiWindow*> windows;
                if (getWindows(windows)) {
                    for (ImGuiWindow* win : windows)
                        if (ImGui::MenuItem(win->Name)) ImGui::FocusWindow(win);
                } else {
                    ImGui::MenuItem("(No floating windows found)", NULL, false, false);
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }
    }
};
