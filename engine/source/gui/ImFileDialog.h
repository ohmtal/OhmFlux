//-----------------------------------------------------------------------------
// Copyright (c) 2026 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Usage:
// 1.) define a global variable like this
//     inline ImFileDialog g_FileDialog;
// 2.) init Example:
//     g_FileDialog.init( getGamePath(), {  ".sfx", ".fmi", ".fms", ".wav", ".ogg" });
// 3.) In IMGui Render Loop:
//
// if (g_FileDialog.Draw()) {
//     if (g_FileDialog.mSaveMode)
//     {
//         if (!g_FileDialog.mCancelPressed)
//         {
//             if (g_FileDialog.mSaveExt == ".fms")
//             {
//                 if (g_FileDialog.selectedExt == "")
//                     g_FileDialog.selectedFile.append(g_FileDialog.mSaveExt);
//                 mFMComposer->saveSong(g_FileDialog.selectedFile);
//             }
//         };
//         g_FileDialog.reset();
//     } else {
//         if ( g_FileDialog.selectedExt == ".fms" )
//             mFMComposer->loadSong(g_FileDialog.selectedFile);
//     }
//
// 4.) Setup for save like this:
// void callSaveSong() {
//     g_FileDialog.setFileName(mSongName);
//     g_FileDialog.mSaveMode = true;
//     g_FileDialog.mSaveExt = ".fms";
//     g_FileDialog.mLabel = "Save Song (.fms)";
//
// }


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
    std::vector<std::string> mDefaultFilters = {  };
    std::vector<std::string> mFilters = { };
    std::string mSaveExt = "";
    bool mCancelPressed = false;
    bool mInitDone = false;

    void setFileName(std::string filename)
    {
        strncpy(fileInput, filename.c_str(), sizeof(fileInput));
    }

    void init(std::string path, std::vector<std::string> filters)
    {
        if (mInitDone)
            return ;
        currentPath = path;
        mDefaultFilters = filters;
        mFilters = filters;
        mInitDone = true;
    }


    void reset() {
        mLabel = "File Browser";
        mSaveMode = false;
        mFilters = mDefaultFilters;
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
