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
#include "audio/AudioInstance.h"

#include "gui/ImFlux.h"

#include <unordered_map>


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



    // -------------------------------------------------------------------------
    void OnConsoleCommand(ImConsole* console, const char* command_line) {
        std::string cmdLineStr = command_line;
        std::string cmd = FluxStr::getWord(cmdLineStr, 0);

        if (cmd == "list") {
            for (auto& [key, val] : AudioResourceManager.getMap()) {
                Log("%s type:%d size:%d", key.c_str(), (int)val->fileType, (int)val->mRawData.size());
            }
        }

    }
    // -------------------------------------------------------------------------

    // fixme still have "single" Instance playing ...
    std::unordered_map<std::string, std::unique_ptr<FluxAudio::AudioInstance>> mInstanceMap;


public:
    bool Initialize() override
    {
        if (!Parent::Initialize()) return false;

        // setting ini here is ok for a testBed
        mGuiGlue = std::make_unique<FluxGuiGlue>(true, false, "AudioTestBed.ini");
        if (!mGuiGlue->Initialize())
            return false;



        SDL_SetLogOutputFunction(ConsoleLogFunction, &console);
        console.OnCommand = [&](ImConsole* console, const char* command_line) {
            OnConsoleCommand(console, command_line);
        };

        fileDialog.init( getGamePath(), { ".ogg", ".wav", ".mp3" , ".sfx" });

        AudioResourceManager.Initialize(); //FIXME move to Ohmflux

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

        // update audio instances
        for (auto& [filename, instance] : mInstanceMap) {
            instance->Update(dt, nullptr);
        }


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
                if (!AudioResourceManager.add(fileDialog.selectedFile)) {
                    mGuiGlue->showMessage("Error", "Failed to load File " + fileDialog.selectedFile + " !");
                } else {
                    Log("[info] file %s loaded.", fileDialog.selectedFile.c_str());
                }
            }
        }
    }

    // -------------------------------------------------------------------------

    // FIXME still working with single instance :P
    FluxAudio::AudioInstance* getAudioInstance(FluxAudio::ResourceData* resource) {
        if (!resource) return nullptr;
        auto it = mInstanceMap.find(resource->fileName);
        if (it != mInstanceMap.end()) {
            return it->second.get();
        }

        auto instance = std::make_unique<FluxAudio::AudioInstance>();
        if (instance->Initialize(resource)) {
            mInstanceMap[resource->fileName] = std::move(instance);
            dLog("Playing from new Instance");
            return mInstanceMap[resource->fileName].get();
        }

        return nullptr;
    }


    //------------------------------------------------------------------------------
    void DrawAudioList(bool* p_open) {
        if (!*p_open) return;
        ImGui::SetNextWindowSize(ImVec2(200, 600), ImGuiCond_FirstUseEver);
        if (!ImGui::Begin("audio streams", p_open)) { ImGui::End(); return; }


        if (ImGui::BeginChild("WaveModule_BOX", ImVec2(0, 0), ImGuiChildFlags_Borders)) {
            ImGui::Separator();

            // List
            std::string waveCaption;
            bool isSelected = false;
            int n = 0;
            if (ImGui::BeginListBox("##WaveList", ImVec2(-FLT_MIN, -FLT_MIN))) {
                for (auto& [filename, resource] : AudioResourceManager.getMap()) {

                    waveCaption = std::format("{:02X} {} {}", n, FluxStr::extractFilename( filename), (int)resource->fileType);


                    if (ImGui::Selectable(waveCaption.c_str(), isSelected)) {
                        // do something on select ?
                    }

                    FluxAudio::AudioInstance*  instance = getAudioInstance(resource.get());


                    if (instance) {
                        float progress = instance->getProgress();
                        ImGui::PushID(instance);
                        if (!instance->isPlaying) {
                            if (ImFlux::ButtonFancy("Play" , ImFlux::SLATE_BUTTON) ) {
                                if (!instance->Play()) Log("[error] failed to play %s", waveCaption.c_str());
                            }
                            if (progress > 0.0f ) {
                                ImGui::SameLine();
                                if (ImFlux::ButtonFancy("Resume" , ImFlux::SLATE_BUTTON) ) {
                                    if (!instance->Resume()) Log("[error] failed to play %s", waveCaption.c_str());
                                }
                            }

                            ImGui::SameLine();
                            if (ImFlux::ButtonFancy("DEL", ImFlux::RED_BUTTON)) {

                                std::string key = filename;
                                FluxSchedule.add(0.0, nullptr, [key, this]()
                                {
                                    mInstanceMap.erase(key);
                                    AudioResourceManager.remove(key);
                                });
                            }

                        } else {
                            if (ImFlux::ButtonFancy("Stop", ImFlux::BLUE_BUTTON)) {
                                if (!instance->Stop()) Log("[error] failed to stop %s", waveCaption.c_str());
                            }
                        }
                        ImGui::SameLine();
                        if (ImGui::Checkbox("Loop" , &instance->doLoop)) {
                        }
                        ImGui::SameLine();ImGui::SetNextItemWidth(70.f);
                        if (ImGui::SliderFloat("Vol" , &instance->volume, 0.1f, 1.f)) {

                        }

                        // if (instance->isPlaying)
                        {
                            ImGui::ProgressBar( progress);
                        }



                        ImGui::PopID();
                    }



                    // if (ImGui::IsItemHovered()) {
                    //
                    //     if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                    //     }
                    // } //IsItemHovered

                    // if (isSelected) {
                    //     ImGui::SetItemDefaultFocus();
                    //
                    //     if (ImGui::BeginPopupContextItem("##WavePopup")) {
                    //         ImGui::TextColored(Color4FIm(cl_SkyBlue), "%s", waveCaption.c_str());
                    //         ImGui::TextDisabled("%s",mAudioStreams[n]->getFileName().c_str());
                    //
                    //         ImGui::Separator();
                    //         ImGui::EndPopup();
                    //     }
                    // }


                    n++;
                } //for auto ...
                ImGui::EndListBox();
            }
        }
        ImGui::EndChild();
        ImGui::End();

    }

    //--------------------------------------------------------------------------------------
    virtual void onDrawTopMost() override {
        Parent::onDrawTopMost();

        if (!mGuiGlue) return;
        mGuiGlue->DrawBegin();

        static bool showConsole = true;
        static bool showFileBrowser = true;
        static bool showMenu = true;
        static bool showAudioList = true;

        if (showMenu) {
            if (ImGui::BeginMainMenuBar()) {
                if (ImGui::BeginMenu("Window")) {
                    ImGui::MenuItem("Main Menu", "F10", &showMenu);
                    ImGui::MenuItem("Files", "F1", &showFileBrowser);
                    ImGui::MenuItem("Audio List", "F2", &showAudioList);
                    ImGui::MenuItem("Console", "GraveAccent", &showConsole);
                    ImGui::EndMenu();
                }
                ImGui::EndMainMenuBar();
            }
        }

        if (ImGui::IsKeyPressed(ImGuiKey_F10)) showMenu = !showMenu;
        if (ImGui::IsKeyPressed(ImGuiKey_F1)) showFileBrowser = !showFileBrowser;
        if (ImGui::IsKeyPressed(ImGuiKey_F2)) showAudioList = !showAudioList;

        if (ImGui::IsKeyPressed(ImGuiKey_GraveAccent)) showConsole = !showConsole;
        console.Draw("Console",&showConsole);
        if (showFileBrowser) DrawFileBrowser();

        DrawAudioList(&showAudioList);

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


