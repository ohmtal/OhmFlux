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
        if (ImFlux::ButtonFancy("Rack Presets") ) { mAppSettings.mShowRackPresets = ! mAppSettings.mShowRackPresets; }

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
    const ImVec4 titles_col  = ImVec4(0.00f, 0.25f, 0.25f, 1.00f); // titles
    const ImVec4 hooverActive_col    = ImVec4(0.00f, 0.45f, 0.45f, 1.00f); // hover/ active
    const ImVec4 hoover2_col         = ImVec4(0.00f, 0.60f, 0.60f, 1.00f); // Hover-Cyan
    const ImVec4 hoover3_col         = ImVec4(0.00f, 0.80f, 0.80f, 1.00f); // actice Cyan
    const ImVec4 dark_bg     = ImVec4(0.10f, 0.10f, 0.12f, 0.80f); // nearly black
    const ImVec4 dark_surface = ImVec4(0.16f, 0.16f, 0.18f, 1.00f); // panels

    const ImVec4 meddark_surface = ImVec4(0.26f, 0.26f, 0.28f, 1.00f); // menu



    // --- UI  ---
    colors[ImGuiCol_TitleBg]                = dark_surface; //cyan_lower;


    colors[ImGuiCol_Text]                   = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
    colors[ImGuiCol_WindowBg]               = dark_bg;
    colors[ImGuiCol_ChildBg]                = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_PopupBg]                = dark_surface;
    colors[ImGuiCol_Border]                 = ImVec4(0.30f, 0.30f, 0.30f, 0.50f);

    // Menu:
    colors[ImGuiCol_MenuBarBg]              = meddark_surface;

    // Header (CollapsingHeader, TreeNodes)
    colors[ImGuiCol_Header]                 = dark_surface;
    colors[ImGuiCol_HeaderHovered]          = hoover2_col;
    colors[ImGuiCol_HeaderActive]           = hoover3_col;

    // Buttons
    colors[ImGuiCol_Button]                 = ImVec4(0.20f, 0.20f, 0.22f, 1.00f);
    colors[ImGuiCol_ButtonHovered]          = hooverActive_col;
    colors[ImGuiCol_ButtonActive]           = hoover2_col;

    // Frame (Checkbox, Input, Slider )
    colors[ImGuiCol_FrameBg]                = ImVec4(0.05f, 0.05f, 0.05f, 1.00f);
    colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.20f, 0.20f, 0.22f, 1.00f);
    colors[ImGuiCol_FrameBgActive]          = hooverActive_col;

    // Tabs
    colors[ImGuiCol_Tab]                    = titles_col;
    colors[ImGuiCol_TabHovered]             = hooverActive_col;
    colors[ImGuiCol_TabActive]              = hoover2_col;

    colors[ImGuiCol_TabDimmed]              = ImLerp(colors[ImGuiCol_Tab],          colors[ImGuiCol_TitleBg], 0.80f);
    colors[ImGuiCol_TabDimmedSelected]      = ImLerp(colors[ImGuiCol_TabSelected],  colors[ImGuiCol_TitleBg], 0.40f);
    colors[ImGuiCol_TabDimmedSelectedOverline] = ImVec4(0.53f, 0.53f, 0.87f, 0.00f);


    // Akzente
    colors[ImGuiCol_CheckMark]              = hoover3_col;
    colors[ImGuiCol_SliderGrab]             = hooverActive_col;
    colors[ImGuiCol_SliderGrabActive]       = hoover3_col;
    colors[ImGuiCol_TitleBgActive]          = hooverActive_col;

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
    float vol = SettingsManager().get("SoundMix::MasterVolume", 1.f);
    mSoundMixModule->setMasterVolume(vol);



    mWaveModule = new WaveModule();
    if (!mWaveModule->Initialize())
        return false;
    getMain()->queueObject(mWaveModule); //we need update here ..


    mInputModule = new InputModule();
    if (!mInputModule->Initialize())
        return false;

    std::string emptyStr = "";
    std::string lInputRackSettings = SettingsManager().get("InputRackSettings", emptyStr);
    if (!lInputRackSettings.empty()) mInputModule->setInputEffectsSettingsBase64(lInputRackSettings);

    mRackModule = new RackModule();
    if (!mRackModule->Initialize())
        return false;

    mKeyBoardModule = new KeyBoardModule();
    if (!mKeyBoardModule->Initialize())
        return false;

    mDrumKitLooperModule = new DrumKitLooperModule();
    if (!mDrumKitLooperModule->Initialize())
        return false;


    g_FileDialog.init( getGamePath(), {".rack",".drum", ".wav" });



    return true;
}
//------------------------------------------------------------------------------
void AppGui::Deinitialize()
{
    if (SettingsManager().IsInitialized()) {
        SettingsManager().set("AppGui::mAppSettings", mAppSettings);
        SettingsManager().set("InputRackSettings", mInputModule->getInputEffectsSettingsBase64());
        SettingsManager().set("SoundMix::MasterVolume", mSoundMixModule->getMasterVolume());
        SettingsManager().save();
    }

    SDL_SetLogOutputFunction(nullptr, nullptr); // log must be unlinked first!! 
    getMain()->unQueueObject(mWaveModule);
    SAFE_DELETE(mWaveModule);

    SAFE_DELETE(mKeyBoardModule);
    SAFE_DELETE(mDrumKitLooperModule);
    SAFE_DELETE(mInputModule);
    SAFE_DELETE(mRackModule);
    SAFE_DELETE(mSoundMixModule);

    SAFE_DELETE(mGuiGlue);


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
            ImGui::SeparatorText("Modules");
            ImGui::MenuItem("Rack", NULL, &mAppSettings.mShowRack);
            ImGui::MenuItem("Rack Presets", NULL, &mAppSettings.mShowRackPresets);

            ImGui::MenuItem("Visualizer", NULL, &mAppSettings.mShowVisualizer);

            ImGui::MenuItem("Drum Kit", NULL, &mAppSettings.mShowDrumKit);
            ImGui::MenuItem("Drum Pads", NULL, &mAppSettings.mShowDrumEffects);


            ImGui::SeparatorText("Tools");
            ImGui::MenuItem("Wave Files", NULL, &mAppSettings.mShowWaveModule);
            ImGui::MenuItem("File Browser", NULL, &mAppSettings.mShowFileBrowser);
            ImGui::MenuItem("Console", NULL, &mAppSettings.mShowConsole);


            ImGui::Separator();
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Actions"))
        {
            ImGui::SeparatorText("Rack");
            if (ImGui::MenuItem("Switch Rack", "SPACE")) getMain()->getAppGui()->getRackModule()->getManager()->switchRack();

            ImGui::SeparatorText("Input Line");
            if (ImGui::MenuItem("Open Input", "F1")) getMain()->getAppGui()->getInputModule()->open();
            if (ImGui::MenuItem("Close Input", "ESC")) getMain()->getAppGui()->getInputModule()->close();

            ImGui::SeparatorText("Drums");
            if (ImGui::MenuItem("Toggle Active", "F5")) getMain()->getAppGui()->getDrumKitLooperModule()->toogleDrumKit();

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Style"))
        {

            if (ImGui::MenuItem("Studio")) {ApplyStudioTheme(); }
            if (ImGui::MenuItem("Dark")) {ImGui::StyleColorsDark(); }
            if (ImGui::MenuItem("Light")) {ImGui::StyleColorsLight(); }
            if (ImGui::MenuItem("Classic")) {ImGui::StyleColorsClassic(); }
            ImGui::SeparatorText("Layout");
            if (ImGui::MenuItem("Restore Factory Layout")) { restoreLayout(); }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Help"))
        {

            if (ImGui::MenuItem("About")) {
                showMessage ( "About",
                 std::format(
                        "Ohmtal Axe\n"
                        "==========\n"
                        "(c)2026 by Thomas Hühn (XXTH)\n"
                        "Version {}\n"
                        "https://ohmtal.com\n"
                        "\n"
                        "Settings are saved to:\n"
                        "{}\n"
                        , getGame()->mSettings.Version
                        , getGame()->mSettings.getPrefsPath()
                    )
                );

            }
            ImGui::EndMenu();
        }



        ImFlux::drawWindowMenu();

        // // ----------- Master Volume
        float rightOffset = 230.0f;
        ImGui::SameLine(ImGui::GetWindowWidth() - rightOffset);

        // ImGui::SetNextItemWidth(100);
        float currentVol = mSoundMixModule->getMasterVolume();
        // if (ImGui::SliderFloat("##MasterVol", &currentVol, 0.0f, 1.0f, "Vol %.1f"))
        if (ImFlux::FaderHWithText("Master Volume", &currentVol, 0.0f, 1.0f, "Vol %.1f"))
        {
            mSoundMixModule->setMasterVolume(currentVol);
        }
        // if (ImGui::IsItemHovered()) ImGui::SetTooltip("Master Volume");

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

    getDrumKitLooperModule()->DrawUI(&mAppSettings.mShowDrumKit); //before soundmix!
    mSoundMixModule->DrawDrums(&mAppSettings.mShowDrumEffects /*&getMain()->getAppSettings()->mShowDrumEffects*/);

    mRackModule->DrawEffectManagerPresetListWindow(&mAppSettings.mShowRackPresets);
    mRackModule->DrawRack( &mAppSettings.mShowRack);


    DrawMsgBoxPopup();
    InitDockSpace();
    mGuiGlue->DrawEnd();
}
//------------------------------------------------------------------------------
void AppGui::onKeyEvent(SDL_KeyboardEvent event)
{
    mKeyBoardModule->onKeyEvent(event);
}
//------------------------------------------------------------------------------
void AppGui::InitDockSpace()
{
    if (mAppSettings.mEditorGuiInitialized)
        return; 

    // "[Window][File Browser]\nCollapsed=0\nDockId=0x00000002\n\n[Window][WindowOverViewport_11111111]\nPos=0,19\nSize=1920,996\nCollapsed=0\n\n[Window][Toolbar]\nPos=0,19\nSize=429,251\nCollapsed=0\nDockId=0x00000003,0\n\n[Window][Input Stream]\nPos=0,272\nSize=429,743\nCollapsed=0\nDockId=0x00000004,0\n\n[Window][Distortion]\nPos=431,19\nSize=412,170\nCollapsed=0\nDockId=0x0000000F,0\n\n[Window][OverDrive]\nPos=431,191\nSize=412,180\nCollapsed=0\nDockId=0x00000012,0\n\n[Window][Metal Distortion]\nPos=431,373\nSize=412,177\nCollapsed=0\nDockId=0x00000013,0\n\n[Window][Analog Glow]\nPos=431,552\nSize=412,241\nCollapsed=0\nDockId=0x00000009,0\n\n[Window][CHORUS / ENSEMBLE]\nPos=431,795\nSize=412,220\nCollapsed=0\nDockId=0x0000000A,0\n\n[Window][REVERB / SPACE]\nPos=845,19\nSize=307,220\nCollapsed=0\nDockId=0x00000018,0\n\n[Window][DELAY]\nPos=845,241\nSize=307,774\nCollapsed=0\nDockId=0x00000019,0\n\n[Window][9-BAND EQUALIZER]\nPos=0,19\nSize=429,251\nCollapsed=0\nDockId=0x00000003,1\n\n[Window][Post Digital Sound Effects Visualizer]\nPos=1049,420\nSize=871,595\nCollapsed=0\nDockId=0x00000008,0\n\n[Window][Drum Kit]\nPos=1355,19\nSize=565,533\nCollapsed=0\nDockId=0x00000011,0\n\n[Window][Debug##Default]\nPos=60,60\nSize=400,400\nCollapsed=0\n\n[Window][Post Digital Sound Effects Rack]\nPos=1188,340\nSize=732,675\nCollapsed=0\nDockId=0x00000008,1\n\n[Window][Effects Rack]\nPos=431,19\nSize=616,996\nCollapsed=0\nDockId=0x00000017,2\n\n[Window][Effects Rack 80th]\nPos=431,19\nSize=616,996\nCollapsed=0\nDockId=0x00000017,1\n\n[Window][Effect Paddles]\nPos=431,19\nSize=616,996\nCollapsed=0\nDockId=0x00000017,0\n\n[Window][Visualizer]\nPos=1355,554\nSize=565,461\nCollapsed=0\nDockId=0x0000001A,1\n\n[Window][Rack]\nPos=431,19\nSize=922,996\nCollapsed=0\nDockId=0x00000017,0\n\n[Window][Console]\nPos=1355,554\nSize=565,461\nCollapsed=0\nDockId=0x0000001A,0\n\n[Window][About]\nPos=833,424\nSize=254,166\nCollapsed=0\n\n[Docking][Data]\nDockSpace               ID=0x08BD597D Window=0x1BBC0F80 Pos=0,19 Size=1920,996 Split=X\n  DockNode              ID=0x0000000B Parent=0x08BD597D SizeRef=429,996 Split=Y Selected=0x5FDD3067\n    DockNode            ID=0x00000003 Parent=0x0000000B SizeRef=276,251 Selected=0x0C01D6D5\n    DockNode            ID=0x00000004 Parent=0x0000000B SizeRef=276,743 Selected=0x5FDD3067\n  DockNode              ID=0x0000000C Parent=0x08BD597D SizeRef=1489,996 Split=X\n    DockNode            ID=0x00000001 Parent=0x0000000C SizeRef=412,990\n    DockNode            ID=0x00000002 Parent=0x0000000C SizeRef=1506,990 Split=X\n      DockNode          ID=0x00000005 Parent=0x00000002 SizeRef=922,996 Split=X\n        DockNode        ID=0x0000000D Parent=0x00000005 SizeRef=412,996 Split=Y Selected=0xA2258052\n          DockNode      ID=0x0000000F Parent=0x0000000D SizeRef=319,170 Selected=0xA2258052\n          DockNode      ID=0x00000010 Parent=0x0000000D SizeRef=319,824 Split=Y Selected=0x1423D8F6\n            DockNode    ID=0x00000014 Parent=0x00000010 SizeRef=319,359 Split=Y Selected=0x1F6C469F\n              DockNode  ID=0x00000012 Parent=0x00000014 SizeRef=319,180 Selected=0x1423D8F6\n              DockNode  ID=0x00000013 Parent=0x00000014 SizeRef=319,177 Selected=0x1F6C469F\n            DockNode    ID=0x00000015 Parent=0x00000010 SizeRef=319,463 Split=Y Selected=0xFD30D3DD\n              DockNode  ID=0x00000009 Parent=0x00000015 SizeRef=412,241 Selected=0xFD30D3DD\n              DockNode  ID=0x0000000A Parent=0x00000015 SizeRef=412,220 Selected=0xDAC16BBC\n        DockNode        ID=0x0000000E Parent=0x00000005 SizeRef=341,996 Split=X\n          DockNode      ID=0x00000016 Parent=0x0000000E SizeRef=307,996 Split=Y Selected=0x236930EC\n            DockNode    ID=0x00000018 Parent=0x00000016 SizeRef=319,220 Selected=0x53D4BCAC\n            DockNode    ID=0x00000019 Parent=0x00000016 SizeRef=319,774 Selected=0x236930EC\n          DockNode      ID=0x00000017 Parent=0x0000000E SizeRef=32,996 CentralNode=1 Selected=0x61C337AE\n      DockNode          ID=0x00000006 Parent=0x00000002 SizeRef=565,996 Split=Y Selected=0xCD6D3427\n        DockNode        ID=0x00000007 Parent=0x00000006 SizeRef=550,399 Split=Y Selected=0xCD6D3427\n          DockNode      ID=0x00000011 Parent=0x00000007 SizeRef=871,362 Selected=0xCD6D3427\n          DockNode      ID=0x0000001A Parent=0x00000007 SizeRef=871,313 Selected=0x5C1B5396\n        DockNode        ID=0x00000008 Parent=0x00000006 SizeRef=550,595 Selected=0x8ECB2A60\n\n"

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
