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
        bool ShowFileBrowser      = false;
        bool ShowConsole          = false;
        bool ShowRadioBrowser     = true;
        bool ShowRadio            = true;
        bool ShowRecorder         = false;
        bool ShowFavo             = true;
        bool ShowEqualizer        = true;
        bool SideBarOpen          = false;
        //<<<<
        int BackGroundRenderId     = 0;
        bool BackGroundScanLines  = false;
    };

    //--------------------------------------------------------------------------
    inline void to_json(nlohmann::json_abi_v3_12_0::json& j, const AppSettings& s){
        j = nlohmann::json{
            // NOT CurrentStation
            {"Volume",              s.Volume},
            {"BackGroundRenderId",  s.BackGroundRenderId},
            {"BackGroundScanLines", s.BackGroundScanLines},
        };
    }
    //--------------------------------------------------------------------------
    inline void from_json(const nlohmann::json_abi_v3_12_0::json& j, AppSettings& s){
        s.Volume                = j.value("Volume", 1.f);
        s.BackGroundRenderId    = j.value("BackGroundRenderId", 1.f);
        s.BackGroundScanLines   = j.value("BackGroundScanLines", 1.f);
    }



};
