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
        .mShowVisualizer = true,
        .mShowRack    = true,
        .mShowRackPresets = false,

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


    void restoreLayout( ) {
        //copied from json :P
       static const std::string layout = "[Window][File Browser]\nCollapsed=0\nDockId=0x00000002\n\n[Window][WindowOverViewport_11111111]\nPos=0,19\nSize=1920,996\nCollapsed=0\n\n[Window][Toolbar]\nPos=0,19\nSize=429,251\nCollapsed=0\nDockId=0x00000003,0\n\n[Window][Input Stream]\nPos=0,272\nSize=429,743\nCollapsed=0\nDockId=0x00000004,0\n\n[Window][Distortion]\nPos=431,19\nSize=412,170\nCollapsed=0\nDockId=0x0000000F,0\n\n[Window][OverDrive]\nPos=431,191\nSize=412,180\nCollapsed=0\nDockId=0x00000012,0\n\n[Window][Metal Distortion]\nPos=431,373\nSize=412,177\nCollapsed=0\nDockId=0x00000013,0\n\n[Window][Analog Glow]\nPos=431,552\nSize=412,241\nCollapsed=0\nDockId=0x00000009,0\n\n[Window][CHORUS / ENSEMBLE]\nPos=431,795\nSize=412,220\nCollapsed=0\nDockId=0x0000000A,0\n\n[Window][REVERB / SPACE]\nPos=845,19\nSize=307,220\nCollapsed=0\nDockId=0x00000018,0\n\n[Window][DELAY]\nPos=845,241\nSize=307,774\nCollapsed=0\nDockId=0x00000019,0\n\n[Window][9-BAND EQUALIZER]\nPos=0,19\nSize=429,251\nCollapsed=0\nDockId=0x00000003,1\n\n[Window][Post Digital Sound Effects Visualizer]\nPos=1049,420\nSize=871,595\nCollapsed=0\nDockId=0x00000008,0\n\n[Window][Drum Kit]\nPos=1355,19\nSize=565,533\nCollapsed=0\nDockId=0x00000011,0\n\n[Window][Debug##Default]\nPos=60,60\nSize=400,400\nCollapsed=0\n\n[Window][Post Digital Sound Effects Rack]\nPos=1188,340\nSize=732,675\nCollapsed=0\nDockId=0x00000008,1\n\n[Window][Effects Rack]\nPos=431,19\nSize=616,996\nCollapsed=0\nDockId=0x00000017,2\n\n[Window][Effects Rack 80th]\nPos=431,19\nSize=616,996\nCollapsed=0\nDockId=0x00000017,1\n\n[Window][Effect Paddles]\nPos=431,19\nSize=616,996\nCollapsed=0\nDockId=0x00000017,0\n\n[Window][Visualizer]\nPos=1355,554\nSize=565,461\nCollapsed=0\nDockId=0x0000001A,1\n\n[Window][Rack]\nPos=431,19\nSize=922,996\nCollapsed=0\nDockId=0x00000017,0\n\n[Window][Console]\nPos=1355,554\nSize=565,461\nCollapsed=0\nDockId=0x0000001A,0\n\n[Window][About]\nPos=833,424\nSize=254,166\nCollapsed=0\n\n[Docking][Data]\nDockSpace               ID=0x08BD597D Window=0x1BBC0F80 Pos=0,19 Size=1920,996 Split=X\n  DockNode              ID=0x0000000B Parent=0x08BD597D SizeRef=429,996 Split=Y Selected=0x5FDD3067\n    DockNode            ID=0x00000003 Parent=0x0000000B SizeRef=276,251 Selected=0x0C01D6D5\n    DockNode            ID=0x00000004 Parent=0x0000000B SizeRef=276,743 Selected=0x5FDD3067\n  DockNode              ID=0x0000000C Parent=0x08BD597D SizeRef=1489,996 Split=X\n    DockNode            ID=0x00000001 Parent=0x0000000C SizeRef=412,990\n    DockNode            ID=0x00000002 Parent=0x0000000C SizeRef=1506,990 Split=X\n      DockNode          ID=0x00000005 Parent=0x00000002 SizeRef=922,996 Split=X\n        DockNode        ID=0x0000000D Parent=0x00000005 SizeRef=412,996 Split=Y Selected=0xA2258052\n          DockNode      ID=0x0000000F Parent=0x0000000D SizeRef=319,170 Selected=0xA2258052\n          DockNode      ID=0x00000010 Parent=0x0000000D SizeRef=319,824 Split=Y Selected=0x1423D8F6\n            DockNode    ID=0x00000014 Parent=0x00000010 SizeRef=319,359 Split=Y Selected=0x1F6C469F\n              DockNode  ID=0x00000012 Parent=0x00000014 SizeRef=319,180 Selected=0x1423D8F6\n              DockNode  ID=0x00000013 Parent=0x00000014 SizeRef=319,177 Selected=0x1F6C469F\n            DockNode    ID=0x00000015 Parent=0x00000010 SizeRef=319,463 Split=Y Selected=0xFD30D3DD\n              DockNode  ID=0x00000009 Parent=0x00000015 SizeRef=412,241 Selected=0xFD30D3DD\n              DockNode  ID=0x0000000A Parent=0x00000015 SizeRef=412,220 Selected=0xDAC16BBC\n        DockNode        ID=0x0000000E Parent=0x00000005 SizeRef=341,996 Split=X\n          DockNode      ID=0x00000016 Parent=0x0000000E SizeRef=307,996 Split=Y Selected=0x236930EC\n            DockNode    ID=0x00000018 Parent=0x00000016 SizeRef=319,220 Selected=0x53D4BCAC\n            DockNode    ID=0x00000019 Parent=0x00000016 SizeRef=319,774 Selected=0x236930EC\n          DockNode      ID=0x00000017 Parent=0x0000000E SizeRef=32,996 CentralNode=1 Selected=0x61C337AE\n      DockNode          ID=0x00000006 Parent=0x00000002 SizeRef=565,996 Split=Y Selected=0xCD6D3427\n        DockNode        ID=0x00000007 Parent=0x00000006 SizeRef=550,399 Split=Y Selected=0xCD6D3427\n          DockNode      ID=0x00000011 Parent=0x00000007 SizeRef=871,362 Selected=0xCD6D3427\n          DockNode      ID=0x0000001A Parent=0x00000007 SizeRef=871,313 Selected=0x5C1B5396\n        DockNode        ID=0x00000008 Parent=0x00000006 SizeRef=550,595 Selected=0x8ECB2A60\n\n";

       // must be scheduled !!
       static FluxScheduler::TaskID loadFactorySchedule = 0;
       if (!FluxSchedule.isPending(loadFactorySchedule))
       {
           std::string tmpLayout = layout;
           loadFactorySchedule = FluxSchedule.add(0.0f, nullptr, [tmpLayout]() {
                     ImGui::LoadIniSettingsFromMemory(tmpLayout.c_str(), tmpLayout.size());
           });
       }
    }


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
