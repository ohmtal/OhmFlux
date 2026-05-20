//-----------------------------------------------------------------------------
// ohmFlux KorkScript Testing
// Issues:
//  [ ] myPlatfromProcess needs to be filled
//  [ ] Script parse error when using id like : 1029.dump();
//       ==> Error parsing ("; expected"; token is dump) at 1:5
//       << error is from astGen.processTokens() cause a exception
//-----------------------------------------------------------------------------
#include <SDL3/SDL_main.h>

#include "fluxMain.h"
#include "gui/fluxGuiGlue.h"
#include "gui/ImConsole.h"
#include "gui/ImFileDialog.h"

// #include "utils/fluxFile.h"
// #include "core/fluxMath.h"
// #include "gui/ImFlux.h"


#include "platform/platform.h"
#include "console/console.h"
#include "console/consoleTypes.h"
#include "sim/simBase.h"
#include "sim/dynamicTypes.h"
#include "core/fileStream.h"

class Player : public SimObject, public FluxBaseObject
{
    typedef SimObject Parent;

    bool mShielded = false;
    bool mSitting = false;
    S32  mReqUInt = 0;

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
        addField("ReqUInt", TypeReqUInt, Offset(mReqUInt, Player));

        addField("Shielded", TypeBool,     Offset(mShielded, Player)        );
        addField("Sitting",	 TypeBool,     Offset(mSitting, Player)        );

    }

    bool onAdd() override {
        Log("Player %d added.", getId());
        return Parent::onAdd();
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

    std::vector<fs::path> scriptFiles;

    std::unique_ptr<FluxGuiGlue> mGuiGlue;



    // -------------------------------------------------------------------------
    void OnConsoleCommand(ImConsole* console, const char* command_line) {
        std::string cmdLineStr = command_line;
        // std::string cmd = FluxStr::getWord(cmdLineStr, 0);

        // add (); this may fail on "bla() ;" but i dont care ;)
        // if (!cmdLineStr.ends_with(");")) cmdLineStr += "();";
        //
        // Con::evaluatef(cmdLineStr.c_str());
        Con::evaluate(command_line);

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
        Con::evaluatef( R"(
            echo("Testing kork script with OhmFlux ...");
            exec("assets/main.cs");
        )");
        // <<<<

        fetchScriptFiles();


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
                if (ImGui::BeginMenu("Scripts")) {
                    //FIXME current script ?!
                    // if (ImGui::MenuItem(std::format("Hot Reload {}", getScript()).c_str(),"CTRL+R")) {
                    //     LoadScript();
                    // }
                    ImGui::SeparatorText("Files");
                    bool selected = false;
                    for (const auto& f : scriptFiles) {
                        // selected = getScript() == f.string();
                        if (ImGui::MenuItem(f.string().c_str(), nullptr, selected)) {
                            // setScript(f.string());
                            std::string cmd = std::format("exec(\"assets/{}\");", f.string());
                            // Log("Loading with command: %s", cmd.c_str());
                            Con::evaluate(cmd.c_str());
                        }
                    }
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


    bool fetchScriptFiles() {
        std::string path = getGamePath() + "assets/";
        scriptFiles.clear();
        try {
            if (fs::exists(path) && fs::is_directory(path)) {
                for (const auto& entry : fs::recursive_directory_iterator(path)) {
                    if (entry.is_regular_file()
                        &&  ( entry.path().extension() == ".cs" || entry.path().extension() == ".tscript")

                    ) {
                        // Full path: luaFiles.push_back(entry.path());
                        scriptFiles.push_back(fs::relative(entry.path(), path));

                    }
                }
            }
        } catch (const fs::filesystem_error& e) {
            Log("[error]%s",e.what());
            return false;
        }
        return true;
    }

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


