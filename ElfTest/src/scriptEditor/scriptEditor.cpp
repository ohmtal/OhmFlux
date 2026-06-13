//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
// NOTE: Ported from OGE3D - and stripped down,
// FIXME Finish Port to  goossens ImGuiColorTextEdit !!!!!!
//-----------------------------------------------------------------------------

#include <gui/ImFlux/led.h>
#include "scriptEditor.h"
#include "gui/fonts/HackNerdFontPropo-Regular.h"
#include <fluxFile.h>
#include "core/Globals.h"

#include <string>
#include <algorithm>
#include <cctype>



ScriptEditor::~ScriptEditor() {
    for (auto& editor :mEditors) {
        if (editor.instance) {
            SAFE_DELETE(editor.instance);
            editor.instance = nullptr;
        }
    }
    mEditors.clear();
}


void ScriptEditor::updateFontSize() {
        switch (mCurrentFontSize) {
            case 13: mCurrentEditorFont = mHackNerdFont13; break;
            case 20: mCurrentEditorFont = mHackNerdFont20; break;
            case 26: mCurrentEditorFont = mHackNerdFont26; break;
            default: mCurrentEditorFont = mHackNerdFont16; break;
        }

    }

void ScriptEditor::setupFont() {
        if ( mHackNerdFont26 ) return;
        ImGuiIO& io = ImGui::GetIO();

        const ImWchar* range = io.Fonts->GetGlyphRangesDefault();  //only default range!

        // io.Fonts->AddFontDefault();
        ImFontConfig config;
        config.FontDataOwnedByAtlas = false;
        mHackNerdFont13 = io.Fonts->AddFontFromMemoryTTF((void*)HackNerdFontPropo_Regular_ttf, HackNerdFontPropo_Regular_ttf_len, 13.0f, &config, range);
        mHackNerdFont16 = io.Fonts->AddFontFromMemoryTTF((void*)HackNerdFontPropo_Regular_ttf, HackNerdFontPropo_Regular_ttf_len, 16.0f, &config, range);
        mHackNerdFont20 = io.Fonts->AddFontFromMemoryTTF((void*)HackNerdFontPropo_Regular_ttf, HackNerdFontPropo_Regular_ttf_len, 20.0f, &config, range);
        mHackNerdFont26 = io.Fonts->AddFontFromMemoryTTF((void*)HackNerdFontPropo_Regular_ttf, HackNerdFontPropo_Regular_ttf_len, 26.0f, &config, range);

        mCurrentFontSize = Con::getIntVariable("$pref::TextEditor::FontSize", 16);

        mCurrentEditorFont = mHackNerdFont16;
    }



// ------------------------ Find in Editor -------------------------------------
//Is Implemented in goossens

