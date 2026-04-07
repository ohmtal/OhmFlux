//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// IMPORTANT:
//  You must draw the gui in onDrawTopMost:
//      DrawBegin {IMGUI calls} DrawEnd
//
// imGui ini is disabled by default you can do;
//      - getGuiIO()->LoadIniSettingsFromDisk(filename)
//      - getGuiIO()->SaveIniSettingsToDisk(filename)
//-----------------------------------------------------------------------------

#pragma once

#ifndef IMGUI_DEFINE_MATH_OPERATORS
    #define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include <core/fluxBaseObject.h>
#include <utils/fluxSettingsManager.h>
#include <imgui.h>
#include <backends/imgui_impl_sdl3.h>
#include <backends/imgui_impl_opengl3.h>


// inline ImVec2 operator+(const ImVec2& lhs, const ImVec2& rhs) {
//     return ImVec2(lhs.x + rhs.x, lhs.y + rhs.y);
// }
// convert Color4F to ImVec4
inline ImVec4 ImColor4F(const Color4F& c) {
    return ImVec4(c.r, c.g, c.b, c.a);
}

inline ImColor Color4FIm(const Color4F& c) {
    return ImColor (c.r, c.g, c.b, c.a);
}

inline ImU32 Color4FImU32(const Color4F& c) {
    return ImGui::ColorConvertFloat4ToU32(ImVec4(c.r, c.g, c.b, c.a));
}



class FluxGuiGlue : public FluxBaseObject
{
private:
    ImGuiIO* mGuiIO = nullptr;
    ImGuiStyle mBaseStyle;

    bool mScaleImGui = false;
    bool mEnableDockSpace = false;
    bool mEnableGamePad = true;

    ImGuiID mDockSpaceId;

    const char* mIniFileName;


    struct FluxFingerTouchData {
        Uint64 touchStartTime = 0;
        float touchX, touchY;
        bool isLongPress = false;
    } ;
    FluxFingerTouchData mTouchData;

    struct MessageBoxData {
        bool active = false;
        std::string caption = "MessageBox";
        std::string text    = "Hello World";
    };
    MessageBoxData mMessageBoxData;

public:


    bool mSimulateRightClick = false;

    FluxGuiGlue( bool lEnableDockSpace , bool lScaleGui = false, const char* IniFileName = nullptr )
    : mTouchData()
    {
        mEnableDockSpace = lEnableDockSpace;
        mScaleImGui = lScaleGui;

        mIniFileName = IniFileName;

        mSimulateRightClick = isAndroidBuild();

    }

    ~FluxGuiGlue() { Deinitialize(); }

    FluxScreen* getScreen()  { return getScreenObject(); }

    ImGuiIO* getGuiIO() { return mGuiIO; }
    ImGuiStyle getBaseStyle()  const { return  mBaseStyle; }

    void setEnableDockSpace( bool value ) { mEnableDockSpace = value; }
    bool getEnableDockSpace() { return mEnableDockSpace; }

    void setScaleGui( bool value ) { mScaleImGui = value; }
    bool getScaleGui() { return mScaleImGui; }

    ImGuiID getDockSpaceId() { return mDockSpaceId; }


    // void Execute() override;
    bool Initialize() override {
        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        mGuiIO = &ImGui::GetIO();

        mGuiIO->IniFilename = mIniFileName;
        mGuiIO->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        mGuiIO->DisplaySize  = ImVec2(getScreen()->getHeight(), getScreen()->getWidth());
        mGuiIO->ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        if (mEnableGamePad) mGuiIO->ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

        mBaseStyle = ImGui::GetStyle();

        // Setup Platform/Renderer backends
        if (!ImGui_ImplSDL3_InitForOpenGL(getScreen()->getWindow(), getScreen()->getGLContext()))
            return false;

        #if defined(__EMSCRIPTEN__) || defined(__ANDROID__)
        const char* glsl_version = "#version 300 es";
        #elif defined(__APPLE__)
        const char* glsl_version = "#version 150"; // Required for GL 3.2+ Core on macOS
        #else
        const char* glsl_version = "#version 130"; // Standard for GL 3.0+ on Desktop
        #endif

        if (!ImGui_ImplOpenGL3_Init(glsl_version))
            return false;

        // load settings...if we dont use inifile
        if (mIniFileName == nullptr && SettingsManager().IsInitialized()) {
            std::string savedLayout = SettingsManager().get("imgui_layout", std::string(""));
            if (!savedLayout.empty()) {
                ImGui::LoadIniSettingsFromMemory(savedLayout.c_str(), savedLayout.size());
            }
        }

        return true;
    };


