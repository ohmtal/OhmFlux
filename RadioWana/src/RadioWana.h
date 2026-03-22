//-----------------------------------------------------------------------------
// Copyright (c) 2012 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// RadioWana
//-----------------------------------------------------------------------------
#pragma once

// #include <audio/fluxAudio.h>
#include <core/fluxBaseObject.h>
#include <core/fluxRenderObject.h>
#include <gui/fluxGuiGlue.h>
#include <gui/ImConsole.h>
#include "net/CurlGlue.h"

#include "StreamHandler.h"
#include "AudioHandler.h"
#include "StreamInfo.h"
#include "AudioRecorder.h"
#include "RadioBrowser.h"



// -----------------------------------------------------------------------------
constexpr  ImFlux::ButtonParams gRadioButtonParams {
    .color = IM_COL32(20, 20, 30, 255),
    .size  = { 40.f, 40.f },
    .gloss = true, //<< does not work so good with round buttons
    .rounding = 6.f,

};

constexpr  ImFlux::GradientParams gRadioDisplayBox {
    // ImVec2 pos  = ImVec2(0, 0);
    // size = ImVec2(0, 0);
    .rounding = 0.f,
    // bool   inset    = true;

    // Background Colors
    .col_top   = IM_COL32(30, 30, 30, 255),
    .col_bot   = IM_COL32(15, 15, 15, 255),

    // Decoration Colors
    // ImU32 col_shadow = IM_COL32(0, 0, 0, 200);      // Inner shadow
    // ImU32 col_rim    = IM_COL32(255, 255, 255, 50); // Light edge
    // ImU32 col_raised = IM_COL32(55, 55, 65, 255);   // Base for !inset
};


// -----------------------------------------------------------------------------


class RadioWana: public FluxBaseObject {
private:
    //FIXME FluxRenderObject* mBackground = nullptr;

    void OnConsoleCommand(ImConsole* console, const char* cmdline) {}

    std::unique_ptr<FluxGuiGlue>  mGuiGlue;

    std::unique_ptr<FluxRadio::StreamHandler> mStreamHandler;
    std::unique_ptr<FluxRadio::AudioHandler>  mAudioHandler;
    std::unique_ptr<FluxRadio::AudioRecorder> mAudioRecorder;
    std::unique_ptr<FluxRadio::RadioBrowser> mRadioBrowser;


    // main
   //moved to AppSettings std::string mUrl = ""; //<< current Stream URL



    // {"ok":true,"message":"retrieved station url","stationuuid":"960594a6-0601-11e8-ae97-52543be04c81","name":"Rock Antenne",
    // "url":"http://mp3channels.webradio.rockantenne.de/rockantenne"}
    const std::vector<FluxRadio::RadioStation> mDefaultFavo = {
        {
            .stationuuid = "960594a6-0601-11e8-ae97-52543be04c81",
            .name= "Rock Antenne",
            .url = "http://mp3channels.webradio.rockantenne.de/rockantenne",
            .countrycode = "DE"
        },
        // {"ok":true,"message":"retrieved station url","stationuuid":"92556f58-20d3-44ae-8faa-322ce5f256c0",
        // "name":"Radio BOB!","url":"http://streams.radiobob.de/bob-national/mp3-192/mediaplayer"},
        {
            .stationuuid = "92556f58-20d3-44ae-8faa-322ce5f256c0",
            .name= "BOB! - Radio Bob",
            .url = "http://streams.radiobob.de/bob-national/mp3-192/mediaplayer",
            .countrycode = "DE"
        }
    };

    // Recordings
    bool mRecording = false;
    bool mRecordingStartsOnNewTile = true;
    bool mRecordingMixTape = false; //<< FIXME !!

    // Radio browser
    std::vector<FluxRadio::RadioStation> mQueryStationData;
    std::string mQueryString = "";
    std::string mSelectedStationUuid = "";

    std::vector<FluxRadio::RadioStation> mFavoStationData;


public:
    struct AppSettings {
        std::string mUrl = "http://mp3channels.webradio.rockantenne.de/rockantenne";
        float Volume = 1.f;
        bool DockSpaceInitialized = false;
        bool ShowFileBrowser      = false;
        bool ShowConsole          = false;
        bool ShowRadioBrowser     = true;
        bool ShowRadio            = true;
        bool ShowRecorder         = true;
        bool ShowFavo             = true;
    };


    ImConsole mConsole;
    AppSettings mAppSettings;

    bool Initialize() override;
    void Deinitialize() override;
    void SaveSettings();
    void onEvent(SDL_Event event);
    // void DrawMsgBoxPopup();
    void ShowMenuBar();
    // void ShowToolbar();

    void DrawGui( );
    void onKeyEvent(SDL_KeyboardEvent event) {};
    void InitDockSpace() {}
    void ShowFileBrowser() {}
    void ApplyStudioTheme() {}
    void setupFonts();
    AppSettings* getAppSettings() {return &mAppSettings;}
    void restoreLayout( ) {}
    void setImGuiScale(float factor) {}

    //-----
    void DrawFavo();

    void DrawStationsList(std::vector<FluxRadio::RadioStation> stations, bool isFavoList );
    void DrawRadioBrowserWindow();
    void DrawRadio();
    void DrawRecorder();

    bool isFavoStation(std::string searchUuid);

}; //class

