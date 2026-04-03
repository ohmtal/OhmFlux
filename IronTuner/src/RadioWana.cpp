//-----------------------------------------------------------------------------
// Copyright (c) 2012 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// RadioWana
//-----------------------------------------------------------------------------
#include "RadioWana.h"
#include "appMain.h"
#include "utils/fluxSettingsManager.h"
#include "gui/fonts/HackNerdFontPropo-Regular.h"
#include "utils/errorlog.h"
#include "utils/fluxStr.h"
#include <algorithm>

#include "imgui_internal.h"
#include <gui/ImFlux/widets/VirtualTapePlayer.h>



// #include <gui/ImConsole.h>
//------------------------------------------------------------------------------
// macro for JSON support not NOT in HEADER !!
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(RadioWana::AppSettings,
                                                // CurrentStation,
                                                Volume,
                                                DockSpaceInitialized,
                                                ShowFileBrowser,
                                                ShowRadio,
                                                ShowConsole,
                                                ShowRadioBrowser,
                                                ShowRecorder,
                                                ShowFavo,
                                                ShowEqualizer,
                                                SideBarOpen,
                                                BackGroundRenderId,
                                                BackGroundScanLines
)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(RadioWana::WindowState,
                                                width,
                                                height,
                                                posX,
                                                posY,
                                                maximized
)


