//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// WindowState
//-----------------------------------------------------------------------------
#pragma once

#include "utils/fluxSettingsManager.h"
#include "fluxRadio/RadioStation.h"
#include <core/fluxGlue.h>
#include "utils/errorlog.h"
#include <nlohmann/json.hpp>
#include <string>
#include <vector>


namespace IronTuner {

    struct AppSettings {
        FluxRadio::RadioStation CurrentStation;
        float Volume = 1.f;
        bool UIInitialized        = false;
        bool ShowConsole          = false;
        bool SideBarOpen          = false;
        int BackGroundRenderId    = 0;
        bool BackGroundScanLines  = false;

        int PageIndex             = 1; //radio
        float Scale               = 1.f;
        bool useVirtualKeyboard   = false;
        bool autoConnectOnStartUp = false;
        bool continuePlayingAfterDisconnect = false;

        bool disconnectOnBackground = true; //android
        bool requestBackgroundPlaying = false; //android

    };

    //--------------------------------------------------------------------------
    inline void to_json(nlohmann::json& j, const AppSettings& s){
        j = nlohmann::json{
            // NOT CurrentStation
            {"Volume",              s.Volume},
            {"UIInitialized",       s.UIInitialized},
            {"SideBarOpen",         s.SideBarOpen},
            {"BackGroundRenderId",  s.BackGroundRenderId},
            {"BackGroundScanLines", s.BackGroundScanLines},
            {"PageIndex", s.PageIndex},
            {"Scale", s.Scale},
            {"useVirtualKeyboard",s.useVirtualKeyboard},
            {"autoConnectOnStartUp",s.autoConnectOnStartUp},
            {"continuePlayingAfterDisconnect", s.continuePlayingAfterDisconnect},
            //android
            {"disconnectOnBackground", s.disconnectOnBackground},
            {"requestBackgroundPlaying", s.requestBackgroundPlaying},


        };
    }
    //--------------------------------------------------------------------------
    inline void from_json(const nlohmann::json& j, AppSettings& s){
        s.Volume                = j.value("Volume", 1.f);
        s.UIInitialized         = j.value("UIInitialized", false);
        s.SideBarOpen           = j.value("SideBarOpen", false);
        s.BackGroundRenderId    = j.value("BackGroundRenderId", 4);
        s.BackGroundScanLines   = j.value("BackGroundScanLines", false);
        s.PageIndex             = j.value("PageIndex", 1);
        s.Scale                 = j.value("Scale", 1.f);
        s.useVirtualKeyboard    = j.value("useVirtualKeyboard", false);
        s.autoConnectOnStartUp    = j.value("autoConnectOnStartUp", false);
        s.continuePlayingAfterDisconnect = j.value("continuePlayingAfterDisconnect", false);
        //android
        s.disconnectOnBackground = j.value("disconnectOnBackground", true);
        s.requestBackgroundPlaying = j.value("requestBackgroundPlaying", false);
    }



};
