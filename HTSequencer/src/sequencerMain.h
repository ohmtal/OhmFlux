//-----------------------------------------------------------------------------
// FluxEditorMain
//-----------------------------------------------------------------------------
#pragma once

#include <fluxMain.h>
#include <OPL3Controller.h>
#include "sequencerGui.h"

class SequencerMain : public FluxMain
{
    typedef FluxMain Parent;
private:

    SequencerGui* mSeqGui = nullptr;

    OPL3Controller* mController = nullptr;

public:
    SequencerMain() {}
    ~SequencerMain() {}


    SequencerGui* getGui() { return mSeqGui; };
    OPL3Controller* getController() { return mController; };

    bool Initialize() override
    {
        if (!Parent::Initialize()) return false;

        mController = new OPL3Controller();
        if (!mController->initController())
        {
            Log("[error] Failed to Initialize OPL3Controller!");
            return false;
        }

        mSeqGui = new SequencerGui();
        if (!mSeqGui->Initialize())
            return false;

        FLUX_EVENT_COMPOSER_OPL_CHANNEL_CHANGED =  SDL_RegisterEvents(1);
        if (FLUX_EVENT_COMPOSER_OPL_CHANNEL_CHANGED == (Uint32)-1) {
            Log("ERROR: Failed to register SDL/FLUX Event: FLUX_EVENT_OPL_CHANNEL_CHANGED !!!!");
        }

        FLUX_EVENT_INSTRUMENT_OPL_CHANNEL_CHANGED =  SDL_RegisterEvents(1);
        if (FLUX_EVENT_INSTRUMENT_OPL_CHANNEL_CHANGED == (Uint32)-1) {
            Log("ERROR: Failed to register SDL/FLUX Event: FLUX_EVENT_INSTRUMENT_OPL_CHANNEL_CHANGED !!!!");
        }

        FLUX_EVENT_INSTRUMENT_OPL_INSTRUMENT_NAME_CHANGED = SDL_RegisterEvents(1);
        if (FLUX_EVENT_INSTRUMENT_OPL_INSTRUMENT_NAME_CHANGED == (Uint32)-1) {
            Log("ERROR: Failed to register SDL/FLUX Event: FLUX_EVENT_INSTRUMENT_OPL_INSTRUMENT_NAME_CHANGED !!!!");
        }





        return true;
    }
    //--------------------------------------------------------------------------------------
    void Deinitialize() override
    {
        mSeqGui->Deinitialize();
        SAFE_DELETE(mSeqGui);

        mController->shutDownController();
        SAFE_DELETE(mController);

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
            mSeqGui->onKeyEvent(event);


    }
    //--------------------------------------------------------------------------------------
    void onMouseButtonEvent(SDL_MouseButtonEvent event) override    {    }
    //--------------------------------------------------------------------------------------
    void onEvent(SDL_Event event) override
    {
        mSeqGui->onEvent(event);
    }
    //--------------------------------------------------------------------------------------
    void Update(const double& dt) override
    {
        Parent::Update(dt);
        if (mSeqGui)
            mSeqGui->Update(dt);
    }
    //--------------------------------------------------------------------------------------
    // imGui must be put in here !!
    void onDrawTopMost() override
    {
        mSeqGui->DrawGui();
    }
    //--------------------------------------------------------------------------------------



}; //classe ImguiTest

extern SequencerMain* g_SeqMain;
SequencerMain* getMain();

