//-----------------------------------------------------------------------------
// ohmFlux ImguiTest
//-----------------------------------------------------------------------------
// FIXME scaling on size changed...
//      => FluxScreen::updateWindowSize need a event for this
//-----------------------------------------------------------------------------

#include <fluxMain.h>
#include <SDL3/SDL_main.h> //<<< Android! and Windows

#include <imgui.h>
#include <backends/imgui_impl_sdl3.h>
#include <backends/imgui_impl_opengl3.h>

#include "fluxGuiGlue.h"
#include "fluxSfxEditor.h"

class ImGuiTest : public FluxMain
{
    typedef FluxMain Parent;
private:

    FluxSfxEditor* mSfxEditor = nullptr;
    FluxGuiGlue* mGuiGlue = nullptr;

public:
    ImGuiTest() {}
    ~ImGuiTest() {}

    bool Initialize() override
    {
        if (!Parent::Initialize()) return false;


        mGuiGlue = new FluxGuiGlue();
        if (!mGuiGlue->Initialize())
            return false;

        mSfxEditor = new FluxSfxEditor();
        if (!mSfxEditor->Initialize())
            return false;

        return true;
    }
    //--------------------------------------------------------------------------------------
    void Deinitialize() override
    {
        SAFE_DELETE(mSfxEditor);
        SAFE_DELETE(mGuiGlue);

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
        mGuiGlue->onEvent(event);
    }
    //--------------------------------------------------------------------------------------
    void Update(const double& dt) override
    {

        Parent::Update(dt);
    }
    //--------------------------------------------------------------------------------------
    void onDraw() override
    {

        mGuiGlue->onRenderStart();

        ImGui::ShowDemoWindow();

        mSfxEditor->Draw();



        mGuiGlue->onRenderEnd();

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