// -----------------------------------------------------------------------------
// Console handling
// -----------------------------------------------------------------------------
void SDLCALL ConsoleLogFunction(void *userdata, int category, SDL_LogPriority priority, const char *message)
{
    if (!userdata) return;
    auto* gui = static_cast<RadioWana*>(userdata);

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
void RadioWana::restoreLayout(){
    //copied from json :P
    static const std::string layout = "[Window][WindowOverViewport_11111111]\nPos=0,26\nSize=1152,622\nCollapsed=0\n\n[Window][Debug##Default]\nPos=60,60\nSize=400,400\nCollapsed=0\n\n[Window][About]\nPos=774,393\nSize=372,228\nCollapsed=0\n\n[Window][Console]\nPos=0,310\nSize=714,338\nCollapsed=0\nDockId=0x00000006,0\n\n[Window][Radio Browser]\nPos=708,26\nSize=444,622\nCollapsed=0\nDockId=0x00000002,0\n\n[Window][RadioWana]\nPos=0,26\nSize=714,622\nCollapsed=0\nDockId=0x00000005,0\n\n[Window][Recorder]\nPos=0,22\nSize=960,993\nCollapsed=0\nDockId=0x00000005,1\n\n[Window][Favorites]\nPos=708,26\nSize=444,622\nCollapsed=0\nDockId=0x00000002,1\n\n[Window][ImFlux ShowCase Widgets]\nPos=0,432\nSize=960,583\nCollapsed=0\nDockId=0x00000004,0\n\n[Window][HTTP Errror]\nPos=840,465\nSize=240,84\nCollapsed=0\n\n[Window][HUHU]\nPos=892,465\nSize=136,84\nCollapsed=0\n\n[Window][Favourite Dialog]\nPos=60,60\nSize=376,138\nCollapsed=0\n\n[Window][Favorite Dialog]\nPos=772,436\nSize=376,162\nCollapsed=0\n\n[Window][Stream Errror 56]\nPos=782,465\nSize=356,84\nCollapsed=0\n\n[Window][Stream Errror 0]\nPos=823,465\nSize=273,84\nCollapsed=0\n\n[Window][##MySidebar]\nSize=36,510\nCollapsed=0\n\n[Window][Stream Errror 1]\nPos=869,465\nSize=182,84\nCollapsed=0\n\n[Window][Stream Errror 52]\nPos=765,465\nSize=389,84\nCollapsed=0\n\n[Window][Radio]\nPos=0,26\nSize=706,622\nCollapsed=0\nDockId=0x00000005,0\n\n[Table][0x5B6633BA,5]\nColumn 0  Weight=1.0000\nColumn 1  Weight=1.0000\nColumn 2  Weight=1.0000\nColumn 3  Weight=1.0000\nColumn 4  Weight=1.0000\n\n[Table][0xD170F5FA,4]\nRefScale=16\nColumn 0  Width=20\nColumn 1  Weight=1.0000\nColumn 2  Width=74 Sort=0^\nColumn 3  Width=41\n\n[Table][0xC55E50B6,2]\nRefScale=16\nColumn 0  Width=20\nColumn 1  Weight=1.0000 Sort=0^\n\n[Docking][Data]\nDockSpace       ID=0x08BD597D Window=0x1BBC0F80 Pos=0,26 Size=1152,622 Split=Y\n  DockNode      ID=0x00000003 Parent=0x08BD597D SizeRef=960,408 Split=X Selected=0xCC2F45C2\n    DockNode    ID=0x00000001 Parent=0x00000003 SizeRef=706,484 Split=Y Selected=0xCC2F45C2\n      DockNode  ID=0x00000005 Parent=0x00000001 SizeRef=714,282 CentralNode=1 Selected=0xDB489985\n      DockNode  ID=0x00000006 Parent=0x00000001 SizeRef=714,338 Selected=0xEA83D666\n    DockNode    ID=0x00000002 Parent=0x00000003 SizeRef=444,484 Selected=0xB58DAB73\n  DockNode      ID=0x00000004 Parent=0x08BD597D SizeRef=960,583 Selected=0xF2A39ADC\n\n";

    FluxRadio::RadioStation savStation = mAppSettings.CurrentStation;
    mAppSettings = AppSettings();
    mAppSettings.CurrentStation = savStation;
    mAppSettings.DockSpaceInitialized = true;

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
void RadioWana::InitDockSpace(){
    if (mAppSettings.DockSpaceInitialized) return;
    mAppSettings.DockSpaceInitialized = true;
    restoreLayout();
}
// -----------------------------------------------------------------------------
void RadioWana::OnConsoleCommand(ImConsole* console, const char* cmdline){
    std::string cmd = FluxStr::getWord(cmdline,0);

    if (cmd == "testes") getMain()->getBackGroundRenderEffect()->mShaderESTesting = true;

    if (cmd == "sc" ) {  //STATION CACHE
        DumpStationCache();
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
void RadioWana::ApplyStudioTheme(){

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
void RadioWana::DrawRecorder(){
//FIXME move recorder here
}
// -----------------------------------------------------------------------------
void RadioWana::DrawEqualizer(){
    if (!mAudioHandler.get() || !mAudioHandler->getManager()) return;
    DSP::Equalizer9Band* effect = static_cast<DSP::Equalizer9Band*>(mAudioHandler->getManager()->getEffectByType(DSP::EffectType::Equalizer9Band));
    if (!effect) return;
    const float boxHeight = 110.f;
    const float boxWidth = 380.f; //340.f;
    const float sliderSpaceing = 12.f;
    DSP::Equalizer9BandSettings currentSettings = effect->getSettings();

    const float sliderWidth = 25.f;
    const float sliderHeight = 70.f;
    const std::string volStr = "VOL";

    ImGui::PushID(effect);
        bool changed = false;
        // effect->renderUIHeader();
        // if (effect->isEnabled())
        {
            if (ImGui::BeginChild("UI_Box", ImVec2(boxWidth, boxHeight)/*, ImGuiChildFlags_Borders*/)) {
                ImFlux::GradientBoxDL(gRadioDisplayBox.WithSize(ImVec2(boxWidth, boxHeight)) );
                ImFlux::ShiftCursor(ImVec2(5.f,5.f));

                //Volume
                // inline bool FaderVertical2(const char* label, ImVec2 size, float* v, float v_min, float v_max, const char* format = "%.2f") {
                ImGui::BeginGroup();
                float vol = mAudioHandler->getVolume();
                if (ImFlux::FaderVertical(volStr.c_str(), ImVec2(sliderWidth,sliderHeight), &vol,0.f,1.f )) {
                    mAudioHandler->setVolume(vol);
                    mAppSettings.Volume = vol;
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

                    // virtual bool FaderVWithText( float sliderWidth = 20.f, float sliderHeight = 80.f ) = 0;

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

}

// -----------------------------------------------------------------------------
void RadioWana::DrawInfoPopup(FluxRadio::StreamInfo* info) {
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
void RadioWana::Update(const double& dt){

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
    // set current favIndex ...
    if (mSelectedFavIndex < 0) {
        setSelectedFavIndex();

        mStationCache.clear();
        for ( auto& station : mFavoStationData ) {
            mStationCache.push_back(station);
        }

        if ( mAppSettings.CurrentStation.favId < 1) {
            mStationCache.push_back( mAppSettings.CurrentStation );
            mSelectedFavIndex = (int)mStationCache.size() - 1;
        }
    }



}
// -----------------------------------------------------------------------------
void RadioWana::DrawRadio() {

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

    const bool fullScreenRadio = false;
    ImGuiWindowFlags window_flags = 0;
    if (fullScreenRadio) {
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);

        window_flags = ImGuiWindowFlags_NoDecoration
        | ImGuiWindowFlags_NoMove
        | ImGuiWindowFlags_NoResize
        | ImGuiWindowFlags_NoSavedSettings
        | ImGuiWindowFlags_NoBringToFrontOnFocus;
    }


    if (ImGui::Begin("Radio", &mAppSettings.ShowRadio, window_flags )) {
        float fullHalfWidth = ImGui::GetContentRegionAvail().x / 2.f;

        if (fullScreenRadio) ImFlux::ShiftCursor(ImVec2(0.0f,20.f));


        const int lcdDigits    = 30;
        const int lcdDigits2   = 38; //40;

        const float displayWidth =  lcdDigits * 16.9f + 32.f; //* 16.5f; // 20 ==> 320.f; //lcdDigits * lcdHeight1 * 0.5f;
        const float displayHeight = 60.f; //(2 * lcdHeight1) + ( 3 * lcdHeight1 * 0.5f);
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

        if (ImGui::BeginChild("##RadioDisplayStation", ImVec2(displayWidth  ,displayHeight ))) {
            ImGui::SameLine();ImFlux::ShiftCursor(ImVec2(5.f,6.f));
            ImGui::BeginGroup();
            ImGui::PushFont(getMain()->mHackNerdFont26);
            if (mTuningMode || isOffline) {
                if (mStationCache.size() > mSelectedFavIndex && mSelectedFavIndex >= 0) {
                    ImFlux::LCDTextScroller(mStationCache[mSelectedFavIndex].name, lcdDigits, ImFlux::COL32_NEON_PURPLE);
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
                if (ImFlux::ButtonFancy("I", gRadioButtonParams.WithColor(IM_COL32(88,88,88,88)).WithSize(ImVec2(24,24) ))) {
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
                bool isFavo = mAppSettings.CurrentStation.favId > 0;
                if (ImFlux::FavouriteStar("Favourite", isFavo)) {
                    if (isFavo) {
                        RmvFavoByFavId(&mAppSettings.CurrentStation);
                    } else {
                        AddFavo(&mAppSettings.CurrentStation);
                    }
                }

            }

            ImGui::EndGroup();
        }
        ImGui::EndChild();

        ImGui::SameLine();
        TuneKnob("Tune Station", ImFlux::DARK_KNOB.WithRadius(48.f));

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
        //----------------------------

        // ImGui::Separator();

        // -------- 3. VOL + VU -----------





        // ~~~ 9BandEQ ~~~
        if (mAppSettings.ShowEqualizer) {
            ImFlux::GradientBox(ImVec2(0.f, 145.f));

            ImGui::BeginGroup(/*EQ*/);
            ImGui::PushFont(getMain()->mHackNerdFont16);
            ImFlux::ShadowText("EQUALIZER");
            ImGui::PopFont();
            ImGui::Separator();
            if (fullHalfWidth - radioHalfWidth > 0.f) {
                ImFlux::ShiftCursor(ImVec2(fullHalfWidth - radioHalfWidth, 0.f));
            }

            ImFlux::ShiftCursor(ImVec2(5.f,5.f));

            const ImVec2 vuSize = {135,70};
            float dbL, dbR;
            auto mapDB = [](float db) {
                float minDB = -20.0f;
                return (db < minDB) ? 0.0f : (db - minDB) / (0.0f - minDB);
            };

            float cursorY = ImGui::GetCursorPosY();
            // ~~~ VU Meter LEFT ~~~
            if ( mAudioHandler->getManager() && mAudioHandler->getManager()->getVisualAnalyzer()) {
                if (!isConnected) dbL = 0.f;
                else  dbL = mapDB(mAudioHandler->getManager()->getVisualAnalyzer()->getDecible(0));
                ImGui::SetCursorPosY(cursorY + 20.f);
                ImFlux::VUMeter70th(vuSize,dbL, "L",  gRadioDisplayBox.col_top, gRadioDisplayBox.col_bot);

            }
            ImGui::SameLine();

            ImGui::SetCursorPosY(cursorY);
            DrawEqualizer();

            ImGui::SameLine();
            if ( mAudioHandler->getManager() && mAudioHandler->getManager()->getVisualAnalyzer()) {
                if (!isConnected) dbR = 0.f;
                else dbR = mapDB(mAudioHandler->getManager()->getVisualAnalyzer()->getDecible(0));
                ImGui::SetCursorPosY(cursorY + 20.f);
                ImFlux::VUMeter70th(vuSize,dbR, "R", gRadioDisplayBox.col_top, gRadioDisplayBox.col_bot);
            }
            ImGui::EndGroup(/*EQ*/);
        } //show Equilizer





        // -------- 5. Recorder ----------
        if (mAppSettings.ShowRecorder) {

            ImFlux::GradientBox(ImVec2(0.f, 180.f));
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
            tapePlayer.size.x = 220;
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
            if (ImGui::BeginChild("RECORDSETTINGS", ImVec2(0.f,110.f))) {
                // float fullWidth = ImGui::GetContentRegionAvail().x;

                ImGui::SeparatorText("Recording");

                ImGui::Checkbox("Wait for new Title.", &mRecordingStartsOnNewTile);
                if (!isConnected) ImGui::BeginDisabled();

                if (ImFlux::LEDCheckBox("Enable Recording", &mRecording, ImVec4(0.8f,0.3f,0.3f,1.f))) {
                    if (mRecording && !mRecordingStartsOnNewTile && !mAudioHandler->getCurrentTitle().empty()) {
                        mAudioRecorder->openFile(mAudioHandler->getCurrentTitle());
                    }
                    if (!mRecording)
                        mAudioRecorder->closeFile();
                }
                if (!isConnected) ImGui::EndDisabled();

                if (mRecording) {
                    ImFlux::DrawLED("Recording", mAudioRecorder->isFileOpen(), ImFlux::LED_GREEN_ANIMATED_GLOW);
                    ImGui::SameLine();
                    ImGui::Text("File: %s", mAudioRecorder->getCurrentFilename().c_str());
                }
            }
            ImGui::EndChild();
            ImGui::EndGroup(/*RECORDER*/);

        } //mAppSettings.ShowRecorder

    }
    ImGui::End();
}
// -----------------------------------------------------------------------------
void RadioWana::DrawFavo() {
    static bool showDialog = false;
    static bool isEdit = false;
    static FluxRadio::RadioStation workStation;
    static uint32_t editId = 0;
    static bool showInfo = false;

    if (ImGui::Begin("Favorites", &mAppSettings.ShowFavo)){

        // ~~~ BUTTONS ~~~
        if (ImFlux::ButtonFancy("NEW")) {
            showDialog = true;
            isEdit = false;
            workStation = FluxRadio::RadioStation();
            workStation.url = "https://";
        }
        if (mSelectedFavId == 0) ImGui::BeginDisabled();
        ImGui::SameLine();
        if (ImFlux::ButtonFancy("EDIT") ) {
            FluxRadio::RadioStation* pStation = getStationByFavId(&mFavoStationData, mSelectedFavId);
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
            FluxRadio::RadioStation* pStation = getStationByFavId(&mFavoStationData, mSelectedFavId);
            if (pStation) {
                for (auto& line: pStation->dump(false)) {
                    ImGui::TextUnformatted(line.c_str());
                }
            } else {
                Log("[error] FAV_INFO: station is null pointer!!");

            }
            ImGui::EndPopup();
        }


        if (mSelectedFavId == 0) ImGui::EndDisabled();

        // ~~~ LIST ~~~

        DrawStationsList(mFavoStationData, true);
    }
    ImGui::End();


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
                // FIXME display error message or display in dialog
                if (validated)
                {
                    if (isEdit) {
                        FluxRadio::RadioStation* pStation = getStationByFavId(&mFavoStationData, editId);
                        if (pStation) {
                            *pStation = workStation;
                        } else {
                            Log("[error] FAV_EDIT::SAVE => station is null pointer!!");
                        }
                    } else {
                        AddFavo(&workStation);
                        // mFavoStationData.push_back(workStation);
                        // FluxRadio::updateFavIds(&mFavoStationData);
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
void RadioWana::DrawStationsList(std::vector<FluxRadio::RadioStation> stations, bool isFavoList ) {

    static char searchBuffer[128] = "";

    //---------------
    ImGui::BeginGroup();
    ImGui::SeparatorText("Filter results");
    ImGui::SetNextItemWidth(150);
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
                if (!isFavoList) isSelected = (mSelectedStationUuid == station->stationuuid);
                else isSelected  = (mSelectedFavId == station->favId);


                // ~~~~~~~~~ FAVO ~~~~~~~~~~~~~~
                ImGui::TableNextColumn();
                bool isFavo = false;
                if (isFavoList) isFavo = true;
                else isFavo = isFavoStation(station);

                if (ImFlux::FavouriteStar("Favourite", isFavo)) {
                    if (isFavoList) {
                        // std::erase_if(mFavoStationData, [&](const FluxRadio::RadioStation& s) {
                        //     // return s.stationuuid == station->stationuuid;
                        //     return s.favId == station->favId;
                        // });
                        RmvFavoByFavId(station);
                    } else {
                        if (!isFavo) {
                            // mFavoStationData.push_back(*station);
                            // FluxRadio::updateFavIds(&mFavoStationData);
                            AddFavo(station);
                        } else {
                            // std::erase_if(mFavoStationData, [&](const FluxRadio::RadioStation& s) {
                            //     return s.stationuuid == station->stationuuid;
                            // });
                            RmvFavoByUUID(station);
                        }
                    }
                    SaveSettings();
                }

                // ~~~~~~~~~ Station ~~~~~~~~~~~~~~
                ImGui::TableNextColumn();
                ImGui::PushFont(getMain()->mHackNerdFont20);
                if (ImGui::Selectable(station->name.c_str(), isSelected, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick)) {
                    if (!isFavoList)  mSelectedStationUuid = station->stationuuid;
                    else mSelectedFavId = station->favId;

                    if (ImGui::IsMouseDoubleClicked(0)) {
                        Tune(*station);
                    }
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
void RadioWana::DrawRadioBrowserWindow() {
        if (ImGui::Begin("Radio Browser", &mAppSettings.ShowRadioBrowser)){
            static char strBuff[64];

            ImGui::BeginGroup();
            ImGui::SeparatorText("radio-browser.info");
            strncpy(strBuff, mQueryString.c_str(), sizeof(strBuff));
            ImGui::SetNextItemWidth(150.f);
            if (ImGui::InputText("##SearchName", strBuff, sizeof(strBuff), ImGuiInputTextFlags_EnterReturnsTrue)) {
                mRadioBrowser->searchStationsByNameAndTag(strBuff, "");
                mQueryString = strBuff;
                dLog("query for %s", strBuff);
            }
            ImFlux::Hint("Search Radio Station by Name");
            ImGui::EndGroup();
            ImFlux::ShiftCursor(ImVec2(30.f,0.f));
            ImGui::SameLine();

            DrawStationsList(mQueryStationData, false);

        }
        ImGui::End();
    }
// -----------------------------------------------------------------------------
void RadioWana::onEvent(SDL_Event event){
    if (mGuiGlue.get()) mGuiGlue->onEvent(event);

    // FIRE TV KEYS:
    // D-Pad Up        SDLK_UP
    // D-Pad Down      SDLK_DOWN
    // D-Pad Left      SDLK_LEFT
    // D-Pad Right     SDLK_RIGHT
    // Center (Select) SDLK_RETURN or SDLK_KP_ENTER
    // Back            SDLK_ESCAPE or SDLK_AC_BACK
    // Play/Pause      SDLK_MEDIA_PLAY_PAUSE


    if (event.type == SDL_EVENT_KEY_DOWN) {
        if (event.key.scancode == SDL_SCANCODE_GRAVE) mAppSettings.ShowConsole = !mAppSettings.ShowConsole;
        if (event.key.scancode == SDL_SCANCODE_ESCAPE) mAppSettings.SideBarOpen = !mAppSettings.SideBarOpen;
        if (event.key.scancode == SDL_SCANCODE_RETURN && (event.key.mod & SDL_KMOD_LALT)) getScreenObject()->toggleFullScreen();
    }



    // --------------------
    // FIXME SWIPE
    //
    // float touchStartX = 0.0f;
    // bool isSwiping = false;
    //
    // if (event.type == SDL_EVENT_FINGER_DOWN) {
    //     touchStartX = event.tfinger.x; // x is normalized  (0.0 bis 1.0)
    //     isSwiping = true;
    // }
    // else if (event.type == SDL_EVENT_FINGER_UP) {
    //     float deltaX = event.tfinger.x - touchStartX;
    // if (deltaX < -0.15f) { // swipe left
    //     if (targetPageIndex < maxPages - 1) targetPageIndex++;
    // }
    // else if (deltaX > 0.15f) { // swipe right
    //     if (targetPageIndex > 0) targetPageIndex--;
    // }    //     isSwiping = false;
    // }
    ///////////////////
    // TODO: ImGuiWindowFlags_NoScrollWithMouse

    // Page Example
    // int targetPageIndex = 0;
    // float currentScrollX = 0.0f;
    // float scrollSpeed = 10.0f;

    // --- ----------
    // float deltaTime = ImGui::GetIO().DeltaTime;
    // float screenWidth = ImGui::GetMainViewport()->Size.x;
    //
    // // Lineare Interpolation (Lerp)
    // float targetX = (float)targetPageIndex * -screenWidth;
    //
    // //  current = current + (target - current) * speed * dt
    // if (std::abs(targetX - currentScrollX) > 0.1f) {
    //     currentScrollX += (targetX - currentScrollX) * scrollSpeed * deltaTime;
    // } else {
    //     currentScrollX = targetX;
    // }

    // // windows
    // for (int i = 0; i < 3; i++) { // Beispiel für 3 Seiten
    //     float xPos = currentScrollX + (i * screenWidth);
    //
    //     // Performance-Check:
    //     if (xPos + screenWidth < 0 || xPos > screenWidth) continue;
    //
    //     ImGui::SetNextWindowPos(ImVec2(xPos, 0));
    //     ImGui::SetNextWindowSize(ImGui::GetMainViewport()->Size);
    //
    //     char buf[32]; sprintf(buf, "Seite %d", i);
    //     ImGui::Begin(buf, nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
    //
    //     ImGui::Text("Inhalt von Seite %d", i);
    //     if (ImGui::Button("NEXT PAGE") && targetPageIndex < 2) targetPageIndex++;
    //
    //     ImGui::End();
    // }



}
// -----------------------------------------------------------------------------
void RadioWana::setBackGroundRenderId(int id, bool enableScanLines){
    // if (mAppSettings.BackGroundRenderId == id ) return;
    mAppSettings.BackGroundRenderId = id;
    if (id >= 0) {
        getMain()->reloadBackGroundEffectsShader( id, enableScanLines );
    }
}


// -----------------------------------------------------------------------------
void RadioWana::ShowMenuBar(){
    if ( isAndroidBuild()) {
        ImGui::PushFont(getMain()->mHackNerdFont26);
    } else {
        ImGui::PushFont(getMain()->mHackNerdFont20);
    }

    static float sideBarWidth = 1.f;
    static float targetWidth = 0.f;
    static std::string savStr = "";

    bool isConnected = mStreamHandler->isConnected();
    FluxRadio::StreamInfo info = FluxRadio::StreamInfo();

    if (mAppSettings.SideBarOpen) {
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

        if (ImGui::Begin("Menu##SidebarOverlay", &mAppSettings.SideBarOpen, window_flags)) {



            ImGui::SeparatorText("Radio");
            // if (ImGui::Selectable("Dummy 1")) {
            //     mAppSettings.SideBarOpen = false;
            //
            // }

            if (mAppSettings.CurrentStation.name != "") {
                std::string curName = FluxStr::truncate(mAppSettings.CurrentStation.name , 35);
                if (savStr != mAppSettings.CurrentStation.name) {
                    savStr = mAppSettings.CurrentStation.name;
                    targetWidth = ImGui::CalcTextSize(curName.c_str()).x + 50.f;
                    if (targetWidth < 250.f) targetWidth=250.f;
                }


                if (ImGui::MenuItem(curName.c_str())) {
                    Tune(mAppSettings.CurrentStation);
                    mAppSettings.SideBarOpen = false;

                }
            }
            if (ImGui::BeginMenu("Tune"))
            {
                for (const auto& s : mStationCache) {
                    std::string tmpStr = FluxStr::truncate( s.name, 35);
                    if (tmpStr != "" && ImGui::MenuItem((tmpStr + "##station").c_str())) {
                        Tune(s);
                        mAppSettings.SideBarOpen = false;
                    }
                }
                ImGui::EndMenu();
            }

            // if (!isConnected) ImGui::BeginDisabled();
            // if (ImGui::MenuItem("Stream-Info")) {
            //     ImGui::OpenPopup("##StationInfo");
            // }
            // if (!isConnected) ImGui::EndDisabled();
            // if (isConnected && mStreamHandler->getStreamInfo()) info = *mStreamHandler->getStreamInfo();
            // DrawInfoPopup(&info);




            ImGui::SeparatorText("Windows");
            ImGui::MenuItem("Radio", NULL, &mAppSettings.ShowRadio);
            ImGui::MenuItem("Favorites", NULL, &mAppSettings.ShowFavo);
            ImGui::MenuItem("Radio Browser", NULL, &mAppSettings.ShowRadioBrowser);
            ImGui::Separator();
            ImGui::MenuItem("Recorder", NULL, &mAppSettings.ShowRecorder);
            ImGui::MenuItem("Equalizer", NULL, &mAppSettings.ShowEqualizer);
            // ImGui::MenuItem("Background Effect", NULL, &mAppSettings.RenderBackGroundEffect);
            if (ImGui::BeginMenu("Background Rendering"))
            {

                bool isSelected = mAppSettings.BackGroundRenderId == -1;
                if (ImGui::MenuItem("Texture##BackGroundRenderId", NULL, isSelected)) { setBackGroundRenderId(-1);}
                ImGui::Separator();

                if (getMain()->getBackGroundRenderEffect() ) {
                    for (size_t i = 0; i < getMain()->getBackGroundRenderEffect()->mFragShaderCaptions.size(); i++ ) {
                        std::string caption =  getMain()->getBackGroundRenderEffect()->mFragShaderCaptions[i] + "##BackGroundRenderId";
                        isSelected = (i == mAppSettings.BackGroundRenderId);
                        if (ImGui::MenuItem(caption.c_str(), NULL, &isSelected)) {
                            setBackGroundRenderId((int)i, mAppSettings.BackGroundScanLines);
                        }
                    }
                    ImGui::Separator();
                    if (ImGui::Checkbox("Render Scanlines",&mAppSettings.BackGroundScanLines)) {
                        if (mAppSettings.BackGroundRenderId >=0) {
                            setBackGroundRenderId(mAppSettings.BackGroundRenderId, mAppSettings.BackGroundScanLines);
                        }
                    }

                }

                ImGui::EndMenu();
            }


            ImGui::Separator();
            ImGui::MenuItem("Console", NULL, &mAppSettings.ShowConsole);
            ImGui::SeparatorText("Layout");
            if (ImGui::MenuItem("Restore Layout")) { restoreLayout(); }


            ImGui::Separator();
            //FIXME ABOUT DIALIOG!!
            if (ImGui::MenuItem("About")) {
                mGuiGlue->showMessage ( "About",
                                        std::format(
                                            "RadioWana II\n"
                                            "============\n"
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
                mAppSettings.SideBarOpen = false;
                getMain()->TerminateApplication();
            }

            // if i want to close it when somewhere else is clicked ==>
            // if (!ImGui::IsWindowFocused() && ImGui::IsMouseClicked(0)) mAppSettings.SideBarOpen = false;



        }
        ImGui::End();
    } else {
        targetWidth = 0.f;
        sideBarWidth = 1.f;
        savStr = "";
        // if (ImGui::IsKeyPressed(ImGuiKey_Escape)) mAppSettings.SideBarOpen = true;

    }



    if (ImGui::BeginMainMenuBar())
    {
        float fullWidth = ImGui::GetContentRegionAvail().x;
        if ( isAndroidBuild()) {
                ImFlux::ShiftCursor(ImVec2(20.f,0.f));
        }

        if (ImGui::Selectable("≡", false, ImGuiSelectableFlags_None, ImVec2(ImGui::GetFrameHeight(), 0))) {
            mAppSettings.SideBarOpen = !mAppSettings.SideBarOpen;
            dLog("Sidebar toggled via Selectable = %d", mAppSettings.SideBarOpen);
        }


        if (!mAppSettings.ShowRadio)
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
        if (ImFlux::FaderHorizontal("Volume", ImVec2(100, 20), &mAppSettings.Volume, 0.0f, 1.0f))
        {
            mAudioHandler->setVolume(mAppSettings.Volume);
        }
        ImGui::PopFont();


        ImGui::EndMainMenuBar();
    }

    ImGui::PopFont();

}
// -----------------------------------------------------------------------------
void RadioWana::setupFonts(){
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
void RadioWana::SaveSettings() {
    if (SettingsManager().IsInitialized()) {
        SettingsManager().set("AppGui::mAppSettings", mAppSettings);
        SettingsManager().set("Radio::Favo", mFavoStationData);
        SettingsManager().set("Radio::CurrentStation", mAppSettings.CurrentStation);

        mWindowState.sync();
        SettingsManager().set("Windows::State", mWindowState);


        if (mAudioHandler.get()) SettingsManager().set("Audio::Effects", mAudioHandler->getEffectsSettingsBase64());

        SettingsManager().save();
    }

}
// -----------------------------------------------------------------------------
void RadioWana::Deinitialize(){
    SaveSettings();
    SDL_SetLogOutputFunction(nullptr, nullptr); // log must be unlinked first!!
    mStreamHandler->OnAudioChunk = nullptr;
    mStreamHandler->stop();
    if (mAudioHandler.get()) mAudioHandler->shutDown();
}
// -----------------------------------------------------------------------------
bool RadioWana::Initialize(){



    std::string lSettingsFile =
    getGame()->mSettings.getPrefsPath()
    .append(getGame()->mSettings.getSafeCaption())
    .append("_prefs.json");
    if (SettingsManager().Initialize(lSettingsFile))
    {

    } else {
        LogFMT("Error: Can not open setting file: {}", lSettingsFile);
    }


    mAppSettings = SettingsManager().get("AppGui::mAppSettings", AppSettings());
    mFavoStationData = SettingsManager().get("Radio::Favo", mDefaultFavo);
    mAppSettings.CurrentStation = SettingsManager().get("Radio::CurrentStation", mDefaultFavo[0]);
    FluxRadio::updateFavIds(&mFavoStationData);


    mWindowState = SettingsManager().get("Windows::State", WindowState());
    mWindowState.updateWindow();

    //update background renderer
    // setBackGroundRenderId(mAppSettings.BackGroundRenderId, mAppSettings.BackGroundScanLines);



    // ~~~~~ GuiGlue ~~~~~
    mGuiGlue = std::make_unique<FluxGuiGlue>(true, false, nullptr);
    if (!mGuiGlue->Initialize())
        return false;

    InitDockSpace();
    ApplyStudioTheme();



    // ~~~~~  Console right after GuiGlue ~~~~~
    mConsole.OnCommand =  [&](ImConsole* console, const char* cmd) { OnConsoleCommand(console, cmd); };
    SDL_SetLogOutputFunction(ConsoleLogFunction, this);
    setupFonts();


    // ~~~~ modules ~~~~~
    mStreamHandler = std::make_unique<FluxRadio::StreamHandler>();
    mAudioHandler  = std::make_unique<FluxRadio::AudioHandler>();
    mAudioRecorder = std::make_unique<FluxRadio::AudioRecorder>();
    mRadioBrowser  = std::make_unique<FluxRadio::RadioBrowser>();


    mAudioHandler->setVolume(mAppSettings.Volume); //sync volume
    std::string emptyStr = "";
    std::string effStr = SettingsManager().get("Audio::Effects", emptyStr);
    if (!effStr.empty() && mAudioHandler.get()) mAudioHandler->setEffectsSettingsBase64(effStr);


    mStreamHandler->OnConnecting = [&]() {
        //FIXME Display connecting .....
    };

    mStreamHandler->OnError = [&](const uint16_t errorCode, const std::string errorMsg) {

        if (errorCode == 28 && mReconnectOnTimeOutCount < 3) {
            mReconnectOnTimeOutCount ++;
            ConnectCurrent();
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
                , mAppSettings.CurrentStation.name.c_str());
            mStreamHandler->getStreamInfo()->name = mAppSettings.CurrentStation.name;
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
        mQueryStationData = stations;
    };

    mRadioBrowser->OnStationResponseError = [&]() {
        mQueryStationData.clear();
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



    return true;
}
// -----------------------------------------------------------------------------
void RadioWana::DrawGui(){
    mGuiGlue->DrawBegin();


    ShowMenuBar();
    if (mAppSettings.ShowConsole) mConsole.Draw("Console", &mAppSettings.ShowConsole);
    if (mAppSettings.ShowRadioBrowser)  {
        DrawRadioBrowserWindow();
    }
    if (mAppSettings.ShowFavo) DrawFavo();


    if (mAppSettings.ShowRadio) {
        ImGui::SetNextWindowBgAlpha(0.05f);
        DrawRadio();
    }


    mGuiGlue->DrawEnd();
}
// -----------------------------------------------------------------------------
void RadioWana::setSelectedFavIndex(){

    // mCurStationIsFavo =

    for (int i =0 ; i < (int)mFavoStationData.size(); i++) {
        if ( mFavoStationData[i].favId == mAppSettings.CurrentStation.favId ) {
            mSelectedFavIndex = i;
            return;
        }
    }
    Log("CALL: setSelectedFavIndex!");
    mSelectedFavIndex = -1; //nothing found!
}
// -----------------------------------------------------------------------------
void RadioWana::TuneKnob(std::string caption, const ImFlux::KnobSettings ks)
{
    ImGui::PushID((caption + "knob").c_str());
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems || mSelectedFavIndex < 0 ) { ImGui::PopID(); return ; }



    float delta = 0.f;
    int step = 1;
    int* v = &mSelectedFavIndex;
    int v_min = 0;
    int v_max = (int)mStationCache.size() - 1;
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
            ImGui::IsKeyPressed(ImGuiKey_RightArrow) ||
            ImGui::IsKeyPressed(ImGuiKey_KeypadAdd)  ||
            ImGui::IsKeyPressed(ImGuiKey_GamepadLStickRight));


        bool minus = (
            ImGui::IsKeyPressed(ImGuiKey_LeftArrow)  ||
            ImGui::IsKeyPressed(ImGuiKey_KeypadSubtract) ||
            ImGui::IsKeyPressed(ImGuiKey_GamepadLStickLeft));
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
            ImGui::IsKeyPressed(ImGuiKey_Space)
            // || ImGui::IsKeyPressed(ImGuiKey_Enter)
            || ImGui::IsKeyPressed(ImGuiKey_GamepadFaceDown)
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
    if (is_hovered) {
        ImGui::SetTooltip("%s", mStationCache[*v].name.c_str());
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
                if (*v < mStationCache.size()) {
                    FluxRadio::RadioStation tmpStation = mStationCache[*v];
                    Tune(tmpStation);
                    dLog("[info] TuneKnob: TUNE Selected Station: %s", tmpStation.name.c_str());
                }
            }
        } else {
            Log("[warn] TuneKnob: Click ignored ... too fast!");
        }
        last_click_time = ImGui::GetTime();
    }
}
// -----------------------------------------------------------------------------
void RadioWana::setImGuiScale(float factor){
       mGuiGlue->getGuiIO()->DisplaySize = ImVec2(getMain()->getScreen()->getHeight(), getMain()->getScreen()->getWidth());
       ImGui::GetStyle() = mGuiGlue->getBaseStyle();
       ImGui::GetStyle().ScaleAllSizes(factor);
       ImGui::GetStyle().FontScaleDpi = factor;
   }
