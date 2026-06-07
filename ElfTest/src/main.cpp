//-----------------------------------------------------------------------------
// ohmFlux KorkScript Testing
// Issues:
//  [ ] myPlatfromProcess needs to be filled
//-----------------------------------------------------------------------------
#include <SDL3/SDL_main.h>
#include "appMain.h"
//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------

int main(int argc, char* argv[])
{
    (void)argc; (void)argv;
    ElfFlux::gMain = new ElfFlux::Main();
    ElfFlux::gMain->mSettings.Company = "Ohmtal";
    ElfFlux::gMain->mSettings.Caption = "KorkTestBed";
    ElfFlux::gMain->mSettings.enableLogFile = true;
    ElfFlux::gMain->mSettings.IconFilename = "assets/icon.png";
    ElfFlux::gMain->mSettings.maxFPS = 240;
    // game->mSettings.CursorFilename = "assets/particles/BloodHand.bmp";
    // game->mSettings.cursorHotSpotX = 10;
    // game->mSettings.cursorHotSpotY = 10;

    // LogFMT("TEST: My pref path would be:{}", SDL_GetPrefPath(game->mSettings.Company, game->mSettings.Caption ));

    ElfFlux::gMain->Execute();
    SAFE_DELETE(ElfFlux::gMain);
    return 0;
}


