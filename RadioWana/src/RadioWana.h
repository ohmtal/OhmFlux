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
#include "net/NetTools.h"
#include "core/fluxTexture.h"
#include "utils/fluxScheduler.h"
#include "utils/errorlog.h"

#include "DSP_VisualAnalyzer.h"

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

    void OnConsoleCommand(ImConsole* console, const char* cmdline);

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
            .countrycode = "DE",
            .favId = 1
        },
        // {"ok":true,"message":"retrieved station url","stationuuid":"92556f58-20d3-44ae-8faa-322ce5f256c0",
        // "name":"Radio BOB!","url":"http://streams.radiobob.de/bob-national/mp3-192/mediaplayer"},
        {
            .stationuuid = "92556f58-20d3-44ae-8faa-322ce5f256c0",
            .name= "BOB! - Radio Bob",
            .url = "http://streams.radiobob.de/bob-national/mp3-192/mediaplayer",
            .countrycode = "DE",
            .favId = 2
        },
        // not my music but can be used for a demo: NCS Radio (NoCopyrightSounds)
        {
            .stationuuid = "92556f58-20d3-44ae-8faa-322ce5f256c0",
            .name= "NCS Radio (NoCopyrightSounds)",
            .url = "https://stream.zeno.fm/ez4m4918n98uv",
            .favId = 3
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
    uint32_t mSelectedFavId = 0;

    std::vector<FluxRadio::RadioStation> mFavoStationData;
    std::vector<FluxRadio::RadioStation> mStationCache;

    int mSelectedFavIndex = -1; //not the id the index in the list
    bool mTuningMode = false;
    FluxScheduler::TaskID mTuningResetTaskID = 0;
    const double mTuningResetSec = 3.0f;

    Point2F mAudioLevels = {0.f, 0.f};

public:
    Point2F getAudioLevels() const { return mAudioLevels; }
    DSP::SpectrumAnalyzer* getSpectrumAnalyzer() {
        if ( mAudioHandler->getManager() && mAudioHandler->getManager()->getSpectrumAnalyzer()) {
            return mAudioHandler->getManager()->getSpectrumAnalyzer();
        }
        return nullptr;
    }

    FluxTexture* mBrushedMetalTex = nullptr;
    FluxTexture* mKnobSilverTex = nullptr;
    FluxTexture* mKnobOffTex = nullptr;
    FluxTexture* mKnobOnTex = nullptr;
    // FluxTexture* mBackgroundTex = nullptr;

    struct AppSettings {
        // removed :P std::string mUrl = "http://mp3channels.webradio.rockantenne.de/rockantenne";
        FluxRadio::RadioStation CurrentStation;
        float Volume = 1.f;
        bool DockSpaceInitialized = false;
        bool ShowFileBrowser      = false;
        bool ShowConsole          = false;
        bool ShowRadioBrowser     = true;
        bool ShowRadio            = true;
        bool ShowRecorder         = false;
        bool ShowFavo             = true;
        bool ShowEqualizer        = true;
        bool SideBarOpen          = false;
        int BackGroundRenderId     = 0;
        bool BackGroundScanLines  = false;
    };

    struct WindowState {
        int width    = 1152;
        int height   = 648;
        int posX     = 0;
        int posY     = 0;
        bool  maximized   = false;

        void sync() {
            SDL_Window* window = getScreenObject()->getWindow();
            if (!window) return;
            maximized = getScreenObject()->getWindowMaximized();
            // Window size
            SDL_GetWindowSize(window, &width, &height);
            // Window position
            SDL_GetWindowPosition(window, &posX, &posY);
        }

        void updateWindow() {
            SDL_Window* window = getScreenObject()->getWindow();
            if (!window) return;
            getScreenObject()->setWindowMaximized(maximized);
            SDL_SetWindowSize(window, width, height);
            if (posX != 0.f && posY != 0.f) SDL_SetWindowPosition(window, posX, posY);
        }
    };


    ImConsole mConsole;
    AppSettings mAppSettings;
    WindowState mWindowState;

    bool Initialize() override;
    void Deinitialize() override;
    void SaveSettings();
    void onEvent(SDL_Event event);
    void Update(const double& dt) override;
    // void DrawMsgBoxPopup();
    void ShowMenuBar();
    // void ShowToolbar();

    void onKeyEvent(SDL_KeyboardEvent event) {};
    void InitDockSpace();
    // void ShowFileBrowser() {}
    void ApplyStudioTheme();
    void setupFonts();
    AppSettings* getAppSettings() {return &mAppSettings;}
    void restoreLayout( );
    void setImGuiScale(float factor);

    //-----
    void DrawGui( );

    void DrawFavo();
    void DrawStationsList(std::vector<FluxRadio::RadioStation> stations, bool isFavoList );
    void DrawRadioBrowserWindow();
    void DrawInfoPopup(FluxRadio::StreamInfo* info);

    void DrawRadio();
    void DrawRecorder();
    void DrawEqualizer();

    // bool isFavoStation(std::string searchUuid);

    bool isFavoStation(const FluxRadio::RadioStation* station) {
        if ( !station ) return false;
        auto it = std::find_if(mFavoStationData.begin(), mFavoStationData.end(),
                               [&station](const FluxRadio::RadioStation& s) {
                                   return (
                                       (s.stationuuid != "" && s.stationuuid == station->stationuuid)
                                       || (s.url != "" &&  s.url == station->url)
                                   );
                               });
        return it != mFavoStationData.end();
    }
    FluxRadio::RadioStation* getFavoStation(const FluxRadio::RadioStation* station) {
        if ( !station ) return nullptr;
        auto it = std::find_if(mFavoStationData.begin(), mFavoStationData.end(),
                               [&station](const FluxRadio::RadioStation& s) {
                                   return (
                                       (s.stationuuid != "" && s.stationuuid == station->stationuuid)
                                       || (s.url != "" &&  s.url == station->url)
                                   );
                               });

        if (it != mFavoStationData.end()) {
            return &(*it);
        }
        return nullptr;

    }

    bool isCacheStation(const FluxRadio::RadioStation* station) {
        if ( !station ) return false;
        auto it = std::find_if(mStationCache.begin(), mStationCache.end(),
                               [&station](const FluxRadio::RadioStation& s) {
                                   return (
                                       (s.favId != 0 && s.favId == station->favId)
                                       || (s.stationuuid != "" && s.stationuuid == station->stationuuid)
                                       || (s.url != "" &&  s.url == station->url)
                                   );
                               });
        return it != mStationCache.end();
    }


    bool AddFavo(const FluxRadio::RadioStation* station) {
        if ( !station ) return false;
        if ( !isFavoStation(station) ) {
            mFavoStationData.push_back(*station);
        }
        FluxRadio::updateFavIds(&mFavoStationData);

        FluxRadio::RadioStation* favStation =  getFavoStation(station);
        if (favStation) {
            if ( !isCacheStation(favStation) ) {
                mStationCache.push_back(*favStation);
            }
            // update current station
            if (
                (favStation->url  != "" && mAppSettings.CurrentStation.url == favStation->url )
                || (favStation->stationuuid  != "" && mAppSettings.CurrentStation.stationuuid == favStation->stationuuid )
            ) {
                mAppSettings.CurrentStation.favId = favStation->favId;
            }
        }


        return true;
    }

    bool RmvFavoByFavId(const FluxRadio::RadioStation* station) {
        if (!station || station->favId < 1 ) return false;
        bool result = std::erase_if(mFavoStationData, [&](const FluxRadio::RadioStation& s) {
            return s.favId == station->favId;
        });

        if (result) {
            std::erase_if(mStationCache, [&](const FluxRadio::RadioStation& s) {
                return s.stationuuid == station->stationuuid;
            });

            // update current station
            if ( station->favId  ==  mAppSettings.CurrentStation.favId) {
                mAppSettings.CurrentStation.favId = 0;
            }

        }
        return result;
    }
    bool RmvFavoByUUID(const FluxRadio::RadioStation* station) {
        if (!station || station->stationuuid == "" ) return false;
        bool result = std::erase_if(mFavoStationData, [&](const FluxRadio::RadioStation& s) {
            return s.stationuuid == station->stationuuid;
        });

        if (result) {
            std::erase_if(mStationCache, [&](const FluxRadio::RadioStation& s) {
                return s.stationuuid == station->stationuuid;
            });
            // update current station
            if ( mAppSettings.CurrentStation.stationuuid == station->stationuuid ) {
                mAppSettings.CurrentStation.favId = 0;
            }
        }
        return result;
    }

    void DumpStationCache() {
        for (auto& s: mStationCache ) {
            Log("%03d %s (%s)", s.favId,  s.name.c_str(), s.stationuuid.c_str());
        }
    }


    // ---------- Tune Station -----------------
    bool ConnectCurrent() {
        if (!FluxNet::NetTools::isValidURL(mAppSettings.CurrentStation.url) ) {
            return false;
        }
        mStreamHandler->Execute(mAppSettings.CurrentStation.url);
        if (!mAppSettings.CurrentStation.stationuuid.empty()) mRadioBrowser->clickStation(mAppSettings.CurrentStation.stationuuid);
        return true;
    }
    void Disconnect() {
        mStreamHandler->stop();
    }

    void Tune(FluxRadio::RadioStation station) {
        mAppSettings.CurrentStation = station;

        if (!isCacheStation(&station)) { mStationCache.push_back(station); }
        ConnectCurrent();
        mSelectedFavId = - 1;
        mTuningMode = false;
    }

    //-------------------- TuneKnob Interger with overflow ---------------------------
    void setSelectedFavIndex();
    void TuneKnob(std::string caption, const ImFlux::KnobSettings ks = ImFlux::DARK_KNOB);


    // BackGroundRenderId
    void setBackGroundRenderId(int id, bool enableScanLines = false);

}; //class

