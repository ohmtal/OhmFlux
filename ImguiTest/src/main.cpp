//-----------------------------------------------------------------------------
// ohmFlux ImguiTest
//-----------------------------------------------------------------------------
// FIXME scaling on size changed...
//      => FluxScreen::updateWindowSize need a event for this
//-----------------------------------------------------------------------------

#include <fluxMain.h>
#include <SDL3/SDL_main.h> //<<< Android! and Windows


#include "soundEditorGui.h"
#include "fluxGuiGlue.h"
#include "fluxSfxEditor.h"

class ImGuiTest : public FluxMain
{
    typedef FluxMain Parent;
private:

    SoundEditorGui* mSoundEditor = nullptr;

public:
    ImGuiTest() {}
    ~ImGuiTest() {}

    bool Initialize() override
    {
        if (!Parent::Initialize()) return false;

        mSoundEditor = new SoundEditorGui();
        if (!mSoundEditor->Initialize())
            return false;

        return true;
    }
    //--------------------------------------------------------------------------------------
    void Deinitialize() override
    {
        mSoundEditor->Deinitialize();
        SAFE_DELETE(mSoundEditor);

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
        mSoundEditor->onEvent(event);
    }
    //--------------------------------------------------------------------------------------
    void Update(const double& dt) override
    {
        Parent::Update(dt);
    }
    //--------------------------------------------------------------------------------------

    void onDraw() override
    {
        mSoundEditor->Draw();
    } //Draw
}; //classe ImguiTest
//------------------------------------------------------------------------------
// Main
//------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
    (void)argc; (void)argv;
    ImGuiTest* game = new ImGuiTest();
    game->mSettings.Company = "Ohmflux";
    game->mSettings.Caption = "ImguiTest";
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


