//-----------------------------------------------------------------------------
// Copyright (c) 2026 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
//FIXME: *** W i P ***
// * add filter
//-----------------------------------------------------------------------------
// Example:
//     static ImFileDialog myDialog;
//     if (myDialog.Draw("File Browser", true)) {
//         printf("User chose: %s\n", myDialog.selectedFile.c_str());
//     }
//-----------------------------------------------------------------------------
#pragma once
#include "imgui.h"
#include <filesystem>
#include <vector>
#include <string>
#include <algorithm>


namespace fs = std::filesystem;

struct ImFileDialog {
    std::string currentPath = fs::current_path().string();
    char pathInput[512];
    char fileInput[256] = "";
    std::string selectedFile = "";
    std::string selectedExt = "";

    bool Draw(const char* label, bool isSaveMode, std::vector<std::string> filters) {
        bool result = false;
        if (ImGui::Begin(label, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {

            // 1. Path Input at the top
            strncpy(pathInput, currentPath.c_str(), sizeof(pathInput));
            ImGui::SetNextItemWidth(-FLT_MIN);
            if (ImGui::InputTextWithHint("##pathInput","Path", pathInput, sizeof(pathInput), ImGuiInputTextFlags_EnterReturnsTrue)) {
                if (fs::exists(pathInput)) currentPath = pathInput;
            }

            // 2. File List
            ImVec2 listSize = ImVec2(500, 300);
            if (ImGui::BeginChild("FileList", listSize, true)) {
                try {
                    if (ImGui::Selectable("..", false, ImGuiSelectableFlags_AllowDoubleClick)) {
                        if (ImGui::IsMouseDoubleClicked(0)) {
                            currentPath = fs::path(currentPath).parent_path().string();
                        }
                    }

                    for (const auto& entry : fs::directory_iterator(currentPath)) {
                        const auto& path = entry.path();
                        std::string name = path.filename().string();

                        if (entry.is_directory()) {
                            // Double-click on directory to enter it
                            if (ImGui::Selectable((name + "/").c_str(), false, ImGuiSelectableFlags_AllowDoubleClick)) {
                                if (ImGui::IsMouseDoubleClicked(0)) {
                                    currentPath = path.string();
                                }
                            }
                        }
                        else {
                            // Check extension filter (as done before)
                            std::string ext = path.extension().string();
                            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

                            if (filters.empty() ||  std::find(filters.begin(), filters.end(), ext) != filters.end()) {

                                // Use AllowDoubleClick flag to ensure the click isn't swallowed
                                bool isSelected = (selectedFile == name);
                                if (ImGui::Selectable(name.c_str(), isSelected, ImGuiSelectableFlags_AllowDoubleClick)) {
                                    selectedFile = name;
                                    strncpy(fileInput, name.c_str(), sizeof(fileInput));
                                }

                                // Check for Double Click on the file
                                if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
                                    if (strlen(fileInput) > 0) {
                                        selectedFile = (fs::path(currentPath) / fileInput).string();
                                        selectedExt  = ext;
                                        result = true; // This triggers the 'Save/Open' action
                                    }
                                }
                            }
                        }
                    }

                } catch (...) {
                    Log("FileBrowser permission error.");
                }

            }
            ImGui::EndChild();

            // 3. Filename Input (Required for Save, handy for Load)
            ImGui::InputText("File", fileInput, sizeof(fileInput));

            // 4. Buttons
            if (ImGui::Button(isSaveMode ? "Save" : "Open")) {
                if (strlen(fileInput) > 0) {
                    selectedFile = (fs::path(currentPath) / fileInput).string();
                    selectedExt = fs::path(selectedFile).extension().string();
                    result = true;
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel")) {
                ImGui::CloseCurrentPopup(); // Or handle state
            }


        }
        ImGui::End();
        return result;
    }
};