// void ScriptEditor::FindNext(EditorState& editorState, const std::string& searchTerm
//         , const bool doReplace, const std::string& replaceTerm) {
//
//
//     if (searchTerm == "") return;
//
//     FindHistory* hist = &editorState.findHistory[searchTerm];
//     if ( !hist ) {
//         FindHistory newHist = { searchTerm };
//         newHist.searchCount = 1;
//         editorState.findHistory[searchTerm]= newHist;
//         hist = &newHist;
//     } else {
//         hist->searchCount++;
//     }
//
//     TextEditor* editor = editorState.instance;
//
//     auto cursor = editor->GetCursorPosition();
//     auto lines = editor->GetTextLines();
//
//
//     std::string stringLine = "";
//
//     for (int i = cursor.mLine; i < (int)lines.size(); ++i) {
//         size_t searchFrom;
//         if (doReplace) searchFrom = (i == cursor.mLine) ? cursor.mColumn : 0;
//         else searchFrom = (i == cursor.mLine) ? cursor.mColumn + 1 : 0;
//
//         if (i == cursor.mLine && searchFrom >= lines[i].length())
//             continue;
//
//         size_t pos = std::string::npos;
//         stringLine = lines[i];
//
//         if (mFindIgnoreCase) {
//             std::string lowerLine = stringLine;
//             std::string lowerSearchTerm = searchTerm;
//
//             std::transform(lowerLine.begin(), lowerLine.end(), lowerLine.begin(), [](unsigned char c){ return std::tolower(c); });
//             std::transform(lowerSearchTerm.begin(), lowerSearchTerm.end(), lowerSearchTerm.begin(), [](unsigned char c){ return std::tolower(c); });
//
//             pos = lowerLine.find(lowerSearchTerm, searchFrom);
//         } else {
//             pos = stringLine.find(searchTerm, searchFrom);
//         }
//
//         if (pos != std::string::npos) {
//             int iPos = (int)pos;
//             editor->SetCursorPosition({ i, iPos });
//
//             if (doReplace) {
//                 stringLine.replace(pos, searchTerm.length(), replaceTerm);
//                 lines[i] = stringLine;
//                 editor->SetTextLines(lines);
//                 editor->SetSelection({ i, iPos }, { i, (int)(pos + replaceTerm.length()) });
//
//                 Point2I targetPos(i, iPos);
//                 auto it = std::remove(hist->positions.begin(), hist->positions.end(), targetPos);
//                 if (it != hist->positions.end()) {
//                     hist->positions.erase(it, hist->positions.end());
//                 }
//
//                 editorState.isDirty = true;
//
//             } else {
//                 editor->SetSelection({ i, iPos }, { i, (int)(pos + searchTerm.length()) });
//                 Point2I targetPos(i, iPos);
//                 auto it = std::find(hist->positions.begin(), hist->positions.end(), targetPos);
//                 if (it == hist->positions.end()) {
//                     hist->positions.push_back(targetPos); // In Vector hinten anfügen
//                 }
//             }
//
//             return;
//         }
//     }
//
//     if ( hist->positions.size() > 0 || hist->searchCount == 1) {
//         editor->SetCursorPosition({ 0, 0 });
//         FindNext(editorState,searchTerm);
//     }
//
//
// }
//------------------------ Render Editors --------------------------------------
void ScriptEditor::renderEditors()
{
    static char findBuffer[256] = "";
    static char replaceBuffer[256] = "";
    // static bool mShowFindReplace = false;

    for (int i = 0; i < mEditors.size(); i++) {
        EditorState& editorState = mEditors[i];

        ImGui::PushID(&editorState);


        std::string windowLabel = editorState.fileName + "###" + editorState.fullPath;

        bool open = true;
        ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);
        if (ImGui::Begin(windowLabel.c_str(), &open,ImGuiWindowFlags_MenuBar )) {

            if  (editorState.initialLoad) {
                editorState.initialLoad = false;
            }

            // static bool  mReclaimFocus = false; //for find input:

            if (ImGui::BeginMenuBar()) {
                if(ImGui::BeginMenu("File")) {
                    // FIXME need save as
                    // if (ImGui::MenuItem("New","")) {
                    //     EditorState newEditorState;
                    //     newEditorState.fileName="new file";
                    //     mEditors.push_back(newEditorState);
                    // }
                    if (ImGui::MenuItem("Save","ctrl s")){
                        saveEditor(editorState);
                    }
                    // if (ImGui::MenuItem("Save as","")) {

                        //FIXME gui for that
                        //     if (!saveEditor(editorState, filename))
                        //         Con::errorf("SAVE OF FILE %s FAILED!!!!", filename.c_str());

                    ImGui::Separator();
                    if (ImGui::MenuItem("Close","")) { open = false; }
                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("Font Size")) {
                    if (ImGui::MenuItem("Small (13px)",  "", mCurrentFontSize == 13)) { mCurrentFontSize = 13; updateFontSize(); }
                    if (ImGui::MenuItem("Normal (16px)", "", mCurrentFontSize == 16)) { mCurrentFontSize = 16; updateFontSize(); }
                    if (ImGui::MenuItem("Large (20px)",  "", mCurrentFontSize == 20)) { mCurrentFontSize = 20; updateFontSize(); }
                    if (ImGui::MenuItem("Huge (26px)",   "", mCurrentFontSize == 26)) { mCurrentFontSize = 26; updateFontSize(); }
                    ImGui::EndMenu();
                }

                // if (ImGui::BeginMenu("Find##Menu")) {
                //     if (ImGui::MenuItem("Find & Replace","ctrl f")) { editorState.mShowFindReplace = true; }
                //     ImGui::Checkbox("Ignore case", &mFindIgnoreCase);
                //     ImGui::EndMenu();
                // }


                if (ImGui::BeginMenu("Action")) {
                    if (ImGui::MenuItem("Hot RELOAD","ctrl r")) {
                        if (!saveEditor(editorState)) {
                            //handle fail saving
                        } else if (!ElfFlux::loadScript(editorState.fullPath.c_str())) {
                            //FIXME handle exec fail!
                        }
                    }
                    ImGui::EndMenu();
                }



                // ~~~~ Find in Editor ~~~~
                if (editorState.mReclaimFocus) {
                    ImGui::SetKeyboardFocusHere();
                    editorState.mReclaimFocus = false;
                }

               // float rightOffset = 200.f;
               //  ImGui::SameLine(ImGui::GetWindowWidth() - rightOffset);
               //
               //  ImGui::SetNextItemWidth(100.f);
               //
               //
               //  std::string findEditID = "##find_" + editorState.fullPath;
               //  if (ImGui::InputText(findEditID.c_str(),findBuffer, 256, ImGuiInputTextFlags_EnterReturnsTrue )) {
               //      FindNext(editorState,findBuffer);
               //       editorState.mReclaimFocus = true;
               //  }
               //  if (ImGui::BeginPopupContextItem()) {
               //      ImGui::SeparatorText("Find History");
               //      for (auto const& [key, val] : editorState.findHistory)
               //      {
               //          if (ImGui::Button(key.c_str())) {
               //              dSprintf(findBuffer, 64, "%s", key.c_str());
               //              FindNext(editorState,findBuffer);
               //          }
               //      }
               //      ImGui::EndPopup();
               //  }
               //  ImGui::SameLine();
               //  if (ImGui::Button("Find")) {
               //      FindNext(editorState,findBuffer);
               //  }
                ImGui::SameLine();
                ImFlux::DrawLED("dirty", editorState.isDirty(), { 8.f, false, true, ImColor(180, 64, 64), 4.f, 2.f});
                ImGui::EndMenuBar();
            } //Menubar

            // ~~~~ Find % Replace in Editor ~~~~
            // static bool mReclaimFocus2 = false;
            // static bool mReclaimFocus3 = false;
            // if ( editorState.mShowFindReplace
            //      && ImGui::BeginChild("_replace_popup_", ImVec2(0.f,60.f) )) {
            //     ImGui::SeparatorText("Find and Replace");
            //     ImGui::SetNextItemWidth(100.f);
            //     if (editorState.mReclaimFocus2) {
            //         ImGui::SetKeyboardFocusHere();
            //         editorState.mReclaimFocus2 = false;
            //     }
            //
            //     if (ImGui::InputText("with ##find_input_popup",findBuffer, 256, ImGuiInputTextFlags_EnterReturnsTrue )) {
            //         FindNext(editorState,findBuffer);
            //         editorState.mReclaimFocus2 = true;
            //     }
            //     ImGui::SameLine();
            //     ImGui::SetNextItemWidth(100.f);
            //     if (editorState.mReclaimFocus3) {
            //         ImGui::SetKeyboardFocusHere();
            //         editorState.mReclaimFocus3 = false;
            //     }
            //     if (ImGui::InputText("##replace_input_popup",replaceBuffer, 256, ImGuiInputTextFlags_EnterReturnsTrue )) {
            //         FindNext(editorState,findBuffer, true, replaceBuffer);
            //         editorState.mReclaimFocus3 = true;
            //     }
            //     ImGui::SameLine();
            //     ImGui::Checkbox("Ignore case", &mFindIgnoreCase);
            //     ImGui::SameLine();
            //     if (ImGui::Button("Find")) {
            //         FindNext(editorState,findBuffer);
            //
            //     }
            //     ImGui::SameLine();
            //     if (ImGui::Button("Replace")) {
            //         FindNext(editorState,findBuffer, true, replaceBuffer);
            //     }
            //     editorState.mShowFindReplace = !ImGui::IsKeyPressed(ImGuiKey_Escape);
            //     ImGui::EndChild();
            // }


            if (mCurrentEditorFont) ImGui::PushFont(mCurrentEditorFont);
// ImGui::GetStyle().FontScaleDpi = 1.2f;

            editorState.instance->Render("Editor");
// ImGui::GetStyle().FontScaleDpi = 1.0f;
            if (mCurrentEditorFont) ImGui::PopFont();

            // short cuts
            if (/*!editorState.instance->IsReadOnly() && */ ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows))
            {
                ImGuiIO& io = ImGui::GetIO();
                auto shift = io.KeyShift;
                auto ctrl = io.ConfigMacOSXBehaviors ? io.KeySuper : io.KeyCtrl;
                auto alt = io.ConfigMacOSXBehaviors ? io.KeyCtrl : io.KeyAlt;
                if (ctrl && !shift && !alt && ImGui::IsKeyPressed(ImGuiKey_S)) {
                    if (!saveEditor(editorState)) {

                        //FIXME Popup! from mGuiGlue
                        Con::errorf("Failed to save %s", editorState.fileName.c_str());
                    }
                }
                if (ctrl && !shift && !alt && ImGui::IsKeyPressed(ImGuiKey_R)) {
                    if (saveEditor(editorState))ElfFlux::loadScript(editorState.fullPath.c_str());
                }
                if (ctrl && !shift && !alt && ImGui::IsKeyPressed(ImGuiKey_F)) {
                    editorState.mReclaimFocus2 = true;
                    editorState.mShowFindReplace = true;
                }

            }

        }
        ImGui::End();

        if (!open) {
            // FIXME : if (tab.isDirty) { "Save changes?" Popup }
            delete editorState.instance;
            mEditors.erase(mEditors.begin() + i);
            i--;
        }

        ImGui::PopID();
    } //for .....
}
// -----------------------------------------------------------------------------
bool ScriptEditor::saveEditor(EditorState& editorState, std::string fullPath)
{

    if (fullPath == "") {
        if (!editorState.isDirty()) return true;
        fullPath = editorState.fullPath;
    } else {
        //it's a save as
        editorState.setFullPath(fullPath);
    }

    if (fullPath == "") return false;
    std::string editortext=editorState.instance->GetText();
    if ( FluxFile::SaveTextFile(fullPath.c_str(), editortext)) {
        editorState.undoVersion =  editorState.instance->GetUndoIndex();
        return true;
    }

    return false;
}
// -----------------------------------------------------------------------------
TextEditor* ScriptEditor::createTextEditor(std::string fullPath)
{

    TextEditor* editor = new TextEditor();

    editor->SetLanguage(TextEditor::Language::Cpp());
    editor->SetShowWhitespacesEnabled(false);
    std::string buffer;
    FluxFile::LoadTextFile(fullPath, buffer);
    editor->SetText(buffer.c_str());


    return editor;
}
// -----------------------------------------------------------------------------
ScriptEditor::EditorState* ScriptEditor::addTextEditor(std::string fullPath) {

    // std::vector<EditorState> mEditors;

    auto it = std::find_if(mEditors.begin(), mEditors.end(),
                           [&](const EditorState& editorstate) { return editorstate.fullPath == fullPath; });

    if (it == mEditors.end()) {
        EditorState newEditorState;
        newEditorState.setFullPath(fullPath);
        newEditorState.instance = createTextEditor(fullPath);
        newEditorState.undoVersion =  newEditorState.instance->GetUndoIndex();

        mEditors.push_back(newEditorState);
        return &mEditors.back();
    } else {
        // Con::printf("Already loaded ...%s", fullPath.c_str());
        std::string windowLabel = it->fileName + "###" + it->fullPath;
        ImGui::SetWindowFocus(windowLabel.c_str());
        return &(*it);
    }
    return nullptr;
}
// -----------------------------------------------------------------------------
void ScriptEditor::init(){

    setupFont();


}
