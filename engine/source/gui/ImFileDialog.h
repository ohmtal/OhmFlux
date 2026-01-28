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
#include <core/fluxGlobals.h>
#include <imgui_internal.h>

namespace fs = std::filesystem;

struct ImFileDialog {
    std::string currentPath = fs::current_path().string();
    char pathInput[512];
    char fileInput[256] = "";
    std::string selectedFile = "";
    std::string selectedExt = "";
    std::string mExt = "";
    std::string mUserData = "";

    std::vector<fs::directory_entry> mEntries;
    bool mDirty = true;

    std::string mLabel = "File Browser";
    bool mSaveMode = false;
    int mSelectedFilterIdx = 0;
    std::vector<std::string> mDefaultFilters = {  };
    std::vector<std::string> mFilters = { };
    std::string mSaveExt = "";
    bool mCancelPressed = false;
    bool mInitDone = false;
    //--------------------------------------------------------------------------
    void setFileName(std::string filename)
    {
        strncpy(fileInput, filename.c_str(), sizeof(fileInput));
    }
    //--------------------------------------------------------------------------
    void init(std::string path, std::vector<std::string> filters)
    {
        if (mInitDone)
            return ;
        currentPath = path;
        mDefaultFilters = filters;
        mFilters = filters;
        mInitDone = true;
    }
    //--------------------------------------------------------------------------
    void reset() {
        mLabel = "File Browser";
        mSaveMode = false;
        mFilters = mDefaultFilters;
        mSaveExt = "";
        mCancelPressed = false;
        mDirty = true;
        setFileName("");
        mUserData = "";
    }
    //--------------------------------------------------------------------------
    bool fetchFiles()
    {
        if (currentPath.empty() || !fs::exists(currentPath) || !fs::is_directory(currentPath)) {
            Log("FileBrowser: Invalid or inaccessible path: %s", currentPath.c_str());
            return false;
        }
        std::vector<fs::directory_entry> localEntries;

        try {
            auto options = fs::directory_options::skip_permission_denied;

            for (const auto& entry : fs::directory_iterator(currentPath, options)) {
                const auto& path = entry.path();

                // Handle directory logic
                if (entry.is_directory()) {
                    localEntries.push_back(entry);
                    continue;
                }
                std::string ext = path.extension().string();
                std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

                if (mFilters.empty() || std::find(mFilters.begin(), mFilters.end(), ext) != mFilters.end()) {
                    localEntries.push_back(entry);
                }
            }
            std::sort(localEntries.begin(), localEntries.end(), [](const fs::directory_entry& a, const fs::directory_entry& b) {
                bool a_is_dir = a.is_directory();
                bool b_is_dir = b.is_directory();

                if (a_is_dir != b_is_dir) {
                    return a_is_dir; // Directories first
                }
                std::string aName = a.path().filename().string();
                std::string bName = b.path().filename().string();
                return std::lexicographical_compare(
                    aName.begin(), aName.end(),
                                                    bName.begin(), bName.end(),
                                                    [](char c1, char c2) { return tolower(c1) < tolower(c2); }
                );
            });
            mEntries.swap(localEntries);
        } catch (const fs::filesystem_error& e) {
            Log("FileBrowser OS Error: %s", e.what());
            return false;
        } catch (const std::exception& e) {
            Log("FileBrowser Generic Error: %s", e.what());
            return false;
        } catch (...) {
            Log("FileBrowser Critical: Unknown permission or memory fault.");
            return false;
        }

        return true;
    }
    //--------------------------------------------------------------------------
    void DrawHeader()
    {
        //--------- Reload
        if (ImGui::Button("R")) {
            mDirty = true;
        }
        ImGui::SetItemTooltip("Reload");

        // -------- Quick path
        ImGui::SameLine();
        if (ImGui::Button("Quick Paths...")) {
            ImGui::OpenPopup("PathPickerPopup");
        }
        ImGui::SetItemTooltip("Select a predefined directory");

        if (ImGui::BeginPopup("PathPickerPopup")) {
            auto selectPath = [&](const std::string& newPath) {
                if (!newPath.empty()) {
                    currentPath = newPath;
                    mDirty = true;
                    ImGui::CloseCurrentPopup();
                }
            };
            if (ImGui::Selectable("App Directory"))  selectPath(getGamePath());
            if (ImGui::Selectable("Home"))           selectPath(getHomePath());
            if (ImGui::Selectable("Desktop"))        selectPath(getDesktopPath());
            if (ImGui::Selectable("Documents"))      selectPath(getDocumentsPath());
            if (ImGui::Selectable("Downloads"))      selectPath(getDownloadPath());
            if (ImGui::Selectable("Music"))          selectPath(getMusicPath());
            if (ImGui::Selectable("Pictures"))       selectPath(getPicturesPath());
            if (ImGui::Selectable("Videos"))         selectPath(getVideosPath());

            ImGui::EndPopup();
        }
        // <<<<<<<<< Quick path

        //  Filter select >>>>>>>>>
        ImGui::SameLine();
        std::string preview = (mSelectedFilterIdx == -1) ? "All" : "";
        if (mSelectedFilterIdx != -1) {
            for (const auto& filterStr : mFilters) {
                preview += "*";   // Add the literal
                preview += filterStr;   // Add the extension string
                preview += " ";   // Add the space
            }
        }

        ImGui::PushItemWidth(80.0f);
        if (ImGui::BeginCombo("##FilterCombo", preview.c_str())) {

            // Option: All Files
            bool isAllSelected = (mSelectedFilterIdx == -1);
            if (ImGui::Selectable("All ", isAllSelected)) {
                mSelectedFilterIdx = -1;
                mFilters.clear(); // Empty filters = show everything
                mFilters = mDefaultFilters;
                mDirty = true;
            }

            for (int i = 0; i < (int)mDefaultFilters.size(); i++) {
                bool isSelected = (mSelectedFilterIdx == i);

                // Corrected Logic: The label is simply the current filter string, prefixed by '*'
                std::string label = "*";
                label += mDefaultFilters[i];

                if (ImGui::Selectable(label.c_str(), isSelected)) {
                    mSelectedFilterIdx = i;
                    mFilters.clear();
                    mFilters.push_back(mDefaultFilters[i]); // This assignment is now correct C++
                    mDirty = true;
                }
            }
            ImGui::EndCombo();
        }

        ImGui::PopItemWidth();
        // <<<<<<<<<<<<< Filter select



        if (ImGui::ArrowButton("##Up", ImGuiDir_Up)) {
            currentPath = fs::path(currentPath).parent_path().string();
            mDirty = true;
        }
        ImGui::SameLine();
        strncpy(pathInput, currentPath.c_str(), sizeof(pathInput));
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

    }
    //--------------------------------------------------------------------------
    bool Draw() {
        bool result = false;
        // -------- Fetch Files and sort ---------------
        if (mDirty)
        {
            mDirty = false;
            if (!fetchFiles())
                return false;

        }
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);

