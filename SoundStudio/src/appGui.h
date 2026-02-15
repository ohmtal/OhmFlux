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

#include "modules/sfxStereoModule.h"
#include "modules/sfxModule.h"
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
        bool mShowDemo;
        bool mShowSFXModule;
        bool mEditorGuiInitialized;
        bool mShowImFluxWidgets;
        bool mShowFileBrowser;
        bool mShowSFXStereoModule;
        bool mShowConsole;
        bool mShowWaveModule;
        bool mShowDrumKit;
    };

    ImConsole mConsole;

private:
    FluxRenderObject* mBackground = nullptr;
    FluxGuiGlue* mGuiGlue = nullptr;
    SfxStereoModule* mSfxStereoModule = nullptr;
    SfxModule* mSfxModule = nullptr;
    SoundMixModule* mSoundMixModule = nullptr;
    WaveModule* mWaveModule = nullptr;
    InputModule* mInputModule = nullptr;


    void OnConsoleCommand(ImConsole* console, const char* cmdline);


    AppSettings mAppSettings;
    AppSettings mDefaultAppSettings = {
        .mShowDemo = false,
        .mShowSFXModule = true,
        .mEditorGuiInitialized = false,
        .mShowImFluxWidgets = false,
        .mShowFileBrowser = false,
        .mShowSFXStereoModule = true,
        .mShowConsole = false,
        .mShowWaveModule = true,
        .mShowDrumKit = true
    };

public:

    bool Initialize() override;
    void Deinitialize() override;
    void onEvent(SDL_Event event);
    void DrawMsgBoxPopup();
    void ShowMenuBar();
    void DrawGui( );
    void onKeyEvent(SDL_KeyboardEvent event);
    void InitDockSpace(); 

    void ShowFileBrowser();

    void ApplyStudioTheme();

    AppSettings* getAppSettings() {return &mAppSettings;}


}; //class

// macro for JSON support
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(AppGui::AppSettings,
    mShowDemo,
    mShowSFXStereoModule,
    mShowSFXModule,
    mEditorGuiInitialized,
    mShowImFluxWidgets,
    mShowFileBrowser,
    mShowConsole,
    mShowWaveModule,
    mShowDrumKit
)
