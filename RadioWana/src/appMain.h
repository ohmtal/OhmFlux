//-----------------------------------------------------------------------------
// Main App
//-----------------------------------------------------------------------------
#pragma once

#include <fluxMain.h>
#include "RadioWana.h"
#include "BackGroundEffects.h"

class AppMain : public FluxMain
{
    typedef FluxMain Parent;
private:

    RadioWana* mAppGui = nullptr;

    FluxRadio::BackGroundEffects* mBackGroundEffects = nullptr;

public:
    AppMain() {}
    ~AppMain() {}

    bool reloadBackGroundEffectsShader(int id = 0, bool scanLines = false) {
        if (mBackGroundEffects) return mBackGroundEffects->LoadShader(id, scanLines);
        return false;
    }
    inline static ImFont* mHackNerdFont12 = nullptr;
    inline static ImFont* mHackNerdFont16 = nullptr;
    inline static ImFont* mHackNerdFont20 = nullptr;
    inline static ImFont* mHackNerdFont26 = nullptr;


    bool Initialize() override
    {
        if (!Parent::Initialize()) return false;

        mAppGui = new RadioWana();
        if (!mAppGui->Initialize())
            return false;

        mBackGroundEffects = new FluxRadio::BackGroundEffects();
        if (!mBackGroundEffects->Initialize()) {
            SAFE_DELETE(mBackGroundEffects);
            mBackGroundEffects = nullptr;
        } else {
            mBackGroundEffects->setAnalyzer(mAppGui->getSpectrumAnalyzer());
        }



        return true;
    }
    //--------------------------------------------------------------------------------------
    void Deinitialize() override
    {
        if (mBackGroundEffects) {
            mBackGroundEffects->Deinitialize();
            SAFE_DELETE(mBackGroundEffects);
        }

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
        if (mAppGui) {
            mAppGui->Update(dt);
            if (mBackGroundEffects && mAppGui->mAppSettings.RenderBackGroundEffect) {
                mBackGroundEffects->UpdateLevels(dt,
                    mAppGui->getAudioLevels());
            }
        }




        Parent::Update(dt);
    }
    //--------------------------------------------------------------------------------------
    // imGui must be put in here !!
    void onDrawTopMost() override
    {
        mAppGui->DrawGui();
    }
    //--------------------------------------------------------------------------------------
    void onDraw() override {

        if (mBackGroundEffects && mAppGui->mAppSettings.RenderBackGroundEffect) {
            mBackGroundEffects->Draw();
        }  else {
            if (mAppGui && mAppGui->mBrushedMetalTex) {
                DrawParams2D dp;
                dp.image = mAppGui->mBrushedMetalTex;
                dp.imgId = 0;
                dp.x = getScreen()->getCenterX();
                dp.y = getScreen()->getCenterY();
                dp.z = 0.f;
                dp.w = getScreen()->getWidth();
                dp.h = getScreen()->getHeight();

                Render2D.drawSprite(dp);
            }

        }

    };
    //--------------------------------------------------------------------------------------
    RadioWana* getAppGui() const {return mAppGui; }
    RadioWana::AppSettings* getAppSettings() {return mAppGui->getAppSettings();}




}; //classe ImguiTest

extern AppMain* gAppMain;
AppMain* getGame();
AppMain* getMain();
