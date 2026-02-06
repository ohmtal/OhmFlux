#include "appGui.h"
#include "appMain.h"
#include <gui/ImFileDialog.h>
#include <imgui_internal.h>
#include <utils/fluxSettingsManager.h>
#include "appGlobals.h"

#include <gui/ImFlux/showCase.h> //demos of ImFlux Widgets
#include <gui/ImConsole.h>

//------------------------------------------------------------------------------
void SDLCALL ConsoleLogFunction(void *userdata, int category, SDL_LogPriority priority, const char *message)
{
    if (!userdata) return;
    auto* gui = static_cast<AppGui*>(userdata);

    if (!gui)
        return;

    char lBuffer[1024];
    if (priority == SDL_LOG_PRIORITY_ERROR)
    {
        snprintf(lBuffer, sizeof(lBuffer), "[ERROR] %s", message);
    }
    else if (priority == SDL_LOG_PRIORITY_WARN)
    {
        snprintf(lBuffer, sizeof(lBuffer), "[WARN] %s", message);
    }
    else
    {
        snprintf(lBuffer, sizeof(lBuffer), "%s", message);
    }

    // bad if we are gone !!
    gui->mConsole.AddLog("%s", message);
}
//------------------------------------------------------------------------------
void AppGui::OnConsoleCommand(ImConsole* console, const char* cmdline){
    std::string cmd = fluxStr::getWord(cmdline,0);

    //if i want to add "help" and autocomplete  mConsole.Commands.push_back("/spam");

    if (cmd == "/spam") {
        for (S32 i = 0; i < 1000; i++) {
            dLog("[info] SPAM %i", i);
        }
    }

}

//------------------------------------------------------------------------------
bool AppGui::Initialize()
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

    mAppSettings = SettingsManager().get("AppGui::mAppSettings", mDefaultAppSettings);



    mGuiGlue = new FluxGuiGlue(true, false, nullptr);
    if (!mGuiGlue->Initialize())
        return false;



    mSfxStereoModule = new FluxSfxStereoModule();
    if (!mSfxStereoModule->Initialize())
        return false;

    //XXTH TEST
    // mSfxEditorStereo->getSFXGeneratorStereo()->detachAudio();
    // getMain()-> queueObject(mSfxEditorStereo); //For update!


    mSfxModule = new FluxSfxModule();
    if (!mSfxModule->Initialize())
        return false;


    // // not centered ?!?!?! i guess center is not in place yet ?
    // mBackground = new FluxRenderObject(getGame()->loadTexture("assets/fluxeditorback.png"));
    // if (mBackground) {
    //     mBackground->setPos(getGame()->getScreen()->getCenterF());
    //     mBackground->setSize(getGame()->getScreen()->getScreenSize());
    //     getGame()->queueObject(mBackground);
    // }


    g_FileDialog.init( getGamePath(), {  ".sfx", ".fmi", ".fms", ".wav", ".ogg" });

    // Console
    mConsole.OnCommand =  [&](ImConsole* console, const char* cmd) { OnConsoleCommand(console, cmd); };
    SDL_SetLogOutputFunction(ConsoleLogFunction, this);


    return true;
}
//------------------------------------------------------------------------------
void AppGui::Deinitialize()
{

    SAFE_DELETE(mSfxModule);
    SAFE_DELETE(mSfxStereoModule);
    SAFE_DELETE(mGuiGlue);

    if (SettingsManager().IsInitialized()) {
        SettingsManager().set("AppGui::mAppSettings", mAppSettings);
        SettingsManager().save();
    }

}
//------------------------------------------------------------------------------
void AppGui::onEvent(SDL_Event event)
{
    mGuiGlue->onEvent(event);

    if (mSfxStereoModule)
        mSfxStereoModule->onEvent(event);

    if (mSfxModule)
        mSfxModule->onEvent(event);

}
//------------------------------------------------------------------------------
void AppGui::DrawMsgBoxPopup() {

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

void AppGui::ShowMenuBar()
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
            ImGui::MenuItem("IMGui Demo", NULL, &mAppSettings.mShowDemo);
            ImGui::MenuItem("IMFlux Widgets ShowCase", NULL, &mAppSettings.mShowImFluxWidgets);
            ImGui::Separator();
            ImGui::MenuItem("File Browser", NULL, &mAppSettings.mShowFileBrowser);
            ImGui::MenuItem("Console", NULL, &mAppSettings.mShowConsole);
            ImGui::Separator();
            ImGui::MenuItem("Sound Effects Generator", NULL, &mAppSettings.mShowSFXModule);
            ImGui::MenuItem("Sound Effects Stereo", NULL, &mAppSettings.mShowSFXStereoModule);
            ImGui::Separator();
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
void AppGui::DrawGui()
{

    mGuiGlue->DrawBegin();
    ShowMenuBar();


    if ( mAppSettings.mShowDemo )
    {
        ImGui::ShowDemoWindow();
    }



    if (mAppSettings.mShowSFXModule) {
        mSfxModule->Draw();
    }

    if (mAppSettings.mShowSFXStereoModule) {
        mSfxStereoModule->DrawGui();
    }



    if (mAppSettings.mShowImFluxWidgets) {
        ImFlux::ShowCaseWidgets();
    }



    if (mAppSettings.mShowConsole)
        mConsole.Draw("Console", &mAppSettings.mShowConsole);



    DrawMsgBoxPopup();



    if (mAppSettings.mShowFileBrowser)
    {
        if (g_FileDialog.Draw()) {
            // LogFMT("File:{} Ext:{}", g_FileDialog.selectedFile, g_FileDialog.selectedExt);
            if (g_FileDialog.mSaveMode)
            {
                g_FileDialog.reset();
            } else {

            }
        }
    }



    InitDockSpace();  

    mGuiGlue->DrawEnd();
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void AppGui::onKeyEvent(SDL_KeyboardEvent event)
{
}
//------------------------------------------------------------------------------
void AppGui::InitDockSpace()
{
    if (mAppSettings.mEditorGuiInitialized)
        return; 

    mAppSettings.mEditorGuiInitialized = true;

    ImGuiID dockspace_id = mGuiGlue->getDockSpaceId();

    // Clear any existing layout
    ImGui::DockBuilderRemoveNode(dockspace_id);
    ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
    ImGui::DockBuilderSetNodeSize(dockspace_id, ImVec2(1920, 990));

    // Replicate your splits (matches your ini data)
    ImGuiID dock_main_id = dockspace_id;
    ImGuiID dock_id_left, dock_id_right, dock_id_central;

    dock_id_left = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, 0.215f, nullptr, &dock_main_id);

    dock_id_right = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 0.172f, nullptr, &dock_id_central);

    // Dock the Windows to these IDs
    // ImGui::DockBuilderDockWindow("FM Instrument Editor", dock_id_left);
    ImGui::DockBuilderDockWindow("File Browser", dock_id_right);
    ImGui::DockBuilderDockWindow("Sound Effects Generator", dock_id_central);
    // ImGui::DockBuilderDockWindow("FM Song Composer", dock_id_central);


    ImGui::DockBuilderFinish(dockspace_id);
}
//------------------------------------------------------------------------------
