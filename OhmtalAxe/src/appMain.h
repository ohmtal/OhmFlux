//-----------------------------------------------------------------------------
// Main App
//-----------------------------------------------------------------------------
#pragma once

#include <fluxMain.h>
#include "appGui.h"

class AppMain : public FluxMain
{
    typedef FluxMain Parent;
private:

    AppGui* mAppGui = nullptr;

public:
    AppMain() {}
    ~AppMain() {}

    bool Initialize() override
    {
        if (!Parent::Initialize()) return false;

        mAppGui = new AppGui();
        if (!mAppGui->Initialize())
            return false;

        return true;
    }
    //--------------------------------------------------------------------------------------
    void Deinitialize() override
    {
        mAppGui->Deinitialize();
        SAFE_DELETE(mAppGui);

        Parent::Deinitialize();
    }
    //--------------------------------------------------------------------------------------
    void onKeyEvent(SDL_KeyboardEvent event) override
    {
        bool isKeyUp = (event.type == SDL_EVENT_KEY_UP);
        bool isAlt =  event.mod & SDLK_LALT || event.mod & SDLK_RALT;
        if (event.key == SDLK_F4 && isAlt  && isKeyUp)
            TerminateApplication();
        else
            mAppGui->onKeyEvent(event);


    }
    //--------------------------------------------------------------------------------------
    void onMouseButtonEvent(SDL_MouseButtonEvent event) override    {    }
    //--------------------------------------------------------------------------------------
    void onEvent(SDL_Event event) override
    {
        mAppGui->onEvent(event);
    }
    //--------------------------------------------------------------------------------------
    void Update(const double& dt) override
    {
        Parent::Update(dt);
    }
    //--------------------------------------------------------------------------------------
    // imGui must be put in here !!
    void onDrawTopMost() override
    {
        mAppGui->DrawGui();
    }
    //--------------------------------------------------------------------------------------
    AppGui* getAppGui() const {return mAppGui; }
    AppGui::AppSettings* getAppSettings() {return mAppGui->getAppSettings();}




}; //classe ImguiTest

extern AppMain* gAppMain;
AppMain* getGame();
AppMain* getMain();
