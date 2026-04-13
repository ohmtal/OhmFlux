//-----------------------------------------------------------------------------
// Iron Tuner
//-----------------------------------------------------------------------------
#include <SDL3/SDL_main.h> //<<< Android! and Windows
#include "appMain.h"
//------------------------------------------------------------------------------
// Main
//------------------------------------------------------------------------------

IronTuner::AppMain* gAppMain = nullptr;

IronTuner::AppMain* getGame() {
    return gAppMain;
}
IronTuner::AppMain* getMain() {
    return gAppMain;
}

int main(int argc, char* argv[])
{
    (void)argc; (void)argv;
    IronTuner::AppMain* app = new IronTuner::AppMain();
    app->mSettings.Company = "Ohmtal";
    app->mSettings.Caption = "Iron Tuner";
    app->mSettings.Version = "0.260414.1";
    app->mSettings.enableLogFile   = true;
    app->mSettings.WindowMaximized = false;
    app->mSettings.ScreenWidth  = 1152; // 1920;
    app->mSettings.ScreenHeight =  648; //1080;
    app->mSettings.minWindowSize = {540,300};
    app->mSettings.IconFilename = "assets/icon64.bmp";

    app->mSettings.FullScreen = isAndroidBuild();

    // Cursor
    // app->mSettings.CursorFilename = "assets/particles/BloodHand.bmp";
    // app->mSettings.cursorHotSpotX = 10;
    // app->mSettings.cursorHotSpotY = 10;




    gAppMain = app;



    app->Execute();
    SAFE_DELETE(app);
    return 0;
}


