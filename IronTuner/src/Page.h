//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// base Window class
//-----------------------------------------------------------------------------
#pragma once

#include "imgui.h"
#include "core/fluxBaseObject.h"
#include <functional>


namespace IronTuner {

    class Page {
    protected:

        // window_flags = ImGuiWindowFlags_NoDecoration
        // | ImGuiWindowFlags_NoMove
        // | ImGuiWindowFlags_NoResize
        // | ImGuiWindowFlags_NoSavedSettings
        // | ImGuiWindowFlags_NoBringToFrontOnFocus;


        const ImGuiWindowFlags mWindowFlags =
              ImGuiWindowFlags_NoDecoration
            | ImGuiWindowFlags_NoMove
            | ImGuiWindowFlags_NoResize
            | ImGuiWindowFlags_NoSavedSettings
            | ImGuiWindowFlags_NoBringToFrontOnFocus;
            ;
        //MAYBE:  ImGuiWindowFlags_NoScrollWithMouse

        int mId = -1;
        std::string mCaption = "BASE WINDOW";
        char mWindowName[256];
        std::function<void()> mOnDrawPage = nullptr;
    public:
        std::string getCaption() const { return mCaption; }
        int getId() const { return mId; }

        Page(std::string caption, std::function<void()> drawfunc, int id) {
            mId = id;
            mCaption = caption;
            mOnDrawPage = drawfunc;

            std::snprintf(mWindowName,sizeof(mWindowName), "%s###%d", mCaption.c_str(), id);

        }
        virtual ~Page() = default;



        void Draw(const ImVec2  Pos,  const ImVec2  Size,  const int currentPageId, bool takeFocus = false)  {
            // const ImGuiWindowFlags baseFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings;

            ImGuiWindowFlags extraFlags = 0;

            if (mId != currentPageId) {
                extraFlags = ImGuiWindowFlags_NoMouseInputs |
                ImGuiWindowFlags_NoNav |
                ImGuiWindowFlags_NoNavFocus;
            }


            ImGui::SetNextWindowPos(Pos);
            ImGui::SetNextWindowSize(Size);
            ImGui::SetNextWindowBgAlpha(0.05f);
            ImGui::Begin(mWindowName, nullptr ,  mWindowFlags | extraFlags);
            if (takeFocus) {
                ImGui::SetWindowFocus();
            }
            if (mOnDrawPage) mOnDrawPage();

            ImGui::End();
        }

    };

}; //namespace
