#include <stdio.h>

#include <fluxMain.h>

#include "amanaGame.h"
#include "globals.h"
#include "globalResources.h"

#include "scenes/evoScene.h"
#include "scenes/mainMenu.h"
#include "scenes/mapEditor.h"
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
    mMapEditor = new MapEditor();
}

AmanaGame::~AmanaGame()
{
    SAFE_DELETE(mMainMenu);
    SAFE_DELETE(mEvoScene);
    SAFE_DELETE(mMapEditor);
}

FluxScene* AmanaGame::getMainMenu() {
    return static_cast<FluxScene*>(mMainMenu);
}
FluxScene* AmanaGame::getEvoScene()  {
    return static_cast<FluxScene*>(mEvoScene);

}
FluxScene* AmanaGame::getEditorScene() {
    return static_cast<FluxScene*>(mMapEditor);
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
}
//--------------------------------------------------------------------------------------
void AmanaGame::onEvent(SDL_Event event)
{
    if ( gRes.GuiEvents != nullptr)
        gRes.GuiEvents->onEvent(event);
    getScene()->onEvent(event);
}
//--------------------------------------------------------------------------------------
void AmanaGame::onKeyEvent(SDL_KeyboardEvent event)
{
    // default key handling
    if ( event.type == SDL_EVENT_KEY_UP ) {
        switch ( event.key )
        {
            case SDLK_RETURN:
                if ( event.mod & SDL_KMOD_LALT)
                    toggleFullScreen();
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
