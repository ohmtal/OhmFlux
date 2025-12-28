#pragma once

#include <errorlog.h>
#include <game/fluxScene.h>

class EvoScene: public FluxScene
{
private:
public:
    EvoScene()  {
        setCaption("EvoScene");
    }

    void onEnter() override {
        Log("Enter EvoScene");
    }
    void onExit() override {
        Log("Exit EvoScene");
    }

    void Update(const double& dt) override {

    };

    void Draw() override;
};