        std::string mWindowIdStr = mLabel+"##FileBrowser";
        if (ImGui::Begin(mWindowIdStr.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {

            DrawHeader();
            // ------- Add to gui -------------------



            ImVec2 listSize = ImVec2(-FLT_MIN, -FLT_MIN - 25);

            // if ( mSaveMode ) listSize.y = 200.f;
            ImGuiWindow* window = ImGui::GetCurrentWindow();
            if ( window && !window->DockIsActive)
                listSize.y = 200.f;


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
                            if (ImGui::IsMouseDoubleClicked(0)) {
                                if (strlen(fileInput) > 0) {
                                    selectedFile = (fs::path(currentPath) / fileInput).string();
                                    selectedExt  = ext;
                                    result = true; // This triggers the 'Save/Open' action
                                }
                            } else {
                                selectedFile = name;
                                strncpy(fileInput, name.c_str(), sizeof(fileInput));
                            }
                        }

                    }
                } //for
            }
            ImGui::EndChild();

            ImGui::BeginGroup();
            ImGui::SetNextItemWidth(150);
            ImGui::InputText("##File", fileInput, sizeof(fileInput));

            ImGui::SameLine();
            if (ImGui::Button(mSaveMode ? "Save" : "Open")) {
                if (strlen(fileInput) > 0) {
                    selectedFile = (fs::path(currentPath) / fileInput).string();
                    selectedExt = fs::path(selectedFile).extension().string();
                    result = true;
                }
            }
             if (!window->DockIsActive)
            {
                ImGui::SameLine();
                if (ImGui::Button("Cancel")) {
                    // ImGui::CloseCurrentPopup(); // Or handle state
                    mCancelPressed = true;
                    result = true;
                }
            }
            ImGui::EndGroup();

        }
        ImGui::PopStyleVar();
        ImGui::End();
        return result;
    } //Draw
    //--------------------------------------------------------------------------
};
