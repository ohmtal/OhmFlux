#include <stdio.h>

#include <fluxMain.h>

#include "amanaGame.h"
#include "globals.h"
#include "globalResources.h"

#include "scenes/evoScene.h"
#include "scenes/mainMenu.h"
//--------------------------------------------------------------------------------------
AmanaGame* g_Game = nullptr;

AmanaGame* getGame()
{
    return g_Game;
}

void AmanaGameMainLoop()
{
    atexit(SDL_Quit);

    g_Game = new AmanaGame;
    g_Game->mSettings.Caption="Amana";
    g_Game->mSettings.Version="0.251228";
    g_Game->mSettings.ScreenWidth=1152;
    g_Game->mSettings.ScreenHeight=648;
    g_Game->mSettings.ScaleScreen = true; //default true
    // g_Game->mSettings.IconFilename = "assets/icon.bmp";
    // g_Game->mSettings.CursorFilename = "assets/fishnet.bmp";
    // g_Game->mSettings.cursorHotSpotX = 11;
    // g_Game->mSettings.cursorHotSpotY = 3;
    g_Game->mSettings.initialVsync = true;

    g_Game->mSettings.maxSprites = 100000;


    g_Game->Execute();
    SAFE_DELETE(g_Game);
}
//--------------------------------------------------------------------------------------
AmanaGame::AmanaGame()
{
    mEvoScene = new EvoScene();
    mMainMenu = new MainMenu();
}
//--------------------------------------------------------------------------------------
// simple state machine onEnter/onExit ....
bool AmanaGame::setScene( FluxScene* lNewScene )
{
    if (!lNewScene)
        return false;

    if (getScene())
        getScene()->onExit();

    mScene = lNewScene;
    mScene->onEnter();

    return true;
}
//--------------------------------------------------------------------------------------
bool AmanaGame::Initialize()
{
    if (!Parent::Initialize()) return false;

    if (!gRes.init())
        return false;

    setScene(mMainMenu);
    // setScene(mEvoScene);
    return true;
}
//--------------------------------------------------------------------------------------
void AmanaGame::Deinitialize()
{
    SAFE_DELETE(mMainMenu);
    SAFE_DELETE(mEvoScene);
}
//--------------------------------------------------------------------------------------
void AmanaGame::onEvent(SDL_Event event) {
    getScene()->onEvent(event);
}
//--------------------------------------------------------------------------------------
void AmanaGame::onKeyEvent(SDL_KeyboardEvent event)
{
    // default key handling ESC/ALT+ENTER/PAUSE
    if ( event.type == SDL_EVENT_KEY_UP ) {
        switch ( event.key ) {
            case SDLK_ESCAPE:
                TerminateApplication();
                break;
            case SDLK_RETURN:
                if ( event.mod & SDL_KMOD_LALT)
                    toggleFullScreen();
            break;
            case SDLK_PAUSE:
                if (togglePause()) {
                    Log("Now paused....");
                }

                else
                    Log("pause off");
            break;
        } //switch

    } //KEY_UP

    // pass to Scene:
    getScene()->onKeyEvent(event);
}
//--------------------------------------------------------------------------------------
void AmanaGame::onMouseButtonEvent(SDL_MouseButtonEvent event){
    getScene()->onMouseButtonEvent(event);
}
//--------------------------------------------------------------------------------------
void AmanaGame::Update(const double& dt){
    getScene()->Update(dt);
}
//--------------------------------------------------------------------------------------
void AmanaGame::onDraw()
{
    gRes.StatusLabel->setCaptionFMT("{} at {}fps ", getScene()->getCaption(), getGame()->getFPS());
    getScene()->Draw();
}
//--------------------------------------------------------------------------------------
