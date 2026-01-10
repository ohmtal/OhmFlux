//-----------------------------------------------------------------------------
// Copyright (c) 2026 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
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
    std::string mExt = "";

    std::vector<fs::directory_entry> mEntries;
    bool mDirty = true;

    std::string mLabel = "File Browser";
    bool mSaveMode = false;
    std::vector<std::string> mFilters = {  ".sfx", ".fmi", ".fms", ".wav", ".ogg" };
    std::string mSaveExt = "";
    bool mCancelPressed = false;

    void setFileName(std::string filename)
    {
        strncpy(fileInput, filename.c_str(), sizeof(fileInput));
    }

    void reset() {
        mLabel = "File Browser";
        mSaveMode = false;
        mFilters = {  ".sfx", ".fmi", ".fms", ".wav", ".ogg" };
        mSaveExt = "";
        mCancelPressed = false;
        mDirty = true;
        setFileName("");
    }

    bool fetchFiles()
    {
        mEntries.clear();
        try {
            for (const auto& entry : fs::directory_iterator(currentPath)) {
                const auto& path = entry.path();

                // Handle directory logic first (usually directories are always shown)
                if (entry.is_directory()) {
                    mEntries.push_back(entry);
                    continue;
                }

                // Apply extension filters to files
                mExt = path.extension().string();
                std::transform(mExt.begin(), mExt.end(), mExt.begin(), ::tolower);

                if (mFilters.empty() || std::find(mFilters.begin(), mFilters.end(), mExt) != mFilters.end()) {
                    mEntries.push_back(entry);
                }
            }

            // Sort: Directories first, then alphabetical by filename
            std::sort(mEntries.begin(), mEntries.end(), [](const fs::directory_entry& a, const fs::directory_entry& b) {
                if (a.is_directory() != b.is_directory()) {
                    return a.is_directory(); // true (1) comes before false (0)
                }
                // Use generic_string() or filename() for comparison to ensure cross-platform consistency
                return a.path().filename().string() < b.path().filename().string();
            });

        } catch (const fs::filesystem_error& e) {
            Log("FileBrowser error: %s", e.what());
            return false;
        } catch (...) {
            Log("FileBrowser unknown permission error.");
            return false;
        }
        return true;
    }


    bool Draw() {
        bool result = false;

        // -------- Fetch Files and sort ---------------

        if (mDirty)
        {
            mDirty = false;
            if (!fetchFiles())
                return false;

        }

        if (ImGui::Begin(mLabel.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {

            // ------- Top: -----------
            strncpy(pathInput, currentPath.c_str(), sizeof(pathInput));

            if (ImGui::Button("R")) {
                mDirty = true;
            }
            ImGui::SameLine();
            if (ImGui::ArrowButton("##Up", ImGuiDir_Up)) {
                currentPath = fs::path(currentPath).parent_path().string();
                mDirty = true;
            }

            ImGui::SameLine();
            ImGui::SetNextItemWidth(-FLT_MIN);
            if (ImGui::InputTextWithHint("##pathInput","Path", pathInput, sizeof(pathInput), ImGuiInputTextFlags_EnterReturnsTrue)) {
                if (fs::exists(pathInput)) {
                      currentPath = pathInput;
                      mDirty = true;
                } else {
                    std::strncpy(pathInput, currentPath.c_str(), sizeof(pathInput) - 1);
                    pathInput[sizeof(pathInput) - 1] = '\0';

                }

            }

            // ------- Add to gui -------------------
            ImVec2 listSize = ImVec2(500, 300);
            if (ImGui::BeginChild("FileList", listSize, true)) {
                if (ImGui::Selectable("..", false, ImGuiSelectableFlags_AllowDoubleClick)) {
                    if (ImGui::IsMouseDoubleClicked(0)) {
                        currentPath = fs::path(currentPath).parent_path().string();
                        mDirty = true;
                    }
                }

                for (const auto& entry : mEntries) {
                    const auto& path = entry.path();
                    std::string name = path.filename().string();

                    if (entry.is_directory()) {
                        // Double-click on directory to enter it
                        if (ImGui::Selectable((name + "/").c_str(), false, ImGuiSelectableFlags_AllowDoubleClick)) {
                            if (ImGui::IsMouseDoubleClicked(0)) {
                                currentPath = path.string();
                                mDirty = true;
                            }
                        }
                    } else {
                        std::string ext = path.extension().string();
                        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

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
                } //for
            }
            ImGui::EndChild();

            // 3. Filename Input (Required for Save, handy for Load)
            ImGui::InputText("File", fileInput, sizeof(fileInput));

            // 4. Buttons
            if (ImGui::Button(mSaveMode ? "Save" : "Open")) {
                if (strlen(fileInput) > 0) {
                    selectedFile = (fs::path(currentPath) / fileInput).string();
                    selectedExt = fs::path(selectedFile).extension().string();
                    result = true;
                }
            }
            if (mSaveMode)
            {
                ImGui::SameLine();
                if (ImGui::Button("Cancel")) {
                    // ImGui::CloseCurrentPopup(); // Or handle state
                    mCancelPressed = true;
                    result = true;
                }
            }



        }
        ImGui::End();
        return result;
    }
};
