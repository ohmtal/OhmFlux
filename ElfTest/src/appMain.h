// FIXME separate console/Menu from "Main"
#pragma once

#include "fluxMain.h"
#include "gui/fluxGuiGlue.h"
#include "gui/ImConsole.h"

namespace fs = std::filesystem;
class ScriptEditor;

namespace ElfFlux {


    //-----------------------------------------------------------------------------
    class Main : public FluxMain
    {
        typedef FluxMain Parent;
        ImConsole console;
        ScriptEditor* mScriptEditor;

        std::vector<fs::path> scriptFiles;
        std::unique_ptr<FluxGuiGlue> mGuiGlue;

        void OnConsoleCommand(ImConsole* console, const char* command_line);
        void OnConsoleTAB(ImConsole* console,  ImGuiInputTextCallbackData* data, bool forward);

    public:
        bool Initialize() override;
        void Deinitialize() override;

        void onKeyEvent(SDL_KeyboardEvent event) override {  }
        void onMouseButtonEvent(SDL_MouseButtonEvent event) override { }
        void onEvent(SDL_Event event) override;
        void Update(const double& dt) override;
        void onDraw() override  {

        } //Draw

        virtual void onDrawTopMost() override;
        bool fetchScriptFiles();

    };


    //--------------------------------
    inline Main* gMain = nullptr;
    //--------------------------------
} //namespace ElfFlux

