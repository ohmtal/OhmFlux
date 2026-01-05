//-----------------------------------------------------------------------------
// ohmFlux FluxEditor
//-----------------------------------------------------------------------------
// FIXME scaling on size changed...
//      => FluxScreen::updateWindowSize need a event for this
//-----------------------------------------------------------------------------

#include <fluxMain.h>
#include <SDL3/SDL_main.h> //<<< Android! and Windows


#include "editorGui.h"

class FluxEditor : public FluxMain
{
    typedef FluxMain Parent;
private:

    EditorGui* mEditorGui = nullptr;

public:
    FluxEditor() {}
    ~FluxEditor() {}

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
        if (event.key == SDLK_ESCAPE && isKeyUp)
            TerminateApplication();
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

    void onDraw() override
    {
        mEditorGui->Draw();
    } //Draw
}; //classe ImguiTest
//------------------------------------------------------------------------------
// Main
//------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
    (void)argc; (void)argv;
    FluxEditor* game = new FluxEditor();
    game->mSettings.Company = "Ohmflux";
    game->mSettings.Caption = "Editor";
    game->mSettings.enableLogFile = true;
    game->mSettings.WindowMaximized = true;
    // game->mSettings.ScreenWidth  = 1920;
    // game->mSettings.ScreenHeight = 1080;
    // game->mSettings.IconFilename = "assets/particles/Skull2.bmp";
    // game->mSettings.CursorFilename = "assets/particles/BloodHand.bmp";
    // game->mSettings.cursorHotSpotX = 10;
    // game->mSettings.cursorHotSpotY = 10;

    // LogFMT("TEST: My pref path would be:{}", SDL_GetPrefPath(game->mSettings.Company, game->mSettings.Caption ));

    game->Execute();
    SAFE_DELETE(game);
    return 0;
}


