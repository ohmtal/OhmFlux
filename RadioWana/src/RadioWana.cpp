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

// #include <gui/ImFlux/showCase.h>




// #include <gui/ImConsole.h>
//------------------------------------------------------------------------------
// macro for JSON support not NOT in HEADER !!
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(RadioWana::AppSettings,
                                                mUrl,
                                                CurrentFavId,
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
// -----------------------------------------------------------------------------
void RadioWana::OnConsoleCommand(ImConsole* console, const char* cmdline){
    std::string cmd = fluxStr::getWord(cmdline,0);

    if (cmd == "contenttype") {
       Log("%s", FluxNet::NetTools::getHeaderValue(mStreamHandler->getHeader(), "Content-Type").c_str());
    }
    if (cmd == "desc") {
        Log("%s", FluxNet::NetTools::getHeaderValue(mStreamHandler->getHeader(), "icy-description").c_str());
    }

}
// -----------------------------------------------------------------------------
void RadioWana::ApplyStudioTheme(){

    // cyan but i think the Title/Tabs are too light

    ImGuiStyle& style = ImGui::GetStyle();

    style.WindowRounding = 6.0f;
    style.FrameRounding = 4.0f;

}
// -----------------------------------------------------------------------------
bool RadioWana::isFavoStation(std::string searchUuid){
       auto it = std::find_if(mFavoStationData.begin(), mFavoStationData.end(),
                              [&searchUuid](const FluxRadio::RadioStation& s) {
                                  return s.stationuuid == searchUuid;
                              });
       return it != mFavoStationData.end();
   }
// -----------------------------------------------------------------------------
void RadioWana::DrawRecorder(){

    if (ImGui::Begin("Recorder", &mAppSettings.ShowRecorder)) {
        // float fullWidth = ImGui::GetContentRegionAvail().x;
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
void RadioWana::DrawRadio() {

    if (!mStreamHandler.get() || !mAudioHandler.get()) return;

    bool isConnected = mStreamHandler->isConnected();
    bool isRunning   = mStreamHandler->isRunning();

    FluxRadio::StreamInfo info = FluxRadio::StreamInfo();
    if (isConnected && mStreamHandler->getStreamInfo()) info = *mStreamHandler->getStreamInfo();
    else {
        if (isRunning) info.name = " * * * connecting * * *          ";
        else if (isConnected) info.name = " * * *          ";
        else info.name = " * * * offline * * *          ";
    }


    if (ImGui::Begin("RadioWana", &mAppSettings.ShowRadio)) {
        // float fullWidth = ImGui::GetContentRegionAvail().x;

        const int lcdDigits    = 30;
        const int lcdDigits2   = 38; //40;

        const float displayWidth =  lcdDigits * 16.9f; //* 16.5f; // 20 ==> 320.f; //lcdDigits * lcdHeight1 * 0.5f;
        const float displayHeight = 60.f; //(2 * lcdHeight1) + ( 3 * lcdHeight1 * 0.5f);


        // -------- 1. TUNE -----------
        ImFlux::GradientBox(ImVec2(0.f, displayHeight + 20));
        ImFlux::ShiftCursor(ImVec2(5.f,5.f));

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

        ImGui::SameLine();

        ImGui::BeginGroup();

        // dont like it
        // // STEPPER
        // static int displayIdx = -1;// FIXME replace mURL !! currentIdx;
        // std::vector<const char*> names;
        // int i = 0;
        // for (const auto& s : mFavoStationData) {
        //     if (displayIdx < 0 && mAppSettings.CurrentFavId == s.favId) {
        //         displayIdx = i;
        //     }
        //
        //     names.push_back(s.name.c_str());
        //     i++;
        // }
        //
        // ImGui::PushFont(getMain()->mHackNerdFont26);
        // if (ImFlux::ValueStepper("##StationStepper", &displayIdx, names.data(), (int)names.size(), 600.f))
        // {
        //     i = 0;
        //     for (const auto& s : mFavoStationData) {
        //         if (displayIdx == i) {
        //             mAppSettings.CurrentFavId = s.favId;
        //             break;
        //         }
        //         i++;
        //     }
        // }
        // ImGui::PopFont();
        // ImGui::SameLine();
        // if (ImFlux::ButtonFancy("TUNE", gRadioButtonParams)) {
        //     FluxRadio::RadioStation* pStation = getStationByFavId(&mFavoStationData, mAppSettings.CurrentFavId);
        //     if (pStation) {
        //         mAppSettings.mUrl = pStation->url;
        //         mStreamHandler->Execute(mAppSettings.mUrl);
        //         if (!pStation->stationuuid.empty()) mRadioBrowser->clickStation(pStation->stationuuid);
        //     } else {
        //         mGuiGlue->showMessage("ERROR","FIXME station not found mAppSettings.CurrentFavId is out of sync ? ");
        //     }
        // }



        // ImGui::SetNextItemWidth(450.f);
        char strBuff[256];
        strncpy(strBuff, mAppSettings.mUrl.c_str(), sizeof(strBuff));
        if (ImGui::InputText("URL", strBuff, sizeof(strBuff))) {
            mAppSettings.mUrl = strBuff;
        }
        ImGui::EndGroup();
        ImGui::Separator();


        // -------- 2. INFO -----------

        ImFlux::GradientBox(ImVec2(0.f, displayHeight + 10.f));
        ImFlux::ShiftCursor(ImVec2(10.f,5.f));
        ImGui::BeginGroup();
        if (!isConnected) ImGui::BeginDisabled();
        if (ImFlux::ButtonFancy("Info", gRadioButtonParams.WithColor(IM_COL32(88,88,88,88) ))) {
            ImGui::OpenPopup("##StationInfo");
        }
        if (!isConnected) ImGui::EndDisabled();
        DrawInfoPopup(&info);

        ImGui::EndGroup();

        // ?? ImGui::SameLine();ImFlux::ShiftCursor(ImVec2(20.f,0.f));

        ImGui::SameLine();
        // ImFlux::ShiftCursor(ImVec2(0.f,10.f));

        if (ImGui::BeginChild("##RadioDisplayStation", ImVec2(displayWidth  ,displayHeight ))) {
            ImFlux::GradientBoxDL(gRadioDisplayBox );
            ImGui::SameLine();ImFlux::ShiftCursor(ImVec2(5.f,6.f));
            ImGui::BeginGroup();
            ImGui::PushFont(getMain()->mHackNerdFont26);
            ImFlux::LCDTextScroller(mAudioHandler->getCurrentTitle(), lcdDigits, ImFlux::COL32_NEON_ORANGE);
            ImGui::PopFont();

            // ImGui::Spacing();
            ImGui::PushFont(getMain()->mHackNerdFont20);
            if ( mAudioHandler->getNextTitle().empty() ) {
                ImFlux::LCDTextScroller(info.name, lcdDigits2, ImFlux::COL32_NEON_ELECTRIC);
            } else {
                ImFlux::LCDTextScroller(mAudioHandler->getNextTitle(), lcdDigits2, ImFlux::COL32_NEON_GREEN);
            }
            ImGui::PopFont();


            ImGui::EndGroup();
        }
        ImGui::EndChild();

        //----------------------------

        ImGui::Separator();

        // -------- 3. VOL + VU -----------


        ImFlux::GradientBox(ImVec2(0.f, displayHeight + 10.f));
        ImFlux::ShiftCursor(ImVec2(5.f,5.f));

        // ~~~ Volume Button ~~~
        ImGui::BeginGroup();
        // ImFlux::ShiftCursor(ImVec2(10.f,0.f));
        ImFlux::GradientBoxDL(gRadioDisplayBox.WithPosSize(ImVec2(0.f,0.f),ImVec2(65.f,60.f)) );
        ImFlux::ShiftCursor(ImVec2(5.f,2.f));
        float vol = mAudioHandler->getVolume();
        if (ImFlux::LEDMiniKnob("Volume", &vol, 0.f, 1.f, ImFlux::DARK_KNOB.WithRadius(28.f))) {
            mAudioHandler->setVolume(vol);
            mAppSettings.Volume = vol;
        }
        ImGui::EndGroup();


        // ~~~ VU Meter ~~~
        ImGui::SameLine();
        if ( mAudioHandler->getManager() && mAudioHandler->getManager()->getVisualAnalyzer()) {
            mAudioHandler->getManager()->getVisualAnalyzer()->renderVU(ImVec2(320,60), 70);
        }
        ImFlux::ShiftCursor(ImVec2(0.f,10.f));

        //----------------------------

        // if ( mAudioHandler->getManager() && mAudioHandler->getManager()->getSpectrumAnalyzer()) {
        //
        //     mAudioHandler->getManager()->getSpectrumAnalyzer()->DrawSpectrumAnalyzer(ImVec2(fullWidth,60), true);
        // }

        // -------- 4. Rack / 9BandEQ -----------

        mAudioHandler->RenderRack(1);



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
        ImGui::OpenPopup("Favorite Dialog");

        if (ImGui::BeginPopupModal("Favorite Dialog", &showDialog, ImGuiWindowFlags_AlwaysAutoResize) ) {
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
                        mFavoStationData.push_back(workStation);
                        FluxRadio::updateFavIds(&mFavoStationData);
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

                bool isSelected =false;
                if (!isFavoList) isSelected = (mSelectedStationUuid == station->stationuuid);
                else isSelected  = (mSelectedFavId == station->favId);


                // ~~~~~~~~~ FAVO ~~~~~~~~~~~~~~
                ImGui::TableNextColumn();
                bool isFavo = false;
                if (isFavoList) isFavo = true;
                else isFavo = isFavoStation(station->stationuuid);

                if (ImFlux::FavoriteStar("Favorite", isFavo)) {
                    if (isFavoList) {
                        std::erase_if(mFavoStationData, [&](const FluxRadio::RadioStation& s) {
                            // return s.stationuuid == station->stationuuid;
                            return s.favId == station->favId;
                        });
                    } else {
                        if (!isFavo) {
                            mFavoStationData.push_back(*station);
                            FluxRadio::updateFavIds(&mFavoStationData);
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
                    if (!isFavoList)  mSelectedStationUuid = station->stationuuid;
                    else mSelectedFavId = station->favId;

                    if (ImGui::IsMouseDoubleClicked(0)) {
                        mAppSettings.mUrl = station->url;
                        mStreamHandler->Execute(mAppSettings.mUrl);
                        if (!station->stationuuid.empty()) mRadioBrowser->clickStation(station->stationuuid);
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


                if (!isFavoList) {
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
    FluxRadio::updateFavIds(&mFavoStationData);


    // ~~~~~ GuiGlue ~~~~~
    mGuiGlue = std::make_unique<FluxGuiGlue>(true, false, nullptr);
    if (!mGuiGlue->Initialize())
        return false;

    ApplyStudioTheme();

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
    std::string emptyStr = "";
    std::string effStr = SettingsManager().get("Audio::Effects", emptyStr);
    if (!effStr.empty() && mAudioHandler.get()) mAudioHandler->setEffectsSettingsBase64(effStr);


    mStreamHandler->OnConnecting = [&]() {
        //FIXME Display connecting .....
    };

    mStreamHandler->OnError = [&](const uint16_t errorCode, const std::string errorMsg) {
        mGuiGlue->showMessage("Stream Errror "+std::to_string(errorCode), errorMsg);
    };

    mStreamHandler->OnConnected = [&]() {
        if (mAudioHandler.get())  mAudioHandler->init(mStreamHandler->getStreamInfo());
        if (isDebugBuild()) mStreamHandler->dumpInfo();
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

    // if (isDebugBuild()) ImFlux::ShowCaseWidgets();

    mGuiGlue->DrawEnd();
}



