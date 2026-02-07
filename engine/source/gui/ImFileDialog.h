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
#include "ImFlux.h"
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
    // //--------------------------------------------------------------------------
    // void setFileName(std::string filename)
    // {
    //     strncpy(fileInput, filename.c_str(), sizeof(fileInput));
    // }
    void setFileName(std::string filename)
    {
        fs::path p(filename);

        // Check if the provided filename is an absolute path or contains directory separators
        if (p.is_absolute() || p.has_parent_path()) {
            if (fs::exists(p.parent_path())) {
                // Update the browser's current directory
                currentPath = p.parent_path().string();

                // Extract only the filename for the input field
                std::string nameOnly = p.filename().string();
                strncpy(fileInput, nameOnly.c_str(), sizeof(fileInput) - 1);
                fileInput[sizeof(fileInput) - 1] = '\0'; // Ensure null-termination

                // Update pathInput if you use it as a text field for the path
                strncpy(pathInput, currentPath.c_str(), sizeof(pathInput) - 1);
                pathInput[sizeof(pathInput) - 1] = '\0';

                // Trigger a refresh of the file list
                mDirty = true;
                return;
            }
        }

        // Fallback: If it's just a filename or path doesn't exist, just set the input
        strncpy(fileInput, filename.c_str(), sizeof(fileInput) - 1);
        fileInput[sizeof(fileInput) - 1] = '\0';
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
        reset();
    }
    //--------------------------------------------------------------------------
    void reset() {
        mLabel = "File Browser";
        mSaveMode = false;
        mFilters = mDefaultFilters;
        mSelectedFilterIdx = -1; // NOTE: ALL - good or bad to set it here
        mSaveExt = "";
        mCancelPressed = false;
        mDirty = true;
        setFileName("");
        mUserData = "";
    }
    //--------------------------------------------------------------------------
    bool fetchFiles()
    {
        if (currentPath.empty() || !fs::exists(currentPath)) return false;

        std::vector<fs::directory_entry> localEntries;
        try {
            for (const auto& entry : fs::directory_iterator(currentPath, fs::directory_options::skip_permission_denied)) {
                // Apply your extension filters here
                if (!entry.is_directory()) {
                    std::string ext = entry.path().extension().string();
                    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
                    if (!mFilters.empty() && std::find(mFilters.begin(), mFilters.end(), ext) == mFilters.end()) {
                        continue;
                    }
                }
                localEntries.push_back(entry);
            }
            mEntries.swap(localEntries);
        } catch (...) { return false; }

        return true;
    }


    //--------------------------------------------------------------------------
    void cdDotDot() {
        fs::path p(currentPath);

        // Remove trailing slash if it exists (except for root like "C:\" or "/")
        if (p.has_relative_path()) {
            if (currentPath.back() == '/' || currentPath.back() == '\\') {
                currentPath.pop_back();
                p = fs::path(currentPath);
            }
        }

        currentPath = p.parent_path().string();
        mDirty = true;
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
            // dLog("mSelectedFilterIdx is %d", mSelectedFilterIdx);
            for (const auto& filterStr : mFilters) {
                preview += "*";   // Add the literal
                preview += filterStr;   // Add the extension string
                preview += " ";   // Add the space
            }
        }

        ImGui::PushItemWidth(100.0f);
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
            cdDotDot();
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
    // this can be overwritten for your custom icons

    virtual void DrawIcon(fs::directory_entry entry)
    {
        const ImVec2 size = { 14.f, 14.f }; // Slightly larger for better visibility
        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 pos = ImGui::GetCursorScreenPos();

        ImU32 col = ImGui::GetColorU32(ImGuiCol_Header);
        ImU32 colDarker = ImGui::GetColorU32(ImGuiCol_Tab);

        if (entry.is_directory()) {
            float tabHeight = size.y * 0.25f;
            float tabWidth = size.x * 0.4f;
            dl->AddRectFilled(
                pos,
                { pos.x + tabWidth, pos.y + tabHeight + 1.0f },
                colDarker, 2.0f, ImDrawFlags_RoundCornersTop
            );

            dl->AddRectFilled(
                { pos.x, pos.y + tabHeight },
                { pos.x + size.x, pos.y + size.y },
                col, 2.0f
            );

            ImGui::Dummy(size);
            ImGui::SameLine();
        }
        else {
            // // Simple file icon (empty rectangle or vertical line)
            // dl->AddRect(pos, { pos.x + size.x, pos.y + size.y }, ImGui::GetColorU32(ImGuiCol_TextDisabled), 1.0f);
            // ImGui::Dummy(size);
            // ImGui::SameLine();
        }
    }

    //--------------------------------------------------------------------------
    bool Draw() {
        bool result = false;

        std::string mWindowIdStr = mLabel+"##FileBrowser";

        ImGui::SetNextWindowSize(ImVec2(520, 600), ImGuiCond_FirstUseEver);

        // if (ImGui::Begin(mWindowIdStr.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        if (ImGui::Begin(mWindowIdStr.c_str()))
        {

            DrawHeader();
            // ------- Add to gui -------------------
            ImVec2 listSize = ImVec2(-FLT_MIN, -FLT_MIN - 25);

            // if ( mSaveMode ) listSize.y = 200.f;
            ImGuiWindow* window = ImGui::GetCurrentWindow();
            // if ( window && !window->DockIsActive)
            //     listSize.y = 200.f;


            if (ImGui::BeginChild("FileList", listSize)) {
                // if (ImGui::Selectable("..", false, ImGuiSelectableFlags_AllowDoubleClick)) {
                //     if (ImGui::IsMouseDoubleClicked(0)) {
                //         currentPath = fs::path(currentPath).parent_path().string();
                //         mDirty = true;
                //     }
                // }
                //------------------> NEW TABLE < -----------------------

                // Table flags for professional behavior
                static ImGuiTableFlags table_flags =
                ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable |
                ImGuiTableFlags_Sortable  | ImGuiTableFlags_ScrollY     | ImGuiTableFlags_RowBg    |
                ImGuiTableFlags_BordersOuter;

                if (ImGui::BeginTable("FileBrowserTable", 3, table_flags, ImVec2(0, 0)))
                {
                    // Column definitions
                    ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_DefaultSort | ImGuiTableColumnFlags_WidthStretch);
                    ImGui::TableSetupColumn("Size", ImGuiTableColumnFlags_WidthFixed, 90.0f);
                    ImGui::TableSetupColumn("Date Modified", ImGuiTableColumnFlags_WidthFixed, 90.0f);
                    ImGui::TableHeadersRow();

                    // --- SORTING LOGIC ---

                    if (ImGuiTableSortSpecs* sort_specs = ImGui::TableGetSortSpecs())
                    {
                        // FORCE update if user clicked header OR if directory changed (mDirty)
                        if (sort_specs->SpecsDirty || mDirty)
                        {
                            if (mDirty) fetchFiles(); // Reload files before sorting

                            std::sort(mEntries.begin(), mEntries.end(), [&](const fs::directory_entry& a, const fs::directory_entry& b) {
                                if (a.is_directory() != b.is_directory()) return a.is_directory(); // Dirs always first

                                const ImGuiTableColumnSortSpecs& spec = sort_specs->Specs[0];
                                bool res = false;
                                switch (spec.ColumnIndex) {
                                    case 0: res = a.path().filename().string() < b.path().filename().string(); break;
                                    case 1: res = (a.is_regular_file() ? fs::file_size(a) : 0) < (b.is_regular_file() ? fs::file_size(b) : 0); break;
                                    case 2: res = fs::last_write_time(a) < fs::last_write_time(b); break;
                                }
                                return (spec.SortDirection == ImGuiSortDirection_Ascending) ? res : !res;
                            });

                            sort_specs->SpecsDirty = false;
                            mDirty = false; // Reset flag after sorting is done
                        }
                    }
                    // --- ROW RENDERING ---
                    for (const auto& entry : mEntries) {
                        ImGui::TableNextRow();

                        const auto& path = entry.path();
                        std::string name = path.filename().string();
                        bool isDir = entry.is_directory();

                        // Column 0: Name and Selection Logic
                        ImGui::TableSetColumnIndex(0);
                        bool isSelected = (selectedFile == name);

                        // Use SpanAllColumns so the selection highlight covers the whole row
                        ImGuiSelectableFlags sel_flags = ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick;
                        // std::string display_name = isDir ? "[D] " + name : name;


                        DrawIcon(entry);

                        if (ImGui::Selectable(name.c_str(), isSelected, sel_flags)) {
                            if (ImGui::IsMouseDoubleClicked(0)) {
                                if (isDir) {
                                    currentPath = path.string();
                                    mDirty = true; // Trigger directory change
                                } else {
                                    if (strlen(fileInput) > 0) {

                                        std::string ext = path.extension().string();
                                        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
                                        selectedFile = (fs::path(currentPath) / fileInput).string();
                                        selectedExt  = ext;
                                        result = true;
                                    }
                                }
                            } else {
                                if (!isDir)
                                {
                                    dLog("Selected File: %s", selectedFile.c_str());
                                    selectedFile = name;
                                    strncpy(fileInput, name.c_str(), sizeof(fileInput));
                                }
                            }
                        }

                        // Column 1: File Size
                        ImGui::TableSetColumnIndex(1);
                        if (isDir) {
                            ImGui::TextDisabled("--");
                        } else {
                            std::error_code ec;
                            uintmax_t size = fs::file_size(entry, ec);
                            if (!ec)
                            {
                                if (size < 1024) ImGui::Text("%llu B", static_cast<unsigned long long>(size));
                                else if (size < 1024 * 1024) ImGui::Text("%.1f KB", size / 1024.0f);
                                else ImGui::Text("%.1f MB", size / (1024.0f * 1024.0f));
                            } else {
                                dLog("[warn] filesize: File does not exists any more... ");
                                mDirty = true;
                            }
                        }

                        // Column 2: Date Modified
                        ImGui::TableSetColumnIndex(2);

                        if (!mDirty)
                        {
                            std::error_code ec;
                            auto ftime = fs::last_write_time(entry, ec);
                            if (!ec)
                            {
                                auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                                    ftime - fs::file_time_type::clock::now() + std::chrono::system_clock::now()
                                );
                                std::time_t ctime = std::chrono::system_clock::to_time_t(sctp);

                                // Format time string (e.g., YYYY-MM-DD HH:MM)
                                char time_str[32];
                                // std::strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M", std::localtime(&ctime));
                                std::strftime(time_str, sizeof(time_str), "%y-%m-%d %H:%M", std::localtime(&ctime));
                                ImGui::TextDisabled("%s", time_str);
                            } else {
                                dLog("[warn] file datetime: File does not exists any more...");
                                mDirty = true;
                            }
                        }
                    }
                    ImGui::EndTable();
                }
            }
            ImGui::EndChild();

            ImGui::BeginGroup();

            // Calculate required width for buttons
            float button_save_width = ImGui::CalcTextSize(mSaveMode ? "Save" : "Open").x + ImGui::GetStyle().FramePadding.x * 2.0f;
            float button_cancel_width = ImGui::CalcTextSize("Cancel").x + ImGui::GetStyle().FramePadding.x * 2.0f;
            float spacing = ImGui::GetStyle().ItemSpacing.x;

            // Calculate remaining width for InputText
            // GetContentRegionAvail().x is the full width of the window/group
            float input_width = ImGui::GetContentRegionAvail().x - button_save_width - spacing;
            if (!window->DockIsActive) {
                input_width -= (button_cancel_width + spacing);
            }

            // Draw InputText with dynamic width
            ImGui::SetNextItemWidth(input_width);
            ImGui::InputText("##File", fileInput, sizeof(fileInput));

            ImGui::SameLine();
            if (ImGui::Button(mSaveMode ? "Save" : "Open", ImVec2(button_save_width, 0))) {
                if (strlen(fileInput) > 0) {
                    selectedFile = (fs::path(currentPath) / fileInput).string();
                    selectedExt = fs::path(selectedFile).extension().string();


                    if (mSaveMode) {
                      if (selectedExt == "" ) {
                          dLog("Adding extension: %s", mSaveExt.c_str());
                          selectedFile += mSaveExt;
                          selectedExt = mSaveExt;
                      }
                    }

                    result = true;
                }
            }

            if (!window->DockIsActive) {
                ImGui::SameLine();
                if (ImGui::Button("Cancel", ImVec2(button_cancel_width, 0))) {
                    mCancelPressed = true;
                    result = true;
                }
            }
            ImGui::EndGroup();

        }
        ImGui::End();
        return result;
    } //Draw
    //--------------------------------------------------------------------------
};