    void Deinitialize() override {
        // save settings if we dont use ini
        if (mIniFileName == nullptr && SettingsManager().IsInitialized()) {
            size_t out_size;
            const char* data = ImGui::SaveIniSettingsToMemory(&out_size);
            SettingsManager().set("imgui_layout", std::string(data, out_size));
            SettingsManager().save();
        }

        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplSDL3_Shutdown();
        ImGui::DestroyContext();
    }

    void setScale(float factor) {
        ImGui::GetStyle() = mBaseStyle;
        ImGui::GetStyle().ScaleAllSizes(factor);
        ImGui::GetStyle().FontScaleDpi = factor;
    }

    void OnScaleChanged() {
        mGuiIO->DisplaySize = ImVec2(getScreen()->getHeight(), getScreen()->getWidth());
        if ( mScaleImGui )
        {
            float lMasterScale = getScreen()->getScaleX();
            setScale(lMasterScale);
            // ImGui::GetStyle() = mBaseStyle;
            // ImGui::GetStyle().ScaleAllSizes(lMasterScale);
            // ImGui::GetStyle().FontScaleDpi = lMasterScale;
        }
    }


    void onEvent(SDL_Event event)
    {
        if (event.type ==  FLUX_EVENT_SCALE_CHANGED)
        {
            OnScaleChanged();
        }

        if (mSimulateRightClick)
        {
            // Android simulate right mouse button with long touch
            if (event.type == SDL_EVENT_FINGER_DOWN) {
                mTouchData.touchStartTime = SDL_GetTicks();
                mTouchData.touchX = event.tfinger.x;
                mTouchData.touchY = event.tfinger.y;
                Log("[info]touch down...");

            }
            else if (event.type == SDL_EVENT_FINGER_UP) {
                if (SDL_GetTicks() - mTouchData.touchStartTime > 500) {
                    float xDiff = std::abs(mTouchData.touchX - event.tfinger.x );
                    float yDiff = std::abs( mTouchData.touchY - event.tfinger.y );

                    if ( xDiff < 0.1f && yDiff < 0.1f ) {
                        mGuiIO->AddMouseButtonEvent(ImGuiMouseButton_Right, true);
                        mGuiIO->AddMouseButtonEvent(ImGuiMouseButton_Right, false);
                        Log("[info]Fire Right mouse button press..... (diffs: %f, %f)", xDiff, yDiff);
                        return;
                    }
                }
                // mTouchData.touchStartTime = 0;
            }
        }



        ImGui_ImplSDL3_ProcessEvent(&event);
    }

    void DrawBegin()
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        if (mEnableDockSpace)
        {
            ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode; //<< this makes it transparent
            mDockSpaceId = ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), dockspace_flags);
        }

    }

    void DrawEnd()
    {
        DrawMsgBoxPopup();
        ImGui::Render();

        ImDrawData* drawData = ImGui::GetDrawData();
        if (drawData)
        {
            ImGui_ImplOpenGL3_RenderDrawData(drawData);
        }
    }


    //------------------------------------------------------------------------------
    // Modal Message Box
    //------------------------------------------------------------------------------

    void showMessage(std::string caption, std::string text)
    {
        mMessageBoxData.caption = caption;
        mMessageBoxData.text    = text;
        mMessageBoxData.active  = true;
    }

private:
    // a modal message box
    void DrawMsgBoxPopup() {

        if (mMessageBoxData.active) {
            ImGui::OpenPopup(mMessageBoxData.caption.c_str());
            mMessageBoxData.active = false;
        }
        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

        if (ImGui::BeginPopupModal(mMessageBoxData.caption.c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("%s",mMessageBoxData.text.c_str());
            ImGui::Separator();

            if (ImGui::Button("OK", ImVec2(120, 0))) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }


}; //class
