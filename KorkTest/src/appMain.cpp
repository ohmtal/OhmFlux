#include "appMain.h"

#include "platform/platform.h"
#include "console/console.h"
#include "console/consoleTypes.h"
#include "sim/simBase.h"
#include "sim/dynamicTypes.h"
#include "core/fileStream.h"
#include <platform/platformString.h>
#include "gui/ImConsole.h"

namespace KorkFlux {


    //------------------------------------------------------------------------------
    void MyLogger(U32 level, const char *consoleLine, void*)
    {
        switch (level) {
            case 1: Log("[warn] %s",  consoleLine); break;
            case 2: Log("[error] %s",  consoleLine); break;
            default: Log("%s",  consoleLine); break;
        }

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
    void Main::onEvent(SDL_Event event)
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

        for (auto* obj : mQueueObjects) {
            if (obj && obj->isEventListener()) obj->onEvent(event);
        }
    }
    //-----------------------------------------------------------------------------
    bool Main::fetchScriptFiles() {
        std::string path = getGamePath() + "assets/";
        scriptFiles.clear();
        try {
            if (fs::exists(path) && fs::is_directory(path)) {
                for (const auto& entry : fs::recursive_directory_iterator(path)) {
                    if (entry.is_regular_file()
                        &&  (
                        entry.path().extension() == ".cs"
                        || entry.path().extension() == ".tscript"
                        || entry.path().extension() == ".korkscript"
                        )

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
    //-----------------------------------------------------------------------------
    void Main::onDrawTopMost() {
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
                if (ImGui::BeginMenu("Gui Scale")) {
                    if (ImGui::MenuItem("0.75")) mGuiGlue->setScale(0.75f);
                    if (ImGui::MenuItem("1.00")) mGuiGlue->setScale(1.00f);
                    if (ImGui::MenuItem("1.25")) mGuiGlue->setScale(1.25f);
                    if (ImGui::MenuItem("1.50")) mGuiGlue->setScale(1.50f);
                    if (ImGui::MenuItem("1.75")) mGuiGlue->setScale(1.75f);
                    if (ImGui::MenuItem("2.00")) mGuiGlue->setScale(2.00f);

                    ImGui::EndMenu();
                }

                ImGui::EndMainMenuBar();
            }
        }

        if (ImGui::IsKeyPressed(ImGuiKey_F10)) showMenu = !showMenu;
        if (ImGui::IsKeyPressed(ImGuiKey_GraveAccent)) showConsole = !showConsole;
        console.Draw("Console",&showConsole);
        // ------
        mGuiGlue->DrawEnd();
    }
    //-----------------------------------------------------------------------------
    void Main::Update(const double& dt)
    {

        // advance Torque Time for schedule
        static U32 lastTick = 0;
        Sim::advanceTime(SDL_GetTicks() - lastTick);
        lastTick = SDL_GetTicks();
        // ----

        Parent::Update(dt);
    }

    void Main::Deinitialize()
    {

        Con::removeConsumer(MyLogger, NULL);
        Sim::shutdown();
        Con::shutdown();

        SDL_SetLogOutputFunction(nullptr, nullptr);
        mGuiGlue->Deinitialize();

        Parent::Deinitialize();
    }

    void  Main::OnConsoleTAB(ImConsole* console, ImGuiInputTextCallbackData* data, bool forward) {


       char buffer[256] = {0};
       snprintf(buffer, sizeof(buffer), "%s", data->Buf);
       U32 newCursorPos = Con::tabComplete(buffer, data->CursorPos, sizeof(buffer) - 1, forward);
        // Log("TESTTAB: %s (%d)", buffer, newCursorPos);

       data->DeleteChars(0, data->BufTextLen);
       data->InsertChars(0, buffer);
       data->CursorPos = newCursorPos;
       data->SelectionStart = newCursorPos;
       data->SelectionEnd = newCursorPos;
       data->BufDirty = true;

    }

    void Main::OnConsoleCommand(ImConsole* console, const char* command_line) {
        Con::evaluate(command_line);
    }
    //-----------------------------------------------------------------------------
    bool Main::Initialize()
    {
        if (!Parent::Initialize()) return false;

        mOverwriteEventListener = true; //THIS class handle the eventlistener!

        // setting ini here is ok for a testBed
        mGuiGlue = std::make_unique<FluxGuiGlue>(true, false, "KorkTestBed.ini");
        if (!mGuiGlue->Initialize())
            return false;


        // SDL_SetLogPriorities(SDL_LOG_PRIORITY_VERBOSE);  //emscript redirect test
        // NOTE: does NOT work with emscritpen !!!!
        SDL_SetLogOutputFunction(ConsoleLogFunction, &console);
        console.OnCommand = [this](ImConsole* console, const char* command_line) {
            OnConsoleCommand(console, command_line);
        };
        console.OnTabCompletion = [this](ImConsole* console, ImGuiInputTextCallbackData* data, bool forward) {
            OnConsoleTAB(console, data, forward);
        };



        // korkscript >>>
        Con::init();
        Sim::init();
        Con::addConsumer(MyLogger, NULL);
        Con::evaluate( R"(
            echo("Testing kork script with OhmFlux ...");
            exec("assets/main.cs");
        )");
        // <<<<

        fetchScriptFiles();

        return true;
    }
    //-----------------------------------------------------------------------------

}

