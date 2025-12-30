#pragma once

#include <errorlog.h>
#include <game/fluxScene.h>
#include "../amanaGame.h"

class EvoScene: public FluxScene
{
    typedef FluxScene Parent;
private:
public:
    EvoScene()  {
        setCaption("EvoScene");
    }
    bool OnInitialize() override
    {

        //... load content here ...

        return true;
    }

    void onEnter() override {
        Log("Enter EvoScene");
    }
    void onExit() override {
        Log("Exit EvoScene");
    }

    void Update(const double& dt) override { };

    void Draw() override { }

    void onKeyEvent(SDL_KeyboardEvent event) override
    {
        if ( event.type == SDL_EVENT_KEY_UP ) {
            switch ( event.key )
            {
                case SDLK_ESCAPE:
                    getGame()->setScene(getGame()->getMainMenu());
                    break;
            }
        }
    }

};
