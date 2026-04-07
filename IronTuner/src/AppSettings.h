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
        // FIXME carussel

        bool DockSpaceInitialized = false;
        bool ShowConsole          = false;
        bool SideBarOpen          = false;
        // bool ShowFileBrowser      = false;
        // bool ShowRadioBrowser     = true;
        // bool ShowRadio            = true;
        // bool ShowRecorder         = false;
        // bool ShowFavo             = true;
        // bool ShowEqualizer        = true;
        //<<<<
        int BackGroundRenderId     = 0;
        bool BackGroundScanLines  = false;
    };

    //--------------------------------------------------------------------------
    inline void to_json(nlohmann::json& j, const AppSettings& s){
        j = nlohmann::json{
            // NOT CurrentStation
            {"Volume",              s.Volume},
            {"BackGroundRenderId",  s.BackGroundRenderId},
            {"BackGroundScanLines", s.BackGroundScanLines},
        };
    }
    //--------------------------------------------------------------------------
    inline void from_json(const nlohmann::json& j, AppSettings& s){
        s.Volume                = j.value("Volume", 1.f);
        s.BackGroundRenderId    = j.value("BackGroundRenderId", 0);
        s.BackGroundScanLines   = j.value("BackGroundScanLines", false);
    }



};
