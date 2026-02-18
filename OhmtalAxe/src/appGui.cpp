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

void AppGui::ShowToolbar() {

        ImFlux::ButtonParams bpOn = ImFlux::SLATE_BUTTON.WithSize(ImVec2(60,32));
        ImFlux::ButtonParams bpOff = bpOn;
        bpOff.textColor = IM_COL32(120,120,120, 255);


        ImGui::SetNextWindowSize(ImVec2(0.f, 32.f), ImGuiCond_FirstUseEver);
        ImGui::Begin("Toolbar");
        if (ImFlux::ButtonFancy("Drums") ) { mAppSettings.mShowDrumKit = ! mAppSettings.mShowDrumKit; }
        ImFlux::SameLineBreak(bpOn.size.x);
        if (ImFlux::ButtonFancy("Rack") ) { mAppSettings.mShowRack = ! mAppSettings.mShowRack; }
        ImFlux::SameLineBreak(bpOn.size.x);
        if (ImFlux::ButtonFancy("Visualizer") ) { mAppSettings.mShowVisualizer = ! mAppSettings.mShowVisualizer; }
        ImFlux::SameLineBreak(bpOn.size.x);



        for (int i = 0; i<20; i++ ) {
            ImFlux::ButtonFancy(std::format("Dummy {}", i));
            ImFlux::SameLineBreak(bpOn.size.x);
        }


        ImGui::End();
    }
