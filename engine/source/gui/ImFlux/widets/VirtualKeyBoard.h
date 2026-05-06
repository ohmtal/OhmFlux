//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// ImFlux VirtualGamepadKeyboard / VirtualQwertyGamepadKeyboard
// TODO MAKE IT NICER!
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

enum class VirtualKeyBoardMode { Qwerty, Qwertz, Sorted };

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
            case VirtualKeyBoardMode::Qwertz:
            default:
                Keyboard();
                break;
        };
    }

    void setScale(float factor) {
        mScale = factor;
    }

private:
    bool mVisible = false;
    float mScale = 1.f;
    bool mUpperCase = true;

    VirtualKeyBoardMode mMode = VirtualKeyBoardMode::Qwerty;
    std::string* mInputBuffer = nullptr;
    std::function<void(const std::string& value)> mOnEnter = nullptr;


    const std::vector<std::vector<std::string>> QwertzLayOut = {
        {"!", "\"", "§", "$", "%", "&", "/", "(", ")", "=", "?"},
        {"1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "BACK"},
        {"Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P"},
        {"A", "S", "D", "F", "G", "H", "J", "K", "L", "#"},
        {"Z", "X", "C", "V", "B", "N", "M", ".", ",","+", "-"},
        {"SPACE",  "CANCEL", "ENTER"}
    };

    const std::vector<std::vector<std::string>> QwertyLayOut = {
        {"!", "\"", "§", "$", "%", "&", "/", "(", ")", "=", "?"},
        {"1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "BACK"},
        {"Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P"},
        {"A", "S", "D", "F", "G", "H", "J", "K", "L", "#"},
        {"Z", "X", "C", "V", "B", "N", "M", ".", ",","+", "-"},
        {"SPACE",  "CANCEL", "ENTER"}
    };

    void Keyboard() {
        if (!mInputBuffer) return;

        std::vector<std::vector<std::string>> layout = QwertyLayOut;
        if (mMode == VirtualKeyBoardMode::Qwertz ) layout = QwertzLayOut;

        std::string& inputBuffer = *mInputBuffer;

        float buttonSize = 42.0f * mScale ;

        ImGui::Begin("Virtual Keyboard", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);

        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));

        char tempBuffer[1024];
        strncpy(tempBuffer, inputBuffer.c_str(), sizeof(tempBuffer));
        ImGui::InputText("##display", tempBuffer, sizeof(tempBuffer), ImGuiInputTextFlags_ReadOnly);

        ImGui::PopStyleColor();
        ImGui::SameLine(); if (ImGui::Button("CLEAR", ImVec2(buttonSize * 1.5f, buttonSize / 2.f))) inputBuffer.clear();
        ImGui::SameLine(); if (ImGui::Button(mUpperCase ? "abc" : "ABC", ImVec2(buttonSize, buttonSize / 2.f))) mUpperCase = !mUpperCase;
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();


        for (int r = 0; r < layout.size(); r++) {
            if (r == 2) ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 20.0f);
            if (r == 3) ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 40.0f);

            for (int i = 0; i < layout[r].size(); i++) {
                const std::string& key = layout[r][i];

                float currentWidth = buttonSize;
                if (key == "SPACE") currentWidth = buttonSize * 5.0f;
                else
                if (key == "BACK" || key == "ENTER" || key == "CANCEL") currentWidth = buttonSize * 1.5f;


                if (ImGui::Button(((mUpperCase) ? key : FluxStr::toLower(key)).c_str(), ImVec2(currentWidth, buttonSize))) {
                    if (key == "SPACE") inputBuffer += " ";
                    else if (key == "BACK") { if (!inputBuffer.empty()) inputBuffer.pop_back(); }
                    else if (key == "CANCEL") { Close(false); }
                    else if (key == "ENTER") { Close(true); }
                    else { inputBuffer += (mUpperCase) ? key : FluxStr::toLower(key); }
                }

                if (i < layout[r].size() - 1) ImGui::SameLine();
            }
            if (ImGui::IsWindowFocused() &&
                (   ImGui::IsKeyPressed(ImGuiKey_Escape)
                    || ImGui::IsKeyPressed(ImGuiKey_GamepadBack)
                    || ImGui::IsKeyPressed(ImGuiKey_AppBack)
                    || ImGui::IsKeyPressed(ImGuiKey_GamepadFaceRight)
                )) {
                Close(false);
            }
        }

    ImGui::End();
    }
    // -------------------------------------------------------------------------

};
