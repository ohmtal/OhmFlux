// Amana
#pragma once

#include <fluxMain.h>
#include <game/fluxScene.h>

//--------------------------------------------------------------------------------------
// Scene forward declatations:
class EvoScene;
class MainMenu;


//--------------------------------------------------------------------------------------

class AmanaGame : public FluxMain
{
    typedef FluxMain Parent;

protected:
    FluxScene* mScene = nullptr;

    EvoScene*  mEvoScene = nullptr;
    MainMenu*  mMainMenu = nullptr;

private:

public:
    FluxScene* getScene()
    {
        //FIXME handle null!
        return mScene;
    }
    bool setScene( FluxScene* lNewScene ); // << simple statemachine


    FluxScene* getMainMenu();
    FluxScene* getEvoScene();

    bool Initialize() override;
    void Deinitialize() override;
    void onEvent(SDL_Event event) override;
    void Update(const double& dt) override;
    void onDraw() override;
    void onKeyEvent(SDL_KeyboardEvent event) override;
    void onMouseButtonEvent(SDL_MouseButtonEvent event) override;

    AmanaGame();
    virtual ~AmanaGame() { };

};

AmanaGame* getGame();
void AmanaGameMainLoop();
