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
#include "audio/fluxAudioBuffer.h"
#include "utils/fluxFile.h"
#include "core/fluxMath.h"

#include "gui/ImFlux.h"

#include <unordered_map>
#include <atomic>

//-----------------------------------------------------------------------------
struct RecordingData {
     std::unique_ptr<FluxAudio::AudioBuffer> mBuffer;
     bool active = false; //atomic ?

     size_t mWritten = 0;
     size_t mWriteCount = 0;

     std::function<void()> OnRecordingDone = nullptr;

     RecordingData( ) {
        mBuffer = std::make_unique<FluxAudio::AudioBuffer>(0);
    }

};

// Recording
void SDLCALL FinalMixCallback(void *userdata, const SDL_AudioSpec *spec, float *buffer, int buflen) {
    if (!userdata || !spec || !buffer || buflen < 1) return;
    auto* rData = static_cast<RecordingData*>(userdata);
    if (!rData  ) return;


    if ( spec->format == SDL_AUDIO_F32)
    {

        if (buflen > 0) {
            static int counter = 0;
            if (counter++ % 50 == 0) { // Nur alle 50 Callbacks loggen
                dLog("Sample[0]: %f | Sample[1]: %f", buffer[0], buffer[1]);
            }
        }

        size_t numSamples = buflen / sizeof(float);
        int channel = 0;
        for(size_t i = 0; i < numSamples; ++i) {
            float s = buffer[i];
            if (channel == 1) s = 0.f;
            rData->mBuffer->push(&s, 1);
            if (++channel >= spec->channels) channel = 0;
        }
        // rData->mBuffer->push(buffer, numSamples);

        rData->mWritten += numSamples;
        if (rData->mWritten >=  rData->mWriteCount)
        {
            if (rData->OnRecordingDone) rData->OnRecordingDone();
            rData->active = false;
        }
    } else {
        dLog("if you see this we have a problem !! format : %d", (int)spec->format);
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

//--------------------------------------------------------------------------------------
struct StreamWorkerThreadData {
    std::unordered_map<std::string, std::unique_ptr<FluxAudio::AudioInstance>>* instanceMap;
    SDL_AtomicInt running;
    SDL_AtomicInt locked;
};

// SDL_Thread test:
int SDLCALL streamWorkerThread(void* userData) {
    auto* data = static_cast<StreamWorkerThreadData*>(userData);
    if (!data || !data->instanceMap) return -1;

    while (SDL_GetAtomicInt(&data->running)) {
        if ( SDL_GetAtomicInt(&data->locked) != 1)
        {
            for (auto& [filename, instance] : *(data->instanceMap)) {
                if (instance) {
                    instance->UpdateStream();
                }
            }
        }

        SDL_Delay(32);
    }

    return 0;
}
//-----------------------------------------------------------------------------
class AudioTestBed : public FluxMain
{
    typedef FluxMain Parent;
    ImConsole console;
    ImFileDialog fileDialog;
    std::unique_ptr<FluxGuiGlue> mGuiGlue;

    RecordingData mRecordingData;


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


     SDL_Thread* mWorkerThreadID = nullptr;
     StreamWorkerThreadData mWorkerThreadData;

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

        fileDialog.init( getGamePath(), { ".ogg", ".wav", ".mp3" , ".sfx", ".flac"});


        {
            std::string listFile = mSettings.getPrefsPath() + "/pathes.list" ;
            dLog("[info]try to load extra pathes from: %s ", listFile.c_str());
            std::string key, value;
            if (FluxFile::Exists(listFile )) {
                std::vector<std::string> lines ;
                FluxFile::LoadTextFile(listFile, lines);
                for (auto& line: lines) {
                    if (FluxStr::getWordCount(line, ';') == 2) {
                        fileDialog.mCustomQuckPathes[FluxStr::getWord(line,0,';')] = FluxStr::getWord(line,1, ';');
                    }

                }
            }
        }

        AudioResourceManager.Initialize(); //FIXME move to Ohmflux


        mWorkerThreadData = { &mInstanceMap, {1}, {0} }; // Start with running = 1 and locked = 0
        mWorkerThreadID = SDL_CreateThread(streamWorkerThread, "AudioWorker", &mWorkerThreadData);


        mRecordingData.OnRecordingDone = [this]() { this->OnRecordingDone(); };

         return true;
    }
    //--------------------------------------------------------------------------------------
    void Deinitialize() override
    {

        SDL_SetLogOutputFunction(nullptr, nullptr);
        mGuiGlue->Deinitialize();

        // stop thread
        SDL_SetAtomicInt(&mWorkerThreadData.running, 0);
        SDL_WaitThread( mWorkerThreadID, NULL );

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
        // for (auto& [filename, instance] : mInstanceMap) {
        //     // instance->Update(dt, nullptr);
        //     instance->UpdateStream();
        // }


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
    void OnRecordingDone() {
        SDL_SetAudioPostmixCallback(AudioManager.getDeviceID(), nullptr, nullptr);

        for (auto& [filename, instance] : mInstanceMap) {
            instance->Stop();
        }

        size_t availableFloats = mRecordingData.mBuffer->getAvailableForRead();
        auto resData = std::make_unique<FluxAudio::ResourceData>();
        std::string idStr = std::format("mix_{}", AudioResourceManager.getMap().size());
        resData->fileName = idStr;
        resData->fileType = FluxAudio::AudioType::WAV;
        resData->wavSrcSpec = AudioManager.getAudioSpec();
        resData->mRawData.resize(availableFloats * sizeof(float));
        mRecordingData.mBuffer->peek(reinterpret_cast<float*>(resData->mRawData.data()), availableFloats);
        AudioResourceManager.add(idStr, std::move(resData));

    }
    //------------------------------------------------------------------------------
     void StartRecording( float fixedSec = 0.f) {
        uint32_t maxFrames = 0;
        if (fixedSec == 0.f) {
            for (auto& [filename, instance] : mInstanceMap) {
                if ( !instance->doRecord ) continue;
                maxFrames = std::max(maxFrames, (uint32_t)instance->getOutOutFrames());
            }
            if ( maxFrames == 0 ) return;
            maxFrames *= 2; //raise
        } else {
            maxFrames = size_t(fixedSec * AudioManager.getAudioSpec().freq);
        }

        if ( maxFrames == 0 ) return;

        mRecordingData.mWritten = 0;
        mRecordingData.mWriteCount = maxFrames * AudioManager.getAudioSpec().channels;
        // make the buffer bigger ;)
        size_t minBufferSize = std::max( (size_t)(maxFrames * AudioManager.getAudioSpec().channels + 100000), (size_t)(512*1024*1024));
        if ( mRecordingData.mBuffer->getCapacity() <  minBufferSize ) {
            mRecordingData.mBuffer->setCapacity(minBufferSize);
        } else {
            mRecordingData.mBuffer->clear();
        }



        SDL_SetAtomicInt(&mWorkerThreadData.locked, 1);

        if (!SDL_SetAudioPostmixCallback(AudioManager.getDeviceID(), FinalMixCallback, &mRecordingData)) {
            dLog("[error] can NOT open PostMix Device !!! %s", SDL_GetError());
        } else {
            Log("[info] PostMix Callback installed.");
        }


        for (auto& [filename, instance] : mInstanceMap) {
            if ( !instance->doRecord ) continue;
            instance->doLoop = true;
            instance->Play();
        }
        mRecordingData.active = true;

        SDL_SetAtomicInt(&mWorkerThreadData.locked, 0);

    }


    //------------------------------------------------------------------------------
    void DrawAudioList(bool* p_open) {
        if (!*p_open) return;
        ImGui::SetNextWindowSize(ImVec2(200, 600), ImGuiCond_FirstUseEver);
        if (!ImGui::Begin("audio streams", p_open)) { ImGui::End(); return; }

        ImGui::TextUnformatted("Mix Audio Sources (where loop enabled!)");
        if (ImFlux::ButtonFancy("Record", ImFlux::YELLOW_BUTTON)) {
               if (!mRecordingData.active) StartRecording();
        }
        ImGui::SameLine();
        if (ImFlux::ButtonFancy("PLAYRING", ImFlux::BLUE_BUTTON)) {
            static SDL_AudioSpec spec = AudioManager.getAudioSpec();
            static SDL_AudioStream* testStream = SDL_CreateAudioStream(&spec, &spec);
            AudioManager.bindStream(testStream);
            SDL_ClearAudioStream(testStream);

            size_t totalFloats = mRecordingData.mBuffer->getAvailableForRead();
            if (totalFloats > 0) {
                std::vector<float> testData(totalFloats);
                mRecordingData.mBuffer->peek(testData.data(), totalFloats);

                SDL_PutAudioStreamData(testStream, testData.data(), (int)(testData.size() * sizeof(float)));
                dLog("Playring: %zu Samples", totalFloats);
            }
        }

        ImGui::TextDisabled("Buffer left: %d", (int)mRecordingData.mBuffer->getAvailableForWrite());

        if (ImGui::BeginChild("WaveModule_BOX", ImVec2(0, 0), ImGuiChildFlags_Borders)) {
            ImGui::Separator();

            // List
            std::string waveCaption;
            bool isSelected = false;
            int n = 0;
            if (ImGui::BeginListBox("##WaveList", ImVec2(-FLT_MIN, -FLT_MIN))) {
                for (auto& [filename, resource] : AudioResourceManager.getMap()) {
                    FluxAudio::AudioInstance*  instance = getAudioInstance(resource.get());

                    waveCaption = std::format("{:02X} {} {} {:.2f}sec",
                                              n
                                              , FluxStr::extractFilename( filename)
                                              , to_string(resource->fileType)
                                              , (instance) ?  instance->getSampleDuration() : 0.f
                    );


                    if (ImGui::Selectable(waveCaption.c_str(), isSelected)) {
                        // do something on select ?
                    }




                    if (instance && !mRecordingData.active) {
                        float progress = instance->getProgress();
                        ImGui::PushID(instance);
                        if (!instance->isPlaying) {
                            if (ImFlux::ButtonFancy("Play" , ImFlux::SLATE_BUTTON) ) {
                                if (!instance->Play()) Log("[error] failed to play %s", waveCaption.c_str());
                            }
                            if (progress > 0.0f && progress < 0.99f ) {
                                ImGui::SameLine();
                                if (ImFlux::ButtonFancy("Resume" , ImFlux::SLATE_BUTTON) ) {
                                    if (!instance->Resume()) Log("[error] failed to play %s", waveCaption.c_str());
                                }
                            }


                        } else {
                            if (ImFlux::ButtonFancy("Stop", ImFlux::BLUE_BUTTON)) {
                                if (!instance->Stop()) Log("[error] failed to stop %s", waveCaption.c_str());
                            }
                        }
                        ImGui::SameLine();
                        if (ImGui::Checkbox("Loop" , &instance->doLoop)) {
                        }
                        ImGui::SameLine();
                        if (ImGui::Checkbox("Rec" , &instance->doRecord)) {
                        }
                        ImGui::SameLine();ImGui::SetNextItemWidth(70.f);
                        if (ImGui::SliderFloat("Vol" , &instance->volume, 0.1f, 1.f)) {

                        }

                        if (!instance->isPlaying) {
                            ImGui::SameLine();
                            if ( instance->resource->fileType == FluxAudio::AudioType::SFX ) {
                                if (ImFlux::ButtonFancy("2WAV", ImFlux::YELLOW_BUTTON)) {
                                    if (!instance->ConvertToWav()) {
                                        mGuiGlue->showMessage("ERROR","Failed to convert to Wav!");
                                    }
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
    game->mSettings.Company = "Ohmtal";
    game->mSettings.Caption = "AudioTestBed";
    game->mSettings.enableLogFile = true;
    game->mSettings.IconFilename = "assets/icon.png";
    // game->mSettings.CursorFilename = "assets/particles/BloodHand.bmp";
    // game->mSettings.cursorHotSpotX = 10;
    // game->mSettings.cursorHotSpotY = 10;

    // LogFMT("TEST: My pref path would be:{}", SDL_GetPrefPath(game->mSettings.Company, game->mSettings.Caption ));

    game->Execute();
    SAFE_DELETE(game);
    return 0;
}


