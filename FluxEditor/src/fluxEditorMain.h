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
