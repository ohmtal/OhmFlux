//-----------------------------------------------------------------------------
// Copyright (c) 2026 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#pragma once

#include <audio/fluxAudio.h>
#include <core/fluxBaseObject.h>
#include <gui/fluxGuiGlue.h>

#include "fluxSfxEditor.h"
#include "fluxFMEditor.h"
#include "fluxEditorGlobals.h"



class EditorGui: public FluxBaseObject
{
public:
    struct soundToolParameter {
        bool mShowDemo                = false;
        bool mShowSFXEditor           = true;
        bool mShowFMInstrumentEditor  = true;
        bool mShowFMComposer          = false;
        bool mShowCompleteScale       = false;
        bool mShowPianoScale          = true;

    };

private:
    FluxGuiGlue* mGuiGlue = nullptr;
    FluxSfxEditor* mSfxEditor = nullptr;
    FluxFMEditor* mFMEditor = nullptr;

    soundToolParameter mParameter;
public:

    bool Initialize() override
    {

        mGuiGlue = new FluxGuiGlue();
        if (!mGuiGlue->Initialize())
            return false;

        mSfxEditor = new FluxSfxEditor();
        if (!mSfxEditor->Initialize())
            return false;

        mFMEditor = new FluxFMEditor();
        if (!mFMEditor->Initialize())
            return false;

        return true;
    }
    //--------------------------------------------------------------------------------------
    void Deinitialize() override
    {
        SAFE_DELETE(mSfxEditor);
        SAFE_DELETE(mGuiGlue);

    }
    //--------------------------------------------------------------------------------------
    void onEvent(SDL_Event event)
    {
        mGuiGlue->onEvent(event);
    }
    //--------------------------------------------------------------------------------------
    // call with: POPUP_NOT_IMPEMENTED_ACTIVE = true;
    void DrawNotImplementedPopup() {
        // 1. Check the flag to trigger the opening once
        if (POPUP_NOT_IMPEMENTED_ACTIVE) {
            ImGui::OpenPopup("NotImplementedPopup");
            POPUP_NOT_IMPEMENTED_ACTIVE = false; // Reset so we don't call OpenPopup every frame
        }

        // 2. Always attempt to begin the modal
        // (ImGui only returns true here if the popup is actually open)
        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

        if (ImGui::BeginPopupModal("NotImplementedPopup", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Sorry. Feature not implemented yet.");
            ImGui::Separator();

            if (ImGui::Button("OK", ImVec2(120, 0))) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }    //--------------------------------------------------------------------------

/*
 Shortcuts:
    ImGui::Separator();
    if (ImGui::MenuItem("Cut", "Ctrl+X")) {}
    if (ImGui::MenuItem("Copy", "Ctrl+C")) {}
    if (ImGui::MenuItem("Paste", "Ctrl+V")) {}
*/
    void ShowManuBar()
    {
        if (ImGui::BeginMainMenuBar())
        {

            if (ImGui::BeginMenu("Window"))
            {
                ImGui::MenuItem("IMGui Demo", NULL, &mParameter.mShowDemo);
                ImGui::Separator();
                ImGui::MenuItem("SFX Editor", NULL, &mParameter.mShowSFXEditor);
                ImGui::Separator();
                ImGui::MenuItem("FM Composer", NULL, &mParameter.mShowFMComposer);
                ImGui::MenuItem("FM Instrument Editor", NULL, &mParameter.mShowFMInstrumentEditor);
                ImGui::MenuItem("FM Piano Scale", NULL, &mParameter.mShowPianoScale);
                ImGui::MenuItem("FM Full Scale", NULL, &mParameter.mShowCompleteScale);


                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Style"))
            {
                if (ImGui::MenuItem("Dark")) {ImGui::StyleColorsDark(); }
                if (ImGui::MenuItem("Light")) {ImGui::StyleColorsLight(); }
                if (ImGui::MenuItem("Classic")) {ImGui::StyleColorsClassic(); }
                ImGui::EndMenu();
            }

            // ----------- Master Volume
            float rightOffset = 230.0f;
            ImGui::SameLine(ImGui::GetWindowWidth() - rightOffset);

            ImGui::SetNextItemWidth(100);
            float currentVol = AudioManager.getMasterVolume();
            if (ImGui::SliderFloat("##MasterVol", &currentVol, 0.0f, 2.0f, "Vol %.1f"))
            {
                if (!AudioManager.setMasterVolume(currentVol))
                    Log("Error: Failed to set SDL Master volume");
            }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Master Volume");


            // -----------
            ImGui::EndMainMenuBar();
        }

    }
    //--------------------------------------------------------------------------------------
    void Draw( ) override
    {

        mGuiGlue->DrawBegin();
        ShowManuBar();

        if ( mParameter.mShowDemo )
            ImGui::ShowDemoWindow();

        if ( mParameter.mShowSFXEditor )
            mSfxEditor->Draw();

        if ( mParameter.mShowFMComposer )
            mFMEditor->DrawComposer();

        if ( mParameter.mShowFMInstrumentEditor )
            mFMEditor->DrawInstrumentEditor();

        if ( mParameter.mShowPianoScale )
            mFMEditor->DrawPianoScale();

        if ( mParameter.mShowCompleteScale )
            mFMEditor->DrawScalePlayer();


        DrawNotImplementedPopup();


        mGuiGlue->DrawEnd();
    }

}; //class
