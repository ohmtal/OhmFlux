//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// WindowState
//-----------------------------------------------------------------------------
#pragma once

#include "utils/fluxSettingsManager.h"
#include <core/fluxGlue.h>
#include "utils/errorlog.h"
#include <nlohmann/json.hpp>
#include <string>
#include <vector>


namespace IronTuner {

    struct WindowState {
        int width    = 1152;
        int height   = 648;
        int posX     = 0;
        int posY     = 0;
        bool  maximized   = false;
        bool  fullScreen   = false;

        void sync() {
            SDL_Window* window = getScreenObject()->getWindow();
            if (!window) return;

            fullScreen = getScreenObject()->getFullScreen();
            maximized = getScreenObject()->getWindowMaximized();
            // Window size
            SDL_GetWindowSize(window, &width, &height);
            // Window position
            SDL_GetWindowPosition(window, &posX, &posY);
        }

        void updateWindow() {
            SDL_Window* window = getScreenObject()->getWindow();
            if (!window) return;
            getScreenObject()->setFullScreen(fullScreen);
            // we have fullScreen ignore maximized and position
            if (!fullScreen)  {
                getScreenObject()->setWindowMaximized(maximized);
                SDL_SetWindowSize(window, width, height);
                if (posX != 0.f && posY != 0.f) SDL_SetWindowPosition(window, posX, posY);
            }
        }
    };

    //--------------------------------------------------------------------------
    inline void to_json(nlohmann::json_abi_v3_12_0::json& j, const WindowState& s){
        j = nlohmann::json{
            {"width",   s.width},
            {"height",  s.height},
            {"posX",    s.posX},
            {"posY",    s.posY},
            {"maximized",   s.maximized},
            {"fullScreen",  s.fullScreen},
        };
    }
    //--------------------------------------------------------------------------
    inline void from_json(const nlohmann::json_abi_v3_12_0::json& j, WindowState& s){
        s.width     = j.value("width", 1152);
        s.height    = j.value("height", 648);
        s.posX      = j.value("posX", 0);
        s.posY      = j.value("posY", 0);
        s.maximized = j.value("maximized", false);
        s.fullScreen    = j.value("fullScreen", false);

    }




};
