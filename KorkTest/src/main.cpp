//-----------------------------------------------------------------------------
// ohmFlux KorkScript Testing
// Issues:
//  [ ] myPlatfromProcess needs to be filled
//  [ ] Script parse error when using id like : 1029.dump();
//       ==> Error parsing ("; expected"; token is dump) at 1:5
//       << error is from astGen.processTokens() cause a exception
//-----------------------------------------------------------------------------
#include <SDL3/SDL_main.h>

#include "appMain.h"

//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------

int main(int argc, char* argv[])
{
    (void)argc; (void)argv;
    KorkFlux::gMain = new KorkFlux::Main();
    KorkFlux::gMain->mSettings.Company = "Ohmtal";
    KorkFlux::gMain->mSettings.Caption = "KorkTestBed";
    KorkFlux::gMain->mSettings.enableLogFile = true;
    // game->mSettings.IconFilename = "assets/icon.png";
    // game->mSettings.CursorFilename = "assets/particles/BloodHand.bmp";
    // game->mSettings.cursorHotSpotX = 10;
    // game->mSettings.cursorHotSpotY = 10;

    // LogFMT("TEST: My pref path would be:{}", SDL_GetPrefPath(game->mSettings.Company, game->mSettings.Caption ));

    KorkFlux::gMain->Execute();
    SAFE_DELETE(KorkFlux::gMain);
    return 0;
}


