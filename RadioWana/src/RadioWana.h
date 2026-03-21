//-----------------------------------------------------------------------------
// Copyright (c) 2012 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// RadioWana
//-----------------------------------------------------------------------------
#pragma once

// #include <audio/fluxAudio.h>
#include <core/fluxBaseObject.h>
#include <core/fluxRenderObject.h>
#include <gui/fluxGuiGlue.h>
#include <gui/ImConsole.h>
#include "net/CurlGlue.h"

#include "StreamHandler.h"
#include "AudioHandler.h"
#include "StreamInfo.h"
#include "AudioRecorder.h"
#include "RadioBrowser.h"


class RadioWana: public FluxBaseObject {
private:
    //FIXME FluxRenderObject* mBackground = nullptr;

    void OnConsoleCommand(ImConsole* console, const char* cmdline) {}

    std::unique_ptr<FluxGuiGlue>  mGuiGlue;

    std::unique_ptr<FluxRadio::StreamHandler> mStreamHandler;
    std::unique_ptr<FluxRadio::AudioHandler>  mAudioHandler;
    std::unique_ptr<FluxRadio::AudioRecorder> mAudioRecorder;
    std::unique_ptr<FluxRadio::RadioBrowser> mRadioBrowser;


    // main
    std::string mUrl = "https://stream.rockantenne.de/rockantenne/stream/mp3"; //<< current Stream URL

    // Recordings
    bool mRecording = false;
    bool mRecordingStartsOnNewTile = true;
    bool mRecordingMixTape = false; //<< FIXME !!

    // Radio browser
    std::vector<FluxRadio::RadioStation> mQueryStationData;
    std::string mQueryString = "";
    std::string mSelectedStationUuid = "";

    std::vector<FluxRadio::RadioStation> mFavoStationData;


public:
    struct AppSettings {
        bool mDockSpaceInitialized = false;
        bool mShowFileBrowser      = false;
        bool mShowConsole          = false;
        bool mShowRadioBrowser     = true;
        bool mShowRadio            = true;
    };


    ImConsole mConsole;
    AppSettings mAppSettings;

    bool Initialize() override;
    void Deinitialize() override;
    void onEvent(SDL_Event event);
    // void DrawMsgBoxPopup();
    void ShowMenuBar();
    // void ShowToolbar();

    void DrawGui( );
    void onKeyEvent(SDL_KeyboardEvent event) {};
    void InitDockSpace() {}
    void ShowFileBrowser() {}
    void ApplyStudioTheme() {}
    void setupFonts() {}
    AppSettings* getAppSettings() {return &mAppSettings;}
    void restoreLayout( ) {}
    void setImGuiScale(float factor) {}

    //-----
    void DrawRadioBrowserWindow();
    void DrawRadio() {
        //FIXME only a copy from the prototype !!!

        if (ImGui::Begin("RadioWana")) {
            float fullWidth = ImGui::GetContentRegionAvail().x;

            // ImGui::SetNextItemWidth(450.f);
            char strBuff[256];
            strncpy(strBuff, mUrl.c_str(), sizeof(strBuff));
            if (ImGui::InputText("URL", strBuff, sizeof(strBuff))) {
                mUrl = strBuff;
            }
            if ( mStreamHandler->isRunning() ) {
                if (ImFlux::ButtonFancy("close")) {
                    mStreamHandler->stop();
                }
                FluxRadio::StreamInfo* info = mStreamHandler->getStreamInfo();
                if (info)
                {
                    // inline void LCDText(std::string text, int display_chars, float height, ImU32 color_on, bool scroll = true, float scroll_speed = 2.0f) {

                    ImFlux::LCDText(mAudioHandler->getCurrentTitle(), 20, 36.f, ImFlux::COL32_NEON_ORANGE);
                    ImGui::SeparatorText(info->streamUrl.c_str());
                    ImGui::SeparatorText(info->name.c_str());
                    ImGui::TextColored(ImVec4(0.3f, 0.3f,0.7f,1.f), "%s", mAudioHandler->getCurrentTitle().c_str());
                    ImGui::TextDisabled("Next: %s", mAudioHandler->getNextTitle().c_str());
                    ImGui::Separator();
                    ImGui::Text("Description: %s", info->description.c_str());
                    ImGui::Text("Audio: %d Hz, %d kbps, %d Channels", info->samplerate, info->bitrate, info->channels);
                    ImGui::Text("Url: %s", info->url.c_str());
                }

                float vol = mAudioHandler->getVolume();
                if (ImFlux::LEDMiniKnob("Volume", &vol, 0.f, 1.f)) {
                    mAudioHandler->setVolume(vol);
                }
                ImFlux::SameLineBreak(200);

                if ( mAudioHandler->getManager() && mAudioHandler->getManager()->getVisualAnalyzer()) {
                    mAudioHandler->getManager()->getVisualAnalyzer()->renderVU(ImVec2(200,60), 70);

                    //FIXME separate VU's:
                    //                         float dbL, dbR;
                    //                         dbL = this->getDecible(0);
                    //                         dbR = this->getDecible(1);
                    //
                    //                         // this->getDecible(dbL, dbR);
                    //
                    //                         auto mapDB = [](float db) {
                    //                             float minDB = -20.0f;
                    //                             return (db < minDB) ? 0.0f : (db - minDB) / (0.0f - minDB);
                    //                         };
                    //
                    //                         ImFlux::VUMeter70th(halfSize, mapDB(dbL));
                    //                         ImGui::SameLine();
                    //                         ImFlux::VUMeter70th(halfSize, mapDB(dbR));


                }
                if ( mAudioHandler->getManager() && mAudioHandler->getManager()->getSpectrumAnalyzer()) {

                    mAudioHandler->getManager()->getSpectrumAnalyzer()->DrawSpectrumAnalyzer(ImVec2(fullWidth,60), true);
                }

                mAudioHandler->RenderRack(1);

                ImGui::SeparatorText("Recording");

                ImGui::Checkbox("Recording starts on when new stream title is triggered", &mRecordingStartsOnNewTile);
                if (ImFlux::LEDCheckBox("Enable Recording", &mRecording, ImVec4(0.8f,0.3f,0.3f,1.f))) {
                    if (mRecording && !mRecordingStartsOnNewTile && !mAudioHandler->getCurrentTitle().empty()) {
                        mAudioRecorder->openFile(mAudioHandler->getCurrentTitle());
                    }
                    if (!mRecording)
                        mAudioRecorder->closeFile();
                }
                if (mRecording) {
                    ImFlux::DrawLED("Recording", mAudioRecorder->isFileOpen(), ImFlux::LED_GREEN_ANIMATED_GLOW);
                    ImGui::SameLine();
                    ImGui::Text("File: %s", mAudioRecorder->getCurrentFilename().c_str());
                }

            } else {
                if (ImFlux::ButtonFancy("open URL")) {
                    mStreamHandler->Execute(mUrl);
                }
            }
        }
        ImGui::End();
    }


//
}; //class

