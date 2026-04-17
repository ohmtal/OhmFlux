//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Iron Tuner
//-----------------------------------------------------------------------------
#include "appGui.h"
#include "appMain.h"
#include "utils/fluxSettingsManager.h"
#include "gui/fonts/HackNerdFontPropo-Regular.h"
#include "utils/errorlog.h"
#include "utils/fluxStr.h"
#include <algorithm>


#include "imgui_internal.h"
#include "gui/ImFlux/widets/VirtualTapePlayer.h"
#include "gui/ImFlux/widets/VirtualKeyBoard.h"
#include "core/fluxInput.h"


#ifdef __ANDROID__
#include <jni.h>
#include <SDL3/SDL_system.h>
#endif

namespace IronTuner {

    void triggerJavaService() {
        #ifdef __ANDROID__
        JNIEnv* env = (JNIEnv*)SDL_GetAndroidJNIEnv();
        if (!env) {
            Log("[error] IronTuner::triggerJavaService env is NULL!");
            return;
        }

        //FIXME com.ohmtal.irontuner.IronTunerActivity !!
        jobject activity = (jobject)SDL_GetAndroidActivity();
        if (!activity) return;

        jclass clazz = env->GetObjectClass(activity);
        if (!clazz) {
            env->DeleteLocalRef(activity);
            Log("[error] IronTuner::triggerJavaService clazz not found! (startSdlForegroundService)");
            return;
        }

        Log("[info] IronTuner::triggerJavaService startSdlForegroundService");
        jmethodID method_id = env->GetMethodID(clazz, "startSdlForegroundService", "()V");

        if (method_id) {
            env->CallVoidMethod(activity, method_id);
        } else {
            Log("[error] IronTuner::triggerJavaService Method not found! (startSdlForegroundService)");
        }

        env->DeleteLocalRef(activity);
        env->DeleteLocalRef(clazz);
        #endif
    }

    void updateAndroidNotification(const std::string& message) {
        #ifdef __ANDROID__
        JNIEnv* env = (JNIEnv*)SDL_GetAndroidJNIEnv();
        if (!env) return;

        jobject activity = (jobject)SDL_GetAndroidActivity();
        if (!activity) return;

        jclass clazz = env->GetObjectClass(activity);
        if (!clazz) {
            env->DeleteLocalRef(activity);
            return;
        }
        jmethodID method_id = env->GetMethodID(clazz, "updateNotificationFromCpp", "(Ljava/lang/String;)V");

        if (method_id) {
            jstring jmsg = env->NewStringUTF(message.c_str());
            env->CallVoidMethod(activity, method_id, jmsg);
            env->DeleteLocalRef(jmsg);
        }

        env->DeleteLocalRef(activity);
        env->DeleteLocalRef(clazz);
        #else
        Log("[info] %s", message.c_str());
        #endif
    }


    // -----------------------------------------------------------------------------
    // Event Watcher - for special events !
    // -----------------------------------------------------------------------------
    bool SDLCALL EventWatcher(void *userdata, SDL_Event *event) {
        if (!userdata) return true;
        auto* appGui = static_cast<AppGui*>(userdata);
        bool handled = false;

        // SDL_EVENT_WILL_ENTER_BACKGROUND
        // SDL_EVENT_TERMINATING
        // SDL_EVENT_LOW_MEMORY
        // SDL_EVENT_WILL_ENTER_FOREGROUND
        // SDL_EVENT_DID_ENTER_FOREGROUND
        // SDL_EVENT_DID_ENTER_BACKGROUND

        switch ( event->type ) {
            case SDL_EVENT_WILL_ENTER_BACKGROUND:  {
                appGui->SaveSettings();
                gAppStatus.Visible = false;
                Log("[info] IronTuner WILL ENTER BACKGROUND...");


                if (getMain()->getAppSettings().disconnectOnBackground) {
                    appGui->Disconnect();
                }
                else  {
                    triggerJavaService();
                    updateAndroidNotification(appGui->getCurrentTitle());
                }

                handled = true;
                break;
            }
            case SDL_EVENT_DID_ENTER_FOREGROUND:  {
                gAppStatus.Visible = true;
                Log("[info] IronTuner ENTER FOREGROUND...");
                if ( getMain()->getAppSettings().disconnectOnBackground &&
                     getMain()->getAppSettings().autoConnectOnStartUp ) {
                    appGui->ConnectCurrent();
                }
                handled = true;
                break;
            }
        }

        return !handled;
    }

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
    DSP::SpectrumAnalyzer* AppGui::getSpectrumAnalyzer(){
        if ( mAudioHandler->getManager() && mAudioHandler->getManager()->getSpectrumAnalyzer()) {
            return mAudioHandler->getManager()->getSpectrumAnalyzer();
        }
        return nullptr;
    }
    // -----------------------------------------------------------------------------
    Point2F AppGui::getAudioLevels() const{
        return mAudioLevels;
    }
    // -----------------------------------------------------------------------------
    void AppGui::Disconnect(){
        mStreamHandler->stop();
        mAudioHandler->reset();
    }
    // -----------------------------------------------------------------------------
    bool AppGui::ConnectCurrent() {
        if (!FluxNet::NetTools::isValidURL(getMain()->getAppSettings().CurrentStation.url) ) {
            return false;
        }
        std::string url = getMain()->getAppSettings().CurrentStation.url;
        FluxNet::NetTools::URLParts parts = FluxNet::NetTools::parseURL(url);

        // FIXME ANDROID SSL but without is better anyway WHY must be a radio station stream encrypted ?
        // TESTING
        // if (isAndroidBuild())
        {
            if (parts.protocol  == "https" ) {
                url = "http://" + parts.hostname +  parts.path;
            }
        }
        // dLog("Connect Current: protocol: %s, url: %s",parts.protocol.c_str(), url.c_str() );


        mAudioHandler->reset();
        mStreamHandler->Execute(url);
        if (!getMain()->getAppSettings().CurrentStation.stationuuid.empty()) mRadioBrowser->clickStation(getMain()->getAppSettings().CurrentStation.stationuuid);
        return true;
    }
    // -----------------------------------------------------------------------------
    void AppGui::Tune(const FluxRadio::RadioStation station) {
        getMain()->getAppSettings().CurrentStation = station;

        mStations.addStation(&station);
        mStations.incClick(&station);
        ConnectCurrent();
        mTuningMode = false;
        mReconnectOnTimeOutCount = 0;
        // reset index so tuneknob get in sync
        mStations.setIndex(-1);
    }


