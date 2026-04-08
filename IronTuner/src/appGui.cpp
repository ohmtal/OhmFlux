//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// RadioWana
//-----------------------------------------------------------------------------
#include "appGui.h"
#include "appMain.h"
#include "utils/fluxSettingsManager.h"
#include "gui/fonts/HackNerdFontPropo-Regular.h"
#include "utils/errorlog.h"
#include "utils/fluxStr.h"
#include <algorithm>

#include "imgui_internal.h"
#include <gui/ImFlux/widets/VirtualTapePlayer.h>

namespace IronTuner {
    // -----------------------------------------------------------------------------
    // Console handling
    // -----------------------------------------------------------------------------
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
        gui->mConsole.AddLog("%s", FluxStr::removePart(message,"\r\n").c_str());
    }

    // -----------------------------------------------------------------------------
    bool AppGui::ConnectCurrent() {
        if (!FluxNet::NetTools::isValidURL(getMain()->getAppSettings().CurrentStation.url) ) {
            return false;
        }


        std::string url = getMain()->getAppSettings().CurrentStation.url;


        FluxNet::NetTools::URLParts parts = FluxNet::NetTools::parseURL(url);

        // FIXME ANDROID SSL but without is better anyway WHY must be a radio station stream encrypted ?
        if (isAndroidBuild())
        {
            if (parts.protocol  == "https" ) {
                url = "http://" + parts.hostname + "/" + parts.path;
            }
        }
        dLog("[error] protocol is: %s final url is: %s",parts.protocol.c_str(), url.c_str() );


        mStreamHandler->Execute(url);
        if (!getMain()->getAppSettings().CurrentStation.stationuuid.empty()) mRadioBrowser->clickStation(getMain()->getAppSettings().CurrentStation.stationuuid);
        return true;
    }
    // -----------------------------------------------------------------------------
    void AppGui::Tune(FluxRadio::RadioStation station) {
        getMain()->getAppSettings().CurrentStation = station;

        mStations.addCache(station);
        ConnectCurrent();
        mStations.setFavId(-1);
        mTuningMode = false;
        mReconnectOnTimeOutCount = 0;
    }


    // -----------------------------------------------------------------------------
    void AppGui::restoreLayout(){
        //copied from json :P
        //FIXME !!!
        static const std::string layout = "[Window][WindowOverViewport_11111111]\nPos=0,26\nSize=1152,622\nCollapsed=0\n\n[Window][Debug##Default]\nPos=60,60\nSize=400,400\nCollapsed=0\n\n[Window][About]\nPos=774,393\nSize=372,228\nCollapsed=0\n\n[Window][Console]\nPos=0,310\nSize=714,338\nCollapsed=0\nDockId=0x00000006,0\n\n[Window][Radio Browser]\nPos=708,26\nSize=444,622\nCollapsed=0\nDockId=0x00000002,0\n\n[Window][RadioWana]\nPos=0,26\nSize=714,622\nCollapsed=0\nDockId=0x00000005,0\n\n[Window][Recorder]\nPos=0,22\nSize=960,993\nCollapsed=0\nDockId=0x00000005,1\n\n[Window][Favorites]\nPos=708,26\nSize=444,622\nCollapsed=0\nDockId=0x00000002,1\n\n[Window][ImFlux ShowCase Widgets]\nPos=0,432\nSize=960,583\nCollapsed=0\nDockId=0x00000004,0\n\n[Window][HTTP Errror]\nPos=840,465\nSize=240,84\nCollapsed=0\n\n[Window][HUHU]\nPos=892,465\nSize=136,84\nCollapsed=0\n\n[Window][Favourite Dialog]\nPos=60,60\nSize=376,138\nCollapsed=0\n\n[Window][Favorite Dialog]\nPos=772,436\nSize=376,162\nCollapsed=0\n\n[Window][Stream Errror 56]\nPos=782,465\nSize=356,84\nCollapsed=0\n\n[Window][Stream Errror 0]\nPos=823,465\nSize=273,84\nCollapsed=0\n\n[Window][##MySidebar]\nSize=36,510\nCollapsed=0\n\n[Window][Stream Errror 1]\nPos=869,465\nSize=182,84\nCollapsed=0\n\n[Window][Stream Errror 52]\nPos=765,465\nSize=389,84\nCollapsed=0\n\n[Window][Radio]\nPos=0,26\nSize=706,622\nCollapsed=0\nDockId=0x00000005,0\n\n[Table][0x5B6633BA,5]\nColumn 0  Weight=1.0000\nColumn 1  Weight=1.0000\nColumn 2  Weight=1.0000\nColumn 3  Weight=1.0000\nColumn 4  Weight=1.0000\n\n[Table][0xD170F5FA,4]\nRefScale=16\nColumn 0  Width=20\nColumn 1  Weight=1.0000\nColumn 2  Width=74 Sort=0^\nColumn 3  Width=41\n\n[Table][0xC55E50B6,2]\nRefScale=16\nColumn 0  Width=20\nColumn 1  Weight=1.0000 Sort=0^\n\n[Docking][Data]\nDockSpace       ID=0x08BD597D Window=0x1BBC0F80 Pos=0,26 Size=1152,622 Split=Y\n  DockNode      ID=0x00000003 Parent=0x08BD597D SizeRef=960,408 Split=X Selected=0xCC2F45C2\n    DockNode    ID=0x00000001 Parent=0x00000003 SizeRef=706,484 Split=Y Selected=0xCC2F45C2\n      DockNode  ID=0x00000005 Parent=0x00000001 SizeRef=714,282 CentralNode=1 Selected=0xDB489985\n      DockNode  ID=0x00000006 Parent=0x00000001 SizeRef=714,338 Selected=0xEA83D666\n    DockNode    ID=0x00000002 Parent=0x00000003 SizeRef=444,484 Selected=0xB58DAB73\n  DockNode      ID=0x00000004 Parent=0x08BD597D SizeRef=960,583 Selected=0xF2A39ADC\n\n";

        FluxRadio::RadioStation savStation = getMain()->getAppSettings().CurrentStation;
        getMain()->getAppSettings() = AppSettings();
        getMain()->getAppSettings().CurrentStation = savStation;
        getMain()->getAppSettings().UIInitialized = true;

        // must be scheduled !!
        static FluxScheduler::TaskID loadFactorySchedule = 0;
        if (!FluxSchedule.isPending(loadFactorySchedule))
        {
            std::string tmpLayout = layout;
            loadFactorySchedule = FluxSchedule.add(0.5f, nullptr, [tmpLayout]() {
                ImGui::LoadIniSettingsFromMemory(tmpLayout.c_str(), tmpLayout.size());
            });
        }
    }
    // -----------------------------------------------------------------------------
    void AppGui::InitDockSpace(){
        //FIXME CARUSEL
        // if (getMain()->getAppSettings().DockSpaceInitialized) return;
        // getMain()->getAppSettings().DockSpaceInitialized = true;
        // restoreLayout();
    }
    // -----------------------------------------------------------------------------
    void AppGui::OnConsoleCommand(ImConsole* console, const char* cmdline){
        std::string cmd = FluxStr::getWord(cmdline,0);

        if (cmd == "fl" )  {
            float limit = std::stof (FluxStr::getWord(cmdline,1));
            getMain()->mSettings.frameLimiter = limit;
            dLog("Setting Framelimit to %f (current fps: %d)", limit, getMain()->getFPS());

            // app->mSettings.frameLimiter = 32.f;
        }

        if (cmd == "testes") getMain()->getBackGroundRenderEffect()->mShaderESTesting = true;

        if (cmd == "sc" ) {  //STATION CACHE
            mStations.DumpStationCache();
        }

        if (cmd == "dd")  {  //DECODE DEBUG
            mAudioHandler->decoderDebug();
        }

        if (cmd == "rl") {
            int id = std::atoi(FluxStr::getWord(cmdline,1).c_str());
            bool scanLines = std::atoi(FluxStr::getWord(cmdline,2).c_str());
            dLog("shader id is: %d", id);
            getMain()->reloadBackGroundEffectsShader( id , scanLines);
        }


        if (cmd == "contenttype") {
            Log("%s", FluxNet::NetTools::getHeaderValue(mStreamHandler->getHeader(), "Content-Type").c_str());
        }
        if (cmd == "desc") {
            Log("%s", FluxNet::NetTools::getHeaderValue(mStreamHandler->getHeader(), "icy-description").c_str());
        }
        // window information
        if (cmd == "window") {
            SDL_Window* window = getScreenObject()->getWindow();

            bool maximized = getScreenObject()->getWindowMaximized();
            // setWindowMaximized

            // Window size
            int w, h;
            SDL_GetWindowSize(window, &w, &h);

            // Window position
            int posX, posY;
            SDL_GetWindowPosition(window, &posX, &posY);

            //----
            Log("Window size:%d,%d. maximized:%d, position:%d,%d",
                w,h,maximized,posX,posY);

        }

    }
    // -----------------------------------------------------------------------------
    void AppGui::ApplyStudioTheme(){

        ImGuiStyle& style = ImGui::GetStyle();
        ImVec4* colors = style.Colors;

        // --- Active Tab Colors ---
        // The tab that is currently selected AND has focus
        colors[ImGuiCol_TabActive]          = ImVec4(0.10f, 0.20f, 0.30f, 1.00f); //= ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        // The tab that is selected but another window has focus (the "Gray-out" fix)
        colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.08f, 0.08f, 0.10f, 1.00f); //= ImVec4(0.18f, 0.40f, 0.65f, 1.00f);

        // --- Inactive Tab Colors ---
        // Tabs in the background
        colors[ImGuiCol_Tab]                = ImVec4(0.12f, 0.12f, 0.14f, 1.00f);
        // Tabs in the background when the window is unfocused
        colors[ImGuiCol_TabUnfocused]       = ImVec4(0.07f, 0.10f, 0.15f, 0.97f);

        // --- Interaction ---
        // When hovering over any tab
        colors[ImGuiCol_TabHovered]         = ImVec4(0.35f, 0.72f, 1.00f, 1.00f);

        // --- Background of the Tab Bar ---
        // This fills the area behind the tabs (useful if tabs don't fill the full width)
        colors[ImGuiCol_TitleBgActive]      = ImVec4(0.08f, 0.08f, 0.19f, 1.00f); //  = ImVec4(0.08f, 0.08f, 0.09f, 1.00f);



        const ImVec4 bgColor = ImVec4(0.1f, 0.1f, 0.1f, 0.85f);
        colors[ImGuiCol_WindowBg] = bgColor;
        colors[ImGuiCol_ChildBg] = bgColor;
        colors[ImGuiCol_TitleBg] = bgColor;


        style.WindowRounding = 6.0f;
        style.FrameRounding = 4.0f;

    }
    // -----------------------------------------------------------------------------
    void AppGui::DrawRecorder(){
        if (!mStreamHandler.get() || !mAudioHandler.get() || !mAudioHandler->getManager()) return;
        float fullHalfWidth = ImGui::GetContentRegionAvail().x / 2.f;
        const float radioHalfWidth = 320.f * getScale(); //FIXME !!

        ImFlux::GradientBox(ImVec2(0.f, 180.f * getScale()));
        ImGui::BeginGroup(/*RECORDER*/);

        ImGui::PushFont(getMain()->mHackNerdFont16);
        ImFlux::ShadowText("RECORDER");
        ImGui::PopFont();
        ImGui::Separator();

        if (fullHalfWidth - radioHalfWidth > 0.f) {
            ImFlux::ShiftCursor(ImVec2(fullHalfWidth - radioHalfWidth, 0.f));
        }

        ImFlux::ShiftCursor(ImVec2(5.f,5.f));

        bool recordAndWrite = mRecording && mAudioRecorder->isFileOpen();

        // 5.1
        static ImFlux::VirtualTapePlayer tapePlayer;
        tapePlayer.type = ImFlux::CassetteType::Chrome;
        tapePlayer.size.x = 220 * getScale();
        tapePlayer.mode = recordAndWrite ? ImFlux::CassetteMode::Record : ImFlux::CassetteMode::Stop;
        if (recordAndWrite) {
            tapePlayer.label = mAudioHandler->getCurrentTitle();
        } else {
            tapePlayer.label = "";
        }
        tapePlayer.Draw();


        ImGui::SameLine();

        // 5.2
        //TODO: Options
        if (ImGui::BeginChild("RECORDSETTINGS", ImVec2(0.f,110.f * getScale()), ImGuiChildFlags_NavFlattened)) {
            // float fullWidth = ImGui::GetContentRegionAvail().x;

            ImGui::SeparatorText("Recording");

            ImGui::Checkbox("Wait for new Title.", &mRecordingStartsOnNewTile);
            if (!mStreamHandler->isConnected()) ImGui::BeginDisabled();

            // if (ImFlux::LEDCheckBox("Enable Recording", &mRecording, ImVec4(0.8f,0.3f,0.3f,1.f))) {
            if (ImGui::Checkbox("Enable Recording", &mRecording)) {
                if (mRecording && !mRecordingStartsOnNewTile && !mAudioHandler->getCurrentTitle().empty()) {
                    mAudioRecorder->openFile(mAudioHandler->getCurrentTitle());
                }
                if (!mRecording)
                    mAudioRecorder->closeFile();
            }
            if (!mStreamHandler->isConnected()) ImGui::EndDisabled();

            if (mRecording) {
                ImFlux::DrawLED("Recording", mAudioRecorder->isFileOpen(), ImFlux::LED_GREEN_ANIMATED_GLOW);
                ImGui::SameLine();
                ImGui::Text("File: %s", mAudioRecorder->getCurrentFilename().c_str());
            }
        }
        ImGui::EndChild();
        ImGui::EndGroup(/*RECORDER*/);
    }
    // -----------------------------------------------------------------------------
    void AppGui::DrawEqualizer(){
        if (!mStreamHandler.get() || !mAudioHandler.get() || !mAudioHandler->getManager()) return;
        float fullHalfWidth = ImGui::GetContentRegionAvail().x / 2.f;
        const float radioHalfWidth = 320.f * getScale(); //FIXME !!

        ImFlux::GradientBox(ImVec2(0.f, 145.f * getScale()));

        ImGui::BeginGroup(/*EQ*/);
        ImGui::PushFont(getMain()->mHackNerdFont16);
        ImFlux::ShadowText("EQUALIZER");
        ImGui::PopFont();
        ImGui::Separator();
        if (fullHalfWidth - radioHalfWidth > 0.f) {
            ImFlux::ShiftCursor(ImVec2(fullHalfWidth - radioHalfWidth, 0.f));
        }

        ImFlux::ShiftCursor(ImVec2(5.f,5.f));

        const ImVec2 vuSize = {135* getScale(),70* getScale()} ;
        float dbL, dbR;
        auto mapDB = [](float db) {
            float minDB = -20.0f;
            return (db < minDB) ? 0.0f : (db - minDB) / (0.0f - minDB);
        };

        float cursorY = ImGui::GetCursorPosY();
        // ~~~ VU Meter LEFT ~~~
        if ( mAudioHandler->getManager() && mAudioHandler->getManager()->getVisualAnalyzer()) {
            if (!mStreamHandler->isConnected()) dbL = 0.f;
            else  dbL = mapDB(mAudioHandler->getManager()->getVisualAnalyzer()->getDecible(0));
            ImGui::SetCursorPosY(cursorY + 20.f);
            ImFlux::VUMeter70th(vuSize,dbL, "L",  gRadioDisplayBox.col_top, gRadioDisplayBox.col_bot);

        }
        ImGui::SameLine();

        ImGui::SetCursorPosY(cursorY);

        // ----- EQ9 ------------
        DSP::Equalizer9Band* effect = static_cast<DSP::Equalizer9Band*>(mAudioHandler->getManager()->getEffectByType(DSP::EffectType::Equalizer9Band));
        if (!effect) return;


        const float boxHeight = 110.f * getScale();
        const float boxWidth  = 380.f * getScale() ;
        const float sliderSpaceing = 12.f * getScale();
        DSP::Equalizer9BandSettings currentSettings = effect->getSettings();

        const float sliderWidth = 25.f * getScale();
        const float sliderHeight = 70.f * getScale();
        const std::string volStr = "VOL";

        ImGui::PushID(effect);
        bool changed = false;
        // if (effect->isEnabled())
        {
            if (ImGui::BeginChild("UI_Box", ImVec2(boxWidth, boxHeight), ImGuiChildFlags_NavFlattened)) {
                ImFlux::GradientBoxDL(gRadioDisplayBox.WithSize(ImVec2(boxWidth, boxHeight)) );
                ImFlux::ShiftCursor(ImVec2(5.f,5.f));

                //Volume
                ImGui::BeginGroup();
                float vol = mAudioHandler->getVolume();
                if (ImFlux::FaderVertical(volStr.c_str(), ImVec2(sliderWidth,sliderHeight), &vol,0.f,1.f )) {
                    mAudioHandler->setVolume(vol);
                    getMain()->getAppSettings().Volume = vol;
                }

                float textWidth = ImGui::CalcTextSize(volStr.c_str()).x;
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (sliderWidth - textWidth) * 0.5f);
                ImGui::TextUnformatted(volStr.c_str());

                ImGui::EndGroup();
                ImGui::SameLine(0, 0);

                ImFlux::SeparatorFancy(ImGuiSeparatorFlags_Vertical, 2.f,  0.f, sliderSpaceing / 1.5f);

                ImGui::BeginGroup();
                // Control Sliders
                int count = currentSettings.getAll().size();
                int i = 0;
                for (auto* param :currentSettings.getAll() ) {

                    changed |= param->FaderVWithText(sliderWidth,sliderHeight);

                    if (i < count) ImGui::SameLine(0, sliderSpaceing);
                    i++;
                }
                ImGui::EndGroup();
            }
            ImGui::EndChild();
        }
        if (ImGui::BeginPopupContextItem()) {
            ImGui::SeparatorText("Equalizer");
            changed |= currentSettings.drawStepper(currentSettings);
            if (ImFlux::FaderButton("Reset", ImVec2(40.f, 20.f)))  {
                currentSettings.resetToDefaults();
                effect->reset();
                changed = true;
            }


            ImGui::EndPopup();
        }



        // effect->renderUIFooter();
        ImGui::PopID();
        // return changed;

        if (changed) {
            effect->setSettings(currentSettings);
        }
        //--------- <<< EQ9

        ImGui::SameLine();
        if ( mAudioHandler->getManager() && mAudioHandler->getManager()->getVisualAnalyzer()) {
            if (!mStreamHandler->isConnected()) dbR = 0.f;
            else dbR = mapDB(mAudioHandler->getManager()->getVisualAnalyzer()->getDecible(0));
            ImGui::SetCursorPosY(cursorY + 20.f);
            ImFlux::VUMeter70th(vuSize,dbR, "R", gRadioDisplayBox.col_top, gRadioDisplayBox.col_bot);
        }
        ImGui::EndGroup(/*EQ*/);

    }

    // -----------------------------------------------------------------------------
    void AppGui::DrawInfoPopup(FluxRadio::StreamInfo* info) {
        if (ImGui::BeginPopup("##StationInfo")) {
            ImGui::PushFont(getMain()->mHackNerdFont20);
            ImGui::SeparatorText(info->name.c_str());
            ImGui::PopFont();
            ImFlux::TextColoredEllipsis(ImVec4(0.6f,0.6f,0.6f,1.f), info->streamUrl, 550.f);
            ImGui::Text("Description: %s", info->description.c_str());
            ImGui::Separator();
            ImFlux::ShadowText(mAudioHandler->getCurrentTitle().c_str(), ImColor(240,240,20));
            ImGui::TextDisabled("Next: %s", mAudioHandler->getNextTitle().c_str());
            ImGui::Separator();
            ImGui::Text("Audio: %d Hz, %d kbps, %d Channels", info->samplerate, info->bitrate, info->channels);
            if (!info->url.empty()) ImGui::Text("Url: %s", info->url.c_str());
            ImGui::EndPopup();
        }
    }
    // -----------------------------------------------------------------------------
    void AppGui::Update(const double& dt){

        if ( mAudioHandler->getManager()) {
            bool isConnected = mStreamHandler->isConnected();
            if (mAudioHandler->getManager()->getVisualAnalyzer()) {
                if (isConnected) {
                    mAudioLevels.x = mAudioHandler->getManager()->getVisualAnalyzer()->getLevel(0);
                    mAudioLevels.y = mAudioHandler->getManager()->getVisualAnalyzer()->getLevel(1);
                } else {
                    mAudioLevels = {0.f, 0.f};
                }
            }
        }

        mStations.update();



    }
    // -----------------------------------------------------------------------------

    void AppGui::DrawRadio() {

        if (!mStreamHandler.get() || !mAudioHandler.get()) return;

        bool isConnected = mStreamHandler->isConnected();

        bool isRunning   = mStreamHandler->isRunning();
        bool isConnecting =  isRunning && !isConnected;
        bool isOffline = !isConnected && !isRunning;


        FluxRadio::StreamInfo info = FluxRadio::StreamInfo();
        if ( mTuningMode ) {
            info.name = " * * * tuning * * * ";
        } else {
            if (isConnected && mStreamHandler->getStreamInfo()) info = *mStreamHandler->getStreamInfo();
            else {
                if (isRunning) info.name = " * * * connecting * * *          ";
                else if (isConnected) info.name = " * * *          ";
                else {
                    info.name = " * * * offline * * *          ";
                    if ( isAndroidBuild()) {
                        info.name += "..... a n d r o i d  b u i l d .....";
                    }

                }
            }

        }
        // if (ImGui::Begin("Radio", &getMain()->getAppSettings().ShowRadio, window_flags ))
        {
            float fullHalfWidth = ImGui::GetContentRegionAvail().x / 2.f;

            const int lcdDigits    = 30;
            const int lcdDigits2   = 38; //40;

            const float displayWidth =  (lcdDigits * 16.9f + 32.f) * getScale();
            const float displayHeight = 60.f * getScale();


            // maybe alternative deffered calc
            const float radioHalfWidth = (displayWidth + 48.f + 50.f ) / 2.f;


            // --------  INFO -----------

            ImFlux::GradientBox(ImVec2(0.f, displayHeight + 10.f));
            if (fullHalfWidth - radioHalfWidth > 0.f) {
                ImFlux::ShiftCursor(ImVec2(fullHalfWidth - radioHalfWidth, 0.f));
            }
            ImGui::BeginGroup(/*RADIO*/);
            ImFlux::ShiftCursor(ImVec2(5.f,5.f));
            ImVec2 CursorPos  =ImGui::GetCursorPos();

            if (ImGui::BeginChild("##RadioDisplayStation", ImVec2(displayWidth  ,displayHeight ), ImGuiChildFlags_NavFlattened)) {

                ImFlux::GradientBoxDL(gRadioDisplayBox.WithSize(ImVec2(displayWidth  ,displayHeight )) );

                ImGui::SameLine();ImFlux::ShiftCursor(ImVec2(5.f,6.f));
                ImGui::BeginGroup();
                ImGui::PushFont(getMain()->mHackNerdFont26);
                if (mTuningMode || isOffline) {
                    FluxRadio::RadioStation station;
                    if (mStations.cachedStationBySelectedIndex(station)) {
                        ImFlux::LCDTextScroller(station.name, lcdDigits, ImFlux::COL32_NEON_PURPLE);
                    }  else {
                        ImFlux::LCDTextScroller("*** failed to get station name ***", lcdDigits, ImFlux::COL32_NEON_RED);
                    }
                } else {
                    ImFlux::LCDTextScroller(mAudioHandler->getCurrentTitle(), lcdDigits, ImFlux::COL32_NEON_ORANGE);
                }
                ImGui::PopFont();

                {   // Info Button
                    ImGui::SameLine();

                    if (!isConnected) ImGui::BeginDisabled();
                    ImFlux::ShiftCursor(ImVec2(0,8));
                    if (ImFlux::ButtonFancy("I", gRadioButtonParams.WithColor(IM_COL32(88,88,88,88)).WithSize(ImVec2(24,24)*getScale() ))) {
                        ImGui::OpenPopup("##StationInfo");
                    }
                    if (!isConnected) ImGui::EndDisabled();
                    DrawInfoPopup(&info);
                }

                // ImGui::Spacing();
                ImGui::PushFont(getMain()->mHackNerdFont20);
                if ( mAudioHandler->getNextTitle().empty() ) {
                    ImFlux::LCDTextScroller(info.name, lcdDigits2, ImFlux::COL32_NEON_ELECTRIC);
                } else {
                    ImFlux::LCDTextScroller(mAudioHandler->getNextTitle(), lcdDigits2, ImFlux::COL32_NEON_GREEN);
                }
                ImGui::PopFont();

                {   // Favourite
                    ImGui::SameLine();
                    ImFlux::ShiftCursor(ImVec2(5,2));
                    bool isFavo = getMain()->getAppSettings().CurrentStation.favId > 0;
                    if (ImFlux::FavouriteStar("Favourite", isFavo, 8.f * getScale())) {
                        if (isFavo) {
                            mStations.RmvFavoByFavId(&getMain()->getAppSettings().CurrentStation);
                        } else {
                            mStations.AddFavo(&getMain()->getAppSettings().CurrentStation);
                        }
                    }

                }

                ImGui::EndGroup();
            }
            ImGui::EndChild();

            ImGui::SameLine();
            TuneKnob("Tune Station", ImFlux::DARK_KNOB.WithRadius(48.f * getScale()));

            ImGui::BeginGroup();
            {

                if ( mAudioHandler->getManager() && mAudioHandler->getManager()->getVisualAnalyzer()) {
                    ImGui::SetCursorPos(ImVec2(CursorPos.x, CursorPos.y + displayHeight + 15.f ));
                    ImGui::BeginGroup();
                    if (isConnecting) ImFlux::DrawLED("Connecting",isConnecting, ImFlux::LED_YELLOW);
                    else ImFlux::DrawLED("Connected",isConnected, ImFlux::LED_GREEN);
                    ImGui::SameLine();
                    ImFlux::VUMeter80th(mAudioLevels.x, 24, ImVec2(4.f, 16.f));
                    ImGui::SameLine();
                    ImFlux::VUMeter80th(mAudioLevels.y, 24, ImVec2(4.f, 16.f));
                    ImGui::EndGroup();
                }
            }
            ImGui::EndGroup();

            ImGui::EndGroup(/*RADIO*/);
        }
        // ImGui::End();
    }
    // -----------------------------------------------------------------------------
    void AppGui::DrawFavo() {
        static bool showDialog = false;
        static bool isEdit = false;
        static FluxRadio::RadioStation workStation;
        static uint32_t editId = 0;
        static bool showInfo = false;

        // if (ImGui::Begin("Favorites", &getMain()->getAppSettings().ShowFavo)){

            // ~~~ BUTTONS ~~~
            if (ImFlux::ButtonFancy("NEW")) {
                showDialog = true;
                isEdit = false;
                workStation = FluxRadio::RadioStation();
                workStation.url = "https://";
            }
            if (mStations.getFavId() == 0) ImGui::BeginDisabled();
            ImGui::SameLine();
            if (ImFlux::ButtonFancy("EDIT") ) {
                FluxRadio::RadioStation* pStation = mStations.getSelectedFavStation();
                if (pStation) {
                    editId = pStation->favId;
                    workStation = *pStation;
                    showDialog = true;
                    isEdit = true;
                } else {
                    Log("[error] FAV_EDIT: station is null pointer!!");
                }
            }
            ImGui::SameLine();
            if (ImFlux::ButtonFancy("INFO") ) {
                ImGui::OpenPopup("Station Info");
            }

            if (ImGui::BeginPopup("Station Info")) {
                FluxRadio::RadioStation* pStation = mStations.getSelectedFavStation();
                if (pStation) {
                    for (auto& line: pStation->dump(false)) {
                        ImGui::TextUnformatted(line.c_str());
                    }
                } else {
                    Log("[error] FAV_INFO: station is null pointer!!");

                }
                ImGui::EndPopup();
            }


            if (mStations.getFavId() == 0) ImGui::EndDisabled();

            // ~~~ LIST ~~~

            DrawStationsList(mStations.getFavoStationData(), true);
        // }
        // ImGui::End();


        if (showDialog ) {
            ImGui::OpenPopup("Favourite Dialog");

            if (ImGui::BeginPopupModal("Favourite Dialog", &showDialog, ImGuiWindowFlags_AlwaysAutoResize) ) {
                ImGui::SeparatorText(isEdit ? "Edit" : "New");
                //mhh what to use as
                char strBuff[256];
                strncpy(strBuff, workStation.name.c_str(), sizeof(strBuff));
                if (ImGui::InputText("Station Name",strBuff, sizeof(strBuff))) {
                    workStation.name = strBuff;
                }
                strncpy(strBuff, workStation.url.c_str(), sizeof(strBuff));
                if (ImGui::InputText("URL", strBuff, sizeof(strBuff))) {
                    workStation.url = strBuff;
                }
                ImFlux::SeparatorFancy();

                if (ImGui::Button("Save")) {
                    bool validated = false;
                    validated = !workStation.name.empty() && FluxNet::NetTools::isValidURL(workStation.url);
                    if (validated)
                    {
                        if (isEdit) {
                            FluxRadio::RadioStation* pStation = mStations.getStationByFavId(editId);
                            if (pStation) {
                                *pStation = workStation;
                            } else {
                                Log("[error] FAV_EDIT::SAVE => station is null pointer!!");
                            }
                        } else {
                            mStations.AddFavo(&workStation);
                        }
                        showDialog = false;
                    }
                }
                ImGui::SameLine();
                if (ImGui::Button("Cancel")) {
                    showDialog = false;
                }

                ImGui::EndPopup();
            }
        }

    }
    // -----------------------------------------------------------------------------
    void AppGui::DrawStationsList(const std::vector<FluxRadio::RadioStation> stations, const bool isFavoList ) {

        static char searchBuffer[128] = "";

        //---------------
        ImGui::BeginGroup();
        ImGui::SeparatorText("Filter results");
        ImGui::SetNextItemWidth( 150.f * getScale());
        ImGui::InputText("##Filter", searchBuffer, IM_ARRAYSIZE(searchBuffer));
        ImGui::EndGroup();
        //---------------
        static ImGuiTableFlags flags =
        ImGuiTableFlags_None
        | ImGuiTableFlags_BordersV
        | ImGuiTableFlags_BordersOuterH
        | ImGuiTableFlags_ScrollY
        // | ImGuiTableFlags_ScrollX
        | ImGuiTableFlags_RowBg
        | ImGuiTableFlags_Sortable
        | ImGuiTableFlags_Resizable
        ;


        int colCount = 4;
        if (isFavoList) colCount = 2;


        if (ImGui::BeginTable("RadioStations", colCount, flags)) {
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableSetupColumn("Favo", ImGuiTableColumnFlags_WidthFixed, 20.f);
            ImGui::TableSetupColumn("Station", ImGuiTableColumnFlags_WidthStretch, 0.f);
            if (!isFavoList) {
                ImGui::TableSetupColumn("Clicks", ImGuiTableColumnFlags_WidthFixed, 120.f);
                ImGui::TableSetupColumn("Bitrate", ImGuiTableColumnFlags_WidthFixed, 60.f);
            }
            ImGui::TableHeadersRow();

            std::vector<const FluxRadio::RadioStation*> displayList;
            std::string searchStr = FluxStr::toLower(searchBuffer);

            //FIXME case sensitive !!
            for (const auto& s : stations) {

                if (searchStr.empty() || FluxStr::toLower(s.name).find(searchStr) != std::string::npos) {
                    displayList.push_back(&s);
                }
            }

            if (ImGuiTableSortSpecs* sortSpecs = ImGui::TableGetSortSpecs()) {
                if (sortSpecs->SpecsDirty) {
                    std::sort(displayList.begin(), displayList.end(), [&](const FluxRadio::RadioStation* a, const FluxRadio::RadioStation* b) {
                        const ImGuiTableColumnSortSpecs& spec = sortSpecs->Specs[0];
                        bool ascending = (spec.SortDirection == ImGuiSortDirection_Ascending);

                        // Sortierung je nach Spalten-ID
                        switch (spec.ColumnIndex) {
                            case 1: return ascending ? (a->name < b->name) : (a->name > b->name);
                            case 2: return ascending ? (a->clickcount < b->clickcount) : (a->clickcount > b->clickcount);
                            case 3: return ascending ? (a->bitrate < b->bitrate) : (a->bitrate > b->bitrate);
                            default: return false;
                        }
                    });
                    // sortSpecs->SpecsDirty = false;
                }
            }


            ImGuiListClipper clipper;
            clipper.Begin(displayList.size());
            while (clipper.Step()) {
                for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++) {
                    const auto* station = displayList[row];

                    // for (const auto& station : stations ) {
                    ImGui::TableNextRow(ImGuiTableRowFlags_None, 20.f);
                    ImGui::PushID(station->stationuuid.c_str());

                    bool isSelected =false;
                    if (!isFavoList) isSelected = (mStations.getUuid() == station->stationuuid);
                    else isSelected  = ( mStations.getFavId() == station->favId);


                    // ~~~~~~~~~ FAVO ~~~~~~~~~~~~~~
                    ImGui::TableNextColumn();
                    bool isFavo = false;
                    if (isFavoList) isFavo = true;
                    else isFavo = mStations.isFavoStation(station);

                    if (ImFlux::FavouriteStar("Favourite", isFavo, 8.f * getScale())) {
                        if (isFavoList) {
                            // std::erase_if(mFavoStationData, [&](const FluxRadio::RadioStation& s) {
                            //     // return s.stationuuid == station->stationuuid;
                            //     return s.favId == station->favId;
                            // });
                            mStations.RmvFavoByFavId(station);
                        } else {
                            if (!isFavo) {
                                // mFavoStationData.push_back(*station);
                                // FluxRadio::updateFavIds(&mFavoStationData);
                                mStations.AddFavo(station);
                            } else {
                                // std::erase_if(mFavoStationData, [&](const FluxRadio::RadioStation& s) {
                                //     return s.stationuuid == station->stationuuid;
                                // });
                                mStations.RmvFavoByUUID(station);
                            }
                        }
                        SaveSettings();
                    }

                    // ~~~~~~~~~ Station ~~~~~~~~~~~~~~
                    ImGui::TableNextColumn();
                    ImGui::PushFont(getMain()->mHackNerdFont20);
                    if (ImGui::Selectable(station->name.c_str(), isSelected, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick)) {
                        if (!isFavoList)  mStations.setUuid( station->stationuuid );
                        else mStations.setFavId(station->favId);


                        if (ImGui::IsMouseDoubleClicked(0)) {
                            Tune(*station);
                        }
                    }
                    if (ImGui::IsItemFocused() && (ImGui::IsKeyPressed(ImGuiKey_Enter) || ImGui::IsKeyPressed(ImGuiKey_Space))) {
                        Tune(*station);
                    }

                    ImGui::PopFont();


                    if (!isFavoList) {
                        // ~~~~~~~~~ Clicks ~~~~~~~~~~~~~~
                        ImGui::TableNextColumn();
                        ImGui::Text("%d (%d)",station->clickcount, station->clicktrend);

                        // ~~~~~~~~~ Bitrate ~~~~~~~~~~~~~~
                        ImGui::TableNextColumn();
                        if (station->bitrate >= 128) {
                            ImGui::TextColored(ImColor(0, 255, 0), "%d", station->bitrate);
                        } else if (station->bitrate >= 64) {
                            ImGui::TextColored(ImColor(255, 255, 0), "%d", station->bitrate);
                        } else if (station->bitrate == 0) {
                            ImGui::TextDisabled("---");
                        } else {
                            ImGui::TextColored(ImColor(255, 0, 0), "%d", station->bitrate);
                        }
                    }


                    ImGui::PopID(/*station->stationuuid.c_str()*/);
                }

            }
            ImGui::EndTable();

        }

    }
    // -----------------------------------------------------------------------------
    void AppGui::DrawRadioBrowserWindow() {
        // if (ImGui::Begin("Radio Browser", &getMain()->getAppSettings().ShowRadioBrowser)){
            static char strBuff[64];

            ImGui::BeginGroup();
            ImGui::SeparatorText("radio-browser.info");
            strncpy(strBuff, mStations.getQueryString().c_str(), sizeof(strBuff));
            ImGui::SetNextItemWidth(150.f * getScale());
            if (ImGui::InputText("##SearchName", strBuff, sizeof(strBuff), ImGuiInputTextFlags_EnterReturnsTrue)) {
                mRadioBrowser->searchStationsByNameAndTag(strBuff, "");
                mStations.getQueryStringMutable() = strBuff;
                dLog("query for %s", strBuff);
            }
            ImFlux::Hint("Search Radio Station by Name");
            ImGui::EndGroup();
            ImFlux::ShiftCursor(ImVec2(30.f,0.f));
            ImGui::SameLine();

            DrawStationsList(mStations.getQueryStationData(), false);

        // }
        // ImGui::End();
    }
    // -----------------------------------------------------------------------------

    void AppGui::handleSwipe(float deltaX) {
        if (deltaX < -0.15f) changePage(1);
        if (deltaX > 0.15f)  changePage(-1);
    }

    void AppGui::changePage(int step) {
        int numPages = (int)mPages.size();
        if (numPages == 0) return;
        mTargetPageIndex = (mTargetPageIndex + step + numPages) % numPages;
    }

    std::string AppGui::getChangePageName(int step) {
        int numPages = (int)mPages.size();
        if (numPages == 0) return "invalid page";
        int id = (mTargetPageIndex + step + numPages) % numPages;
        return mPages[id].getCaption();
    }

    void AppGui::onEvent(SDL_Event event){
        if (mGuiGlue.get()) mGuiGlue->onEvent(event);

        // FIRE TV KEYS:
        // D-Pad Up        SDLK_UP
        // D-Pad Down      SDLK_DOWN
        // D-Pad Left      SDLK_LEFT
        // D-Pad Right     SDLK_RIGHT
        // Center (Select) SDLK_RETURN or SDLK_KP_ENTER
        // Back            SDLK_ESCAPE or SDLK_AC_BACK
        // Play/Pause      SDLK_MEDIA_PLAY_PAUSE
        // Menu            SDLK_MENU


        if (event.type == SDL_EVENT_KEY_DOWN) {
            if (event.key.scancode == SDL_SCANCODE_GRAVE) getMain()->getAppSettings().ShowConsole = !getMain()->getAppSettings().ShowConsole;
            if (
                event.key.key == SDLK_MENU
                || event.key.key == SDLK_F1
            ) {
                getMain()->getAppSettings().SideBarOpen = !getMain()->getAppSettings().SideBarOpen;
            }
            if (event.key.key == SDLK_RETURN && (event.key.mod & SDL_KMOD_LALT)) getScreenObject()->toggleFullScreen();
        }



        // --------------------
        // CARUSEL windows test:
        // In deiner Event-Loop:
        switch (event.type) {
            // TOUCH & MOUSE START
            case SDL_EVENT_FINGER_DOWN:
                if (event.button.which != SDL_TOUCH_MOUSEID) {
                    mTouchStartX = event.tfinger.x;
                }
                break;
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                mTouchStartX = event.button.x / ImGui::GetMainViewport()->Size.x;
                break;

            // TOUCH & MOUSE
            case SDL_EVENT_FINGER_UP:
                handleSwipe(event.tfinger.x - mTouchStartX);
                break;
            case SDL_EVENT_MOUSE_BUTTON_UP:  {
                if (event.button.which != SDL_TOUCH_MOUSEID) {
                    float mouseEndX = event.button.x / ImGui::GetMainViewport()->Size.x;
                    handleSwipe(mouseEndX - mTouchStartX);
                }
                break;
            }


            // FIRE TV / KEYBOARD (Left/Right)
            case SDL_EVENT_KEY_DOWN:
                if (event.key.repeat) break;
                if (event.key.key == SDLK_LEFT || event.key.key == SDLK_RIGHT) {
                    mCursorKeyDownStart = SDL_GetTicks();
                    mCursorKeyDown = event.key.key;
                }
                break;
            case SDL_EVENT_KEY_UP:
                if ((event.key.key == SDLK_LEFT || event.key.key == SDLK_RIGHT)) {
                    // if (SDL_GetTicks() - mCursorKeyDownStart > mCursorChangeTime ) {
                    //     if (event.key.key == SDLK_LEFT)  changePage(-1);
                    //     if (event.key.key == SDLK_RIGHT) changePage(1);
                    // } else {
                    //     dLog("ticks ellapsed = %d", (int)(SDL_GetTicks() - mCursorKeyDownStart));
                    // }
                    mCursorKeyDownStart = 0;
                }
                break;
        }

/*
        float deltaX = 0.f;
        if (event.type == SDL_EVENT_FINGER_DOWN || event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
            mTouchStartX = event.tfinger.x; // x is normalized  (0.0 bis 1.0)

        }
        else if (event.type == SDL_EVENT_FINGER_UP || event.type == SDL_EVENT_MOUSE_BUTTON_UP) {
            deltaX = event.tfinger.x - mTouchStartX;
        }
        if (deltaX < -0.15f) { // swipe left
            if (mTargetPageIndex < mPages.size() - 1) mTargetPageIndex++;

        }
        else if (deltaX > 0.15f) { // swipe right
            if (mTargetPageIndex > 0) mTargetPageIndex--;
        }*/





    }


    // -----------------------------------------------------------------------------
    void AppGui::ShowMenuBar(){
        // if ( isAndroidBuild()) {
        //     ImGui::PushFont(getMain()->mHackNerdFont26);
        // } else {
        //     ImGui::PushFont(getMain()->mHackNerdFont20);
        // }
        ImGui::PushFont(getMain()->mHackNerdFont20);





        static float sideBarWidth = 1.f;
        static float targetWidth = 0.f;
        static std::string savStr = "";

        bool isConnected = mStreamHandler->isConnected();
        FluxRadio::StreamInfo info = FluxRadio::StreamInfo();

        if (getMain()->getAppSettings().SideBarOpen) {
            ImGuiViewport* viewport = ImGui::GetMainViewport();

            if (targetWidth > 0.f ) {
                sideBarWidth = ImLerp(sideBarWidth,
                                      targetWidth,
                                      ImGui::GetIO().DeltaTime * 8.0f);
            }
            ImGui::SetNextWindowPos(viewport->WorkPos);
            ImGui::SetNextWindowSize(ImVec2(sideBarWidth, viewport->WorkSize.y));

            ImGuiWindowFlags window_flags =
            /*ImGuiWindowFlags_NoDecoration |*/
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoSavedSettings;

            if (ImGui::Begin("Menu##SidebarOverlay", &getMain()->getAppSettings().SideBarOpen, window_flags)) {



                // ImGui::SeparatorText("Radio");
                // if (ImGui::Selectable("Dummy 1")) {
                //     getMain()->getAppSettings().SideBarOpen = false;
                //
                // }

                if (getMain()->getAppSettings().CurrentStation.name != "") {
                    std::string curName = FluxStr::truncate(getMain()->getAppSettings().CurrentStation.name , 35);
                    if (savStr != getMain()->getAppSettings().CurrentStation.name) {
                        savStr = getMain()->getAppSettings().CurrentStation.name;
                        targetWidth = ImGui::CalcTextSize(curName.c_str()).x + 50.f;
                        if (targetWidth < 250.f) targetWidth=250.f;
                    }


                    if (ImGui::MenuItem(curName.c_str())) {
                        Tune(getMain()->getAppSettings().CurrentStation);
                        getMain()->getAppSettings().SideBarOpen = false;

                    }
                }
                if (ImGui::BeginMenu("Tune"))
                {
                    for (const auto& s : mStations.getStationCache()) {
                        std::string tmpStr = FluxStr::truncate( s.name, 35);
                        if (tmpStr != "" && ImGui::MenuItem((tmpStr + "##station").c_str())) {
                            Tune(s);
                            getMain()->getAppSettings().SideBarOpen = false;
                        }
                    }
                    ImGui::EndMenu();
                }

                ImGui::Separator();
                if (ImGui::BeginMenu("Pages")) {
                    bool isSelected = false;
                    for(int id = 0; id < mPages.size(); id++  ) {
                        isSelected = id == mTargetPageIndex;
                        if (ImGui::MenuItem(mPages[id].getCaption().c_str(), NULL, &isSelected)) {
                            mTargetPageIndex = id;
                            getMain()->getAppSettings().SideBarOpen = false;
                        }
                        // mTargetPageIndex
                    }
                    ImGui::EndMenu();
                }

                if (!isAndroidBuild()) {
                    std::vector<float> scales = { 0.75f, 1.f, 1.5f, 2.f};
                    bool isCurrent = false;
                    std::string scaleCap = "";
                    if (ImGui::BeginMenu("Scale")) {
                        for (auto scale: scales) {
                            isCurrent = getScale() == scale;
                            scaleCap = std::format("{}x", scale);
                            if (ImGui::MenuItem(scaleCap.c_str(), nullptr, isCurrent))  setImGuiScale(scale);
                        }

                        ImGui::EndMenu();
                    }
                }


                if (ImGui::BeginMenu("Background")) {

                    bool isSelected = getMain()->getAppSettings().BackGroundRenderId == -1;
                    if (ImGui::MenuItem("Texture##BackGroundRenderId", NULL, isSelected)) { getMain()->setBackGroundRenderId(-1);}
                    ImGui::Separator();

                    if (getMain()->getBackGroundRenderEffect() ) {
                        for (size_t i = 0; i < getMain()->getBackGroundRenderEffect()->mFragShaderCaptions.size(); i++ ) {
                            std::string caption =  getMain()->getBackGroundRenderEffect()->mFragShaderCaptions[i] + "##BackGroundRenderId";
                            isSelected = (i == getMain()->getAppSettings().BackGroundRenderId);
                            if (ImGui::MenuItem(caption.c_str(), NULL, &isSelected)) {
                                getMain()->setBackGroundRenderId((int)i, getMain()->getAppSettings().BackGroundScanLines);
                            }
                        }
                        ImGui::Separator();
                        if (ImGui::Checkbox("Render Scanlines",&getMain()->getAppSettings().BackGroundScanLines)) {
                            if (getMain()->getAppSettings().BackGroundRenderId >=0) {
                                getMain()->setBackGroundRenderId(getMain()->getAppSettings().BackGroundRenderId, getMain()->getAppSettings().BackGroundScanLines);
                            }
                        }

                    }

                    ImGui::EndMenu();
                }


                ImGui::MenuItem("Console", NULL, &getMain()->getAppSettings().ShowConsole);
                // ImGui::SeparatorText("Layout");
                // if (ImGui::MenuItem("Restore Layout")) { restoreLayout(); }


                ImGui::Separator();
                //FIXME ABOUT DIALOG!!
                if (ImGui::MenuItem("About")) {
                    mGuiGlue->showMessage ( "About",
                                            std::format(

                                                "IronTuner\n"
                                                "=========\n"
                                                "(c)2026 by Thomas Hühn \n"
                                                "Version {}\n"
                                                "https://ohmtal.com\n"
                                                "\n"
                                                "Settings are saved to:\n"
                                                "{}\n"
                                                "Recordings are saved to:\n"
                                                "{}\n"
                                                , getGame()->mSettings.Version
                                                , getGame()->mSettings.getPrefsPath()
                                                , mAudioRecorder->getPath()
                                            )
                    );
                }


                if (ImGui::Selectable("Quit")) {
                    getMain()->getAppSettings().SideBarOpen = false;
                    getMain()->TerminateApplication();
                }

                // if i want to close it when somewhere else is clicked ==>
                if (ImGui::IsWindowFocused() && ImGui::IsKeyDown(ImGuiKey_Escape)) getMain()->getAppSettings().SideBarOpen = false;



            }
            ImGui::End();
        } else {
            targetWidth = 0.f;
            sideBarWidth = 1.f;
            savStr = "";
            // if (ImGui::IsKeyPressed(ImGuiKey_Escape)) getMain()->getAppSettings().SideBarOpen = true;

        }



        if (ImGui::BeginMainMenuBar())
        {
            float fullWidth = ImGui::GetContentRegionAvail().x;
            if ( isAndroidBuild()) {
                ImFlux::ShiftCursor(ImVec2(20.f,0.f));
            }

            if (ImGui::Selectable("≡", false, ImGuiSelectableFlags_None, ImVec2(ImGui::GetFrameHeight(), 0))) {
                getMain()->getAppSettings().SideBarOpen = !getMain()->getAppSettings().SideBarOpen;
                dLog("Sidebar toggled via Selectable = %d", getMain()->getAppSettings().SideBarOpen);
            }


            // change page here !!
            float progress = 0.f;
            if (mCursorKeyDownStart > 0) {
                Uint64 duration = SDL_GetTicks() - mCursorKeyDownStart;
                progress = std::min(1.0f, (float)duration / (float)mCursorChangeTime);

                if (progress > 0.25f) {
                    // ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x + viewport->WorkSize.x * 0.4f,
                    //                                viewport->WorkPos.y + 20));
                    // ImGui::SetNextWindowSize(ImVec2(viewport->WorkSize.x * 0.2f, 0));

                    // ImGui::Begin("HoldProgress", nullptr,
                    //              ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs |
                    //              ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings);
                    //
                    // ImGui::Text("%s", getChangePageName(mCursorKeyDown == SDLK_LEFT ? -1 : 1).c_str());
                    // ImGui::ProgressBar(progress, ImVec2(-1, 0), "");

                    // ImGui::End();

                    // ImGui::Text("%s", getChangePageName(mCursorKeyDown == SDLK_LEFT ? -1 : 1).c_str());
                    // ImGui::SameLine();
                    ImGui::ProgressBar(progress, ImVec2(200.f, 0.f), getChangePageName(mCursorKeyDown == SDLK_LEFT ? -1 : 1).c_str());


                    if (progress == 1.f ) {
                        changePage(mCursorKeyDown == SDLK_LEFT ? -1 : 1);
                        mCursorKeyDownStart = SDL_GetTicks();
                    }
                }
            }


            if ( mTargetPageIndex != 1 )
            {
                bool isConnected = mStreamHandler->isConnected();
                std::string displayStr = "";
                if (isConnected && mStreamHandler->getStreamInfo() && mAudioHandler)
                {
                    FluxRadio::StreamInfo info = *mStreamHandler->getStreamInfo();
                    displayStr = info.name + "  . . .  " + mAudioHandler->getCurrentTitle();
                }


                float availWidth = fullWidth - 115.f - ImGui::GetCursorPosX();
                const float w =  ImFlux::CalcLCDTextScrollerWidth(28);
                ImFlux::ShiftCursor(ImVec2( availWidth / 2.f -  w / 2, 3.f));
                ImFlux::LCDTextScroller(displayStr.c_str(), 28,  ImFlux::COL32_NEON_ORANGE);



            }

            ImGui::PushFont(getMain()->mHackNerdFont12);
            float rightOffset = 115.f; //230.0f;
            ImGui::SameLine(ImGui::GetWindowWidth() - rightOffset);
            ImFlux::ShiftCursor(ImVec2(0.f,3.f));
            if (ImFlux::FaderHorizontal("Volume", ImVec2(100, 20), &getMain()->getAppSettings().Volume, 0.0f, 1.0f))
            {
                mAudioHandler->setVolume(getMain()->getAppSettings().Volume);
            }
            ImGui::PopFont();


            ImGui::EndMainMenuBar();
        }

        ImGui::PopFont();

    }
    // -----------------------------------------------------------------------------
    void AppGui::setupFonts(){
        if ( getMain()->mHackNerdFont26 ) return;
        ImGuiIO& io = ImGui::GetIO();

        const ImWchar* range = io.Fonts->GetGlyphRangesDefault();  //only default range!

        // io.Fonts->AddFontDefault();
        ImFontConfig config;
        config.FontDataOwnedByAtlas = false;
        getMain()->mHackNerdFont16 = io.Fonts->AddFontFromMemoryTTF((void*)HackNerdFontPropo_Regular_ttf, HackNerdFontPropo_Regular_ttf_len, 16.0f, &config, range);
        getMain()->mHackNerdFont20 = io.Fonts->AddFontFromMemoryTTF((void*)HackNerdFontPropo_Regular_ttf, HackNerdFontPropo_Regular_ttf_len, 20.0f, &config, range);
        getMain()->mHackNerdFont26 = io.Fonts->AddFontFromMemoryTTF((void*)HackNerdFontPropo_Regular_ttf, HackNerdFontPropo_Regular_ttf_len, 26.0f, &config, range);
        getMain()->mHackNerdFont12 = io.Fonts->AddFontFromMemoryTTF((void*)HackNerdFontPropo_Regular_ttf, HackNerdFontPropo_Regular_ttf_len, 12.0f, &config, range);

    }
    // -----------------------------------------------------------------------------
    void AppGui::SaveSettings() {
        if (SettingsManager().IsInitialized()) {
            getMain()->getAppSettings().PageIndex = mTargetPageIndex;
            SettingsManager().set("AppGui::mAppSettings", getMain()->getAppSettings());
            mStations.Save();
            SettingsManager().set("Radio::CurrentStation", getMain()->getAppSettings().CurrentStation);

            getMain()->getWindowState().sync();
            SettingsManager().set("Windows::State", getMain()->getWindowState());


            if (mAudioHandler.get()) SettingsManager().set("Audio::Effects", mAudioHandler->getEffectsSettingsBase64());

            SettingsManager().save();
        }

    }
    // -----------------------------------------------------------------------------
    void AppGui::Deinitialize(){
        SaveSettings();
        SDL_SetLogOutputFunction(nullptr, nullptr); // log must be unlinked first!!
        mStreamHandler->shutdown();
        if (mAudioHandler.get()) mAudioHandler->shutDown();
    }
    // -----------------------------------------------------------------------------
    bool AppGui::Initialize(){



        std::string lSettingsFile =
        getGame()->mSettings.getPrefsPath()
        .append(getGame()->mSettings.getSafeCaption())
        .append("_prefs.json");
        if (SettingsManager().Initialize(lSettingsFile))
        {

        } else {
            LogFMT("Error: Can not open setting file: {}", lSettingsFile);
        }


        getMain()->getAppSettings() = SettingsManager().get("AppGui::mAppSettings", AppSettings());
        getMain()->getAppSettings().CurrentStation = SettingsManager().get("Radio::CurrentStation", DefaultFavo[0]);
        mStations.Load();


        getMain()->getWindowState() = SettingsManager().get("Windows::State", WindowState());
        getMain()->getWindowState().updateWindow();



        // ~~~~~ GuiGlue ~~~~~
        mGuiGlue = std::make_unique<FluxGuiGlue>(true, false, nullptr);
        if (!mGuiGlue->Initialize())
            return false;

        // CARUSEL InitDockSpace();

        ApplyStudioTheme();

        // ~~~~~  Console right after GuiGlue ~~~~~
        mConsole.OnCommand =  [&](ImConsole* console, const char* cmd) { OnConsoleCommand(console, cmd); };
        SDL_SetLogOutputFunction(ConsoleLogFunction, this);
        setupFonts();


        // ~~~~ modules ~~~~~
        mStreamHandler = std::make_unique<FluxRadio::StreamHandler>("IronTuner/1.0");
        mAudioHandler  = std::make_unique<FluxRadio::AudioHandler>();
        mAudioRecorder = std::make_unique<FluxRadio::AudioRecorder>();
        mRadioBrowser  = std::make_unique<FluxRadio::RadioBrowser>("IronTuner/1.0");

        mAudioHandler->setVolume(getMain()->getAppSettings().Volume); //sync volume
        std::string emptyStr = "";
        std::string effStr = SettingsManager().get("Audio::Effects", emptyStr);
        if (!effStr.empty() && mAudioHandler.get()) mAudioHandler->setEffectsSettingsBase64(effStr);


        mStreamHandler->OnConnecting = [&]() {
            //FIXME Display connecting .....
        };

        mStreamHandler->OnError = [&](const uint16_t errorCode, const std::string errorMsg) {

            if (errorCode == 28 && mReconnectOnTimeOutCount < 3) {
                mReconnectOnTimeOutCount ++;
                //deffered connect !
                FluxSchedule.add(0.5f, this,[&]() { ConnectCurrent(); });
            } else {
                mGuiGlue->showMessage("Stream Errror "+std::to_string(errorCode), errorMsg);
            }




        };

        mStreamHandler->OnConnected = [&]() {
            if (mAudioHandler.get())  mAudioHandler->init(mStreamHandler->getStreamInfo());
            if (isDebugBuild()) mStreamHandler->dumpInfo();
            // fill name with stationname if empty
            if (mStreamHandler->getStreamInfo()->name == "")  {
                Log("[info] Overwriting empty stream station name with database station name: %s"
                , getMain()->getAppSettings().CurrentStation.name.c_str());
                mStreamHandler->getStreamInfo()->name = getMain()->getAppSettings().CurrentStation.name;
            }

        };
        mStreamHandler->OnStreamTitleUpdate = [&](std::string title, size_t streamPosition) {
            if (mAudioHandler.get()) mAudioHandler->OnStreamTitleUpdate(title, streamPosition);
        };
        mStreamHandler->OnAudioChunk = [&](const void* buffer , size_t size) {  mAudioHandler->OnAudioChunk(buffer, size); };
        mStreamHandler->onDisConnected = [&]() {
            if (mAudioHandler.get()) mAudioHandler->onDisConnected();
            if (mAudioRecorder.get()) mAudioRecorder->closeFile();
            mRecording = false;
            Log("[info] Stream disconncted.");
        };

        mAudioHandler->OnTitleTrigger = [&]() {
            Log("Streamtitle %s", mAudioHandler->getCurrentTitle().c_str());
            //FIXME toggle delay ... needed for some stations
            if (mRecording && mAudioRecorder.get()) mAudioRecorder->openFile(mAudioHandler->getCurrentTitle());
        };

        mAudioHandler->OnAudioStreamData = [&](const uint8_t* buffer, size_t bufferSize)  {
            if (mAudioRecorder.get() && mAudioRecorder->isFileOpen()) {
                mAudioRecorder->OnStreamData(buffer,bufferSize);
            }
        };

        mRadioBrowser->OnStationResponse = [&](std::vector<FluxRadio::RadioStation> stations) {
            mStations.getQueryStationDataMutable() = stations;
            // mQueryStationData = stations;
        };

        mRadioBrowser->OnStationResponseError = [&]() {
            mStations.getQueryStationDataMutable().clear();
            // mQueryStationData.clear();
        };

        std::string texPath = std::format("{}assets/metal_linear_brush_HD.png", getGamePath());
        mBrushedMetalTex = getMain()->loadTexture(texPath);

        // texPath = std::format("{}assets/metal_round_brush.png", getGamePath());
        // mBackgroundTex = getMain()->loadTexture(texPath);

        texPath = std::format("{}assets/knobs/silber256.png", getGamePath());
        mKnobSilverTex = getMain()->loadTexture(texPath);

        texPath = std::format("{}assets/knobs/roterrand.png", getGamePath());
        mKnobOffTex = getMain()->loadTexture(texPath);

        texPath = std::format("{}assets/knobs/gruenerrand.png", getGamePath());
        mKnobOnTex =  getMain()->loadTexture(texPath);


        if ( isAndroidBuild())
        {
            setImGuiScale(2.f);
        }



        // CARUSEL windows test:
        mPages.emplace_back("Relax", nullptr, mPages.size());
        mPages.emplace_back("Radio", [this]() { DrawRadio(); }, mPages.size());
        mPages.emplace_back("Equalizer", [this]() { DrawEqualizer(); }, mPages.size());
        if (!isAndroidBuild()) mPages.emplace_back("Recorder", [this]() { DrawRecorder(); }, mPages.size());
        mPages.emplace_back("Favorites", [this]() { DrawFavo(); }, mPages.size());
        mPages.emplace_back("Radio Browser", [this]() { DrawRadioBrowserWindow(); }, mPages.size());

        if (!isAndroidBuild()) mPages.emplace_back("Rack", [this]() {
             DrawRadio();
             DrawEqualizer();
             DrawRecorder();
        }, mPages.size());


        setImGuiScale(getMain()->getAppSettings().Scale);
        mTargetPageIndex =  getMain()->getAppSettings().PageIndex;


        return true;
    }
    // -----------------------------------------------------------------------------
    void AppGui::DrawGui(){
        mGuiGlue->DrawBegin();
        ShowMenuBar();
        if (getMain()->getAppSettings().ShowConsole) mConsole.Draw("Console", &getMain()->getAppSettings().ShowConsole);

        // --------------------
        // CARUSEL
        auto* viewport = ImGui::GetMainViewport();

        float deltaTime = ImGui::GetIO().DeltaTime;
        float screenWidth = viewport->WorkSize.x;
        float screenHeight = viewport->WorkSize.y;
        float startY = viewport->WorkPos.y;

        // Lineare Interpolation (Lerp)
        float targetX = (float)mTargetPageIndex * - screenWidth;
        static int lastPageIndex = -1;

        bool scrollDone = false;
        //  current = current + (target - current) * speed * dt
        if (std::abs(targetX - mCurrentScrollX) > 0.1f) {
            mCurrentScrollX += (targetX - mCurrentScrollX) * mScrollSpeed * deltaTime;
        } else {
            mCurrentScrollX = targetX;
            scrollDone = true;
        }

        // pages
        ImVec2 Size = ImVec2(screenWidth, screenHeight);

        if ( scrollDone ) {
            float xPos = mCurrentScrollX + (mTargetPageIndex * screenWidth);
            ImVec2 Pos = ImVec2(xPos, startY);

            bool focus = ( lastPageIndex != mTargetPageIndex )
                        && !getMain()->getAppSettings().SideBarOpen
                        && !getMain()->getAppSettings().ShowConsole;

            mPages[mTargetPageIndex].Draw(Pos, Size, mTargetPageIndex, focus);
            lastPageIndex = mTargetPageIndex;
        } else {
            for (int i = 0; i < mPages.size(); i++) {
                float xPos = mCurrentScrollX + (i * screenWidth);
                if (xPos + screenWidth < -10.0f || xPos > screenWidth + 10.0f) continue;
                ImVec2 Pos = ImVec2(xPos, startY);
                mPages[i].Draw(Pos, Size, mTargetPageIndex, false);
            }
        }




        // -------------
        // direct draw !
        // auto* viewport = ImGui::GetMainViewport();
        // static int lastPageIndex = -1;
        // float screenWidth = viewport->WorkSize.x;
        // float screenHeight = viewport->WorkSize.y;
        // float startY = viewport->WorkPos.y;
        // float xPos = viewport->WorkPos.x;
        // ImVec2 Pos = ImVec2(xPos, startY);
        // ImVec2 Size = ImVec2(screenWidth, screenHeight);
        // mPages[mTargetPageIndex].Draw(Pos, Size, mTargetPageIndex, lastPageIndex != mTargetPageIndex);
        // lastPageIndex = mTargetPageIndex;

        // CARUSEL






        // if (getMain()->getAppSettings().ShowRadioBrowser)  {
        //     DrawRadioBrowserWindow();
        // }
        // if (getMain()->getAppSettings().ShowFavo) DrawFavo();
        //
        //
        // if (getMain()->getAppSettings().ShowRadio) {
        //     ImGui::SetNextWindowBgAlpha(0.05f);
        //     DrawRadio();
        // }


        mGuiGlue->DrawEnd();
    }
    // -----------------------------------------------------------------------------
    void AppGui::TuneKnob(std::string caption, const ImFlux::KnobSettings ks)
    {
        ImGui::PushID((caption + "knob").c_str());
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems || mStations.getFavIndex() < 0 ) { ImGui::PopID(); return ; }


        float delta = 0.f;
        int step = 1;
        int* v = &mStations.getFavIndexMutable();
        int v_min = 0;
        int v_max = mStations.getCacheSize() - 1;
        if (v_max < 1) return ; //empty list fixme ?!
        if (*v > v_max ) *v = 0;



        ImVec2 pos = ImGui::GetCursorScreenPos();
        ImVec2 size = ImVec2(ks.radius * 2, ks.radius * 2);
        ImRect bb(pos, pos + size);

        // NOTE: keyboard:
        // ImGui::InvisibleButton(caption.c_str(), size);
        ImGui::ItemSize(size);
        ImGuiID id = window->GetID(caption.c_str());
        if (!ImGui::ItemAdd(bb, id)) { ImGui::PopID(); return; }
        //<<< keyboard



        bool value_changed = false;
        bool isConnected = mStreamHandler->isConnected();



        //NOTE: keyboard ~~~~~~
        ImGuiIO& io = ImGui::GetIO();
        bool is_hovered, is_held;

        ImGui::ButtonBehavior(bb, id, &is_hovered, &is_held, ImGuiButtonFlags_None);
        bool is_clicked = ImGui::IsItemClicked();
        bool is_active = is_held;
        bool is_focused = ImGui::IsItemFocused(); // Now this works!

        bool is_mouseRelease = ImGui::IsItemDeactivated();
        static bool is_Pressed = false;
        if (is_clicked) is_Pressed = true;

        //.......
        int new_v = *v;

        // if (is_Pressed) dLog("PRESSED!");
        // if (is_mouseRelease) dLog("RELEASED! pressed is: %d", is_Pressed);


        if (is_focused) {
            bool plus =  (
                ImGui::IsKeyPressed(ImGuiKey_DownArrow) ||
                ImGui::IsKeyPressed(ImGuiKey_KeypadAdd)  ||
                ImGui::IsKeyPressed(ImGuiKey_GamepadLStickDown));


            bool minus = (
                ImGui::IsKeyPressed(ImGuiKey_UpArrow)  ||
                ImGui::IsKeyPressed(ImGuiKey_KeypadSubtract) ||
                ImGui::IsKeyPressed(ImGuiKey_GamepadLStickUp));
            static float keyboardDelta = 0.f;
            if (plus || minus) {
                float multi = 0.f;
                // TELL IMGUI: "I am using the navigation keys, don't move focus!"
                // ImGui::SetNavCursorVisible(true);

                if (plus) {new_v = *v + (int)keyboardDelta;multi = 0.05f;}
                if (minus) {new_v = *v + (int)keyboardDelta;multi = -0.05f;}

                delta = (v_max - v_min) * multi; // only for visual
                keyboardDelta += delta;

                if (new_v != *v) {
                    value_changed = true;
                    keyboardDelta = 0.f;
                }
            }

            if   (
                 io.KeyMods == ImGuiMod_None &&
                 ( ImGui::IsKeyPressed(ImGuiKey_Space)
                    || ImGui::IsKeyPressed(ImGuiKey_Enter)
                    || ImGui::IsKeyPressed(ImGuiKey_GamepadFaceDown)
                 )
            ) {
                is_Pressed  = true;
                is_mouseRelease = true;
            }
        }

        //NOTE <<< KEYBOARD ~~~~~
        // --- INTERACTION ---
        if (is_hovered && io.MouseWheel != 0) {
            delta = ImGui::GetIO().MouseWheel;
            new_v = *v + (int)delta * step;
            if (new_v != *v) value_changed = true;
        }


        if (is_active && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
            delta = ImGui::GetIO().MouseDelta.y;
            if (std::abs(delta) > 0.0f) {
                static float accumulator = 0.0f;
                accumulator -= delta;
                if (std::abs(accumulator) >= 5.0f) {
                    int steps = (int)(accumulator / 5.0f) * step;
                    new_v = *v + steps;
                    accumulator -= (float)steps * 5.0f; // keep the remainder
                    value_changed = true;
                }
            }
        } else {

        }


        // clamp
        if (new_v < v_min) new_v = v_max;
        if (new_v > v_max) new_v = v_min;
        *v = new_v ;


        // --- DRAWING ---
        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 center = ImVec2(pos.x + ks.radius, pos.y + ks.radius);

        // Outer Border / Housing (Integration of bg_outer)
        dl->AddCircle(center, ks.radius, ks.bg_inner, 32, 1.5f);
        // dl->AddCircleFilled(center, ks.radius + 1.f, ks.bg_inner);
        dl->AddCircleFilled(center, ks.radius, ks.bg_outer);

        // Main Knob Body
        float knob_radius = ks.radius * 0.90f; // 0.65f;
        //seamless movement
        static float visual_value = 0.0f;
        visual_value += delta * 0.01f;
        float needle_ang = visual_value  * M_2PI;
        // ImGui::SameLine(); ImGui::Text("%.3f", visual_value);


        if (mKnobSilverTex) {
            float r = knob_radius;
            GLuint handle = 0;

            if ( !isConnected ) handle = mKnobOffTex->getHandle();
            else if (mTuningMode) handle = mKnobSilverTex->getHandle();
            else handle = mKnobOnTex->getHandle();
            ImTextureID texID = (ImTextureID)(intptr_t)handle;

            // Die 4 Eckpunkte des Bildes (ungerotiert)
            ImVec2 p0 = center + ImVec2(-r, -r); // Oben Links
            ImVec2 p1 = center + ImVec2( r, -r); // Oben Rechts
            ImVec2 p2 = center + ImVec2( r,  r); // Unten Rechts
            ImVec2 p3 = center + ImVec2(-r,  r); // Unten Links

            // Mit deiner Funktion rotieren
            dl->AddImageQuad(
                texID,
                ImFlux::Rotate(p0, center, needle_ang),
                             ImFlux::Rotate(p1, center, needle_ang),
                             ImFlux::Rotate(p2, center, needle_ang),
                             ImFlux::Rotate(p3, center, needle_ang),
                             ImVec2(0, 0), ImVec2(1, 0), ImVec2(1, 1), ImVec2(0, 1)
            );
        }


        //bevel
        dl->AddCircle(center, ks.radius, ks.bevel, 32, 1.0f);


        // NOTE: keyboard
        if (is_focused) {
            ImGui::RenderNavHighlight(bb, id);
        }


        // mouse over hint
        if (is_hovered && mStations.getCachedStation(*v)) {
            ImGui::SetTooltip("%s", mStations.getCachedStation(*v)->name.c_str());
        }

        ImGui::PopID();

        static double last_click_time = 0.0f;
        const double cooldown_duration = 1.f;  //sec cooldown

        if (value_changed) {
            // dLog("TuneKnob: value changed: %d", mSelectedFavIndex);
            mTuningMode = true;
            if (FluxSchedule.isPending(mTuningResetTaskID)) {
                FluxSchedule.extend(mTuningResetTaskID,mTuningResetSec );
            } else {
                mTuningResetTaskID = FluxSchedule.add(mTuningResetSec, nullptr,[&]() { mTuningMode = false; });
            }

            is_Pressed = false;
        }
        // ImGui::SameLine();  ImFlux::DrawLED("clicki",is_clicked, ImFlux::LED_RED);
        // ImGui::SameLine(); ImFlux::DrawLED("pressed",is_Pressed, ImFlux::LED_BLUE);
        // ImGui::SameLine(); ImFlux::DrawLED("connected",isConnected, ImFlux::LED_GREEN);

        if (is_mouseRelease && is_Pressed ) {
            is_Pressed = false;
            if (ImGui::GetTime() - last_click_time > cooldown_duration) {

                if (isConnected && !mTuningMode) {
                    dLog("[info] TuneKnow:: Disconnecting...");
                    Disconnect();

                } else {
                    if (*v < mStations.getCacheSize() ) {
                        FluxRadio::RadioStation* tmpStation = mStations.getCachedStation(*v);
                        if ( tmpStation ) {
                            Tune(*tmpStation);
                            dLog("[info] TuneKnob: TUNE Selected Station: %s", tmpStation->name.c_str());
                        } else {
                            Log("[error] Failed to get cached station!");
                        }
                    }
                }
            } else {
                Log("[warn] TuneKnob: Click ignored ... too fast!");
            }
            last_click_time = ImGui::GetTime();
        }
    }
    // -----------------------------------------------------------------------------
    void AppGui::setImGuiScale(float factor){
        mGuiGlue->setScale(factor);
        getMain()->getAppSettings().Scale = factor;
    }
    float AppGui::getScale() const {
        return getMain()->getAppSettings().Scale;
    }



}; //namespace
