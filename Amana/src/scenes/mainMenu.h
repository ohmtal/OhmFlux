#pragma once

#include <errorlog.h>
#include <game/fluxScene.h>
#include "../amanaGame.h"

class MainMenu: public FluxScene
{
private:
public:
    MainMenu()  {
        setCaption("MainMenu");
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
};
