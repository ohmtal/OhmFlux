#pragma once

#include "fluxMain.h"
#include "gui/fluxGuiGlue.h"
#include "gui/ImConsole.h"

#include "console/simBase.h"

namespace fs = std::filesystem;
class ScriptEditor;

namespace ElfFlux {


    //-----------------------------------------------------------------------------
    class Main : public SimSet, public FluxMain
    {
        typedef SimSet Parent;

        ImConsole console;
        ScriptEditor* mScriptEditor;

        std::vector<fs::path> scriptFiles;
        std::unique_ptr<FluxGuiGlue> mGuiGlue;

        void OnConsoleCommand(ImConsole* console, const char* command_line);
        void OnConsoleTAB(ImConsole* console,  ImGuiInputTextCallbackData* data, bool forward);

    public:
        // must have a directory!!!!
        String mStartScript = "assets/main.cs";

        bool Initialize() override;
        void Deinitialize() override;
        void IterateFrame() override;

        void onKeyEvent(SDL_KeyboardEvent event) override {  }
        void onMouseButtonEvent(SDL_MouseButtonEvent event) override { }
        void onEvent(SDL_Event event) override;
        void Update(const double& dt) override;
        void onDraw() override  {

        } //Draw

        virtual void onDrawTopMost() override;
        bool fetchScriptFiles();



        bool mShowConsole = true;
        bool mShowMenu = true;
        bool mOpenScriptEditor = false;

        // ------------- Console -------------
        DECLARE_CONOBJECT(Main);
        static void initPersistFields();
        virtual void deleteObject() override {}
    };


    //--------------------------------
    inline Main* gMain = nullptr;
    //--------------------------------
} //namespace ElfFlux

