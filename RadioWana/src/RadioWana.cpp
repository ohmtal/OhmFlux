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
void RadioWana::ApplyStudioTheme(){

    // cyan but i think the Title/Tabs are too light

    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    // --- GRUNDFARBEN ( Cyan & Anthrazit) ---
    const ImVec4 titles_col  = ImVec4(0.00f, 0.25f, 0.25f, 1.00f); // titles
    const ImVec4 hooverActive_col    = ImVec4(0.00f, 0.45f, 0.45f, 1.00f); // hover/ active
    const ImVec4 hoover2_col         = ImVec4(0.00f, 0.60f, 0.60f, 1.00f); // Hover-Cyan
    const ImVec4 hoover3_col         = ImVec4(0.00f, 0.80f, 0.80f, 1.00f); // actice Cyan
    const ImVec4 dark_bg     = ImVec4(0.10f, 0.10f, 0.12f, 0.80f); // nearly black => window background
    const ImVec4 dark_surface = ImVec4(0.26f, 0.26f, 0.28f, 1.00f); // panels

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
    colors[ImGuiCol_FrameBg]                = ImVec4(0.25f, 0.35f, 0.35f, 1.00f);
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
        if (isRunning) info.name = " * * * conecting * * *          ";
        else if (isConnected) info.name = " * * *          ";
        else info.name = " * * * offline * * *          ";
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
        ImFlux::ShiftCursor(ImVec2(70.f,5.f));



        // ~~~ Volume Button ~~~
        ImGui::BeginGroup();
        ImFlux::ShiftCursor(ImVec2(10.f,0.f));
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
            mAudioHandler->getManager()->getVisualAnalyzer()->renderVU(ImVec2(350,60), 70);
        }
        // ImFlux::ShiftCursor(ImVec2(0.f,10.f));

        //----------------------------

        // if ( mAudioHandler->getManager() && mAudioHandler->getManager()->getSpectrumAnalyzer()) {
        //
        //     mAudioHandler->getManager()->getSpectrumAnalyzer()->DrawSpectrumAnalyzer(ImVec2(fullWidth,60), true);
        // }

        mAudioHandler->RenderRack(1);

    }
    ImGui::End();
}
// -----------------------------------------------------------------------------
void RadioWana::DrawFavo() {
    if (ImGui::Begin("Favorites", &mAppSettings.ShowFavo)){
            DrawStationsList(mFavoStationData, true);
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

    // if (isDebugBuild()) ImFlux::ShowCaseWidgets();

    mGuiGlue->DrawEnd();
}



