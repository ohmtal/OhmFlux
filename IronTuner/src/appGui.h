//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// appGui
//-----------------------------------------------------------------------------
#pragma once

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

#include "fluxRadio/StreamHandler.h"
#include "fluxRadio/AudioHandler.h"
#include "fluxRadio/StreamInfo.h"
#include "fluxRadio/AudioRecorder.h"
#include "fluxRadio/RadioBrowser.h"

#include "StationHandler.h"
#include "Page.h"

namespace IronTuner {

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


    class AppGui: public FluxBaseObject {
    private:
        //FIXME FluxRenderObject* mBackground = nullptr;

        void OnConsoleCommand(ImConsole* console, const char* cmdline);

        std::unique_ptr<FluxGuiGlue>  mGuiGlue;

        std::unique_ptr<FluxRadio::StreamHandler> mStreamHandler;
        std::unique_ptr<FluxRadio::AudioHandler>  mAudioHandler;
        std::unique_ptr<FluxRadio::AudioRecorder> mAudioRecorder;
        std::unique_ptr<FluxRadio::RadioBrowser> mRadioBrowser;

        StationHandler mStations;

        std::vector<Page> mPages;



        // Recordings
        bool mRecording = false;
        bool mRecordingStartsOnNewTile = true;
        bool mRecordingMixTape = false; //<< FIXME !!


        int mReconnectOnTimeOutCount = 0;
        bool mTuningMode = false;
        FluxScheduler::TaskID mTuningResetTaskID = 0;
        const double mTuningResetSec = 3.0f;

        Point2F mAudioLevels = {0.f, 0.f};


        // CARUSEL windows test:
        int mTargetPageIndex = 0;
        float mCurrentScrollX = 0.0f;
        float mScrollSpeed = 10.0f;
        float mTouchStartX = 0.0f;
        Uint64 mCursorKeyDownStart = 0;
        SDL_Keycode  mCursorKeyDown = 0;
        const Uint64 mCursorChangeTime = 750;

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


        ImConsole mConsole;

        bool Initialize() override;
        void Deinitialize() override;
        void SaveSettings();

        void changePage(int step);
        std::string getChangePageName(int step);
        void handleSwipe(float deltaX);
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
        void restoreLayout( );
        void setImGuiScale(float factor);

        //-----
        void DrawGui( );

        void DrawFavo();
        void DrawRadioBrowserWindow();
        void DrawStationsList(const std::vector<FluxRadio::RadioStation> stations, const bool isFavoList );
        void DrawInfoPopup(FluxRadio::StreamInfo* info);

        void DrawRadio();
        void DrawRecorder();
        void DrawEqualizer();

        // bool isFavoStation(std::string searchUuid);





        // ---------- Tune Station -----------------
        bool ConnectCurrent();
        void Disconnect() {
            mStreamHandler->stop();
        }



        void Tune(FluxRadio::RadioStation station);

        //-------------------- TuneKnob Interger with overflow ---------------------------
        void TuneKnob(std::string caption, const ImFlux::KnobSettings ks = ImFlux::DARK_KNOB);




    }; //class

};