    // -----------------------------------------------------------------------------
    void AppGui::restoreLayout(){
        //copied from json :P
        //FIXME !!!
        static const std::string layout = "";

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

        if (cmd=="skip") SkipToNextTitle();

        if (cmd == "pause" ) {

            togglePause();
        }

        if (cmd == "stop" ) {
            mStreamHandler->stop();
            Log("[info] Stop but audiostream continue");
        }



        if (cmd == "ff" ) {
              // size_t bytes = std::stoi (FluxStr::getWord(cmdline,1));
            size_t bytes = 128;
            if (mAudioHandler->fastForward(bytes * 1024)) {
                Log("FAST FORWARD %d KB", (int)bytes);
            } else {
                Log("[error]FAST FORWARD %d KB FAILED!!", (int)bytes);
            }
        }

        if (cmd == "fl" )  {
            float limit = std::stof (FluxStr::getWord(cmdline,1));
            getMain()->mSettings.frameLimiter = limit;
            dLog("Setting Framelimit to %f (current fps: %d)", limit, getMain()->getFPS());

            // app->mSettings.frameLimiter = 32.f;
        }

        if (cmd == "testes") getMain()->getBackGroundRenderEffect()->mShaderESTesting = true;

        if (cmd == "dumpLocal" ) {  //STATION CACHE
            mStations.dumpStations();
        }

        if (cmd == "dumpQuery" ) {  //STATION CACHE
            mStations.DumpQueryStations();
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


        // is ignored !?!
        // if ( isAndroidBuild() )
        // {
        //     style.ScrollbarSize =  60.f; // default 14.f
        //     style.GrabMinSize = 40.f;
        // }



        // ANDROID TOUCH TOLERANCE : Der "Wurstfinger"-Faktor
        ImGuiIO& io = ImGui::GetIO();
        io.MouseDoubleClickMaxDist = 20.0f;
        io.MouseDoubleClickTime = 0.50f;

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
    void AppGui::DrawInfoContent(FluxRadio::StreamInfo* info) {
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
        if (!info->url.empty() && ImGui::TextLink(info->url.c_str())) SDL_OpenURL(info->url.c_str());

    }
    void AppGui::DrawInfoPopup(FluxRadio::StreamInfo* info) {
        if (ImGui::BeginPopup("##StationInfo")) {
            DrawInfoContent(info);
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
                    // if ( isAndroidBuild()) {
                    //     info.name += "..... a n d r o i d  b u i l d .....";
                    // }

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
                    if (mStations.getSelectedStation(station)) {
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
                    bool isFavo = mStations.isFavoStation(&getMain()->getAppSettings().CurrentStation);
                    if (ImFlux::FavouriteStar("Favourite", isFavo, 8.f * getScale())) {
                        mStations.setFavo(&getMain()->getAppSettings().CurrentStation, !isFavo);
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

        // if (ImGui::Begin("Favorites", &getMain()->getAppSettings().ShowFavo)){

            // ~~~ BUTTONS ~~~
            if (ImFlux::ButtonFancy("NEW", gRadioButtonParams)) {
                mStationContextData.showDialog = true;
                mStationContextData.isEdit = false;
                mStationContextData.workStation = FluxRadio::RadioStation();
                mStationContextData.workStation.url = "http://";
            }
            ImFlux::Hint("Add a new Station.");

            ImGui::SameLine();
            if (ImFlux::ButtonFancy("CLEAN", gRadioButtonParams)) {
                mStations.cleanup();
            }
            ImFlux::Hint("Remove non favourite Stations");
            // ~~~ LIST ~~~

            DrawStationsTable(mStations.getStations(), true);
        // }
        // ImGui::End();


        if (mStationContextData.showDialog ) {

            if (mStationContextData.workStation.url.size() > 1023 || mStationContextData.workStation.name.size() > 1023)
            {
                mGuiGlue->showMessage("ERROR", "URL or Name too long!");
                mStationContextData.showDialog = false;
            } else {
                // ImGui::OpenPopup("Favourite Dialog");
                // if (ImGui::BeginPopupModal("Favourite Dialog", &mStationContextData.showDialog, ImGuiWindowFlags_AlwaysAutoResize) ) {
                if (ImGui::Begin("Favourite Dialog", &mStationContextData.showDialog, ImGuiWindowFlags_AlwaysAutoResize) ) {
                    ImGui::SeparatorText(mStationContextData.isEdit ? "Edit" : "New");

                    // char strBuff[1024];
                    // strncpy(strBuff, mStationContextData.workStation.name.c_str(), sizeof(strBuff));
                    InputText("Station Name",mStationContextData.workStation.name);
                    // if (InputText("Station Name",strBuff, sizeof(strBuff))) {
                    //     mStationContextData.workStation.name = strBuff;
                    // }

                    InputText("URL",mStationContextData.workStation.url);
                    // strncpy(strBuff, mStationContextData.workStation.url.c_str(), sizeof(strBuff));
                    // if (InputText("URL", strBuff, sizeof(strBuff))) {
                    //     mStationContextData.workStation.url = strBuff;
                    // }
                    ImFlux::SeparatorFancy();

                    if (ImGui::Button("Save")) {
                        bool validated = false;
                        validated = !mStationContextData.workStation.name.empty() && FluxNet::NetTools::isValidURL(mStationContextData.workStation.url);
                        if (validated)
                        {
                            if (mStationContextData.isEdit) {
                                if (mStationContextData.pStation) {
                                    *mStationContextData.pStation = mStationContextData.workStation;
                                } else {
                                    Log("[error] FAV_EDIT::SAVE => station is null pointer!!");
                                }
                            } else {
                                mStations.addStation(&mStationContextData.workStation, true);
                            }
                            mStationContextData.showDialog = false;
                        }
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Cancel")) {
                        mStationContextData.showDialog = false;
                    }

                    // ImGui::EndPopup();
                    ImGui::End();
                }

            }
        }

    }
    // -----------------------------------------------------------------------------
    void AppGui::DrawStationsTable(const std::vector<FluxRadio::RadioStation> stations, const bool isFavoList ) {

        // static char searchBuffer[128] = "";
        static std::string searchBuffer = "";

        //---------------
        ImGui::BeginGroup();
        ImGui::SeparatorText("Filter results");
        ImGui::SetNextItemWidth( 180.f * getScale());
        // InputText("##Filter", searchBuffer, IM_ARRAYSIZE(searchBuffer));
        InputText("##Filter", searchBuffer);

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

        const bool uuid_DEBUG = false;
        bool tableIsFocused = false;

        int colCount = 3;
        if (uuid_DEBUG) colCount = 5;


        if (ImGui::BeginTable("RadioStations", colCount, flags)) {
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableSetupColumn("Favo", ImGuiTableColumnFlags_WidthFixed, 36.f * getScale());

            if (isFavoList) {
                ImGui::TableSetupColumn("Station", ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_DefaultSort | ImGuiTableColumnFlags_PreferSortDescending, 0.f);
                ImGui::TableSetupColumn("Clicks", ImGuiTableColumnFlags_WidthFixed ,  120.f);
            } else {
                ImGui::TableSetupColumn("Station", ImGuiTableColumnFlags_WidthStretch, 0.f);
                ImGui::TableSetupColumn("Clicks", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_DefaultSort | ImGuiTableColumnFlags_PreferSortDescending,  120.f);
            }
            if (uuid_DEBUG) {
                ImGui::TableSetupColumn("UUID", ImGuiTableColumnFlags_WidthFixed, 240.f);
                ImGui::TableSetupColumn("URL", ImGuiTableColumnFlags_WidthFixed, 240.f);
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

            float rowHeight = (isAndroidBuild() ? 45.f : 30.f) * getScale();

            ImGuiListClipper clipper;
            clipper.Begin(displayList.size());
            while (clipper.Step()) {
                for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++) {
                    const auto* station = displayList[row];

                    ImGui::TableNextRow(ImGuiTableRowFlags_None, rowHeight);
                    ImGui::PushID((station->stationuuid + "#" +  std::to_string(row)).c_str());

                    //FIXME SELECTED ?!
                    bool isSelected =false;
                    // if (!isFavoList) isSelected = (mStations.getUuid() == station->stationuuid);
                    // else isSelected  = ( mStations.getFavId() == station->favId);


                    // ~~~~~~~~~ FAVO ~~~~~~~~~~~~~~
                    ImGui::TableNextColumn();
                    bool isFavo = false;
                    if (isFavoList) isFavo = station->isLocalFavo;
                    else isFavo = mStations.isFavoStation(station);

                    FavoStar(isFavo, isFavoList, 14.f * getScale(), station );

                    // ~~~~~~~~~ Station ~~~~~~~~~~~~~~
                    ImGui::TableNextColumn();
                    ImGui::PushFont(getMain()->mHackNerdFont20);

                    // SELECT ?!?!
                    if (ImGui::Selectable(station->name.c_str(), isSelected
                        , ImGuiSelectableFlags_SpanAllColumns /*| ImGuiSelectableFlags_AllowDoubleClick*/
                        ,  ImVec2(0, rowHeight)
                    )) {
                    }

                    if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                        Tune(*station);
                    }
                    if (ImGui::IsItemFocused() &&
                        (   ImGui::IsKeyPressed(ImGuiKey_Enter)
                            || ImGui::IsKeyPressed(ImGuiKey_Space)
                            || ImGui::IsKeyPressed(ImGuiKey_GamepadFaceDown)
                            || ImGui::IsKeyPressed(ImGuiKey_KeypadEnter)
                        )
                    ) {
                        Tune(*station);
                    }

                    ImGui::PopFont();

                    if (station && ImGui::BeginPopupContextItem()) {
                        ImGui::SeparatorText(station->name.c_str());

                        //FIXME GAP -- ShiftCursor before did not help
                        ImFlux::GradientBox(ImVec2(40.f, 40.f) * getScale());
                        ImFlux::ShiftCursor(ImVec2(5.f,5.f));
                        FavoStar(isFavo, isFavoList, 14.f * getScale(), station );


                        if (isFavoList)
                        {
                            ImGui::SameLine();
                            if (ImFlux::ButtonFancy("Edit", gRadioButtonParams)) {
                                mStationContextData.pStation = mStations.getStation(station);
                                mStationContextData.workStation = *station;
                                mStationContextData.showDialog = true;
                                mStationContextData.isEdit = true;
                            }
                        }
                        ImGui::SameLine();
                        if (ImFlux::ButtonFancy("Tune", gRadioButtonParams)) {
                            Tune(*station);
                        }

                        ImGui::SeparatorText("Info");
                        for (const auto& line: station->dump(false)) {
                            ImGui::TextUnformatted(FluxStr::truncate( line , 60).c_str());
                        }



                        ImGui::EndPopup();
                    }


                    // ~~~~~~~~~ Clicks ~~~~~~~~~~~~~~
                    ImGui::TableNextColumn();
                    ImGui::Text("%d",station->clickcount);

                    // if (!isFavoList) {
                    //
                    //     // ~~~~~~~~~ Bitrate ~~~~~~~~~~~~~~
                    //     ImGui::TableNextColumn();
                    //     if (station->bitrate >= 128) {
                    //         ImGui::TextColored(ImColor(0, 255, 0), "%d", station->bitrate);
                    //     } else if (station->bitrate >= 64) {
                    //         ImGui::TextColored(ImColor(255, 255, 0), "%d", station->bitrate);
                    //     } else if (station->bitrate == 0) {
                    //         ImGui::TextDisabled("---");
                    //     } else {
                    //         ImGui::TextColored(ImColor(255, 0, 0), "%d", station->bitrate);
                    //     }
                    //
                    //
                    // }
                    // ~~~~~~~~~ UUID + URL~~~~~~~~~~~~~~
                    if (uuid_DEBUG) {
                        ImGui::TableNextColumn();
                        ImGui::Text("%s",station->stationuuid.c_str());
                        ImGui::TableNextColumn();
                        ImGui::Text("%s",station->url.c_str());
                    }



                    ImGui::PopID(/*station->stationuuid.c_str()*/);
                }

            }
            ImGui::EndTable();


        }
        if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)  && (ImGui::IsKeyPressed(ImGuiKey_AppBack) || ImGui::IsKeyPressed(ImGuiKey_End)) ) {
            // dLog("call window focus ....");
            ImGui::SetWindowFocus();
        }

    }
    // -----------------------------------------------------------------------------
    void AppGui::DrawRadioBrowserWindow() {
        // if (ImGui::Begin("Radio Browser", &getMain()->getAppSettings().ShowRadioBrowser)){
            // static char strBuff[64];
            static std::string strBuff;

            ImGui::BeginGroup();
            ImGui::SeparatorText("radio-browser.info");
            // strncpy(strBuff, mStations.getQueryString().c_str(), sizeof(strBuff));
            ImGui::SetNextItemWidth(180.f * getScale());

            // if (InputText("##SearchName", strBuff, sizeof(strBuff), ImGuiInputTextFlags_EnterReturnsTrue)) {


            InputText("##SearchName", strBuff, [this](const std::string& value) {
                    mRadioBrowser->searchStationsByNameAndTag(value, "");
                    mStations.getQueryStringMutable() = value;
                    dLog("query for %s", value.c_str());
                }
            );
            ImFlux::Hint("Search Radio Station by Name");
            ImGui::EndGroup();
            ImFlux::ShiftCursor(ImVec2(30.f,0.f));
            ImGui::SameLine();

            DrawStationsTable(mStations.getQueryStationData(), false);

        // }
        // ImGui::End();
    }
    // -----------------------------------------------------------------------------

    void AppGui::handleSwipe(float deltaX) {
        if (!mPageWindowFocused) {
            // dLog("[warn] Swipe but not focused.....");
            return ;
        }
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



        if (event.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN) {
            switch (event.gbutton.button) {
                case SDL_GAMEPAD_BUTTON_DPAD_LEFT: {
                    if (mGuiGlue->getGuiIO()->WantTextInput) break;
                    mCursorKeyDownStart = SDL_GetTicks();
                    mCursorKeyDown = SDLK_LEFT;
                    break;
                }
                case SDL_GAMEPAD_BUTTON_DPAD_RIGHT: {
                    if (mGuiGlue->getGuiIO()->WantTextInput) break;
                    mCursorKeyDownStart = SDL_GetTicks();
                    mCursorKeyDown = SDLK_RIGHT;
                    break;
                }
                case SDL_GAMEPAD_BUTTON_START: {
                    getMain()->getAppSettings().SideBarOpen = !getMain()->getAppSettings().SideBarOpen;
                    break;
                }
            }
        }
        if (event.type == SDL_EVENT_GAMEPAD_BUTTON_UP) {
            switch (event.gbutton.button) {
                case SDL_GAMEPAD_BUTTON_DPAD_LEFT:
                case SDL_GAMEPAD_BUTTON_DPAD_RIGHT:
                    mCursorKeyDownStart = 0;
                    break;
            }
        }



        if (event.type == SDL_EVENT_KEY_DOWN) {
            if (event.key.scancode == SDL_SCANCODE_GRAVE) getMain()->getAppSettings().ShowConsole = !getMain()->getAppSettings().ShowConsole;
            if (
                event.key.key == SDLK_MENU
                || event.key.key == SDLK_F1
                || ((mPageWindowFocused || getMain()->getAppSettings().SideBarOpen) && event.key.key == SDLK_ESCAPE)
            ) {
                getMain()->getAppSettings().SideBarOpen = !getMain()->getAppSettings().SideBarOpen;
            }
            if (event.key.key == SDLK_RETURN && (event.key.mod & SDL_KMOD_LALT)) getScreenObject()->toggleFullScreen();
        }



        // --------------------
        // CARUSEL windows test:
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
                     if ( !mGuiGlue->getGuiIO()->WantTextInput ) handleSwipe(mouseEndX - mTouchStartX);
                }
                break;
            }


            // KEYBOARD (Left/Right)
            case SDL_EVENT_KEY_DOWN:
                if (event.key.repeat || mGuiGlue->getGuiIO()->WantTextInput ) break;
                if (event.key.key == SDLK_LEFT || event.key.key == SDLK_RIGHT) {
                    mCursorKeyDownStart = SDL_GetTicks();
                    mCursorKeyDown = event.key.key;
                }
                break;
            case SDL_EVENT_KEY_UP:
                if ((event.key.key == SDLK_LEFT || event.key.key == SDLK_RIGHT)) {
                    mCursorKeyDownStart = 0;
                }
                break;
        }




    }


    // -----------------------------------------------------------------------------
    void AppGui::DrawMenuBar(){
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

                float baseTargetWidth = 250.f * getScale();

                if (isConnected) {
                    if (ImGui::MenuItem("Stop")) Disconnect();
                    targetWidth=baseTargetWidth;
                } else if (getMain()->getAppSettings().CurrentStation.name != "") {
                    std::string curName = FluxStr::truncate(getMain()->getAppSettings().CurrentStation.name , 25);
                    if (savStr != getMain()->getAppSettings().CurrentStation.name) {
                        savStr = getMain()->getAppSettings().CurrentStation.name;
                        targetWidth = ImGui::CalcTextSize((curName + " F2").c_str()).x + 50.f * getScale();
                        if (targetWidth < baseTargetWidth) targetWidth=baseTargetWidth;
                    }


                    if (ImGui::MenuItem(curName.c_str(), "F2")) {
                        Tune(getMain()->getAppSettings().CurrentStation);
                        getMain()->getAppSettings().SideBarOpen = false;

                    }
                }

                if (ImGui::BeginMenu("Tune")) {
                    for (const auto* s : mStations.getSortedStations()) {
                        std::string label = FluxStr::truncate(s->name, 35) + "##" + s->stationuuid;
                        if (ImGui::MenuItem(label.c_str())) {
                            Tune(*s);
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
                        std::string shortCut = (id < 9) ? ("ALT " + std::to_string(id + 1)) : "";
                        if (ImGui::MenuItem(mPages[id].getCaption().c_str(), shortCut.c_str(), &isSelected)) {
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
                            if (ImGui::MenuItem(scaleCap.c_str(), nullptr, isCurrent))  {
                                setImGuiScale(scale);
                                targetWidth = 0;
                                savStr = "";
                            }
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
                        // ImGui::Separator();
                        // if (ImGui::Checkbox("Render Scanlines",&getMain()->getAppSettings().BackGroundScanLines)) {
                        //     if (getMain()->getAppSettings().BackGroundRenderId >=0) {
                        //         getMain()->setBackGroundRenderId(getMain()->getAppSettings().BackGroundRenderId, getMain()->getAppSettings().BackGroundScanLines);
                        //     }
                        // }

                    }

                    ImGui::EndMenu();
                }


                if (ImGui::BeginMenu("Options")) {
                    bool changed = false;
                    changed |= ImGui::Checkbox("Auto Connect on start", &getMain()->getAppSettings().autoConnectOnStartUp);
                    changed |= ImGui::Checkbox("Continue playing...", &getMain()->getAppSettings().continuePlayingAfterDisconnect);
                    ImFlux::Hint("Continue playing after unplaned disconnect.");
                    ImGui::Separator();
                    if (!isAndroidBuild()) {
                        bool fullScreen = getScreenObject()->getFullScreen();
                        if (ImGui::Checkbox("Full Screen", &fullScreen)) {
                            getScreenObject()->setFullScreen(fullScreen);
                            changed = true;
                        }

                        changed |= ImGui::Checkbox("Virtual Keyboard", &getMain()->getAppSettings().useVirtualKeyboard);
                    } else {
                        changed |= ImGui::Checkbox("Stop on enter Background", &getMain()->getAppSettings().disconnectOnBackground);
                    }

                    if (changed) SaveSettings();
                    ImGui::EndMenu();
                }

                ImGui::Separator();
                ImGui::MenuItem("Console", NULL, &getMain()->getAppSettings().ShowConsole);
                // ImGui::SeparatorText("Layout");
                // if (ImGui::MenuItem("Restore Layout")) { restoreLayout(); }

                //TODO ? 1. remote control / key, 2. overlay
                if (ImGui::BeginMenu("Controls")) {
                    ImGui::SeparatorText("Experimental");
                    // if (ImGui::MenuItem("STOP")) Disconnect();
                    if (mAudioHandler->getPause())  { if (ImGui::MenuItem("RESUME")) mAudioHandler->setPause(false); }
                    else { if (ImGui::MenuItem("PAUSE")) mAudioHandler->setPause(true); }
                    if (ImGui::MenuItem("FF 5sec"))  mAudioHandler->fastForward(44100 * 5);
                    bool disable =  (mAudioHandler->getNextTitle() == "" || FluxSchedule.isPending(mSkipToNextTitleTaskID));
                    if (disable) ImGui::BeginDisabled();
                    if (ImGui::MenuItem("FF to next title")) SkipToNextTitle();
                    if (disable) ImGui::EndDisabled();
                    ImGui::EndMenu();
                }

                ImGui::Separator();

                if (ImGui::Selectable("Quit")) {
                    getMain()->getAppSettings().SideBarOpen = false;
                    getMain()->TerminateApplication();
                }

                // if i want to close it when somewhere else is clicked ==>
                // if (ImGui::IsWindowFocused() && ImGui::IsKeyPressed(ImGuiKey_Escape)) getMain()->getAppSettings().SideBarOpen = false;




            }
            ImGui::End();
        } else {
            targetWidth = 0.f;
            sideBarWidth = 1.f;
            savStr = "";
            // if (ImGui::IsKeyPressed(ImGuiKey_Escape)) getMain()->getAppSettings().SideBarOpen = true;

        }

        bool raiseHeight = isAndroidBuild();
        float verticalPadding = (raiseHeight ? 6.f : 3.f) * getScale();
        ImVec2 hamburgerSize = ImVec2(55.f * getScale() ,26.f * getScale());

        // default: ImVec2(4,3)
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(ImGui::GetStyle().FramePadding.x, verticalPadding));

        if (ImGui::BeginMainMenuBar())
        {
            float fullWidth = ImGui::GetContentRegionAvail().x;
            if ( raiseHeight)
            {
                ImFlux::ShiftCursor(ImVec2(20.f,4.f));
                ImGui::SetWindowFontScale(1.5f);
            }

            if (ImFlux::ButtonFancy("≡##BTN", gRadioButtonParams.WithSize(hamburgerSize))) {
                getMain()->getAppSettings().SideBarOpen = !getMain()->getAppSettings().SideBarOpen;
            }
            if (!isAndroidBuild()) ImFlux::Hint("MENU (F1)");

            if ( raiseHeight) {
                ImGui::SetWindowFontScale(1.f);
            }


            // ~~~~ change page progress (from keyborard switch)~~~~
            float progress = 0.f;
            if (mCursorKeyDownStart > 0) {
                Uint64 duration = SDL_GetTicks() - mCursorKeyDownStart;
                progress = std::min(1.0f, (float)duration / (float)mCursorChangeTime);

                if (progress > 0.25f) {
                    ImGui::ProgressBar(progress, ImVec2(200.f, 0.f), getChangePageName(mCursorKeyDown == SDLK_LEFT ? -1 : 1).c_str());
                    if (progress == 1.f ) {
                        changePage(mCursorKeyDown == SDLK_LEFT ? -1 : 1);
                        mCursorKeyDownStart = SDL_GetTicks();
                    }
                }
            }


            if ( std::ranges::find(mTopScrollerIgnorePages, mTargetPageIndex) == mTopScrollerIgnorePages.end() )
            {
                bool isConnected = mStreamHandler->isConnected();
                ImU32 col =  ImFlux::COL32_NEON_ORANGE;
                std::string displayStr = "";
                if (  mStreamHandler->getStreamInfo() && mAudioHandler)
                {
                    if (isConnected) {
                        FluxRadio::StreamInfo info = *mStreamHandler->getStreamInfo();
                        displayStr = info.name + "  . . .  " + mAudioHandler->getCurrentTitle();
                    } else if (mStreamHandler->isConnecting()) {
                        col = ImFlux::COL32_NEON_PURPLE;
                        displayStr =  " * * * connecting * * * " + getMain()->getAppSettings().CurrentStation.name;
                    } else {
                        col = ImFlux::COL32_NEON_RED;
                        displayStr =  " * * * offline * * * " + getMain()->getAppSettings().CurrentStation.name ;
                    }
                }


                float availWidth = fullWidth - 115.f - ImGui::GetCursorPosX();
                const float w =  ImFlux::CalcLCDTextScrollerWidth(28);
                ImFlux::ShiftCursor(ImVec2( availWidth / 2.f -  w / 2, 3.f));
                ImFlux::LCDTextScroller(displayStr.c_str(), 28,  col);



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
        } //BeginMainMenuBar
        ImGui::PopStyleVar();

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
        mIsShuttingDown.store(true);
        SaveSettings();
        SDL_RemoveEventWatch(EventWatcher, this);
        SDL_SetLogOutputFunction(nullptr, nullptr); // log must be unlinked first!!
        SAFE_DELETE(mVirtualKeyBoard);
        mStreamHandler->shutdown();
        mAudioHandler->reset();
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

            bool reconnect =  errorCode == 28 || errorCode == 56 ;

            if ( getMain()->getAppSettings().CurrentStation.clickcount > 1 && errorCode == 6 ) reconnect = true;

            if ( reconnect && mReconnectOnTimeOutCount < 5 ) {
                int currentAttempt = ++mReconnectOnTimeOutCount;
                //deffered connect !
                const float baseDelay = 2.0f;
                float delay = baseDelay * (1.0f + std::log(static_cast<float>(currentAttempt)));
                {
                    updateAndroidNotification(std::format("Reconnecting #{} in {}....", std::to_string(currentAttempt), delay));
                }
                std::thread([this, delay]() {
                    std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(delay * 1000)));
                    if (mIsShuttingDown.load()) return;
                    try {
                        if (!mStreamHandler->isConnected()) this->ConnectCurrent();
                    } catch (...) {
                    }
                }).detach();

            } else {
                mGuiGlue->showMessage("Stream Errror "+std::to_string(errorCode), errorMsg);
                updateAndroidNotification("offline Error:" + errorMsg );
                mReconnectOnTimeOutCount = 0;
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

            if (!getMain()->getAppSettings().continuePlayingAfterDisconnect) {
              if (mAudioHandler.get()) mAudioHandler->onDisConnected();
            }

            if (mAudioRecorder.get()) mAudioRecorder->closeFile();
            mRecording = false;
            Log("[info] Stream disconncted.");
        };

        mAudioHandler->OnTitleTrigger = [&]() {
            // Log("[info]Streamtitle %s", mAudioHandler->getCurrentTitle().c_str());

            updateAndroidNotification(mAudioHandler->getCurrentTitle());
            if ( FluxSchedule.isPending(mSkipToNextTitleTaskID) ) {
                FluxSchedule.cancel(mSkipToNextTitleTaskID);
                mSkipToNextTitleTaskID = 0;
            }


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


        mVirtualKeyBoard = new ImFlux::VirtualKeyBoard();


        if ( isAndroidBuild())
        {
            setImGuiScale(2.f);

        }

        //--------- Pages ------------
        // empty page
        mPages.emplace_back("Relax", nullptr, mPages.size());

        // radio pnly
        mTopScrollerIgnorePages.push_back(mPages.size()); //1
        mPages.emplace_back("Radio", [this]() { DrawRadio(); }, mPages.size());

        // rack on desktop / Equalizer9Band on android
        if (!isAndroidBuild()) {
            mTopScrollerIgnorePages.push_back(mPages.size());
            mPages.emplace_back("Rack", [this]() {
                DrawRadio();
                DrawEqualizer();
                DrawRecorder();
            }, mPages.size());
        } else {
            mPages.emplace_back("Equalizer", [this]() { DrawEqualizer(); }, mPages.size());
        }


        mPages.emplace_back("Favorites", [this]() { DrawFavo(); }, mPages.size());
        mPages.emplace_back("Station Search", [this]() { DrawRadioBrowserWindow(); }, mPages.size());

        mPages.emplace_back("Info", [this]() { DrawInfo(); }, mPages.size());


        setImGuiScale(getMain()->getAppSettings().Scale);
        mTargetPageIndex =  getMain()->getAppSettings().PageIndex;
        if (mTargetPageIndex >= mPages.size()) mTargetPageIndex = 1;

        //--------- check for virtual keyboard ------------

        // dummfug
        // mUseVirtualKeyBoard = false;
        // if (isAndroidBuild())  {
        //     const char* mfg = SDL_GetHint("SDL_ANDROID_MANUFACTURER");
        //     const char* model = SDL_GetHint("SDL_ANDROID_DEVICE_MODEL_NAME");
        //
        //     if (mfg ) Log("[info] ANDROID MANUFACTURER: %s", mfg);
        //     else Log("[error] SDL_ANDROID_MANUFACTURER not available");
        //     if (model ) Log("[info] ANDROID DEVICE MODEL NAME: %s", model);
        //     else Log("[error] SDL_ANDROID_DEVICE_MODEL_NAME not available");
        //
        //     if ((mfg && strstr(mfg, "Amazon")) || (model && strstr(model, "AFT"))) {
        //         mUseVirtualKeyBoard = true;
        //         Log("[info] Amazon FireTV device detected");
        //     } else {
        //         int numTouchDevices = 0;
        //         SDL_GetTouchDevices(&numTouchDevices);
        //         if (numTouchDevices <= 0) {
        //             mUseVirtualKeyBoard = true;
        //         }
        //         Log("[info] Detected TouchDevices: %d", numTouchDevices);
        //     }
        // }

        if (getMain()->getAppSettings().autoConnectOnStartUp) {
            ConnectCurrent();
        }

        // ADD Event Watcher
        SDL_AddEventWatch(EventWatcher, this);



        return true;
    }
    // -----------------------------------------------------------------------------
    void AppGui::DrawGui(){
        mGuiGlue->DrawBegin();
        if ( isAndroidBuild() )
        {
            ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 50.f);
            ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 35.f);
            ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarRounding, 12.f );
        }

        DrawMenuBar();
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
            mPageWindowFocused = mPages[mTargetPageIndex].isFocused();
            lastPageIndex = mTargetPageIndex;
        } else {
            for (int i = 0; i < mPages.size(); i++) {
                float xPos = mCurrentScrollX + (i * screenWidth);
                if (xPos + screenWidth < -10.0f || xPos > screenWidth + 10.0f) continue;
                ImVec2 Pos = ImVec2(xPos, startY);
                mPages[i].Draw(Pos, Size, mTargetPageIndex, false);

            }
        }

        if ( isAndroidBuild() )
        {
            ImGui::PopStyleVar(3);
        }


        // keyboard page number handling
        if (mGuiGlue->getGuiIO()->KeyAlt) {
            for (size_t i = 0; i < mPages.size() && i < 9; i++) {
                if (ImGui::IsKeyPressed((ImGuiKey)(ImGuiKey_1 + i))) {
                    mTargetPageIndex = i;
                }
            }
        }

        if (ImGui::IsKeyPressed(ImGuiKey_F2)) {
            if (mStreamHandler->isOffline()) ConnectCurrent();
        }

        mVirtualKeyBoard->Draw();


        mGuiGlue->DrawEnd();
    }
    // -----------------------------------------------------------------------------
    void AppGui::FavoStar(bool isFavo, bool isFavoList, float radius, const FluxRadio::RadioStation* station){
        if (ImFlux::FavouriteStar("", isFavo, radius)) {
            if (isFavoList) {
                mStations.setFavo(station, !isFavo);
            } else {
                if (!isFavo) {
                    mStations.addStation(station, true);
                } else {
                    mStations.setFavo(station, false);
                }
            }
            SaveSettings();
        }
    }
    // -----------------------------------------------------------------------------
    void AppGui::TuneKnob(std::string caption, const ImFlux::KnobSettings ks)
    {
        ImGui::PushID((caption + "knob").c_str());
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems /* FIXME ? || mStations.getFavIndex() < 0*/ ) { ImGui::PopID(); return ; }


        float delta = 0.f;
        int step = 1;
        int* v = &mStations.getIndexMutable();
        int v_min = 0;
        int v_max = mStations.getSize() - 1;
        if (v_max < 1) { ImGui::PopID(); return ; } //empty list fixme ?!
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
                ImGui::IsKeyPressed(ImGuiKey_GamepadLStickDown) ||
                ImGui::IsKeyPressed(ImGuiKey_GamepadDpadDown)
            );


            bool minus = (
                ImGui::IsKeyPressed(ImGuiKey_UpArrow)  ||
                ImGui::IsKeyPressed(ImGuiKey_KeypadSubtract) ||
                ImGui::IsKeyPressed(ImGuiKey_GamepadLStickUp) ||
                ImGui::IsKeyPressed(ImGuiKey_GamepadDpadUp)
            );
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
                    || ImGui::IsKeyPressed(ImGuiKey_KeypadEnter)
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
                const float accuStep = 20.f; // 10.f; //orig: 5.f
                accumulator -= delta;
                if (std::abs(accumulator) >= accuStep) {
                    int steps = (int)(accumulator / accuStep) * step;
                    new_v = *v + steps;
                    accumulator -= (float)steps * accuStep; // keep the remainder
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
        if (is_hovered && mStations.getStation(*v)) {
            ImGui::SetTooltip("%s", mStations.getStation(*v)->name.c_str());
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
                    if (*v < mStations.getSize() ) {
                        FluxRadio::RadioStation* tmpStation = mStations.getStation(*v);
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
        mVirtualKeyBoard->setScale(factor);
        getMain()->getAppSettings().Scale = factor;
    }
    float AppGui::getScale() const {
        return getMain()->getAppSettings().Scale;
    }
    // -----------------------------------------------------------------------------
    void AppGui::DrawInfo() {
        if (!mStreamHandler.get() || !mAudioHandler.get()) return;

        float fullHalfWidth = ImGui::GetContentRegionAvail().x / 2.f;
        const float displayWidth  = 600.f * getScale();
        const float halfWidht = displayWidth / 2.f;

        const  ImVec4 captionColor = ImColor4F(cl_AcidGreen);
        //-----
        // ImFlux::GradientBox(ImVec2(0.f, displayHeight + 10.f));
        if (fullHalfWidth - halfWidht > 0.f) {
            ImFlux::ShiftCursor(ImVec2(fullHalfWidth - halfWidht, 0.f));
        }

        if (ImGui::BeginChild("##RadioInfo", ImVec2(displayWidth  ,0.f ) , ImGuiChildFlags_NavFlattened ))
        {
             // ImFlux::GradientBoxDL(gRadioDisplayBox.WithSize(ImVec2(displayWidth  ,displayHeight )) );
            ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.1f, 0.1f, 0.1f, 0.2f));        // background
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.2f, 0.2f, 0.3f, 1.0f)); // hover
            ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.4f, 0.4f, 0.7f, 1.0f));  // click


             ImFlux::ShiftCursor(ImVec2(15.f,15.f));
             ImGui::BeginGroup();
             ImGui::PushFont(getMain()->mHackNerdFont26);
             ImFlux::ShadowText( getMain()->mSettings.Caption, ImGui::ColorConvertFloat4ToU32(captionColor)) ;
             ImGui::PopFont();
             ImFlux::ShadowText( std::format("Version: {}",getMain()->mSettings.Version).c_str() );
             ImFlux::ShadowText( std::format("(c)2026 Thomas Hühn /  {}",getMain()->mSettings.Company).c_str()  );
             ImGui::Spacing();

             // if ( getMain()->getAppSettings().CurrentStation.name != "" ) {
             //     ImGui::Spacing();
             //     // ImGui::SeparatorText("Station");
             //     if (ImGui::CollapsingHeader("Station")) {
             //         for (const auto& line: (&getMain()->getAppSettings().CurrentStation)->dump(false)) {
             //             ImFlux::ShadowText(FluxStr::truncate( line , 60).c_str());
             //         }
             //    }
             // }
