//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH) 
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
//     g_FileDialog.mFilters = {".fms"};
//
// }
//

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



class ImFileDialog {

    struct ListCache {
        std::string pathStr;
        std::string name;
        std::string ext;
        bool isDir;
        std::string fileSize;
        std::string fileDateTime;
        uintmax_t rawSize;
        fs::file_time_type rawTime;
    };


    std::string mCurrentPath = fs::current_path().string();
    char pathInput[512];
    char fileInput[256] = "";
    std::string mExt = "";

    std::vector<ListCache>mListCache;

    int mSelectedFilterIdx = 0;
    std::vector<std::string> mDefaultFilters = {  };
    bool mInitDone = false;


public:
    bool mDirty = true;
    std::string mLabel = "File Browser";

    std::string selectedFile = "";
    std::string selectedExt = "";
    std::vector<std::string> mFilters = { };

    bool mCancelPressed = false;
    bool mSaveMode = false;
    std::string mSaveExt = "";
    std::string mUserData = "";
    bool mWasOpen = true;

    std::map<std::string /*caption*/, std::string /*path*/> mCustomQuckPathes;


    //--------------------------------------------------------------------------
    ~ImFileDialog() {
        mListCache.clear();
    }
    //--------------------------------------------------------------------------

    void setFileName(std::string filename)
    {
        fs::path p(filename);

        // Check if the provided filename is an absolute path or contains directory separators
        if (p.is_absolute() || p.has_parent_path()) {
            if (fs::exists(p.parent_path())) {
                // Update the browser's current directory
                mCurrentPath = p.parent_path().string();

                // Extract only the filename for the input field
                std::string nameOnly = p.filename().string();
                strncpy(fileInput, nameOnly.c_str(), sizeof(fileInput) - 1);
                fileInput[sizeof(fileInput) - 1] = '\0'; // Ensure null-termination

                // Update pathInput if you use it as a text field for the path
                strncpy(pathInput, mCurrentPath.c_str(), sizeof(pathInput) - 1);
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
        mCurrentPath = path;
        mDefaultFilters = filters;
        mFilters = filters;
        mInitDone = true;
        reset();
    }
    //--------------------------------------------------------------------------
    void reset(bool resetFilters=true) {
        mLabel = "File Browser";
        mSaveMode = false;
        if (resetFilters) {
            mFilters = mDefaultFilters;
            mSelectedFilterIdx = -1;
        }
        mSaveExt = "";
        mCancelPressed = false;
        mDirty = true;
        setFileName("");
        mUserData = "";
    }
    //--------------------------------------------------------------------------
private:
    bool fetchFiles()
    {
        if (mCurrentPath.empty() || !fs::exists(mCurrentPath)) return false;

        std::vector<fs::directory_entry> localEntries;
        try {
            for (const auto& entry : fs::directory_iterator(mCurrentPath, fs::directory_options::skip_permission_denied)) {
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
        } catch (...) { return false; }


        // 2026-04-30 Build ListCache.
        mListCache.clear();
        for (const auto& entry : localEntries) {
            std::error_code ec;

            const auto& path = entry.path();
            std::string name = path.filename().string();

            ListCache lc;

            lc.pathStr  = path.string();
            lc.name     = path.filename().string();
            lc.ext      = path.extension().string();
            lc.isDir    = entry.is_directory();

            // .... filesize .....
            if (lc.isDir) {
                lc.fileSize = "---";
            } else {
                uintmax_t size = entry.is_regular_file() ? fs::file_size(entry, ec) : 0;
                if (!ec)
                {
                    if (size < 1024) {
                        lc.fileSize = std::format("{} B", size);
                    }
                    else if (size < 1024 * 1024) {
                        lc.fileSize = std::format("{:.1f} KB", size / 1024.0f);
                    }
                    else if (size < 1024LL * 1024 * 1024) {
                        lc.fileSize = std::format("{:.1f} MB", size / (1024.0f * 1024.0f));
                    }
                    else {
                        lc.fileSize = std::format("{:.2f} GB", size / (1024.0f * 1024.0f * 1024.0f));
                    }
                    lc.rawSize = size;
                } else {
                    lc.rawSize = 0;
                }
            }

            // ... file last write time ...
            {

                auto ftime = fs::last_write_time(entry, ec);
                if (!ec)
                {
                    auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                        ftime - fs::file_time_type::clock::now() + std::chrono::system_clock::now()
                    );
                    std::time_t ctime = std::chrono::system_clock::to_time_t(sctp);

                    // Format time string (e.g., YYYY-MM-DD HH:MM)
                    char time_str[32];
                    std::strftime(time_str, sizeof(time_str), "%y-%m-%d %H:%M", std::localtime(&ctime));
                    // ImGui::TextDisabled("%s", time_str);

                    lc.fileDateTime = time_str;

                    lc.rawTime = ftime;

                } else {
                    lc.fileDateTime = "---";
                    lc.rawTime = fs::file_time_type();
                }
            }


            // push ;)
            mListCache.push_back(lc);
        }

        return true;
    }


    //--------------------------------------------------------------------------
    void cdDotDot() {
        fs::path p(mCurrentPath);

        // Remove trailing slash if it exists (except for root like "C:\" or "/")
        if (p.has_relative_path()) {
            if (mCurrentPath.back() == '/' || mCurrentPath.back() == '\\') {
                mCurrentPath.pop_back();
                p = fs::path(mCurrentPath);
            }
        }

        mCurrentPath = p.parent_path().string();
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
                    mCurrentPath = newPath;
                    mDirty = true;
                    ImGui::CloseCurrentPopup();
                }
            };
            if ( mCustomQuckPathes.size() > 0 ) {

                for (auto& [caption,path] : mCustomQuckPathes) {
                    if (ImGui::Selectable(caption.c_str())) selectPath(path);
                }
                ImGui::Separator();
            }

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
        strncpy(pathInput, mCurrentPath.c_str(), sizeof(pathInput));
        ImGui::SetNextItemWidth(-FLT_MIN);
        if (ImGui::InputTextWithHint("##pathInput","Path", pathInput, sizeof(pathInput), ImGuiInputTextFlags_EnterReturnsTrue)) {
            if (fs::exists(pathInput)) {
                mCurrentPath = pathInput;
                mDirty = true;
            } else {
                std::strncpy(pathInput, mCurrentPath.c_str(), sizeof(pathInput) - 1);
                pathInput[sizeof(pathInput) - 1] = '\0';

            }

        }

    }
    //--------------------------------------------------------------------------
    // this can be overwritten for custom icons
    virtual void DrawIcon(const bool isDir, const std::string ext)
    {
        const ImVec2 size = { 14.f, 14.f }; // Slightly larger for better visibility
        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 pos = ImGui::GetCursorScreenPos();

        ImU32 col = ImGui::GetColorU32(ImGuiCol_Header);
        ImU32 colDarker = ImGui::GetColorU32(ImGuiCol_Tab);

        if (isDir) {
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
            // using ext ....
            // // Simple file icon (empty rectangle or vertical line)
            // dl->AddRect(pos, { pos.x + size.x, pos.y + size.y }, ImGui::GetColorU32(ImGuiCol_TextDisabled), 1.0f);
            // ImGui::Dummy(size);
            // ImGui::SameLine();
        }
    }

