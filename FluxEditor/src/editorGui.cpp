#include "editorGui.h"
#include "fluxEditorMain.h"
#include "fileDialog.h"
#include <imgui_internal.h>
#include <utils/fluxSettingsManager.h>

//------------------------------------------------------------------------------
bool EditorGui::Initialize()
{
    std::string lSettingsFile =
        getGame()->mSettings.getPrefsPath()
        .append(getGame()->mSettings.getSafeCaption())
        .append("_prefs.json");
    if (SettingsManager().Initialize(lSettingsFile))
    {

    } else {
        LogFMT("Error: Can not open setting file: {}", lSettingsFile);
    }

    mEditorSettings = SettingsManager().get("EditorGui::mEditorSettings", mDefaultEditorSettings);



    mGuiGlue = new FluxGuiGlue(true, false, nullptr);
    if (!mGuiGlue->Initialize())
        return false;



    mSfxEditor = new FluxSfxEditor();
    if (!mSfxEditor->Initialize())
        return false;

    mFMEditor = new FluxFMEditor();
    if (!mFMEditor->Initialize())
        return false;

    mFMComposer = new FluxComposer( mFMEditor->getController());
    if (!mFMComposer->Initialize())
        return false;

    // not centered ?!?!?! i guess center is not in place yet ?
    mBackground = new FluxRenderObject(getGame()->loadTexture("assets/fluxeditorback.png"));
    if (mBackground) {
        mBackground->setPos(getGame()->getScreen()->getCenterF());
        mBackground->setSize(getGame()->getScreen()->getScreenSize());
        getGame()->queueObject(mBackground);
    }

    return true;
}
//------------------------------------------------------------------------------
void EditorGui::Deinitialize()
{

    SAFE_DELETE(mFMEditor);
    SAFE_DELETE(mFMComposer);
    SAFE_DELETE(mSfxEditor);
    SAFE_DELETE(mGuiGlue);

    if (SettingsManager().IsInitialized()) {
        SettingsManager().set("EditorGui::mEditorSettings", mEditorSettings);
        SettingsManager().save();
    }

}
//------------------------------------------------------------------------------
void EditorGui::onEvent(SDL_Event event)
{
    mGuiGlue->onEvent(event);

    if (mSfxEditor)
        mSfxEditor->onEvent(event);
    if (mFMComposer)
        mFMComposer->onEvent(event);
    if (mFMEditor)
        mFMEditor->onEvent(event);

}
//------------------------------------------------------------------------------
void EditorGui::DrawMsgBoxPopup() {

    if (POPUP_MSGBOX_ACTIVE) {
        ImGui::OpenPopup(POPUP_MSGBOX_CAPTION.c_str());
        POPUP_MSGBOX_ACTIVE = false;
    }

    // 2. Always attempt to begin the modal
    // (ImGui only returns true here if the popup is actually open)
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal(POPUP_MSGBOX_CAPTION.c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("%s",POPUP_MSGBOX_TEXT.c_str());
        ImGui::Separator();

        if (ImGui::Button("OK", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void EditorGui::ShowManuBar()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Exit")) { getGame()->TerminateApplication(); }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Window"))
        {
            ImGui::MenuItem("IMGui Demo", NULL, &mEditorSettings.mShowDemo);
            ImGui::Separator();
            ImGui::MenuItem("SFX Editor", NULL, &mEditorSettings.mShowSFXEditor);
            ImGui::Separator();
            ImGui::MenuItem("FM Composer", NULL, &mEditorSettings.mShowFMComposer);
            ImGui::MenuItem("FM Instrument Editor", NULL, &mEditorSettings.mShowFMInstrumentEditor);
            ImGui::MenuItem("FM Full Scale", NULL, &mEditorSettings.mShowCompleteScale);


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
//------------------------------------------------------------------------------
void EditorGui::DrawGui()
{

    mGuiGlue->DrawBegin();
    ShowManuBar();


    // // docking test found at: https://github.com/ocornut/imgui/wiki/Docking
    // // does not work so far. i guess i missed something
    //
    // if (ImGui::DockBuilderGetNode(mGuiGlue->getDockSpaceId()) == nullptr)
    // {
    //     ImGui::DockBuilderAddNode(mGuiGlue->getDockSpaceId(), ImGuiDockNodeFlags_DockSpace);
    //     ImGui::DockBuilderSetNodeSize(mGuiGlue->getDockSpaceId(), ImGui::GetMainViewport()->Size);
    //     ImGuiID dock_id_left = 0;
    //     ImGuiID dock_id_main = mGuiGlue->getDockSpaceId();
    //     ImGui::DockBuilderSplitNode(dock_id_main, ImGuiDir_Left, 0.20f, &dock_id_left, &dock_id_main);
    //     ImGuiID dock_id_left_top = 0;
    //     ImGuiID dock_id_left_bottom = 0;
    //     ImGui::DockBuilderSplitNode(dock_id_left, ImGuiDir_Up, 0.50f, &dock_id_left_top, &dock_id_left_bottom);
    //     ImGui::DockBuilderDockWindow("FM Instrument Editor", dock_id_main);
    //     ImGui::DockBuilderDockWindow("File Browser", dock_id_left_top);
    //     // ImGui::DockBuilderDockWindow("Scene", dock_id_left_bottom);
    //     ImGui::DockBuilderFinish(mGuiGlue->getDockSpaceId());
    // }

    if ( mEditorSettings.mShowDemo )
    {
        // ImGui::SetNextWindowDockID(mGuiGlue->getDockSpaceId(), ImGuiCond_FirstUseEver);
        ImGui::ShowDemoWindow();
    }


    if ( mEditorSettings.mShowSFXEditor ){
        // ImGui::SetNextWindowDockID(mGuiGlue->getDockSpaceId(), ImGuiCond_FirstUseEver);
        mSfxEditor->Draw();
    }


    if ( mEditorSettings.mShowFMComposer ) {
        // ImGui::SetNextWindowDockID(mGuiGlue->getDockSpaceId(), ImGuiCond_FirstUseEver);
        mFMComposer->DrawComposer();
    }

    if ( mEditorSettings.mShowFMInstrumentEditor ) {
        // ImGui::SetNextWindowDockID(mGuiGlue->getDockSpaceId(), ImGuiCond_FirstUseEver);
        mFMEditor->DrawInstrumentEditor();
    }

    // if ( mParameter.mShowPianoScale ) {
    //     // ImGui::SetNextWindowDockID(mGuiGlue->getDockSpaceId(), ImGuiCond_FirstUseEver);
    //     mFMEditor->DrawPianoScale();
    // }


    if ( mEditorSettings.mShowCompleteScale ) {
        mFMEditor->DrawScalePlayer();
    }


    DrawMsgBoxPopup();



    if (g_FileDialog.Draw()) {
        LogFMT("File:{} Ext:{}", g_FileDialog.selectedFile, g_FileDialog.selectedExt);

        if (g_FileDialog.mSaveMode)
        {
            if (!g_FileDialog.mCancelPressed)
            {
                if (g_FileDialog.mSaveExt == ".fms")
                {
                    if (g_FileDialog.selectedExt == "")
                        g_FileDialog.selectedFile.append(g_FileDialog.mSaveExt);
                    mFMComposer->saveSong(g_FileDialog.selectedFile);
                }
                else
                if (g_FileDialog.mSaveExt == ".fmi")
                {
                    if (g_FileDialog.selectedExt == "")
                        g_FileDialog.selectedFile.append(g_FileDialog.mSaveExt);
                    mFMEditor->saveInstrument(g_FileDialog.selectedFile);
                }
            }


            //FIXME sfx
            //reset
            g_FileDialog.reset();
        } else {
            if ( g_FileDialog.selectedExt == ".fmi" )
                mFMEditor->loadInstrument(g_FileDialog.selectedFile);
            else
            if ( g_FileDialog.selectedExt == ".fms" )
                mFMComposer->loadSong(g_FileDialog.selectedFile);

            //FIXME also load sfx here !!
        }
    }




    mGuiGlue->DrawEnd();
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------


void EditorGui::onKeyEvent(SDL_KeyboardEvent event)
    {
        if ( mEditorSettings.mShowFMComposer )
            mFMComposer->onKeyEvent(event);
    }
