//-----------------------------------------------------------------------------
// ohmFlux FluxEditor
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
    app->mSettings.Caption = "Ohmtal Sound Studio";
    app->mSettings.enableLogFile = true;
    app->mSettings.WindowMaximized = true;
    // game->mSettings.ScreenWidth  = 1920;
    // game->mSettings.ScreenHeight = 1080;
    // game->mSettings.IconFilename = "assets/particles/Skull2.bmp";
    // game->mSettings.CursorFilename = "assets/particles/BloodHand.bmp";
    // game->mSettings.cursorHotSpotX = 10;
    // game->mSettings.cursorHotSpotY = 10;


    gAppMain = app;



    app->Execute();
    SAFE_DELETE(app);
    return 0;
}


