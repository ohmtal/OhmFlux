#include "editorGui.h"
#include "fluxEditorMain.h"
#include "fileDialog.h"
#include <imgui_internal.h>
//------------------------------------------------------------------------------
bool EditorGui::Initialize()
{
    mGuiGlue = new FluxGuiGlue(true, false, "flux.ini");
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
    SAFE_DELETE(mSfxEditor);
    SAFE_DELETE(mGuiGlue);

}
//------------------------------------------------------------------------------
void EditorGui::onEvent(SDL_Event event)
{
    mGuiGlue->onEvent(event);
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
            ImGui::MenuItem("IMGui Demo", NULL, &mParameter.mShowDemo);
            ImGui::Separator();
            ImGui::MenuItem("SFX Editor", NULL, &mParameter.mShowSFXEditor);
            ImGui::Separator();
            ImGui::MenuItem("FM Composer", NULL, &mParameter.mShowFMComposer);
            ImGui::MenuItem("FM Instrument Editor", NULL, &mParameter.mShowFMInstrumentEditor);
            // ImGui::MenuItem("FM Piano Scale", NULL, &mParameter.mShowPianoScale);
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

    if ( mParameter.mShowDemo )
    {
        // ImGui::SetNextWindowDockID(mGuiGlue->getDockSpaceId(), ImGuiCond_FirstUseEver);
        ImGui::ShowDemoWindow();
    }


    if ( mParameter.mShowSFXEditor ){
        // ImGui::SetNextWindowDockID(mGuiGlue->getDockSpaceId(), ImGuiCond_FirstUseEver);
        mSfxEditor->Draw();
    }


    if ( mParameter.mShowFMComposer ) {
        // ImGui::SetNextWindowDockID(mGuiGlue->getDockSpaceId(), ImGuiCond_FirstUseEver);
        mFMComposer->DrawComposer();
    }

    if ( mParameter.mShowFMInstrumentEditor ) {
        // ImGui::SetNextWindowDockID(mGuiGlue->getDockSpaceId(), ImGuiCond_FirstUseEver);
        mFMEditor->DrawInstrumentEditor();
    }

    // if ( mParameter.mShowPianoScale ) {
    //     // ImGui::SetNextWindowDockID(mGuiGlue->getDockSpaceId(), ImGuiCond_FirstUseEver);
    //     mFMEditor->DrawPianoScale();
    // }


    if ( mParameter.mShowCompleteScale ) {
        mFMEditor->DrawScalePlayer();
    }


    DrawMsgBoxPopup();

//XXTH_TEST
    static ImFileDialog myDialog;
    static std::string myCaption;
    if (myDialog.Draw("File Browser", false, { ".png", ".bmp", ".wav", ".ogg", ".sfx", ".fmi", ".fms" })) {
         myCaption = "User chose:" + myDialog.selectedFile;
        LogFMT("File:{} Ext:{}", myDialog.selectedFile, myDialog.selectedExt);

        if ( myDialog.selectedExt == ".fmi" )
            mFMEditor->loadInstrument(myDialog.selectedFile);
        if ( myDialog.selectedExt == ".fms" )
            mFMComposer->loadSong(myDialog.selectedFile);
        // showMessage("File Browser Message", myCaption);
    }




    mGuiGlue->DrawEnd();
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

