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
#include <algorithm>




// #include <gui/ImConsole.h>
//------------------------------------------------------------------------------
// macro for JSON support not NOT in HEADER !!
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(RadioWana::AppSettings,
                                                mUrl,
                                                Volume,
                                                DockSpaceInitialized,
                                                ShowFileBrowser,
                                                ShowConsole,
                                                ShowRadioBrowser,
                                                ShowRadio,
                                                ShowRecorder,
                                                ShowFavo
)

// NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(FluxRadio::RadioStation,
//                                                 stationuuid, name, url,
//                                                 codec, bitrate, country,
//                                                 tags,
//                                                 homepage, favicon, countrycode,
//                                                 languages, clickcount, clicktrend
// )

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
    gui->mConsole.AddLog("%s", fluxStr::removePart(message,"\r\n").c_str());
}

bool RadioWana::isFavoStation(std::string searchUuid){
       auto it = std::find_if(mFavoStationData.begin(), mFavoStationData.end(),
                              [&searchUuid](const FluxRadio::RadioStation& s) {
                                  return s.stationuuid == searchUuid;
                              });
       return it != mFavoStationData.end();
   }
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
void RadioWana::DrawRecorder(){

    if (ImGui::Begin("Recorder", &mAppSettings.ShowRadio)) {
        float fullWidth = ImGui::GetContentRegionAvail().x;
        bool isConnected = mStreamHandler->isConnected();
        ImGui::SeparatorText("Recording");

        ImGui::Checkbox("Recording starts on when new stream title is triggered", &mRecordingStartsOnNewTile);
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
    ImGui::End();

}
// -----------------------------------------------------------------------------
void RadioWana::DrawRadio() {

    if (!mStreamHandler.get() || !mAudioHandler.get()) return;

    bool isConnected = mStreamHandler->isConnected();
    bool isRunning   = mStreamHandler->isRunning();

    FluxRadio::StreamInfo info = FluxRadio::StreamInfo();
    if (isConnected && mStreamHandler->getStreamInfo()) info = *mStreamHandler->getStreamInfo();
    else {
        if (isRunning) info.name = " . . . conecting . . . ";
        else if (isConnected) info.name = " . . . . . . ";
        else info.name = " . . . offline . . . ";
    }


    if (ImGui::Begin("RadioWana", &mAppSettings.ShowRadio)) {
        float fullWidth = ImGui::GetContentRegionAvail().x;

        // ImGui::SetNextItemWidth(450.f);
        char strBuff[256];
        strncpy(strBuff, mAppSettings.mUrl.c_str(), sizeof(strBuff));
        if (ImGui::InputText("URL", strBuff, sizeof(strBuff))) {
            mAppSettings.mUrl = strBuff;
        }

        const int lcdDigits = 20;
        const float lcdHeight1 = 24.f;
        const float displayWidth = 320.f; //lcdDigits * lcdHeight1 * 0.5f;
        const float displayHeight = 60.f; //(2 * lcdHeight1) + ( 3 * lcdHeight1 * 0.5f);

        ImFlux::GradientBox(ImVec2(0.f, displayHeight + 30.f));

        ImFlux::ShiftCursor(ImVec2(5.f,5.f));

        ImGui::BeginGroup();

        //connect button
        if (isConnected) {
            if (ImFlux::ButtonFancy("OFF", gRadioButtonParams)) {
                mStreamHandler->stop();
            }
        } else {
            if (ImFlux::ButtonFancy("ON", gRadioButtonParams)) {
                mStreamHandler->Execute(mAppSettings.mUrl);
            }
        }

        // static bool showInfo = false;
        if (!isConnected) ImGui::BeginDisabled();
        if (ImFlux::ButtonFancy("Info", gRadioButtonParams.WithColor(IM_COL32(88,88,88,88) ))) {
            ImGui::OpenPopup("##StationInfo");
        }
        if (!isConnected) ImGui::EndDisabled();
        if (ImGui::BeginPopup("##StationInfo")) {
            ImGui::PushFont(getMain()->mHackNerdFont20);
            ImGui::SeparatorText(info.name.c_str());
            ImGui::PopFont();
            ImGui::Text("Address: %s", info.streamUrl.c_str());
            ImGui::Text("Description: %s", info.description.c_str());
            ImGui::Separator();
            ImGui::TextColored(ImVec4(0.3f, 0.3f,0.7f,1.f), "%s", mAudioHandler->getCurrentTitle().c_str());
            ImGui::TextDisabled("Next: %s", mAudioHandler->getNextTitle().c_str());
            ImGui::Separator();
            ImGui::Text("Description: %s", info.description.c_str());
            ImGui::Text("Audio: %d Hz, %d kbps, %d Channels", info.samplerate, info.bitrate, info.channels);
            ImGui::Text("Url: %s", info.url.c_str());

            ImGui::EndPopup();
        }


        ImGui::EndGroup();

        ImGui::SameLine();ImFlux::ShiftCursor(ImVec2(20.f,0.f));



        ImGui::SameLine();ImFlux::ShiftCursor(ImVec2(20.f,10.f));

        if (ImGui::BeginChild("##RadioDisplayStation", ImVec2(displayWidth,displayHeight))) {
            ImFlux::GradientBoxDL(gRadioDisplayBox );
            ImGui::BeginGroup();
            ImFlux::LCDText(info.name, 20, 24.f, ImFlux::COL32_NEON_ELECTRIC);
            ImGui::Spacing();
            std::string longInfo ;
            if (isConnected ) longInfo = std::format( "DESC {} ADDR {} WEB {} Audio {} Hz, {} kbps, {} Channels",
                info.description, info.streamUrl, info.url, info.samplerate, info.bitrate, info.channels);
            else longInfo = "";
            ImFlux::LCDText(longInfo, lcdDigits*2, lcdHeight1 * 0.5f, ImFlux::COL32_NEON_ELECTRIC);

            ImGui::EndGroup();
        }
        ImGui::EndChild();

        ImGui::SameLine();ImFlux::ShiftCursor(ImVec2(10.f,10.f));

        if (ImGui::BeginChild("##RadioDisplayTitle", ImVec2(displayWidth,displayHeight))) {
            ImFlux::GradientBoxDL(gRadioDisplayBox );
            ImGui::BeginGroup();
            ImFlux::LCDText(mAudioHandler->getCurrentTitle(), 20, 24.f, ImFlux::COL32_NEON_ORANGE);
            ImGui::Spacing();
            ImFlux::LCDText(mAudioHandler->getNextTitle(), lcdDigits*2, lcdHeight1 * 0.5f, ImFlux::COL32_NEON_ORANGE);
            ImGui::EndGroup();
        }
        ImGui::EndChild();


        // ImGui::SameLine();ImFlux::ShiftCursor(ImVec2(20.f,0.f));
        //----------------------------

        ImGui::Separator();

        ImFlux::GradientBox(ImVec2(0.f, displayHeight + 20.f));
        ImFlux::ShiftCursor(ImVec2(80.f,5.f));


        if ( mAudioHandler->getManager() && mAudioHandler->getManager()->getVisualAnalyzer()) {
            mAudioHandler->getManager()->getVisualAnalyzer()->renderVU(ImVec2(200,60), 70);
        }

        ImGui::SameLine();
        ImFlux::ShiftCursor(ImVec2(20.f,10.f));

        float vol = mAudioHandler->getVolume();
        if (ImFlux::LEDMiniKnob("Volume", &vol, 0.f, 1.f, ImFlux::ksBlack.WithRadius(28.f))) {
            mAudioHandler->setVolume(vol);
            mAppSettings.Volume = vol;
        }

        ImFlux::ShiftCursor(ImVec2(0.f,10.f));
        //----------------------------

        if ( mAudioHandler->getManager() && mAudioHandler->getManager()->getSpectrumAnalyzer()) {

            mAudioHandler->getManager()->getSpectrumAnalyzer()->DrawSpectrumAnalyzer(ImVec2(fullWidth,60), true);
        }

        mAudioHandler->RenderRack(1);

    }
    ImGui::End();
}
// -----------------------------------------------------------------------------
void RadioWana::DrawFavo() {
    if (ImGui::Begin("Favorites", &mAppSettings.ShowFavo)){

            DrawStationsList(mFavoStationData, true);

        // ImGui::SeparatorText("radio-browser.info");
        // char strBuff[64];
        // strncpy(strBuff, mQueryString.c_str(), sizeof(strBuff));
        //
        // if (ImGui::InputText("Name:", strBuff, sizeof(strBuff), ImGuiInputTextFlags_EnterReturnsTrue)) {
        //     mRadioBrowser->searchStationsByNameAndTag(strBuff, "");
        //     mQueryString = strBuff;
        //     dLog("query for %s", strBuff);
        // }
        //
        // if (ImGui::BeginTable("FAVO_RadioStations", 5, ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY | ImGuiTableFlags_Resizable)) {
        //     ImGui::TableSetupColumn("Station");
        //     ImGui::TableSetupColumn("Homepage");
        //     // std::string favicon; << TODO would be cool but
        //     // ImGui::TableSetupColumn("Codec"); //we can mp3 only at the moment
        //     ImGui::TableSetupColumn("Clicks");
        //     ImGui::TableSetupColumn("Country");
        //     ImGui::TableSetupColumn("Bitrate");
        //     ImGui::TableHeadersRow();
        //
        //     for (const auto& station : mFavoStationData) {
        //         ImGui::PushID(station.name.c_str());
        //         ImGui::TableNextRow();
        //         ImGui::TableNextColumn();
        //
        //
        //         bool isSelected = (mSelectedStationUuid == station.stationuuid);
        //         if (ImGui::Selectable(station.name.c_str(), isSelected,
        //             ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick)) {
        //             mSelectedStationUuid = station.stationuuid;
        //             if (ImGui::IsMouseDoubleClicked(0)) {
        //                 mUrl = station.url;
        //                 mStreamHandler->Execute(mUrl);
        //                 if (!station.stationuuid.empty()) mRadioBrowser->clickStation(station.stationuuid);
        //             }
        //         }
        //
        //         ImGui::TableNextColumn();
        //         ImGui::TextUnformatted(station.homepage.c_str());
        //
        //         // ImGui::TableNextColumn();
        //         // ImGui::TextUnformatted(station.Codec.c_str());
        //
        //         ImGui::TableNextColumn();
        //         ImGui::Text("%d  Trend:%d",station.clickcount, station.clicktrend);
        //
        //         ImGui::TableNextColumn();
        //         ImGui::TextUnformatted(station.countrycode.c_str());
        //
        //         ImGui::TableNextColumn();
        //         if (station.bitrate >= 128) {
        //             ImGui::TextColored(ImColor(0, 255, 0), "%d kbps", station.bitrate);
        //         } else if (station.bitrate >= 64) {
        //             ImGui::TextColored(ImColor(255, 255, 0), "%d kbps", station.bitrate);
        //         } else if (station.bitrate == 0) {
        //             ImGui::TextDisabled("---");
        //         } else {
        //             ImGui::TextColored(ImColor(255, 0, 0), "%d kbps", station.bitrate);
        //         }
        //
        //         ImGui::PopID(/*station.stationuuid.c_str()*/);
        //     }
        //     ImGui::EndTable();
        // }
    }
    ImGui::End();
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
    ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersOuterH
    | ImGuiTableFlags_ScrollY  | ImGuiTableFlags_ScrollX
    | ImGuiTableFlags_RowBg | ImGuiTableFlags_Sortable
    | ImGuiTableFlags_Resizable
    ;

    if (ImGui::BeginTable("RadioStations", 4, flags)) {
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableSetupColumn("Favo", ImGuiTableColumnFlags_WidthFixed, 20.f);
        ImGui::TableSetupColumn("Station", ImGuiTableColumnFlags_WidthStretch, 0.f);
        ImGui::TableSetupColumn("Clicks", ImGuiTableColumnFlags_WidthFixed, 120.f);
        ImGui::TableSetupColumn("Bitrate", ImGuiTableColumnFlags_WidthFixed, 60.f);
        ImGui::TableHeadersRow();

        std::vector<const FluxRadio::RadioStation*> displayList;
        std::string searchStr = fluxStr::toLower(searchBuffer);

                //FIXME case sensitive !!
        for (const auto& s : stations) {

            if (searchStr.empty() || fluxStr::toLower(s.name).find(searchStr) != std::string::npos) {
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

                bool isSelected = (mSelectedStationUuid == station->stationuuid);

                // ~~~~~~~~~ FAVO ~~~~~~~~~~~~~~
                ImGui::TableNextColumn();
                bool isFavo = false;
                if (isFavoList) isFavo = true;
                else isFavo = isFavoStation(station->stationuuid);

                if (ImFlux::FavoriteStar("Favorite", isFavo)) {
                    if (isFavoList) {
                        std::erase_if(mFavoStationData, [&](const FluxRadio::RadioStation& s) {
                            return s.stationuuid == station->stationuuid;
                        });
                    } else {
                        if (!isFavo) {
                            mFavoStationData.push_back(*station);
                        } else {
                            std::erase_if(mFavoStationData, [&](const FluxRadio::RadioStation& s) {
                                return s.stationuuid == station->stationuuid;
                            });
                        }
                    }
                    SaveSettings();
                }

                // ~~~~~~~~~ Station ~~~~~~~~~~~~~~
                ImGui::TableNextColumn();
                ImGui::PushFont(getMain()->mHackNerdFont20);
                if (ImGui::Selectable(station->name.c_str(), isSelected, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick)) {
                    mSelectedStationUuid = station->stationuuid;

                    if (ImGui::IsMouseDoubleClicked(0)) {
                        mAppSettings.mUrl = station->url;
                        mStreamHandler->Execute(mAppSettings.mUrl);
                        mRadioBrowser->clickStation(station->stationuuid);
                    }
                }
                ImGui::PopFont();

                // ~~~ annoying ~~~
                // if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) {
                //     ImGui::BeginTooltip();
                //     ImGui::SeparatorText(station->name.c_str());
                //     ImGui::TextUnformatted(station->homepage.c_str());
                //     // FIXME too long ... ImGui::TextUnformatted(station->url.c_str());
                //     ImGui::Separator();
                //     ImGui::Text("%d (%d)",station->clickcount, station->clicktrend);
                //     ImGui::Separator();
                //     ImGui::TextUnformatted(station->stationuuid.c_str());
                //     ImGui::TextUnformatted(station->codec.c_str());
                //     ImGui::TextUnformatted(station->countrycode.c_str());
                //     ImGui::EndTooltip();
                // }

                // ~~~~~~~~~ Clicks ~~~~~~~~~~~~~~
                ImGui::TableNextColumn();
                ImGui::Text("%d  Trend:%d",station->clickcount, station->clicktrend);

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
   }
// -----------------------------------------------------------------------------
void RadioWana::ShowMenuBar(){
    if (ImGui::BeginMainMenuBar())
    {

        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Exit")) { getGame()->TerminateApplication(); }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Window"))
        {
            ImGui::MenuItem("Radio", NULL, &mAppSettings.ShowRadio);
            ImGui::MenuItem("Favorites", NULL, &mAppSettings.ShowFavo);
            ImGui::MenuItem("Recorder", NULL, &mAppSettings.ShowRecorder);

            ImGui::MenuItem("Radio Browser", NULL, &mAppSettings.ShowRadioBrowser);
            ImGui::Separator();
            ImGui::MenuItem("Console", NULL, &mAppSettings.ShowConsole);
            ImGui::EndMenu();
        }


        if (ImGui::BeginMenu("Help"))
        {

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
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }


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

}
// -----------------------------------------------------------------------------
void RadioWana::SaveSettings() {
    if (SettingsManager().IsInitialized()) {
        SettingsManager().set("AppGui::mAppSettings", mAppSettings);
        SettingsManager().set("Radio::Favo", mFavoStationData);

        if (mAudioHandler.get()) SettingsManager().set("Audio::Effects", mAudioHandler->getEffectsSettingsBase64());

        SettingsManager().save();
    }

}
// -----------------------------------------------------------------------------
void RadioWana::Deinitialize(){
    SaveSettings();
    SDL_SetLogOutputFunction(nullptr, nullptr); // log must be unlinked first!!
    mStreamHandler->OnAudioChunk = nullptr;
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

    std::string emptyStr = "";
    std::string effStr = SettingsManager().get("Audio::Effects", emptyStr);
    if (!effStr.empty() && mAudioHandler.get()) mAudioHandler->setEffectsSettingsBase64(effStr);




    // ~~~~~ GuiGlue ~~~~~
    mGuiGlue = std::make_unique<FluxGuiGlue>(true, false, nullptr);
    if (!mGuiGlue->Initialize())
        return false;

    if ( isAndroidBuild()) {
        setImGuiScale(2.f);
    }


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

    mStreamHandler->OnConnecting = [&]() {
        //FIXME Display connecting .....
    };

    mStreamHandler->OnError = [&](const std::string mErrorMsg) {
        mGuiGlue->showMessage("HTTP Errror", mErrorMsg);
    };

    mStreamHandler->OnConnected = [&]() {
        if (mAudioHandler.get())  mAudioHandler->init(mStreamHandler->getStreamInfo());
        mStreamHandler->dumpInfo();

    };
    mStreamHandler->OnStreamTitleUpdate = [&](std::string title, size_t streamPosition) {
        if (mAudioHandler.get()) mAudioHandler->OnStreamTitleUpdate(title, streamPosition);
    };
    mStreamHandler->OnAudioChunk = [&](const void* buffer , size_t size) {  mAudioHandler->OnAudioChunk(buffer, size); };
    mStreamHandler->onDisConnected = [&]() {
        if (mAudioHandler.get()) mAudioHandler->onDisConnected();
        if (mAudioRecorder.get()) mAudioRecorder->closeFile();
        mRecording = false;
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

    if (mAppSettings.ShowRadio) DrawRadio();
    if (mAppSettings.ShowRecorder) DrawRecorder();
    if (mAppSettings.ShowFavo) DrawFavo();

    mGuiGlue->DrawEnd();
}



