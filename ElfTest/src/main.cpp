//-----------------------------------------------------------------------------
// ohmFlux KorkScript Testing
// Issues:
//  [ ] myPlatfromProcess needs to be filled
//-----------------------------------------------------------------------------
#include <SDL3/SDL_main.h>
#include "appMain.h"


//--------------------------------------------------------------------------------------

int main(int argc, char* argv[])
{
    (void)argc; (void)argv;

    auto obj = std::make_unique<ElfFlux::Main>();
    ElfFlux::gMain = obj.get();

    // ElfFlux::gMain->mSettings.ScreenWidth = 800;
    // ElfFlux::gMain->mSettings.ScreenHeight = 600;

    ElfFlux::gMain->mSettings.Company = "Ohmtal";
    ElfFlux::gMain->mSettings.Caption = "ElfTestBed";
    ElfFlux::gMain->mSettings.enableLogFile = true;
    ElfFlux::gMain->mSettings.IconFilename = "assets/icon.png";
    ElfFlux::gMain->mSettings.maxFPS = 0;
    ElfFlux::gMain->Execute();


    return 0;
}


