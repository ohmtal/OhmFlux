//-----------------------------------------------------------------------------
// ohmFlux AudiTestBed
// Reimplementation of better Sound System
//-----------------------------------------------------------------------------
#include <SDL3/SDL_main.h>

#include "fluxMain.h"
#include "gui/fluxGuiGlue.h"
#include "gui/ImConsole.h"
#include "gui/ImFileDialog.h"
#include "audio/AudioResourceManager.h"

//-----------------------------------------------------------------------------
// console redirect ....
void SDLCALL ConsoleLogFunction(void *userdata, int category, SDL_LogPriority priority, const char *message)
{
    if (!userdata) return;
    auto* console = static_cast<ImConsole*>(userdata);
    if (!console) return;

    char lBuffer[1024];
    snprintf(lBuffer, sizeof(lBuffer), "%s", message);
    console->AddLog("%s", message);
}
//-----------------------------------------------------------------------------
class AudioTestBed : public FluxMain
{
    typedef FluxMain Parent;
    ImConsole console;
    ImFileDialog fileDialog;
    std::unique_ptr<FluxGuiGlue> mGuiGlue;
    std::unique_ptr<FluxAudio::AudioResourceManager> mAudioResourceManager;



    // -------------------------------------------------------------------------
    void OnConsoleCommand(ImConsole* console, const char* command_line) {
        std::string cmdLineStr = command_line;
        std::string cmd = FluxStr::getWord(cmdLineStr, 0);

        if (cmd == "list") {
            for (auto& [key, val] : mAudioResourceManager->getMap()) {
                Log("%s type:%d size:%d", key.c_str(), (int)val->fileType, (int)val->mRawData.size());
            }
        }

    }
    // -------------------------------------------------------------------------


public:
    bool Initialize() override
    {
        if (!Parent::Initialize()) return false;

        // setting ini here is ok for a testBed
        mGuiGlue = std::make_unique<FluxGuiGlue>(true, false, "AudioTestBed.ini");
        if (!mGuiGlue->Initialize())
            return false;

        // setting ini here is ok for a testBed
        mAudioResourceManager = std::make_unique<FluxAudio::AudioResourceManager>();
        if (!mAudioResourceManager->Initialize())
            return false;


        SDL_SetLogOutputFunction(ConsoleLogFunction, &console);
        console.OnCommand = [&](ImConsole* console, const char* command_line) {
            OnConsoleCommand(console, command_line);
        };

        fileDialog.init( getGamePath(), { ".ogg", ".wav", ".mp3", ".sfx" });


         return true;
    }
    //--------------------------------------------------------------------------------------
    void Deinitialize() override
    {

        SDL_SetLogOutputFunction(nullptr, nullptr);
        mGuiGlue->Deinitialize();
        mAudioResourceManager->Deinitialize();

        Parent::Deinitialize();
    }
    //--------------------------------------------------------------------------------------
    void onKeyEvent(SDL_KeyboardEvent event) override
    {

    }
    //--------------------------------------------------------------------------------------
    void onMouseButtonEvent(SDL_MouseButtonEvent event) override
    {

    }
    //--------------------------------------------------------------------------------------
    void onEvent(SDL_Event event) override
    {
        if (mGuiGlue) {
            mGuiGlue->onEvent(event);
            if (mGuiGlue->getGuiIO() && mGuiGlue->getGuiIO()->WantTextInput) {
                if (event.type == SDL_EVENT_KEY_DOWN ||
                    event.type == SDL_EVENT_KEY_UP ||
                    event.type == SDL_EVENT_TEXT_INPUT) {
                    return;
                    }
            }
        }
        // other stuff...

    }
    //--------------------------------------------------------------------------------------
    void Update(const double& dt) override
    {


        Parent::Update(dt);
    }
    //--------------------------------------------------------------------------------------
    void onDraw() override
    {

    } //Draw

    //------------------------------------------------------------------------------
    void DrawFileBrowser(){
        if (fileDialog.Draw()) {
            // LogFMT("File:{} Ext:{}", g_FileDialog.selectedFile, g_FileDialog.selectedExt);
            if (fileDialog.mSaveMode)
            {
                fileDialog.reset();
            } else {
                // NOTE: LOAD RESOURCE TEST
                if (!mAudioResourceManager->add(fileDialog.selectedFile)) {
                    mGuiGlue->showMessage("Error", "Failed to load File " + fileDialog.selectedFile + " !");
                } else {
                    Log("[info] file %s loaded :)", fileDialog.selectedFile.c_str());
                }
            }
        }
    }
    //--------------------------------------------------------------------------------------
    virtual void onDrawTopMost() override {
        Parent::onDrawTopMost();

        if (!mGuiGlue) return;
        mGuiGlue->DrawBegin();

        static bool showConsole = true;
        static bool showFileBrowser = true;
        static bool showMenu = true;

        if (showMenu) {
            if (ImGui::BeginMainMenuBar()) {
                if (ImGui::BeginMenu("Window")) {
                    ImGui::MenuItem("Main Menu", "F10", &showMenu);
                    ImGui::MenuItem("Files", "F1", &showFileBrowser);
                    ImGui::MenuItem("Console", "GraveAccent", &showConsole);
                    ImGui::EndMenu();
                }
                ImGui::EndMainMenuBar();
            }
        }

        if (ImGui::IsKeyPressed(ImGuiKey_F10)) showMenu = !showMenu;
        if (ImGui::IsKeyPressed(ImGuiKey_F1)) showFileBrowser = !showFileBrowser;

        if (ImGui::IsKeyPressed(ImGuiKey_GraveAccent)) showConsole = !showConsole;
        console.Draw("Console",&showConsole);
        if (showFileBrowser) DrawFileBrowser();

        // ------
        mGuiGlue->DrawEnd();
    };

};
//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
    (void)argc; (void)argv;
    AudioTestBed* game = new AudioTestBed();
    game->mSettings.Company = "Ohmflux";
    game->mSettings.Caption = "AudioTestBed";
    game->mSettings.enableLogFile = true;
    // game->mSettings.IconFilename = "assets/particles/Skull2.bmp";
    // game->mSettings.CursorFilename = "assets/particles/BloodHand.bmp";
    // game->mSettings.cursorHotSpotX = 10;
    // game->mSettings.cursorHotSpotY = 10;

    // LogFMT("TEST: My pref path would be:{}", SDL_GetPrefPath(game->mSettings.Company, game->mSettings.Caption ));

    game->Execute();
    SAFE_DELETE(game);
    return 0;
}