    //--------------------------------------------------------------------------
public:
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

            ImGuiWindow* window = ImGui::GetCurrentWindow();

            if (ImGui::BeginChild("FileList", listSize)) {

                static ImGuiTableFlags table_flags =
                ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable |
                ImGuiTableFlags_Sortable  | ImGuiTableFlags_ScrollY     | ImGuiTableFlags_RowBg    |
                ImGuiTableFlags_BordersOuter;

                if (ImGui::BeginTable("FileBrowserTable", 3, table_flags, ImVec2(0, 0)))
                {
                    ImGui::TableSetupScrollFreeze(0, 1);
                    // Column definitions
                    ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_DefaultSort | ImGuiTableColumnFlags_WidthStretch);
                    ImGui::TableSetupColumn("Size", ImGuiTableColumnFlags_WidthFixed, 90.0f);
                    ImGui::TableSetupColumn("Date Modified", ImGuiTableColumnFlags_WidthFixed, 90.0f);
                    ImGui::TableHeadersRow();

                    if (mDirty) {
                        if (!fetchFiles() ) {
                            Log("[error] Failed to fetch files !!!");
                            mDirty = false;

                        }
                        // mDirty  is reseted after sort
                    }
                    // --- SORTING LOGIC ---
                    if (ImGuiTableSortSpecs* sort_specs = ImGui::TableGetSortSpecs())
                    {
                        if (sort_specs->SpecsDirty || mDirty) {

                            std::sort(mListCache.begin(), mListCache.end(), [&](const ListCache& a, const ListCache& b) {
                                if (a.isDir != b.isDir) return a.isDir;

                                const ImGuiTableColumnSortSpecs& spec = sort_specs->Specs[0];

                                bool ascending = (spec.SortDirection == ImGuiSortDirection_Ascending);

                                auto compare = [&]() -> bool {
                                    switch (spec.ColumnIndex) {
                                        case 0: return a.name < b.name;
                                        case 1: return a.rawSize < b.rawSize;
                                        case 2: return a.rawTime < b.rawTime;
                                        default: return false;
                                    }
                                };

                                bool isSmaller = compare();
                                bool isGreater = [&]() -> bool {
                                    switch (spec.ColumnIndex) {
                                        case 0: return b.name < a.name;
                                        case 1: return b.rawSize < a.rawSize;
                                        case 2: return b.rawTime < a.rawTime;
                                        default: return false;
                                    }
                                }();

                                if (ascending) return isSmaller;
                                return isGreater;
                            });

                            dLog("sort file list ....");
                            sort_specs->SpecsDirty = false;
                            mDirty = false; // << reset here !!!
                        }
                    }




                    // --- ROW RENDERING ---
                    ImGuiListClipper clipper;
                    clipper.Begin(mListCache.size());
                    while (clipper.Step()) for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++) {
                        auto entry = mListCache[row];

                        ImGui::TableNextRow();
                        //  ------------ Column 0: Name and Selection Logic
                        ImGui::TableSetColumnIndex(0);
                        bool isSelected = (selectedFile == entry.name);
                        ImGuiSelectableFlags sel_flags = ImGuiSelectableFlags_SpanAllColumns; // | ImGuiSelectableFlags_AllowDoubleClick;

                        DrawIcon(entry.isDir, entry.ext);

                        if (ImGui::Selectable(entry.name.c_str(), isSelected, sel_flags)) {
                            if (!entry.isDir)
                            {
                                selectedFile = entry.name;
                                strncpy(fileInput, entry.name.c_str(), sizeof(fileInput));
                            }

                        }

                        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
                            if (entry.isDir) {
                                mCurrentPath = entry.pathStr;
                                mDirty = true; // Trigger directory change
                            } else {
                                if (strlen(fileInput) > 0) {

                                    std::string ext = entry.ext;
                                    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
                                    selectedFile = (fs::path(mCurrentPath) / fileInput).string();
                                    selectedExt  = ext;
                                    result = true;
                                }
                            }
                        }
                        // ------------ Column 1: File Size
                        ImGui::TableSetColumnIndex(1);

                        float width = ImGui::GetColumnWidth();
                        float posX = ImGui::GetCursorPosX();
                        ImGui::SetCursorPosX(posX + width - ImGui::CalcTextSize(entry.fileSize.c_str()).x - ImGui::GetStyle().ItemSpacing.x);
                        ImGui::TextUnformatted(entry.fileSize.c_str());

                        // ------------ Column 2: Date Modified
                        ImGui::TableSetColumnIndex(2);
                        ImGui::TextUnformatted(entry.fileDateTime.c_str());
                        //--------

                    } // table loop

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
                    selectedFile = (fs::path(mCurrentPath) / fileInput).string();
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
public:
    std::string pwd() const { return mCurrentPath; }

    void changeDirectory(std::string path) {
        mCurrentPath = path;
        mDirty = true;
    }

};
