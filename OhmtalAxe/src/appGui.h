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
#include "modules/rackModule.h"
#include "modules/keyboardModule.h"
#include "modules/drumKitLooper.h"

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
        bool mShowRackPresets;
    };

    ImConsole mConsole;

private:
    FluxRenderObject* mBackground = nullptr;
    FluxGuiGlue* mGuiGlue = nullptr;



    // ... Modules
    SoundMixModule*     mSoundMixModule = nullptr;
    WaveModule*         mWaveModule = nullptr;
    InputModule*        mInputModule = nullptr;
    RackModule*         mRackModule = nullptr;
    KeyBoardModule*     mKeyBoardModule = nullptr;
    DrumKitLooperModule*      mDrumKitLooperModule = nullptr;



    void OnConsoleCommand(ImConsole* console, const char* cmdline);


    AppSettings mAppSettings;
    AppSettings mDefaultAppSettings = {

        .mShowFileBrowser = false,
        .mShowConsole = false,
        .mShowWaveModule = false,
        .mShowDrumKit = true,
        .mShowVisualizer = false,
        .mShowRack    = true,
        .mShowRackPresets = true,

    };

public:

    bool Initialize() override;
    void Deinitialize() override;
    void onEvent(SDL_Event event);
    void DrawMsgBoxPopup();
    void ShowMenuBar();

    void ShowToolbar();

    //... Modules getter
    SoundMixModule* getSoundMixModule() const {return mSoundMixModule; }
    WaveModule* getWaveModule() const { return mWaveModule;}
    InputModule* getInputModule() const { return mInputModule;}
    RackModule* getRackModule() const { return mRackModule;}
    KeyBoardModule* getKeyBoardModule() const { return  mKeyBoardModule;}
    DrumKitLooperModule* getDrumKitLooperModule() const { return mDrumKitLooperModule; }

    void DrawGui( );
    void onKeyEvent(SDL_KeyboardEvent event);
    void InitDockSpace(); 

    void ShowFileBrowser();

    void ApplyStudioTheme();
    void setupFonts();

    AppSettings* getAppSettings() {return &mAppSettings;}


    void restoreLayout( );

    void setImGuiScale(float factor);



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
    mShowVisualizer,
    mShowRackPresets
)
