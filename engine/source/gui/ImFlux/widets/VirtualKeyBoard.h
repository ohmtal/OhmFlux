//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// ImFlux VirtualGamepadKeyboard / VirtualQwertyGamepadKeyboard
// TODO TEST !
//-----------------------------------------------------------------------------
#pragma once
#include "imgui.h"

#include <string>
#include <vector>
#include <functional>

namespace ImFlux {



    /*
     * --- enable gamepad => is enable by default: fluxGuiGlue / mEnableGamePad
     *
     * io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad
     *
     * --- focus keys
     *
     * if (keyboardJustOpened) {
     *   ImGui::SetNextWindowFocus();
     *   ImGui::SetItemDefaultFocus();
     *   keyboardJustOpened = false;
     * }
     * --- test...
     * if (ImGui::GetIO().WantCaptureKeyboard || ImGui::GetIO().WantCaptureMouse) {
     *
     * }
     *
     * ---  DETECT use virtual keyboard:
     *  ???
     *
     */

enum class VirtualKeyBoardMode { Qwerty, Sorted };

struct VirtualKeyBoard {


    void Open(std::string& buffer, std::function<void(const std::string& value)> onEnter = nullptr) {
        mVisible = true;
        mInputBuffer = &buffer;
        mOnEnter = onEnter;
    }

    void Close(bool isEnter ) {
        mVisible = false;
        if (isEnter && mOnEnter) mOnEnter(*mInputBuffer);
        // bad idea mInputBuffer = nullptr;

    }

    void Draw() {
        if (!mVisible || mInputBuffer == nullptr ) return;
        switch (mMode) {
            case VirtualKeyBoardMode::Sorted:
                SortedKeyboard();
                break;

            case VirtualKeyBoardMode::Qwerty:
            default:
                QwertyKeyboard();
                break;
        };
    }

    void setScale(float factor) {
        mScale = factor;
    }

private:
    bool mVisible = false;
    float mScale = 1.f;

    VirtualKeyBoardMode mMode = VirtualKeyBoardMode::Qwerty;
    std::string* mInputBuffer = nullptr;
    std::function<void(const std::string& value)> mOnEnter = nullptr;

    void QwertyKeyboard() {
        const std::vector<std::vector<std::string>> layout = {
            {"!", "\"", "§", "$", "%", "&", "/", "(", ")", "=", "?"},
            {"1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "BACK"},
            {"Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P"},
            {"A", "S", "D", "F", "G", "H", "J", "K", "L", "#"},
            {"Z", "X", "C", "V", "B", "N", "M", ".", ",","+", "-"},
            {"SPACE", "CLEAR", "CANCEL", "ENTER"}
        };

        if (!mInputBuffer) return;
        std::string& inputBuffer = *mInputBuffer;


        ImGui::Begin("Virtual Keyboard", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);

        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
        ImGui::InputText("##display", (char*)inputBuffer.c_str(), inputBuffer.capacity(), ImGuiInputTextFlags_ReadOnly);
        ImGui::PopStyleColor();
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        float buttonSize = 42.0f * mScale ;

        for (int r = 0; r < layout.size(); r++) {
            if (r == 2) ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 20.0f);
            if (r == 3) ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 40.0f);

            for (int i = 0; i < layout[r].size(); i++) {
                const std::string& key = layout[r][i];

                float currentWidth = buttonSize;
                if (key == "SPACE") currentWidth = buttonSize * 3.0f;
                if (key == "BACK" || key == "ENTER") currentWidth = buttonSize * 1.5f;

                if (ImGui::Button(key.c_str(), ImVec2(currentWidth, buttonSize))) {
                    if (key == "SPACE") inputBuffer += " ";
                    else if (key == "BACK") { if (!inputBuffer.empty()) inputBuffer.pop_back(); }
                    else if (key == "CLEAR") { inputBuffer.clear(); }
                    else if (key == "CANCEL") { Close(false); }
                    else if (key == "ENTER") { Close(true); }
                    else { inputBuffer += key; }
                }

                if (i < layout[r].size() - 1) ImGui::SameLine();
            }
            if (ImGui::IsWindowFocused() &&
                (   ImGui::IsKeyPressed(ImGuiKey_Escape)
                    || ImGui::IsKeyPressed(ImGuiKey_GamepadBack)
                    || ImGui::IsKeyPressed(ImGuiKey_GamepadFaceRight)
                )) {
                Close(false);
            }
        }

    ImGui::End();
    }
    // -------------------------------------------------------------------------

    void SortedKeyboard() {
        const std::vector<std::string> keys = {
            "A", "B", "C", "D", "E", "F",
            "G", "H", "I", "J", "K", "L",
            "M", "N", "O", "P", "Q", "R",
            "S", "T", "U", "V", "W", "X",
            "Y", "Z", "0", "1", "2", "3",
            "4", "5", "6", "7", "8", "9",
            "SPACE", "BACKSPACE", "CLEAR", "CANCEL", "ENTER"
        };

        if (!mInputBuffer) return;
        std::string& inputBuffer = *mInputBuffer;

        ImGui::Begin("Virtual Keyboard", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

        ImGui::Text("Text: %s", inputBuffer.c_str());
        ImGui::Separator();

        int columns = 6;
        for (int i = 0; i < keys.size(); i++) {
            if (ImGui::Button(keys[i].c_str(), ImVec2(30, 30)* mScale)) {
                if (keys[i] == "SPACE") inputBuffer += " ";
                else if (keys[i] == "BACKSPACE") {
                    if (!inputBuffer.empty()) inputBuffer.pop_back();
                }
                else if (keys[i] == "CLEAR") inputBuffer.clear();
                else if (keys[i] == "CANCEL") { Close(false); }
                else if (keys[i] == "ENTER") { Close(true); }
                else inputBuffer += keys[i];
            }

            if ((i + 1) % columns != 0) ImGui::SameLine();
        }

        ImGui::End();
    }
};


};
