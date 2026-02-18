//-----------------------------------------------------------------------------
// Ohmtal Tracker
//-----------------------------------------------------------------------------
#include <SDL3/SDL_main.h> //<<< Android! and Windows
#include "otMain.h"
//------------------------------------------------------------------------------
// Main
//------------------------------------------------------------------------------

OTMain* g_TrackerMain = nullptr;

OTMain* getMain() {
    return g_TrackerMain;
}



int main(int argc, char* argv[])
{
    (void)argc; (void)argv;
    OTMain* app = new OTMain();
    app->mSettings.Company = "Ohmtal";
    app->mSettings.Caption = "Tracker";
    app->mSettings.enableLogFile = true;
    app->mSettings.WindowMaximized = false;
    // app->mSettings.ScreenWidth  = 1920;
    // app->mSettings.ScreenHeight = 1080;
    // app->mSettings.IconFilename = "assets/particles/Skull2.bmp";
    // app->mSettings.CursorFilename = "assets/particles/BloodHand.bmp";
    // app->mSettings.cursorHotSpotX = 10;
    // app->mSettings.cursorHotSpotY = 10;


    g_TrackerMain = app;



    app->Execute();
    SAFE_DELETE(app);
    return 0;
}


