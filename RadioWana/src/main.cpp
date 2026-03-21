//-----------------------------------------------------------------------------
// RadioWana II
//-----------------------------------------------------------------------------
#include <SDL3/SDL_main.h> //<<< Android! and Windows
#include "appMain.h"
//------------------------------------------------------------------------------
// Main
//------------------------------------------------------------------------------

AppMain* gAppMain = nullptr;

AppMain* getGame() {
    return gAppMain;
}
AppMain* getMain() {
    return gAppMain;
}

int main(int argc, char* argv[])
{
    (void)argc; (void)argv;
    AppMain* app = new AppMain();
    app->mSettings.Company = "Ohmtal";
    app->mSettings.Caption = "RadioWana II";
    app->mSettings.Version = "1.260321";
    app->mSettings.enableLogFile   = true;
    app->mSettings.WindowMaximized = true;
    app->mSettings.ScreenWidth  = 1152; // 1920;
    app->mSettings.ScreenHeight =  648; //1080;

    //FIXME app->mSettings.IconFilename = "assets/icon.bmp";

    //FIXME Cursor
    // app->mSettings.CursorFilename = "assets/particles/BloodHand.bmp";
    // app->mSettings.cursorHotSpotX = 10;
    // app->mSettings.cursorHotSpotY = 10;


    gAppMain = app;



    app->Execute();
    SAFE_DELETE(app);
    return 0;
}


