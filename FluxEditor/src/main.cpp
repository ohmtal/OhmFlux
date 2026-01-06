//-----------------------------------------------------------------------------
// ohmFlux FluxEditor
//-----------------------------------------------------------------------------
// FIXME scaling on size changed...
//      => FluxScreen::updateWindowSize need a event for this
//-----------------------------------------------------------------------------

#include <SDL3/SDL_main.h> //<<< Android! and Windows
#include "fluxEditorMain.h"
//------------------------------------------------------------------------------
// Main
//------------------------------------------------------------------------------

FluxEditorMain* g_FluxEditor = nullptr;

FluxEditorMain* getGame() {
    return g_FluxEditor;
}

int main(int argc, char* argv[])
{
    (void)argc; (void)argv;
    FluxEditorMain* game = new FluxEditorMain();
    game->mSettings.Company = "Ohmflux";
    game->mSettings.Caption = "Flux Editor";
    game->mSettings.enableLogFile = true;
    game->mSettings.WindowMaximized = true;
    // game->mSettings.ScreenWidth  = 1920;
    // game->mSettings.ScreenHeight = 1080;
    // game->mSettings.IconFilename = "assets/particles/Skull2.bmp";
    // game->mSettings.CursorFilename = "assets/particles/BloodHand.bmp";
    // game->mSettings.cursorHotSpotX = 10;
    // game->mSettings.cursorHotSpotY = 10;

    // LogFMT("TEST: My pref path would be:{}", SDL_GetPrefPath(game->mSettings.Company, game->mSettings.Caption ));

    g_FluxEditor = game;

    game->Execute();
    SAFE_DELETE(game);
    return 0;
}


