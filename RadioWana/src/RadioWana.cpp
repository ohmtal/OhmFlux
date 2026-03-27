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
#include <gui/ImFlux/widets/VirtualTapePlayer.h>



// #include <gui/ImConsole.h>
//------------------------------------------------------------------------------
// macro for JSON support not NOT in HEADER !!
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(RadioWana::AppSettings,
                                                // CurrentStation,
                                                Volume,
                                                DockSpaceInitialized,
                                                ShowFileBrowser,
                                                ShowConsole,
                                                ShowRadioBrowser,
                                                ShowRadio,
                                                ShowRecorder,
                                                ShowFavo,
                                                ShowEquilizer
)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(RadioWana::WindowState,
                                                width,
                                                height,
                                                posX,
                                                posY,
                                                maximized
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
//FIXME move recorder here
}
// -----------------------------------------------------------------------------
void RadioWana::DrawEquilizer(){
    if (!mAudioHandler.get() || !mAudioHandler->getManager()) return;
    DSP::Equalizer9Band* effect = static_cast<DSP::Equalizer9Band*>(mAudioHandler->getManager()->getEffectByType(DSP::EffectType::Equalizer9Band));
    if (!effect) return;
    const float boxHeight = 110.f;
    const float boxWidth = 400.f; //340.f;
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
            ImGui::SeparatorText("Equilizer");
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
            else info.name = " * * * offline * * *          ";
        }

    }


    if (ImGui::Begin("RadioWana", &mAppSettings.ShowRadio)) {
        // float fullWidth = ImGui::GetContentRegionAvail().x;

        const int lcdDigits    = 30;
        const int lcdDigits2   = 38; //40;

        const float displayWidth =  lcdDigits * 16.9f; //* 16.5f; // 20 ==> 320.f; //lcdDigits * lcdHeight1 * 0.5f;
        const float displayHeight = 60.f; //(2 * lcdHeight1) + ( 3 * lcdHeight1 * 0.5f);


        // -------- 1. TUNE -----------
        // ImFlux::GradientBox(ImVec2(0.f, 30.f));
        // if (mBrushedMetalTex) {
        //     ImFlux::TextureBox(ImVec2(0.f, 60.f), (ImTextureID)(intptr_t)mBrushedMetalTex->getHandle());
        // }
        //
        //
        // ImFlux::ShiftCursor(ImVec2(5.f,5.f));
        //




        // --------  INFO -----------

        ImFlux::GradientBox(ImVec2(0.f, displayHeight + 10.f));
        ImFlux::ShiftCursor(ImVec2(5.f,5.f));

        if (ImGui::BeginChild("##RadioDisplayStation", ImVec2(displayWidth  ,displayHeight ))) {
            ImFlux::GradientBoxDL(gRadioDisplayBox );
            ImGui::SameLine();ImFlux::ShiftCursor(ImVec2(5.f,6.f));
            ImGui::BeginGroup();
            ImGui::PushFont(getMain()->mHackNerdFont26);
            if (mTuningMode || isOffline) {
                if (mFavoStationData.size() > mSelectedFavIndex && mSelectedFavIndex >= 0) {
                    ImFlux::LCDTextScroller(mFavoStationData[mSelectedFavIndex].name, lcdDigits, ImFlux::COL32_NEON_PURPLE);
                }  else {
                    ImFlux::LCDTextScroller("*** failed to get station name ***", lcdDigits, ImFlux::COL32_NEON_RED);
                }
            } else {
                ImFlux::LCDTextScroller(mAudioHandler->getCurrentTitle(), lcdDigits, ImFlux::COL32_NEON_ORANGE);
            }


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

        ImGui::SameLine();
        TuneKnob("Tune Station", ImFlux::DARK_KNOB.WithRadius(48.f));


        //FIXME INFO MUST BE SOMEWHERE ELSE
        ImGui::SameLine();
        ImGui::BeginGroup();
        if (!isConnected) ImGui::BeginDisabled();
        if (ImFlux::ButtonFancy("Info", gRadioButtonParams.WithColor(IM_COL32(88,88,88,88) ))) {
            ImGui::OpenPopup("##StationInfo");
        }
        if (!isConnected) ImGui::EndDisabled();
        DrawInfoPopup(&info);

        ImGui::EndGroup();


        //----------------------------

        ImGui::Separator();

        // -------- 3. VOL + VU -----------


        ImFlux::GradientBox(ImVec2(0.f, 140.f));
        ImFlux::ShiftCursor(ImVec2(5.f,5.f));

        // // ~~~ Volume Button ~~~
        // ImGui::BeginGroup();
        // // ImFlux::ShiftCursor(ImVec2(10.f,0.f));
        // ImFlux::GradientBoxDL(gRadioDisplayBox.WithPosSize(ImVec2(0.f,0.f),ImVec2(65.f,60.f)) );
        // float vol = mAudioHandler->getVolume();
        // if (ImFlux::LEDMiniKnob("Volume", &vol, 0.f, 1.f, ImFlux::DARK_KNOB.WithRadius(28.f))) {
        //     mAudioHandler->setVolume(vol);
        //     mAppSettings.Volume = vol;
        // }
        // ImGui::EndGroup();
        //
        // ImGui::SameLine();

        // ~~~ VU Meter ~~~

        const ImVec2 vuSize = {140,70};
        float dbL, dbR;
        auto mapDB = [](float db) {
            float minDB = -20.0f;
            return (db < minDB) ? 0.0f : (db - minDB) / (0.0f - minDB);
        };
        // ~~~ VU Meter LEFT ~~~

        if ( mAudioHandler->getManager() && mAudioHandler->getManager()->getVisualAnalyzer()) {
            dbL = mAudioHandler->getManager()->getVisualAnalyzer()->getDecible(0);
            ImFlux::VUMeter70th(vuSize,mapDB(dbL), "L",  gRadioDisplayBox.col_top, gRadioDisplayBox.col_bot);

        }
        ImGui::SameLine();


        // ~~~ 9BandEQ ~~~
        if (mAppSettings.ShowEquilizer) {
            DrawEquilizer();
        } //show Equilizer


        ImGui::SameLine();
        if ( mAudioHandler->getManager() && mAudioHandler->getManager()->getVisualAnalyzer()) {
            dbR = mAudioHandler->getManager()->getVisualAnalyzer()->getDecible(0);
            ImFlux::VUMeter70th(vuSize,mapDB(dbR), "R", gRadioDisplayBox.col_top, gRadioDisplayBox.col_bot);
        }


        // dbR = mAudioHandler->getManager()->getVisualAnalyzer()->getDecible(1);
        // ImFlux::VUMeter70th(halfSize, mapDB(dbR), "R");


        // ~~~ VU Meter ~~~
        // ImGui::SameLine();
        // if ( mAudioHandler->getManager() && mAudioHandler->getManager()->getVisualAnalyzer()) {
        //     mAudioHandler->getManager()->getVisualAnalyzer()->renderVU(ImVec2(280,90), 70);
        // }


        //----------------------------

        // if ( mAudioHandler->getManager() && mAudioHandler->getManager()->getSpectrumAnalyzer()) {
        //
        //     ImFlux::ShiftCursor(ImVec2(0.f,10.f));
        //     mAudioHandler->getManager()->getSpectrumAnalyzer()->DrawSpectrumAnalyzer(ImVec2(fullWidth,60), true);
        // }




        // -------- 5. Recorder ----------
        if (mAppSettings.ShowRecorder) {

            ImFlux::GradientBox(ImVec2(0.f, 180.f));

            ImGui::PushFont(getMain()->mHackNerdFont16);
            ImFlux::ShadowText("RECORDER");
            ImGui::PopFont();
            ImGui::Separator();

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
            //FIXME
            if (ImGui::BeginChild("RECORDSETTINGS", ImVec2(0.f,110.f))) {
                float fullWidth = ImGui::GetContentRegionAvail().x;

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
            ImGui::EndChild();

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
                        Tune(*station);
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
    ImGui::PushFont(getMain()->mHackNerdFont20);
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
            ImGui::MenuItem("Equilizer", NULL, &mAppSettings.ShowEquilizer);

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

    std::string texPath = std::format("{}assets/brushed_black_metal_linear.png", getGamePath());
    mBrushedMetalTex = getMain()->loadTexture(texPath);

    // texPath = std::format("{}assets/metal_round_brush.png", getGamePath());
    // mBackgroundTex = getMain()->loadTexture(texPath);

    texPath = std::format("{}assets/knobs/silber256.png", getGamePath());
    mKnobSilverTex = getMain()->loadTexture(texPath);

    texPath = std::format("{}assets/knobs/roterrand.png", getGamePath());
    mKnobOffTex = getMain()->loadTexture(texPath);

    texPath = std::format("{}assets/knobs/gruenerrand.png", getGamePath());
    mKnobOnTex =  getMain()->loadTexture(texPath);



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
    if (mAppSettings.ShowFavo) DrawFavo();

    // if (isDebugBuild()) ImFlux::ShowCaseWidgets();

    mGuiGlue->DrawEnd();
}



