#include <stdio.h>
#include <fluxMain.h>
#include <gui/fluxGuiGlue.h>
#include <gui/ImConsole.h>
#include "luabind/luaBindings.h"
#include "luabind/luaState.h"
#include <sol/sol.hpp>
#include <SDL3/SDL.h>

#include <format>
#include <iostream>
#include <filesystem>
#include <vector>
namespace fs = std::filesystem;

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
class LuaGame: public OhmFlux::Lua::LuaState {
    typedef OhmFlux::Lua::LuaState Parent;

    ImConsole console;
    std::vector<fs::path> luaFiles;

    std::unique_ptr<FluxGuiGlue> mGuiGlue;

public:
    // -------------------------------------------------------------------------
    bool Initialize() override {
        if (!Parent::Initialize()) return false;
        mGuiGlue = std::make_unique<FluxGuiGlue>(true, false, nullptr);
        if (!mGuiGlue->Initialize())
            return false;

        SDL_SetLogOutputFunction(ConsoleLogFunction, &console);
        console.OnCommand = [&](ImConsole* console, const char* command_line) {
            OnConsoleCommand(console, command_line);
        };


        fetchLuaFiles();
        return true;
    }
    // -------------------------------------------------------------------------
    void OnConsoleCommand(ImConsole* console, const char* command_line) {
        std::string cmdLineStr = command_line;
        std::string cmd = FluxStr::getWord(cmdLineStr, 0);

        if (cmd == "load" || cmd == "exec" ) {
            std::string file = FluxStr::getWord(cmdLineStr,1);
            if (file != "") {
                if (file.find(".lua") == std::string::npos) {
                    file = file + ".lua";
                }
                setScript(file);
            }
            return;
        }


        if (cmd == "rl") {
            LoadScript();
            return;
        }


        // Lua commands:
        try {
            if (!getLua()) {
                SDL_Log("[error] failed to get LuaState!");
                return;
            }
            auto result =  getLua()->safe_script(cmdLineStr, sol::script_pass_on_error);

            if (!result.valid()) {
                sol::error err = result;
                console->AddLog("Error: %s", err.what());
                return;
            }

            if (result.return_count() == 0 || result[0].is<sol::nil_t>()) {
                return;
            }

            sol::object obj = result[0];
            std::string output = (*getLua())["tostring"](obj);

            console->AddLog("%s", output.c_str());

        } catch (const std::exception &e) {
            console->AddLog("C++ Exception: %s", e.what());
        }

    }
    // -------------------------------------------------------------------------
    void Deinitialize() override {
        SDL_SetLogOutputFunction(nullptr, nullptr);
        mGuiGlue->Deinitialize();

        // Parent::Deinitialize();
    }
    // -------------------------------------------------------------------------
    virtual void onKeyEvent(SDL_KeyboardEvent event) override {
        if (mGuiGlue && mGuiGlue->getGuiIO() && mGuiGlue->getGuiIO()->WantTextInput) {
            return;
        }

        Parent::onKeyEvent(event);

    }
    // -------------------------------------------------------------------------
    virtual void onEvent(SDL_Event event) override {

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

        Parent::onEvent(event);
    }
    // -------------------------------------------------------------------------
    virtual void onDrawTopMost() override {
        Parent::onDrawTopMost();

        if (!mGuiGlue) return;
        mGuiGlue->DrawBegin();

        static bool showConsole = false;
        static bool showMenu = true;

        if (showMenu) {
            if (ImGui::BeginMainMenuBar()) {
                if (ImGui::BeginMenu("Window")) {
                    ImGui::MenuItem("Console", "GraveAccent", &showConsole);
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Scripts")) {
                    if (ImGui::MenuItem(std::format("Hot Reload {}", getScript()).c_str(),"CTRL+R")) {
                        LoadScript();
                    }

                    ImGui::SeparatorText("Files");
                    bool selected = false;
                    for (const auto& f : luaFiles) {
                        selected = getScript() == f.string();
                        if (ImGui::MenuItem(f.string().c_str(), nullptr, selected)) {
                            setScript(f.string());
                        }

                    }

                    ImGui::EndMenu();
                }

                ImGui::EndMainMenuBar();
            }
        }

        if (ImGui::IsKeyPressed(ImGuiKey_F10)) showMenu = !showMenu;
        if (ImGui::IsKeyPressed(ImGuiKey_GraveAccent)) showConsole = !showConsole;
        if (mGuiGlue->getGuiIO()->KeyCtrl) {
            if (ImGui::IsKeyPressed(ImGuiKey_R)) LoadScript();
        }
        console.Draw("Lua Console",&showConsole);

        // ------
        mGuiGlue->DrawEnd();
    };
    // -------------------------------------------------------------------------
    bool fetchLuaFiles() {
        std::string path = getGamePath() + "assets/";

        try {
            if (fs::exists(path) && fs::is_directory(path)) {
                for (const auto& entry : fs::recursive_directory_iterator(path)) {
                    if (entry.is_regular_file() && entry.path().extension() == ".lua") {
                        // Full path: luaFiles.push_back(entry.path());
                        luaFiles.push_back(fs::relative(entry.path(), path));

                    }
                }
            }
        } catch (const fs::filesystem_error& e) {
            SDL_Log("[error]%s",e.what());
            return false;
        }
        // -------------------------------------------------------------------------



        return true;
    }


};
//------------------------------------------------------------------------------

int main(int argc, char* argv[]) {
    LuaGame gGame;

    gGame.Execute();

    return 0;
}
