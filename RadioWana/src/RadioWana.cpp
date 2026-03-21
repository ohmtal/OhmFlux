#include "RadioWana.h"
#include "appMain.h"
#include "utils/fluxSettingsManager.h"
#include "utils/errorlog.h"



// #include <gui/ImConsole.h>
//------------------------------------------------------------------------------
// macro for JSON support not NOT in HEADER !!
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(RadioWana::AppSettings,
                                                mDockSpaceInitialized,
                                                mShowFileBrowser,
                                                mShowConsole,
                                                mShowRadioBrowser,
                                                mShowRadio
)
// -----------------------------------------------------------------------------
// Console handling
// -----------------------------------------------------------------------------
void SDLCALL ConsoleLogFunction(void *userdata, int category, SDL_LogPriority priority, const char *message)
{
    if (!userdata) return;
    auto* gui = static_cast<RadioWana*>(userdata);

    if (!gui)
        return;

    char lBuffer[1024];
    if (priority == SDL_LOG_PRIORITY_ERROR)
    {
        snprintf(lBuffer, sizeof(lBuffer), "[ERROR] %s", message);
    }
    else if (priority == SDL_LOG_PRIORITY_WARN)
    {
        snprintf(lBuffer, sizeof(lBuffer), "[WARN] %s", message);
    }
    else
    {
        snprintf(lBuffer, sizeof(lBuffer), "%s", message);
    }

    // bad if we are gone !!
    gui->mConsole.AddLog("%s", fluxStr::removePart(message,"\r\n").c_str());
}

void RadioWana::DrawRadioBrowserWindow() {
        if (ImGui::Begin("Radio Browser", &mAppSettings.mShowRadioBrowser)){

            ImGui::SeparatorText("radio-browser.info");
            char strBuff[64];
            strncpy(strBuff, mQueryString.c_str(), sizeof(strBuff));

            if (ImGui::InputText("Name:", strBuff, sizeof(strBuff), ImGuiInputTextFlags_EnterReturnsTrue)) {
                mRadioBrowser->searchStationsByNameAndTag(strBuff, "");
                dLog("query for %s", strBuff);
            }

            if (ImGui::BeginTable("RadioStations", 5, ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY | ImGuiTableFlags_Resizable)) {
                ImGui::TableSetupColumn("Station");
                ImGui::TableSetupColumn("Homepage");
                // std::string favicon; << TODO would be cool but
                // ImGui::TableSetupColumn("Codec"); //we can mp3 only at the moment
                ImGui::TableSetupColumn("Clicks");
                ImGui::TableSetupColumn("Country");
                ImGui::TableSetupColumn("Bitrate");
                ImGui::TableHeadersRow();

                for (const auto& station : mQueryStationData) {
                    ImGui::PushID(station.stationuuid.c_str());
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();

                    bool isSelected = (mSelectedStationUuid == station.stationuuid);



                    if (ImGui::Selectable(station.name.c_str(), isSelected,
                        ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick)) {

                        //FIXME favo or save last
                        mSelectedStationUuid = station.stationuuid;

                    if (ImGui::IsMouseDoubleClicked(0)) {
                        mUrl = station.url;
                        mStreamHandler->Execute(mUrl);
                        mRadioBrowser->clickStation(station.stationuuid);
                    }
                        }

                        ImGui::TableNextColumn();
                        ImGui::TextUnformatted(station.homepage.c_str());

                        // ImGui::TableNextColumn();
                        // ImGui::TextUnformatted(station.Codec.c_str());

                        ImGui::TableNextColumn();
                        ImGui::Text("%d  Trend:%d",station.clickcount, station.clicktrend);

                        ImGui::TableNextColumn();
                        ImGui::TextUnformatted(station.countrycode.c_str());

                        ImGui::TableNextColumn();
                        if (station.bitrate >= 128) {
                            ImGui::TextColored(ImColor(0, 255, 0), "%d kbps", station.bitrate);
                        } else if (station.bitrate >= 64) {
                            ImGui::TextColored(ImColor(255, 255, 0), "%d kbps", station.bitrate);
                        } else if (station.bitrate == 0) {
                            ImGui::TextDisabled("---");
                        } else {
                            ImGui::TextColored(ImColor(255, 0, 0), "%d kbps", station.bitrate);
                        }

                        ImGui::PopID(/*station.stationuuid.c_str()*/);
                }
                ImGui::EndTable();
            }
        }
        ImGui::End();
    }
// -----------------------------------------------------------------------------
void RadioWana::onEvent(SDL_Event event){
       if (mGuiGlue.get()) mGuiGlue->onEvent(event);
   }
