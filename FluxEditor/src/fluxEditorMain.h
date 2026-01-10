//-----------------------------------------------------------------------------
// FluxEditorMain
//-----------------------------------------------------------------------------
#pragma once

#include <fluxMain.h>
#include "editorGui.h"

class FluxEditorMain : public FluxMain
{
    typedef FluxMain Parent;
private:

    EditorGui* mEditorGui = nullptr;

public:
    FluxEditorMain() {}
    ~FluxEditorMain() {}

    bool Initialize() override
    {
        if (!Parent::Initialize()) return false;

        mEditorGui = new EditorGui();
        if (!mEditorGui->Initialize())
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
        mEditorGui->Deinitialize();
        SAFE_DELETE(mEditorGui);

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
            mEditorGui->onKeyEvent(event);


    }
    //--------------------------------------------------------------------------------------
    void onMouseButtonEvent(SDL_MouseButtonEvent event) override    {    }
    //--------------------------------------------------------------------------------------
    void onEvent(SDL_Event event) override
    {
        mEditorGui->onEvent(event);
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
        mEditorGui->DrawGui();
    }


}; //classe ImguiTest

extern FluxEditorMain* g_FluxEditor;
FluxEditorMain* getGame();
