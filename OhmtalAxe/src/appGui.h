//-----------------------------------------------------------------------------
// Copyright (c) 2026 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// App Gui (Main Gui)
//-----------------------------------------------------------------------------

#pragma once

#include <audio/fluxAudio.h>
#include <core/fluxBaseObject.h>
#include <core/fluxRenderObject.h>
#include <gui/fluxGuiGlue.h>

#include <gui/ImConsole.h>

#include "modules/soundMixModule.h"
#include "modules/waveModule.h"
#include "modules/inputModule.h"

class AppGui: public FluxBaseObject
{
public:
    // dont forget to add a parameter 
    // a.) mDefaultEditorSettings
    // b.) on the bottom to the json macro!!! 
    struct AppSettings {
        bool mEditorGuiInitialized;

        bool mShowFileBrowser;
        bool mShowConsole;

        bool mShowWaveModule;
        bool mShowDrumKit;
        bool mShowDrumEffects;

        bool mShowVisualizer;
        bool mShowRack;
    };

    ImConsole mConsole;

private:
    FluxRenderObject* mBackground = nullptr;
    FluxGuiGlue* mGuiGlue = nullptr;
    SoundMixModule* mSoundMixModule = nullptr;
    WaveModule* mWaveModule = nullptr;
    InputModule* mInputModule = nullptr;


    void OnConsoleCommand(ImConsole* console, const char* cmdline);


    AppSettings mAppSettings;
    AppSettings mDefaultAppSettings = {

        .mShowFileBrowser = false,
        .mShowConsole = false,
        .mShowWaveModule = false,
        .mShowDrumKit = true,
        .mShowVisualizer = true,
        .mShowRack    = true,

    };

public:

    bool Initialize() override;
    void Deinitialize() override;
    void onEvent(SDL_Event event);
    void DrawMsgBoxPopup();
    void ShowMenuBar();

    void ShowToolbar();


    void DrawGui( );
    void onKeyEvent(SDL_KeyboardEvent event);
    void InitDockSpace(); 

    void ShowFileBrowser();

    void ApplyStudioTheme();

    AppSettings* getAppSettings() {return &mAppSettings;}


}; //class

// macro for JSON support
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(AppGui::AppSettings,
    mEditorGuiInitialized,
    mShowFileBrowser,
    mShowConsole,
    mShowWaveModule,
    mShowDrumKit,
    mShowDrumEffects,
    mShowRack,
    mShowVisualizer
)
