//-----------------------------------------------------------------------------
// ohmFlux AudiTestBed
// Reimplementation of better Sound System
//-----------------------------------------------------------------------------
#include <SDL3/SDL_main.h>

#include "fluxMain.h"
#include "gui/fluxGuiGlue.h"
#include "gui/ImConsole.h"

// #include "gui/ImFileDialog.h"
// #include "utils/fluxFile.h"
// #include "core/fluxMath.h"
// #include "gui/ImFlux.h"


#include "platform/platform.h"
#include "console/console.h"
#include "sim/simBase.h"
#include "sim/dynamicTypes.h"
#include "core/fileStream.h"

class Player : public SimObject
{
    typedef SimObject Parent;

public:

    // Vector2 mPosition;

    Player()
    {
        // mPosition = {};
    }

    static void initPersistFields()
    {
        Parent::initPersistFields();
        // Does not have the default types like TypeS32 :(
        // addField("position", TypeReqUInt, Offset(mPosition, Player));
    }

    DECLARE_CONOBJECT(Player);
};

IMPLEMENT_CONOBJECT(Player);

ConsoleMethod(Player, jump, void, 2, 2, "")
{
    Log("[info] Player jump :D");
}

//------------------------------------------------------------------------------
void MyLogger(U32 level, const char *consoleLine, void*)
{
    // Log("[%d] %s", level, consoleLine);
    Log("%s", consoleLine);
}

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
class KorkTestBed : public FluxMain
{
    typedef FluxMain Parent;
    ImConsole console;
    // ImFileDialog fileDialog;

    std::unique_ptr<FluxGuiGlue> mGuiGlue;



    // -------------------------------------------------------------------------
    void OnConsoleCommand(ImConsole* console, const char* command_line) {
        // std::string cmdLineStr = command_line;
        // std::string cmd = FluxStr::getWord(cmdLineStr, 0);

        //FIXME call torque script
        Con::evaluatef(command_line);

    }
    // -------------------------------------------------------------------------


public:
    bool Initialize() override
    {
        if (!Parent::Initialize()) return false;

        // setting ini here is ok for a testBed
        mGuiGlue = std::make_unique<FluxGuiGlue>(true, false, "KorkTestBed.ini");
        if (!mGuiGlue->Initialize())
            return false;



        SDL_SetLogOutputFunction(ConsoleLogFunction, &console);
        console.OnCommand = [&](ImConsole* console, const char* command_line) {
            OnConsoleCommand(console, command_line);
        };

        // fileDialog.init( getGamePath(), { ".ogg", ".wav", ".mp3" , ".sfx", ".flac"});
        // {
        //     std::string listFile = mSettings.getPrefsPath() + "/pathes.list" ;
        //     dLog("[info]try to load extra pathes from: %s ", listFile.c_str());
        //     std::string key, value;
        //     if (FluxFile::Exists(listFile )) {
        //         std::vector<std::string> lines ;
        //         FluxFile::LoadTextFile(listFile, lines);
        //         for (auto& line: lines) {
        //             if (FluxStr::getWordCount(line, ';') == 2) {
        //                 fileDialog.mCustomQuckPathes[FluxStr::getWord(line,0,';')] = FluxStr::getWord(line,1, ';');
        //             }
        //
        //         }
        //     }
        // }



        // korkscript >>>
        Con::init();
        Sim::init();
        Con::addConsumer(MyLogger, NULL);
        Con::evaluatef("echo(\"Testing kork script... ...\");");
        // <<<<


        return true;
    }
    //--------------------------------------------------------------------------------------
    void Deinitialize() override
    {

        SDL_SetLogOutputFunction(nullptr, nullptr);
        mGuiGlue->Deinitialize();

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
    // void DrawFileBrowser(){
    //     if (fileDialog.Draw()) {
    //         // LogFMT("File:{} Ext:{}", g_FileDialog.selectedFile, g_FileDialog.selectedExt);
    //         if (fileDialog.mSaveMode)
    //         {
    //             fileDialog.reset();
    //         } else {
    //             // NOTE: LOAD RESOURCE TEST
    //             if (!AudioResourceManager.add(fileDialog.selectedFile)) {
    //                 mGuiGlue->showMessage("Error", "Failed to load File " + fileDialog.selectedFile + " !");
    //             } else {
    //                 Log("[info] file %s loaded.", fileDialog.selectedFile.c_str());
    //             }
    //         }
    //     }
    // }

    // -------------------------------------------------------------------------

    //--------------------------------------------------------------------------------------
    virtual void onDrawTopMost() override {
        Parent::onDrawTopMost();

        if (!mGuiGlue) return;
        mGuiGlue->DrawBegin();

        static bool showConsole = true;
        static bool showMenu = true;

        if (showMenu) {
            if (ImGui::BeginMainMenuBar()) {
                if (ImGui::BeginMenu("Window")) {
                    ImGui::MenuItem("Main Menu", "F10", &showMenu);
                    ImGui::MenuItem("Console", "GraveAccent", &showConsole);
                    ImGui::EndMenu();
                }
                ImGui::EndMainMenuBar();
            }
        }

        if (ImGui::IsKeyPressed(ImGuiKey_F10)) showMenu = !showMenu;
        if (ImGui::IsKeyPressed(ImGuiKey_GraveAccent)) showConsole = !showConsole;
        console.Draw("Console",&showConsole);
        // if (showFileBrowser) DrawFileBrowser();
        // ------
        mGuiGlue->DrawEnd();
    };

};
//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------


int main(int argc, char* argv[])
{
    (void)argc; (void)argv;
    KorkTestBed* game = new KorkTestBed();
    game->mSettings.Company = "Ohmtal";
    game->mSettings.Caption = "KorkTestBed";
    game->mSettings.enableLogFile = true;
    // game->mSettings.IconFilename = "assets/icon.png";
    // game->mSettings.CursorFilename = "assets/particles/BloodHand.bmp";
    // game->mSettings.cursorHotSpotX = 10;
    // game->mSettings.cursorHotSpotY = 10;

    // LogFMT("TEST: My pref path would be:{}", SDL_GetPrefPath(game->mSettings.Company, game->mSettings.Caption ));

    game->Execute();
    SAFE_DELETE(game);
    return 0;
}