//------------------------------------------------------------------------------
void AppGui::ApplyStudioTheme() {

    // cyan but i think the Title/Tabs are too light

    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    // --- GRUNDFARBEN ( Cyan & Anthrazit) ---
    const ImVec4 cyan_lower  = ImVec4(0.00f, 0.25f, 0.25f, 1.00f); // Cyan
    const ImVec4 cyan_low    = ImVec4(0.00f, 0.45f, 0.45f, 1.00f); // Cyan
    const ImVec4 cyan_mid    = ImVec4(0.00f, 0.60f, 0.60f, 1.00f); // Hover-Cyan
    const ImVec4 cyan_high   = ImVec4(0.00f, 0.80f, 0.80f, 1.00f); // actice Cyan
    const ImVec4 dark_bg     = ImVec4(0.10f, 0.10f, 0.12f, 0.80f); // nearly black
    const ImVec4 dark_surface = ImVec4(0.16f, 0.16f, 0.18f, 1.00f); // panels



    // --- UI  ---
    colors[ImGuiCol_TitleBg]                = dark_surface; //cyan_lower;


    colors[ImGuiCol_Text]                   = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
    colors[ImGuiCol_WindowBg]               = dark_bg;
    colors[ImGuiCol_ChildBg]                = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_PopupBg]                = dark_surface;
    colors[ImGuiCol_Border]                 = ImVec4(0.30f, 0.30f, 0.30f, 0.50f);


    // Header (CollapsingHeader, TreeNodes)
    colors[ImGuiCol_Header]                 = dark_surface;  // cyan_low;
    colors[ImGuiCol_HeaderHovered]          = cyan_mid;
    colors[ImGuiCol_HeaderActive]           = cyan_high;

    // Buttons
    colors[ImGuiCol_Button]                 = ImVec4(0.20f, 0.20f, 0.22f, 1.00f);
    colors[ImGuiCol_ButtonHovered]          = cyan_low;
    colors[ImGuiCol_ButtonActive]           = cyan_mid;

    // Frame (Checkbox, Input, Slider )
    colors[ImGuiCol_FrameBg]                = ImVec4(0.05f, 0.05f, 0.05f, 1.00f);
    colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.20f, 0.20f, 0.22f, 1.00f);
    colors[ImGuiCol_FrameBgActive]          = cyan_low;

    // Tabs
    colors[ImGuiCol_Tab]                    = cyan_lower;
    colors[ImGuiCol_TabHovered]             = cyan_low;
    colors[ImGuiCol_TabActive]              = cyan_mid;

    colors[ImGuiCol_TabDimmed]              = ImLerp(colors[ImGuiCol_Tab],          colors[ImGuiCol_TitleBg], 0.80f);
    colors[ImGuiCol_TabDimmedSelected]      = ImLerp(colors[ImGuiCol_TabSelected],  colors[ImGuiCol_TitleBg], 0.40f);
    colors[ImGuiCol_TabDimmedSelectedOverline] = ImVec4(0.53f, 0.53f, 0.87f, 0.00f);


    // Akzente
    colors[ImGuiCol_CheckMark]              = cyan_high;
    colors[ImGuiCol_SliderGrab]             = cyan_low;
    colors[ImGuiCol_SliderGrabActive]       = cyan_high;
    colors[ImGuiCol_TitleBgActive]          = cyan_low;

    style.WindowRounding = 6.0f;
    style.FrameRounding = 4.0f;

    // classic theme
    // ImGuiStyle* style = dst ? dst : &ImGui::GetStyle();
    // ImVec4* colors = style->Colors;
    //
    // colors[ImGuiCol_Text]                   = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
    // colors[ImGuiCol_TextDisabled]           = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
    // colors[ImGuiCol_WindowBg]               = ImVec4(0.00f, 0.00f, 0.00f, 0.85f);
    // colors[ImGuiCol_ChildBg]                = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    // colors[ImGuiCol_PopupBg]                = ImVec4(0.11f, 0.11f, 0.14f, 0.92f);
    // colors[ImGuiCol_Border]                 = ImVec4(0.50f, 0.50f, 0.50f, 0.50f);
    // colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    // colors[ImGuiCol_FrameBg]                = ImVec4(0.43f, 0.43f, 0.43f, 0.39f);
    // colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.47f, 0.47f, 0.69f, 0.40f);
    // colors[ImGuiCol_FrameBgActive]          = ImVec4(0.42f, 0.41f, 0.64f, 0.69f);
    // colors[ImGuiCol_TitleBg]                = ImVec4(0.27f, 0.27f, 0.54f, 0.83f);
    // colors[ImGuiCol_TitleBgActive]          = ImVec4(0.32f, 0.32f, 0.63f, 0.87f);
    // colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.40f, 0.40f, 0.80f, 0.20f);
    // colors[ImGuiCol_MenuBarBg]              = ImVec4(0.40f, 0.40f, 0.55f, 0.80f);
    // colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.20f, 0.25f, 0.30f, 0.60f);
    // colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.40f, 0.40f, 0.80f, 0.30f);
    // colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.40f, 0.40f, 0.80f, 0.40f);
    // colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.41f, 0.39f, 0.80f, 0.60f);
    // colors[ImGuiCol_CheckMark]              = ImVec4(0.90f, 0.90f, 0.90f, 0.50f);
    // colors[ImGuiCol_SliderGrab]             = ImVec4(1.00f, 1.00f, 1.00f, 0.30f);
    // colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.41f, 0.39f, 0.80f, 0.60f);
    // colors[ImGuiCol_Button]                 = ImVec4(0.35f, 0.40f, 0.61f, 0.62f);
    // colors[ImGuiCol_ButtonHovered]          = ImVec4(0.40f, 0.48f, 0.71f, 0.79f);
    // colors[ImGuiCol_ButtonActive]           = ImVec4(0.46f, 0.54f, 0.80f, 1.00f);
    // colors[ImGuiCol_Header]                 = ImVec4(0.40f, 0.40f, 0.90f, 0.45f);
    // colors[ImGuiCol_HeaderHovered]          = ImVec4(0.45f, 0.45f, 0.90f, 0.80f);
    // colors[ImGuiCol_HeaderActive]           = ImVec4(0.53f, 0.53f, 0.87f, 0.80f);
    // colors[ImGuiCol_Separator]              = ImVec4(0.50f, 0.50f, 0.50f, 0.60f);
    // colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.60f, 0.60f, 0.70f, 1.00f);
    // colors[ImGuiCol_SeparatorActive]        = ImVec4(0.70f, 0.70f, 0.90f, 1.00f);
    // colors[ImGuiCol_ResizeGrip]             = ImVec4(1.00f, 1.00f, 1.00f, 0.10f);
    // colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.78f, 0.82f, 1.00f, 0.60f);
    // colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.78f, 0.82f, 1.00f, 0.90f);
    // colors[ImGuiCol_InputTextCursor]        = colors[ImGuiCol_Text];
    // colors[ImGuiCol_TabHovered]             = colors[ImGuiCol_HeaderHovered];
    // colors[ImGuiCol_Tab]                    = ImLerp(colors[ImGuiCol_Header],       colors[ImGuiCol_TitleBgActive], 0.80f);
    // colors[ImGuiCol_TabSelected]            = ImLerp(colors[ImGuiCol_HeaderActive], colors[ImGuiCol_TitleBgActive], 0.60f);
    // colors[ImGuiCol_TabSelectedOverline]    = colors[ImGuiCol_HeaderActive];
    // colors[ImGuiCol_TabDimmed]              = ImLerp(colors[ImGuiCol_Tab],          colors[ImGuiCol_TitleBg], 0.80f);
    // colors[ImGuiCol_TabDimmedSelected]      = ImLerp(colors[ImGuiCol_TabSelected],  colors[ImGuiCol_TitleBg], 0.40f);
    // colors[ImGuiCol_TabDimmedSelectedOverline] = ImVec4(0.53f, 0.53f, 0.87f, 0.00f);
    // colors[ImGuiCol_DockingPreview]         = colors[ImGuiCol_Header] * ImVec4(1.0f, 1.0f, 1.0f, 0.7f);
    // colors[ImGuiCol_DockingEmptyBg]         = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    // colors[ImGuiCol_PlotLines]              = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    // colors[ImGuiCol_PlotLinesHovered]       = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    // colors[ImGuiCol_PlotHistogram]          = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    // colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    // colors[ImGuiCol_TableHeaderBg]          = ImVec4(0.27f, 0.27f, 0.38f, 1.00f);
    // colors[ImGuiCol_TableBorderStrong]      = ImVec4(0.31f, 0.31f, 0.45f, 1.00f);   // Prefer using Alpha=1.0 here
    // colors[ImGuiCol_TableBorderLight]       = ImVec4(0.26f, 0.26f, 0.28f, 1.00f);   // Prefer using Alpha=1.0 here
    // colors[ImGuiCol_TableRowBg]             = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    // colors[ImGuiCol_TableRowBgAlt]          = ImVec4(1.00f, 1.00f, 1.00f, 0.07f);
    // colors[ImGuiCol_TextLink]               = colors[ImGuiCol_HeaderActive];
    // colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.00f, 0.00f, 1.00f, 0.35f);
    // colors[ImGuiCol_TreeLines]              = colors[ImGuiCol_Border];
    // colors[ImGuiCol_DragDropTarget]         = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
    // colors[ImGuiCol_DragDropTargetBg]       = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    // colors[ImGuiCol_UnsavedMarker]          = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
    // colors[ImGuiCol_NavCursor]              = colors[ImGuiCol_HeaderHovered];
    // colors[ImGuiCol_NavWindowingHighlight]  = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    // colors[ImGuiCol_NavWindowingDimBg]      = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    // colors[ImGuiCol_ModalWindowDimBg]       = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);

}
//------------------------------------------------------------------------------
void AppGui::ShowFileBrowser(){
    if (g_FileDialog.Draw()) {
        // LogFMT("File:{} Ext:{}", g_FileDialog.selectedFile, g_FileDialog.selectedExt);
        if (g_FileDialog.mSaveMode)
        {
            g_FileDialog.reset();
        } else {
            if (g_FileDialog.selectedExt == ".wav" ) {
                mWaveModule->loadWave(g_FileDialog.selectedFile);
            }


        }
    }
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

    // Console right after GuiGlue
    mConsole.OnCommand =  [&](ImConsole* console, const char* cmd) { OnConsoleCommand(console, cmd); };
    SDL_SetLogOutputFunction(ConsoleLogFunction, this);


    // apply custom Theme
    ApplyStudioTheme();

    mBackground = new FluxRenderObject(getMain()->loadTexture(getGamePath()+"assets/back.bmp"));
    if (mBackground) {
        mBackground->setPos(getMain()->getScreen()->getCenterF());
        mBackground->setSize(getMain()->getScreen()->getScreenSize());
        getMain()->queueObject(mBackground);
    }




    mSoundMixModule  = new SoundMixModule();
    if (!mSoundMixModule ->Initialize())
        return false;


    mWaveModule = new WaveModule();
    if (!mWaveModule->Initialize())
        return false;
    getMain()->queueObject(mWaveModule); //we need update here ..


    mInputModule = new InputModule();
    if (!mInputModule->Initialize())
        return false;


    g_FileDialog.init( getGamePath(), {".rack",".drum", ".wav" });



    return true;
}
//------------------------------------------------------------------------------
void AppGui::Deinitialize()
{
    SDL_SetLogOutputFunction(nullptr, nullptr); // log must be unlinked first!! 
    // getMain()->unQueueObject(mInputModule);
    getMain()->unQueueObject(mWaveModule);
    SAFE_DELETE(mWaveModule);
    SAFE_DELETE(mSoundMixModule);
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


//------------------------------------------------------------------------------

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
            ImGui::TextDisabled("Modules");
            ImGui::MenuItem("Rack", NULL, &mAppSettings.mShowRack);
            ImGui::MenuItem("Visualizer", NULL, &mAppSettings.mShowVisualizer);

            ImGui::MenuItem("Drum Kit", NULL, &mAppSettings.mShowDrumKit);
            ImGui::MenuItem("Drum Pads", NULL, &mAppSettings.mShowDrumEffects);

            ImGui::Separator();
            ImGui::TextDisabled("Tools");
            ImGui::MenuItem("Wave Files", NULL, &mAppSettings.mShowWaveModule);
            ImGui::MenuItem("File Browser", NULL, &mAppSettings.mShowFileBrowser);
            ImGui::MenuItem("Console", NULL, &mAppSettings.mShowConsole);


            ImGui::Separator();
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Style"))
        {

            if (ImGui::MenuItem("Studio")) {ApplyStudioTheme(); }
            if (ImGui::MenuItem("Dark")) {ImGui::StyleColorsDark(); }
            if (ImGui::MenuItem("Light")) {ImGui::StyleColorsLight(); }
            if (ImGui::MenuItem("Classic")) {ImGui::StyleColorsClassic(); }
            ImGui::EndMenu();
        }
        ImFlux::drawWindowMenu();

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

        //................


        // -----------
        ImGui::EndMainMenuBar();
    }
    ShowToolbar();
}
//------------------------------------------------------------------------------
void AppGui::DrawGui()
{

    mGuiGlue->DrawBegin();


    ShowMenuBar();
    if (mAppSettings.mShowFileBrowser) ShowFileBrowser();
    if (mAppSettings.mShowConsole) mConsole.Draw("Console", &mAppSettings.mShowConsole);

    if (mAppSettings.mShowWaveModule ) mWaveModule->DrawWaveStreams();

    //FIXME settings
    mInputModule->DrawInputModuleUI();



    mSoundMixModule->DrawVisualAnalyzer( &mAppSettings.mShowVisualizer);
    mSoundMixModule->DrawRack( &mAppSettings.mShowRack);
    mSoundMixModule->DrawDrums(&mAppSettings.mShowDrumEffects /*&getMain()->getAppSettings()->mShowDrumEffects*/);
    mSoundMixModule->mDrumKitLooper.DrawUI(&mAppSettings.mShowDrumKit);



    DrawMsgBoxPopup();
    InitDockSpace();
    mGuiGlue->DrawEnd();
}
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
    ImGuiID dock_id_left, dock_id_right, dock_id_central, dock_id_top;

    dock_id_left = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, 0.215f, nullptr, &dock_main_id);
    dock_id_right = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 0.172f, nullptr, &dock_id_central);

    // dock_id_top = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Up, 0.215f, nullptr, &dock_main_id);


    // Dock the Windows to these IDs
    ImGui::DockBuilderDockWindow("Toolbar", dock_id_right);

    ImGui::DockBuilderDockWindow("File Browser", dock_id_right);
    // ImGui::DockBuilderDockWindow("Sound Effects Generator", dock_id_central);


    ImGui::DockBuilderFinish(dockspace_id);

    // we tile the rest ...
    ImFlux::TileWindows();
}
//------------------------------------------------------------------------------
