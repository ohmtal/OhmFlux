//-----------------------------------------------------------------------------
// Copyright (c) 2026 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// IMPORTAND:
//   when your object's Draw is called you must call :
//      DrawBegin {IMGUI calls} DrawEnd
//
// imGui ini is disabled by default you can do;
//      - getGuiIO()->LoadIniSettingsFromDisk(filename)
//      - getGuiIO()->SaveIniSettingsToDisk(filename)
//-----------------------------------------------------------------------------

#pragma once

#include <core/fluxBaseObject.h>
#include <imgui.h>
#include <backends/imgui_impl_sdl3.h>
#include <backends/imgui_impl_opengl3.h>


// convert Color4F to ImVec4
inline ImVec4 ImColor4F(const Color4F& c) {
    return ImVec4(c.r, c.g, c.b, c.a);
}

class FluxGuiGlue : public FluxBaseObject
{
private:
    ImGuiIO* mGuiIO = nullptr;
    ImGuiStyle mBaseStyle;

    bool mScaleImGui = false;


public:
    ~FluxGuiGlue() { Deinitialize(); }

    FluxScreen* getScreen()  { return getScreenObject(); }

    ImGuiIO* getGuiIO() { return mGuiIO; }

    void setScaleGui( bool value ) { mScaleImGui = value; }
    bool getScaleGui() { return mScaleImGui; }


    // void Execute() override;
    bool Initialize() override {
        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        mGuiIO = &ImGui::GetIO();

        mGuiIO->IniFilename = nullptr;
        mGuiIO->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        mGuiIO->DisplaySize  = ImVec2(getScreen()->getHeight(), getScreen()->getWidth());
        mGuiIO->ConfigFlags |= ImGuiConfigFlags_DockingEnable;

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

        return true;
    };
    void Deinitialize() override {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplSDL3_Shutdown();
        ImGui::DestroyContext();
    }

    void OnScaleChanged() {
        mGuiIO->DisplaySize = ImVec2(getScreen()->getHeight(), getScreen()->getWidth());
        if ( mScaleImGui )
        {
            float lMasterScale = getScreen()->getScaleX(); // Usually better to scale UI based on height
            ImGui::GetStyle() = mBaseStyle; // Reset to the clean copy
            ImGui::GetStyle().ScaleAllSizes(lMasterScale);
            ImGui::GetStyle().FontScaleDpi = lMasterScale;
        }
    }
    void onEvent(SDL_Event event)
    {
        if (event.type ==  FLUX_EVENT_SCALE_CHANGED)
        {
            OnScaleChanged();
        }

        ImGui_ImplSDL3_ProcessEvent(&event);
    }

    void DrawBegin()
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();
        ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_None);
    }

    void DrawEnd()
    {
        ImGui::Render();

        ImDrawData* drawData = ImGui::GetDrawData();
        if (drawData)
        {
            ImGui_ImplOpenGL3_RenderDrawData(drawData);
        }
    }

}; //class
