//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#pragma once


#include <vector>
#include <string>
#include <map>
#include "ImGuiColorTextEdit/TextEditor.h"
#include <core/fluxGlobals.h>
// #include "utils/errorlog.h"



class ScriptEditor {

    // struct FindHistory {
    //     std::string searchTerm = "";
    //     std::vector<Point2I> positions;
    //     int searchCount = 0;
    // };


    struct EditorState {
        std::string fullPath;
        std::string fileName;
        TextEditor* instance;
        // bool isDirty = false;
        size_t undoVersion = 0;
        bool isDirty() const {
            // Log("isDirty undoindex:%d version:%d", instance->GetUndoIndex(), undoVersion);
            return instance->GetUndoIndex() != undoVersion;

        }

        bool initialLoad = true; //TextEditor set changed flag on setText !!


        //find in buffer
        bool mShowFindReplace = false;
        bool mReclaimFocus = false;
        bool mReclaimFocus2 = false;
        bool mReclaimFocus3 = false;

        // std::map<std::string, FindHistory> findHistory;

        EditorState() = default;
        void setFullPath( const std::string lFullPath) {
            this->fullPath = lFullPath;
            this->fileName = lFullPath; //?? Torque::Path(lFullPath).getFullFileName();
        }

    };


    inline static ImFont* mHackNerdFont13 = nullptr;
    inline static ImFont* mHackNerdFont16 = nullptr;
    inline static ImFont* mHackNerdFont20 = nullptr;
    inline static ImFont* mHackNerdFont26 = nullptr;

    S32 EditorFontSizes[4] = { 13,16,20,26 };

public:

    ScriptEditor() {
        init();
    }
    ~ScriptEditor() = default;

    void onImGuiRender(Point2I offset, const RectI& updateRect);
    void init();
    void setupFont();
    void updateFontSize();


    void renderEditors();


    bool saveEditor(EditorState &editorState, std::string fullPath = "");
    EditorState* addTextEditor( std::string fullPath);
    TextEditor* createTextEditor(std::string fullPath);




    // bool mFindIgnoreCase = true;
    // void FindNext(EditorState& editorState, const std::string& searchTerm, const bool doReplace = false, const std::string& replaceTerm = "") ;

private:
    std::vector<EditorState> mEditors;
    ImFont* mCurrentEditorFont = nullptr;
    S32     mCurrentFontSize = 16;

};
