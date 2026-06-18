#include "appMain.h"

#include "console/script.h"
#include "console/engineAPI.h"

#include "gui/ImConsole.h"
#include "scriptEditor/scriptEditor.h"

#include "main/engineGlue.h"
#include "core/Globals.h"
//------------------------------------------------------------------------------
//
// NOTE: On Console it's: $Main
//
//------------------------------------------------------------------------------

namespace ElfFlux {
    IMPLEMENT_CONOBJECT(Main);
    //------------------------------------------------------------------------------
    void MyLogger(U32 level, const char *consoleLine)
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
                        || entry.path().extension() == ".elfscript"

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
        FluxMain::onDrawTopMost();

        if (!mGuiGlue) return;
        mGuiGlue->DrawBegin();


        static bool showConsole = true;
        static bool showMenu = true;
        static bool openScriptEditor = false; //FIXME settings ?

        if (showMenu) {
            if (ImGui::BeginMainMenuBar()) {
                if (ImGui::BeginMenu("Window")) {
                    ImGui::MenuItem("Main Menu", "F10", &showMenu);
                    ImGui::MenuItem("Console", "GraveAccent", &showConsole);
                    if (ImGui::MenuItem("calling shutdown test")) {
                        Con::executef(this, "ShutDown");
                    }
                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("Scripts")) {

                    ImGui::Checkbox("Open Script Editor", &openScriptEditor);

                    ImGui::SeparatorText("Files");
                    bool selected = false;
                    for (const auto& f : scriptFiles) {
                        // selected = getScript() == f.string();
                        if (ImGui::MenuItem(f.string().c_str(), nullptr, selected)) {

                            std::string fileName = /*"assets/" +*/  f.string();
                            if (loadScript(fileName.c_str())) {
                                if (mScriptEditor) mScriptEditor->addTextEditor(fileName);
                            }
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
        if (openScriptEditor && mScriptEditor) mScriptEditor->renderEditors();
        // ------

        mGuiGlue->DrawEnd();
    }
    //-----------------------------------------------------------------------------
    void  Main::IterateFrame() {
        // first the FluxMainLoop
        FluxMain::IterateFrame();

        // FrameAllocator::setWaterMark(0);
        //NOTE very very important to clear temp memory
        //     maybe ok to put it in update
        // ConsoleValue::resetConversionBuffer();

    }
    void Main::Update(const double& dt)
    {

        // advance Torque Time for schedule
        static U32 lastTick = 0;
        engineGlue::process(SDL_GetTicks() - lastTick);
        lastTick = SDL_GetTicks();
        // ----

        // here we go lets fetch the log
        // ------ output log entries:
        // not longer needed i use the consumer
        // ConsoleLogEntry *log;
        // U32 size;
        // static U32 lastLogEntry = 0;
        // Con::getLockLog(log, size);
        // if (lastLogEntry < size) {
        //     for (U32 i = lastLogEntry; i < size; i++)
        //     {
        //         ConsoleLogEntry &entry = log[i];
        //         MyLogger(entry.mLevel, entry.mString);
        //     }
        //     lastLogEntry = size;
        // }
        // Con::unlockLog();

        // -------- finallize

        // <<< log

        FluxMain::Update(dt);
    }

    void Main::Deinitialize()
    {
        // ------------------------
        // Tell console we are shutting down!
        Con::executef(this, "ShutDown");
        Update(0.f); // << add a tick so shutdown is executed!
        Update(0.f); // << once again
        // ------------------------



        SAFE_DELETE(mScriptEditor);
        mScriptEditor = nullptr;

        SDL_SetLogOutputFunction(nullptr, nullptr);
        mGuiGlue->Deinitialize();


        unregisterObject(); // tell console we are off
        engineGlue::shutDown(); //Before Deinitialize else crash!
        // Con::removeConsumer(MyLogger); // remove the LogConsumer
        FluxMain::Deinitialize();
        // no need i called unregister :P _destroySelf(); // tell simset we are removed. it complains about is unregisted now ....

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
        if (!FluxMain::Initialize()) return false;
        mOverwriteEventListener = true; //THIS class handle the eventlistener!

        // setting ini here is ok for a testBed
        mGuiGlue = std::make_unique<FluxGuiGlue>(true, false, "ElfTestBed.ini");
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

        //  -------------- Console >>>>
        String workingDir = getGamePath().c_str();
        workingDir += "assets/";


        engineGlue::init(MyLogger, workingDir);
        ElfFlux::init(); // init my stuff

        //TEST
        mStartScript = "main.cs";
        if (!loadScript(mStartScript.c_str())) {
            return false;
        }

        fetchScriptFiles();
        mScriptEditor = new ScriptEditor();

        // ---- console
        this->registerObject(); //make available on Console
        Con::setIntVariable("$Main", this->getId());
        // -----

        return true;
    }

    //-----------------------------------------------------------------------------
    // Console
    //-----------------------------------------------------------------------------
    void Main::initPersistFields() {
        Parent::initPersistFields();
        addField("maxFPS", TypeS32, Offset(mSettings.maxFPS, Main), "maximum fps - using frameLimiter");
        addField("frameLimit", TypeF64, Offset(mSettings.frameLimiter, Main), "Frame Limiter - default 0");

    }

    DefineEngineMethod(Main, delete, void, (), , "Override Delete ... this would be a bad idea on Main object") {
        Con::errorf("Delete not allowed on Main Object");
    }
    DefineEngineMethod(Main, clone, void, (), , "Override Clone ... this would be a bad idea on Main object") {
        Con::errorf("Clone not allowed on Main Object");
    }

    DefineEngineMethod(Main, setScreenSize, void, (Point2I size), , "set the base screen size") {

        if (size.isZero()) {
            size.x =  object->mSettings.ScreenWidth;
            size.y =  object->mSettings.ScreenHeight;
        }
        getScreenObject()->setNewScreenSize(size);
    }

}

