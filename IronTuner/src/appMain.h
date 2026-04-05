//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// APP Main
//-----------------------------------------------------------------------------
#pragma once

#include <fluxMain.h>
#include "appGui.h"
#include "BackGroundEffects.h"
#include "WindowState.h"
#include "AppSettings.h"

namespace IronTuner {


    class AppMain : public FluxMain
    {
        typedef FluxMain Parent;
    private:

        AppGui* mAppGui = nullptr;
        BackGroundEffects* mBackGroundEffects = nullptr;

        WindowState mWindowState;
        AppSettings mAppSettings;



    public:
        AppMain() {}
        ~AppMain() {}



        BackGroundEffects* getBackGroundRenderEffect() const {return mBackGroundEffects; }
        bool reloadBackGroundEffectsShader(int id = 0, bool scanLines = false) {
            if (mBackGroundEffects) return mBackGroundEffects->LoadShader(id, scanLines);
            return false;
        }
        inline static ImFont* mHackNerdFont12 = nullptr;
        inline static ImFont* mHackNerdFont16 = nullptr;
        inline static ImFont* mHackNerdFont20 = nullptr;
        inline static ImFont* mHackNerdFont26 = nullptr;


        bool Initialize() override;
        void Deinitialize() override;

        void onKeyEvent(SDL_KeyboardEvent event) override;
        void onMouseButtonEvent(SDL_MouseButtonEvent event) override    {    }
        void onEvent(SDL_Event event) override;

        void Update(const double& dt) override;
        void onDrawTopMost() override;
        void onDraw() override;;

        AppGui* getAppGui() const {return mAppGui; }
        //mutable!
        AppSettings& getAppSettings() {return mAppSettings; }
        WindowState& getWindowState() { return mWindowState; }


    };
};

extern IronTuner::AppMain* gAppMain;
IronTuner::AppMain* getGame();
IronTuner::AppMain* getMain();