//
             if ( mStreamHandler->isConnected() && mStreamHandler->getStreamInfo() ) {
                 if (ImGui::CollapsingHeader("Stream Info")) {
                    FluxRadio::StreamInfo info = *mStreamHandler->getStreamInfo();
                    ImGui::Spacing();
                    DrawInfoContent(&info);
                 }

             }
             if (!isAndroidBuild() ) {
                 if (ImGui::CollapsingHeader("Pathes")) {
                     ImFlux::ShadowText(("Installation path: " + getGamePath()).c_str());
                     ImFlux::ShadowText(("Settings path: " + getMain()->mSettings.getPrefsPath()).c_str());
                     if (!isAndroidBuild()) ImFlux::ShadowText( ("Recording path: " + mAudioRecorder->getPath()).c_str());
                 }
             }
             if (ImGui::CollapsingHeader("Audio - Buffer")) {
                 static std::array<size_t, 3> bufferValues = {0, 0, 0};
                 static double nextUpdate = 0.0;
                 const double updateInterval = 0.2;

                 if (ImGui::GetTime() >= nextUpdate) {
                     bufferValues[0] = mAudioHandler->getRingBufferAvailableForWrite();
                     bufferValues[1] = mAudioHandler->getRingBufferAvailableForRead();
                     bufferValues[2] = mAudioHandler->getRawBufferSize();
                     nextUpdate = ImGui::GetTime() + updateInterval;
                 }
                 ImGui::Spacing();
                 ImFlux::ShadowText(std::format("Raw Buffer  {:8} bytes buffered",bufferValues[2]).c_str() );
                 ImGui::Spacing();
                 ImFlux::ShadowText(std::format("Ring Buffer {:8} samples free", bufferValues[0]).c_str());
                 ImFlux::ShadowText(std::format("Ring Buffer {:8} samples buffered", bufferValues[1]).c_str());
             }


             ImGui::EndGroup();
             ImGui::PopStyleColor(3);
        } //BeginChild
        ImGui::EndChild();
    }
    // -----------------------------------------------------------------------------
    bool AppGui::InputText(const char* label, std::string& buffer, std::function<void(const std::string& value)> onEnter) {
        char tempBuffer[1024];
        strncpy(tempBuffer, buffer.c_str(), sizeof(tempBuffer));

        ImGuiInputTextFlags flags = 0;
        if (onEnter /* && !mUseVirtualKeyBoard*/) {
            flags = ImGuiInputTextFlags_EnterReturnsTrue;
        }

        if (ImGui::InputText(label, tempBuffer, sizeof(tempBuffer), flags)) {
            buffer = tempBuffer;
            if ( onEnter ) onEnter(tempBuffer);
            return true;
        }

        if ( (getMain()->getAppSettings().useVirtualKeyboard  && ImGui::IsItemActive())
            || ( ImGui::IsKeyPressed(ImGuiKey_GamepadFaceDown) &&  ImGui::IsItemFocused() ) ){
                mVirtualKeyBoard->Open(buffer, onEnter);
        }
        return false;
    }

    //FIXME store title to cancel endless skipping ?!
    void AppGui::SkipToNextTitle() {
        if ( FluxSchedule.isPending(mSkipToNextTitleTaskID) ) return;
        if (!mAudioHandler.get() || !mStreamHandler.get()) return;
        if (mAudioHandler->getNextTitle() == "") {
            Log("[error]Next title empty can not skip.");
            return;
        }
        // 0.1 sec to let the ringbuffer update
        mSkipToNextTitleTaskID = FluxSchedule.add(0.1f, nullptr, [this]() {
            if (!mStreamHandler->isConnected()) return;
            if (mAudioHandler->fastForward(44100 * 8)) {
                SkipToNextTitle();
            }
        });
    }

}; //namespace