// -----------------------------------------------------------------------------
void RadioWana::ShowMenuBar(){
    if (ImGui::BeginMainMenuBar())
    {

        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Exit")) { getGame()->TerminateApplication(); }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Window"))
        {
            ImGui::MenuItem("Radio", NULL, &mAppSettings.mShowRadio);
            ImGui::MenuItem("Radio Browser", NULL, &mAppSettings.mShowRadioBrowser);
            ImGui::Separator();
            ImGui::MenuItem("Console", NULL, &mAppSettings.mShowConsole);
            ImGui::EndMenu();
        }


        if (ImGui::BeginMenu("Help"))
        {

            if (ImGui::MenuItem("About")) {
                mGuiGlue->showMessage ( "About",
                 std::format(
                        "RadioWana II\n"
                        "============\n"
                        "(c)2026 by Thomas Hühn \n"
                        "Version {}\n"
                        "https://ohmtal.com\n"
                        "\n"
                        "Settings are saved to:\n"
                        "{}\n"
                        "Recordings are saved to:\n"
                        "{}\n"
                        , getGame()->mSettings.Version
                        , getGame()->mSettings.getPrefsPath()
                        , mAudioRecorder->getPath()
                    )
                );
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }


}
// -----------------------------------------------------------------------------
void RadioWana::Deinitialize(){
    if (SettingsManager().IsInitialized()) {
        SettingsManager().set("AppGui::mAppSettings", mAppSettings);
        // SettingsManager().set("InputRackSettings", mInputModule->getInputEffectsSettingsBase64());
        // SettingsManager().set("SoundMix::MasterVolume", mSoundMixModule->getMasterVolume());
        // SettingsManager().set("Rack::curIdx",  mRackModule->getManager()->getActiveRackIndex());
        // SettingsManager().set("Rack::tabIdx",  mRackModule->mRackTabCurId);
        // SettingsManager().set("DrumKit::curIdx",  mDrumKitLooperModule->getManager()->getActiveRackIndex());

        SettingsManager().save();
    }
    SDL_SetLogOutputFunction(nullptr, nullptr); // log must be unlinked first!!
}
// -----------------------------------------------------------------------------
bool RadioWana::Initialize(){

    std::string lSettingsFile =
    getGame()->mSettings.getPrefsPath()
    .append(getGame()->mSettings.getSafeCaption())
    .append("_prefs.json");
    if (SettingsManager().Initialize(lSettingsFile))
    {

    } else {
        LogFMT("Error: Can not open setting file: {}", lSettingsFile);
    }

    mAppSettings = SettingsManager().get("AppGui::mAppSettings", AppSettings());

    // ~~~~~ GuiGlue ~~~~~
    mGuiGlue = std::make_unique<FluxGuiGlue>(true, false, nullptr);
    if (!mGuiGlue->Initialize())
        return false;

    if ( isAndroidBuild()) {
        setImGuiScale(2.f);
    }
    // ~~~~~  Console right after GuiGlue ~~~~~
    mConsole.OnCommand =  [&](ImConsole* console, const char* cmd) { OnConsoleCommand(console, cmd); };
    SDL_SetLogOutputFunction(ConsoleLogFunction, this);


    // ~~~~ modules ~~~~~
    mStreamHandler = std::make_unique<FluxRadio::StreamHandler>();
    mAudioHandler  = std::make_unique<FluxRadio::AudioHandler>();
    mAudioRecorder = std::make_unique<FluxRadio::AudioRecorder>();
    mRadioBrowser  = std::make_unique<FluxRadio::RadioBrowser>();


    mStreamHandler->OnConnected = [&]() {
        if (mAudioHandler.get())  mAudioHandler->init(mStreamHandler->getStreamInfo());
        mStreamHandler->dumpInfo();

    };
    mStreamHandler->OnStreamTitleUpdate = [&](std::string title, size_t streamPosition) {
        if (mAudioHandler.get()) mAudioHandler->OnStreamTitleUpdate(title, streamPosition);
    };
    mStreamHandler->OnAudioChunk = [&](const void* buffer , size_t size) {  mAudioHandler->OnAudioChunk(buffer, size); };
    mStreamHandler->onDisConnected = [&]() {
        if (mAudioHandler.get()) mAudioHandler->onDisConnected();
        if (mAudioRecorder.get()) mAudioRecorder->closeFile();
        mRecording = false;
    };

    mAudioHandler->OnTitleTrigger = [&]() {
        Log("Streamtitle %s", mAudioHandler->getCurrentTitle().c_str());
        //FIXME toggle delay ... needed for some stations
        if (mRecording && mAudioRecorder.get()) mAudioRecorder->openFile(mAudioHandler->getCurrentTitle());
    };

    mAudioHandler->OnAudioStreamData = [&](const uint8_t* buffer, size_t bufferSize)  {
        if (mAudioRecorder.get() && mAudioRecorder->isFileOpen()) {
            mAudioRecorder->OnStreamData(buffer,bufferSize);
        }
    };

    mRadioBrowser->OnStationResponse = [&](std::vector<FluxRadio::RadioStation> stations) {
        mQueryStationData = stations;
    };

    mRadioBrowser->OnStationResponseError = [&]() {
        mQueryStationData.clear();
    };

    return true;
}
// -----------------------------------------------------------------------------
void RadioWana::DrawGui(){
    mGuiGlue->DrawBegin();

    ShowMenuBar();
    if (mAppSettings.mShowConsole) mConsole.Draw("Console", &mAppSettings.mShowConsole);
    if (mAppSettings.mShowRadioBrowser) DrawRadioBrowserWindow();
    if (mAppSettings.mShowRadio) DrawRadio();

    mGuiGlue->DrawEnd();
}



