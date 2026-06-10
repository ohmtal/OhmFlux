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

    ElfFlux::gMain = new ElfFlux::Main();
    ElfFlux::gMain->mSettings.Company = "Ohmtal";
    ElfFlux::gMain->mSettings.Caption = "ElfTestBed";
    ElfFlux::gMain->mSettings.enableLogFile = true;
    ElfFlux::gMain->mSettings.IconFilename = "assets/icon.png";
    ElfFlux::gMain->mSettings.maxFPS = 0;
    ElfFlux::gMain->Execute();

    if (ElfFlux::gMain) SAFE_DELETE(ElfFlux::gMain);
    ElfFlux::gMain = nullptr;
    return 0;
}


