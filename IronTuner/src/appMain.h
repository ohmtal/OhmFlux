//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// APP Main
//-----------------------------------------------------------------------------
#pragma once

#include <fluxMain.h>
#include "RadioWana.h"
#include "BackGroundEffects.h"

class AppMain : public FluxMain
{
    typedef FluxMain Parent;
private:

    RadioWana* mAppGui = nullptr;

    FluxRadio::BackGroundEffects* mBackGroundEffects = nullptr;

public:
    AppMain() {}
    ~AppMain() {}


    FluxRadio::BackGroundEffects* getBackGroundRenderEffect() const {return mBackGroundEffects; }
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

    RadioWana* getAppGui() const {return mAppGui; }
    RadioWana::AppSettings* getAppSettings() {return mAppGui->getAppSettings();}


};

extern AppMain* gAppMain;
AppMain* getGame();
AppMain* getMain();
