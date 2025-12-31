#pragma once

#include <utils/errorlog.h>
#include <game/fluxScene.h>
#include "../amanaGame.h"

class MapEditor: public FluxScene
{
    typedef FluxScene Parent;
private:
public:
    MapEditor()  {
        setCaption("MapEditor");
    }
    bool OnInitialize() override
    {

        //... load content here ...

        return true;
    }

    void onEnter() override {
        Log("Enter MapEditor");
    }
    void onExit() override {
        Log("Exit MapEditor");
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
