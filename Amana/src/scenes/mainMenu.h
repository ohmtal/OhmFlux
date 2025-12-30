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
    FluxBitmapFont* mMapEditorButton = nullptr;
public:
    MainMenu()
    {
        setCaption("MainMenu");
    }
    ~MainMenu(){
        SAFE_DELETE(mEvoSceneButton);
        SAFE_DELETE(mMapEditorButton);
    }
    //--------------------------------------------------------------------------
    bool loadFontButton(FluxBitmapFont* &lButton, std::string lCaption, F32 lY, Color4F lColor = cl_AcidGreen)
    {
        lButton = new FluxBitmapFont(gRes.FontSourceCodeTexture);
        lButton->set(lCaption.c_str(), getGame()->getScreen()->getCenterX(), lY, 36, 40, lColor );
        lButton->setAlign(FontAlign_Center);
        return true;
    }
    //--------------------------------------------------------------------------
    bool OnInitialize() override
    {
        loadFontButton(mEvoSceneButton, "[  evolutuion  ]", 100);
        loadFontButton(mMapEditorButton, "[  Map Editor  ]", 150);

        return true;
    }
    //--------------------------------------------------------------------------
    void bindButton(FluxRenderObject* lButton, FluxScene* lTargetScene) {
        if (!lButton || !lTargetScene) return;

        // 1. Queue the object (replaces repeated getGame()->queueObject)
        getGame()->queueObject(lButton);

        setupClickEvent(gRes.GuiEvents, lButton, [lTargetScene](){ getGame()->setScene(lTargetScene); });

    }
    //--------------------------------------------------------------------------
    void onEnter() override
    {
        Log("Enter MainMenu");

        if (!Initialize())
            return ;

        bindButton(mEvoSceneButton, getGame()->getEvoScene());
        bindButton(mMapEditorButton, getGame()->getEditorScene());
    }
    //--------------------------------------------------------------------------
    void onExit() override {
        Log("Exit MainMenu");

        getGame()->unQueueObject(mEvoSceneButton);
        getGame()->unQueueObject(mMapEditorButton);
        gRes.GuiEvents->clear();
    }
    //--------------------------------------------------------------------------
    void Update(const double& dt) override {

    };
    //--------------------------------------------------------------------------
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
