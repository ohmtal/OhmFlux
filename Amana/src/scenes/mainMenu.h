#pragma once

#include <errorlog.h>
#include <fluxMain.h>
#include <game/fluxScene.h>
#include "../amanaGame.h"

class MainMenu: public FluxScene
{
private:
public:
    MainMenu()  {
        setCaption("MainMenu press F1 for evoScene");
    }

    void onEnter() override {
        Log("Enter MainMenu");
    }
    void onExit() override {
        Log("Exit MainMenu");
    }

    void Update(const double& dt) override {

    };


    void Draw() override {

    }

    //--------------------------------------------------------------------------
    void onKeyEvent(SDL_KeyboardEvent event) override
    {
        if ( event.type == SDL_EVENT_KEY_UP ) {
            switch ( event.key )
            {
                case SDLK_ESCAPE:
                    getGame()->TerminateApplication();
                    break;
                case SDLK_F1:
                    getGame()->setScene(getGame()->getEvoScene());
                    break;
            }
        }
    }
    //--------------------------------------------------------------------------

};
