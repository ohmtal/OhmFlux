#pragma once

#include <errorlog.h>
#include <fluxMain.h>
#include <gui/fluxGuiEventManager.h>
#include <game/fluxScene.h>
#include "../amanaGame.h"

class MainMenu: public FluxScene
{
    typedef FluxScene Parent;
private:
    FluxBitmapFont* mEvoSceneButton = nullptr;

public:
    MainMenu()
    {
        setCaption("MainMenu");
    }

    bool Initialize() override
    {
        if (!Parent::Initialize())
            return false;
        if (mInitialized)
            return true;

        // mEvoSceneButton = std::make_shared<FluxBitmapFont>(gRes.FontSourceCodeTexture, getGame()->getScreen());
        mEvoSceneButton = new FluxBitmapFont(gRes.FontSourceCodeTexture, getGame()->getScreen());
        mEvoSceneButton->set("[  evolutuion  ]", getGame()->getScreen()->getCenterX(), 100, 40, 48, cl_AcidGreen );
        mEvoSceneButton->setAlign(FontAlign_Center);

        mInitialized = true;
        return true;
    }


    void onEnter() override
    {
        Log("Enter MainMenu");

        if (!Initialize())
            return ;

        if (!mEvoSceneButton) {
            dLog("mEvoSceneButton is NULL!!!");
            return;
        }


        getGame()->queueObject(mEvoSceneButton);
        gRes.GuiEvents->bind(mEvoSceneButton ,
            SDL_EVENT_MOUSE_BUTTON_DOWN, [](const SDL_Event& e)
            {

                Log("Hello MouseButton! %d", e.button.button);
                if ( e.button.button == SDL_BUTTON_LEFT )
                    getGame()->setScene(getGame()->getEvoScene());
            }
        );

    }
    void onExit() override {
        Log("Exit MainMenu");

        getGame()->unQueueObject(mEvoSceneButton);
        gRes.GuiEvents->clear();
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
            }
        }
    }
    //--------------------------------------------------------------------------

};
