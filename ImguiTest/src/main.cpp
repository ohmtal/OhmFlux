//-----------------------------------------------------------------------------
// ohmFlux ImguiTest
//-----------------------------------------------------------------------------

#include <fluxMain.h>
#include <SDL3/SDL_main.h> //<<< Android! and Windows
#include <gui/fluxGuiGlue.h>


class FluxGuiTest : public FluxMain
{
    typedef FluxMain Parent;
private:

    FluxGuiGlue* mGuiGlue = nullptr;
    FluxTexture* mBackgroundTex = nullptr;
    FluxRenderObject* mBackground = nullptr;

public:
    FluxGuiTest() {}
    ~FluxGuiTest() {}

    bool Initialize() override
    {
        if (!Parent::Initialize()) return false;

        mGuiGlue = new FluxGuiGlue(true);
        if (!mGuiGlue->Initialize())
            return false;

        mBackgroundTex = loadTexture("assets/background.bmp");

        // not centered ?!?!?! i guess center is not in place yet ?
        mBackground = new FluxRenderObject(mBackgroundTex);
        if (mBackground) {
            mBackground->setPos(getScreen()->getCenterF());
            mBackground->setSize(getScreen()->getScreenSize());
            queueObject(mBackground);
        }

        return true;
    }
    //--------------------------------------------------------------------------------------
    void Deinitialize() override
    {
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

    void onDrawTopMost() override
    {
        mGuiGlue->DrawBegin();
        ImGui::ShowDemoWindow();
        mGuiGlue->DrawEnd();

    } //Draw
}; //classe ImguiTest
//------------------------------------------------------------------------------
// Main
//------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
    (void)argc; (void)argv;
    FluxGuiTest* game = new FluxGuiTest();
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


