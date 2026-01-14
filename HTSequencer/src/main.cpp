//-----------------------------------------------------------------------------
// ohmFlux FluxEditor
//-----------------------------------------------------------------------------
#include <SDL3/SDL_main.h> //<<< Android! and Windows
#include "sequencerMain.h"
//------------------------------------------------------------------------------
// Main
//------------------------------------------------------------------------------

SequencerMain* g_SeqMain = nullptr;

SequencerMain* getMain() {
    return g_SeqMain;
}

int main(int argc, char* argv[])
{
    (void)argc; (void)argv;
    SequencerMain* game = new SequencerMain();
    game->mSettings.Company = "Ohmtal";
    game->mSettings.Caption = "HT-Sequencer";
    game->mSettings.enableLogFile = true;
    game->mSettings.WindowMaximized = false;
    // game->mSettings.ScreenWidth  = 1920;
    // game->mSettings.ScreenHeight = 1080;
    // game->mSettings.IconFilename = "assets/particles/Skull2.bmp";
    // game->mSettings.CursorFilename = "assets/particles/BloodHand.bmp";
    // game->mSettings.cursorHotSpotX = 10;
    // game->mSettings.cursorHotSpotY = 10;


    g_SeqMain = game;



    game->Execute();
    SAFE_DELETE(game);
    return 0;
}


