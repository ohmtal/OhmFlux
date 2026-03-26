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
#include "net/NetTools.h"
#include "core/fluxTexture.h"
#include "utils/fluxScheduler.h"

#include "StreamHandler.h"
#include "AudioHandler.h"
#include "StreamInfo.h"
#include "AudioRecorder.h"
#include "RadioBrowser.h"



// -----------------------------------------------------------------------------
constexpr  ImFlux::ButtonParams gRadioButtonParams {
    .color = IM_COL32(20, 20, 30, 255),
    .size  = { 40.f, 40.f },
    .gloss = true, //<< does not work so good with round buttons
    .rounding = 6.f,

};

constexpr  ImFlux::GradientParams gRadioDisplayBox {
    // ImVec2 pos  = ImVec2(0, 0);
    // size = ImVec2(0, 0);
    .rounding = 0.f,
    // bool   inset    = true;

    // Background Colors
    .col_top   = IM_COL32(30, 30, 30, 255),
    .col_bot   = IM_COL32(15, 15, 15, 255),

    // Decoration Colors
    // ImU32 col_shadow = IM_COL32(0, 0, 0, 200);      // Inner shadow
    // ImU32 col_rim    = IM_COL32(255, 255, 255, 50); // Light edge
    // ImU32 col_raised = IM_COL32(55, 55, 65, 255);   // Base for !inset
};


// -----------------------------------------------------------------------------


class RadioWana: public FluxBaseObject {
private:
    //FIXME FluxRenderObject* mBackground = nullptr;

    void OnConsoleCommand(ImConsole* console, const char* cmdline);

    std::unique_ptr<FluxGuiGlue>  mGuiGlue;

    std::unique_ptr<FluxRadio::StreamHandler> mStreamHandler;
    std::unique_ptr<FluxRadio::AudioHandler>  mAudioHandler;
    std::unique_ptr<FluxRadio::AudioRecorder> mAudioRecorder;
    std::unique_ptr<FluxRadio::RadioBrowser> mRadioBrowser;


    // main
   //moved to AppSettings std::string mUrl = ""; //<< current Stream URL



    // {"ok":true,"message":"retrieved station url","stationuuid":"960594a6-0601-11e8-ae97-52543be04c81","name":"Rock Antenne",
    // "url":"http://mp3channels.webradio.rockantenne.de/rockantenne"}
    const std::vector<FluxRadio::RadioStation> mDefaultFavo = {
        {
            .stationuuid = "960594a6-0601-11e8-ae97-52543be04c81",
            .name= "Rock Antenne",
            .url = "http://mp3channels.webradio.rockantenne.de/rockantenne",
            .countrycode = "DE",
            .favId = 1
        },
        // {"ok":true,"message":"retrieved station url","stationuuid":"92556f58-20d3-44ae-8faa-322ce5f256c0",
        // "name":"Radio BOB!","url":"http://streams.radiobob.de/bob-national/mp3-192/mediaplayer"},
        {
            .stationuuid = "92556f58-20d3-44ae-8faa-322ce5f256c0",
            .name= "BOB! - Radio Bob",
            .url = "http://streams.radiobob.de/bob-national/mp3-192/mediaplayer",
            .countrycode = "DE",
            .favId = 2
        }
    };

    // Recordings
    bool mRecording = false;
    bool mRecordingStartsOnNewTile = true;
    bool mRecordingMixTape = false; //<< FIXME !!

    // Radio browser
    std::vector<FluxRadio::RadioStation> mQueryStationData;
    std::string mQueryString = "";
    std::string mSelectedStationUuid = "";
    uint32_t mSelectedFavId = 0;

    std::vector<FluxRadio::RadioStation> mFavoStationData;


    int mSelectedFavIndex = -1; //not the id the index in the list
    bool mTuningMode = false;
    FluxScheduler::TaskID mTuningResetTaskID = 0;
    const double mTuningResetSec = 3.0f;

public:
    FluxTexture* mBrushedMetalTex = nullptr;
    FluxTexture* mKnobSilverTex = nullptr;
    FluxTexture* mKnobOffTex = nullptr;
    FluxTexture* mKnobOnTex = nullptr;
    // FluxTexture* mBackgroundTex = nullptr;

    struct AppSettings {
        // removed :P std::string mUrl = "http://mp3channels.webradio.rockantenne.de/rockantenne";
        FluxRadio::RadioStation CurrentStation;
        float Volume = 1.f;
        bool DockSpaceInitialized = false;
        bool ShowFileBrowser      = false;
        bool ShowConsole          = false;
        bool ShowRadioBrowser     = true;
        bool ShowRadio            = true;
        bool ShowRecorder         = true;
        bool ShowFavo             = true;

    };


    ImConsole mConsole;
    AppSettings mAppSettings;

    bool Initialize() override;
    void Deinitialize() override;
    void SaveSettings();
    void onEvent(SDL_Event event);
    // void DrawMsgBoxPopup();
    void ShowMenuBar();
    // void ShowToolbar();

    void onKeyEvent(SDL_KeyboardEvent event) {};
    void InitDockSpace() {}
    void ShowFileBrowser() {}
    void ApplyStudioTheme();
    void setupFonts();
    AppSettings* getAppSettings() {return &mAppSettings;}
    void restoreLayout( ) {}
    void setImGuiScale(float factor) {}

    //-----
    void DrawGui( );

    void DrawFavo();
    void DrawStationsList(std::vector<FluxRadio::RadioStation> stations, bool isFavoList );
    void DrawRadioBrowserWindow();
    void DrawInfoPopup(FluxRadio::StreamInfo* info);

    void DrawRadio();
    void DrawRecorder();

    bool isFavoStation(std::string searchUuid);


    // ---------- Tune Station -----------------
    bool ConnectCurrent() {
        if (!FluxNet::NetTools::isValidURL(mAppSettings.CurrentStation.url) ) {
            return false;
        }
        mStreamHandler->Execute(mAppSettings.CurrentStation.url);
        if (!mAppSettings.CurrentStation.stationuuid.empty()) mRadioBrowser->clickStation(mAppSettings.CurrentStation.stationuuid);
        return true;
    }
    void Disconnect() {
        mStreamHandler->stop();
    }

    void Tune(FluxRadio::RadioStation station) {
        mAppSettings.CurrentStation = station;
        ConnectCurrent();
        mSelectedFavId = -1; //Reset
        mTuningMode = false;

    }

    //-------------------- TuneKnob Interger with overflow ---------------------------
    void TuneKnob(std::string caption, const ImFlux::KnobSettings ks = ImFlux::DARK_KNOB)
    {
        ImGui::PushID((caption + "knob").c_str());
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems) { ImGui::PopID(); return ; }

        if (mSelectedFavIndex < 0) {
            mSelectedFavIndex = 0; //<< fallback
            for (int i =0 ; i < (int)mFavoStationData.size(); i++) {
                if ( mFavoStationData[i].favId == mAppSettings.CurrentStation.favId ) {
                   mSelectedFavIndex = i;
                   break;
                }
            }
        }

        float delta = 0.f;
        int step = 1;
        int* v = &mSelectedFavIndex;
        int v_min = 0;
        int v_max = (int)mFavoStationData.size() - 1;
        if (v_max < 1) return ; //empty list fixme ?!
        if (*v > v_max ) *v = 0;



        ImVec2 pos = ImGui::GetCursorScreenPos();
        ImVec2 size = ImVec2(ks.radius * 2, ks.radius * 2);
        ImRect bb(pos, pos + size);

        ImGui::InvisibleButton(caption.c_str(), size);

        bool value_changed = false;
        bool isConnected = mStreamHandler->isConnected();
        bool is_active = ImGui::IsItemActive();
        bool is_hovered = ImGui::IsItemHovered();
        bool is_clicked = ImGui::IsItemClicked();
        static bool is_Pressed = false;
        bool is_mouseRelease = ImGui::IsItemDeactivated();

        if (is_clicked) is_Pressed = true;

        ImGuiIO& io = ImGui::GetIO();

        int new_v = *v;
        // --- INTERACTION ---
        if (is_hovered && io.MouseWheel != 0) {
            delta = ImGui::GetIO().MouseWheel;
            new_v = *v + (int)delta * step;
            if (new_v != *v) value_changed = true;
        }

        //FIXME THIS DOES IS CLICKED ?!?!?!?

        if (is_active && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
            delta = ImGui::GetIO().MouseDelta.y;
            if (std::abs(delta) > 0.0f) {
                static float accumulator = 0.0f;
                accumulator -= delta;
                if (std::abs(accumulator) >= 5.0f) {
                    int steps = (int)(accumulator / 5.0f) * step;
                    new_v = *v + steps;
                    accumulator -= (float)steps * 5.0f; // keep the remainder
                    value_changed = true;
                }
            }
        } else {

        }


        // clamp
        if (new_v < v_min) new_v = v_max;
        if (new_v > v_max) new_v = v_min;
        *v = new_v ;


        // --- DRAWING ---
        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 center = ImVec2(pos.x + ks.radius, pos.y + ks.radius);

        // Outer Border / Housing (Integration of bg_outer)
        dl->AddCircle(center, ks.radius, ks.bg_inner, 32, 1.5f);
        // dl->AddCircleFilled(center, ks.radius + 1.f, ks.bg_inner);
        dl->AddCircleFilled(center, ks.radius, ks.bg_outer);

        // Main Knob Body
        float knob_radius = ks.radius * 0.90f; // 0.65f;
        //seamless movement
        static float visual_value = 0.0f;
        visual_value += delta * 0.01f;
        float needle_ang = visual_value  * M_2PI;
        // ImGui::SameLine(); ImGui::Text("%.3f", visual_value);


        if (mKnobSilverTex) {
            float r = knob_radius;
            GLuint handle = 0;

            if ( !isConnected ) handle = mKnobOffTex->getHandle();
            else if (mTuningMode) handle = mKnobSilverTex->getHandle();
            else handle = mKnobOnTex->getHandle();
            ImTextureID texID = (ImTextureID)(intptr_t)handle;

            // Die 4 Eckpunkte des Bildes (ungerotiert)
            ImVec2 p0 = center + ImVec2(-r, -r); // Oben Links
            ImVec2 p1 = center + ImVec2( r, -r); // Oben Rechts
            ImVec2 p2 = center + ImVec2( r,  r); // Unten Rechts
            ImVec2 p3 = center + ImVec2(-r,  r); // Unten Links

            // Mit deiner Funktion rotieren
            dl->AddImageQuad(
                texID,
                ImFlux::Rotate(p0, center, needle_ang),
                ImFlux::Rotate(p1, center, needle_ang),
                ImFlux::Rotate(p2, center, needle_ang),
                ImFlux::Rotate(p3, center, needle_ang),
                ImVec2(0, 0), ImVec2(1, 0), ImVec2(1, 1), ImVec2(0, 1)
            );
        }


        //bevel
        dl->AddCircle(center, ks.radius, ks.bevel, 32, 1.0f);

        // 4. Indicator Needle (On the knob body)
        // ImVec2 n_start = center + ImVec2(cosf(needle_ang) * (knob_radius * 0.3f), sinf(needle_ang) * (knob_radius * 0.3f));
        // ImVec2 n_end   = center + ImVec2(cosf(needle_ang) * (knob_radius - 2.0f), sinf(needle_ang) * (knob_radius - 2.0f));
        // dl->AddLine(n_start, n_end, ks.needle, 2.0f);


        if (is_hovered) ImGui::SetTooltip("%s: #%d %s", caption.c_str(), *v + 1, mFavoStationData[*v].name.c_str());

        ImGui::PopID();

        static double last_click_time = 0.0;
        const double cooldown_duration = 1.f;  //sec cooldown

        if (value_changed) {
            dLog("TuneKnob: value changed: %d", mSelectedFavIndex);
            mTuningMode = true;
            if (FluxSchedule.isPending(mTuningResetTaskID)) {
                FluxSchedule.extend(mTuningResetTaskID,mTuningResetSec );
            } else {
                mTuningResetTaskID = FluxSchedule.add(mTuningResetSec, nullptr,[&]() { mTuningMode = false; });
            }

            is_Pressed = false;
        }
        // ImGui::SameLine();  ImFlux::DrawLED("clicki",is_clicked, ImFlux::LED_RED);
        // ImGui::SameLine(); ImFlux::DrawLED("pressed",is_Pressed, ImFlux::LED_BLUE);
        // ImGui::SameLine(); ImFlux::DrawLED("connected",isConnected, ImFlux::LED_GREEN);

        if (is_mouseRelease && is_Pressed ) {
            is_Pressed = false;
            if (ImGui::GetTime() - last_click_time > cooldown_duration) {

                if (isConnected && !mTuningMode) {
                    Log("[error] GO OFFLINE");
                    Disconnect();

                } else {
                   Tune(mFavoStationData[*v]);
                   Log("[error] TuneKnob: TUNE Selected Station: %s", mFavoStationData[*v].name.c_str());
                }


                // mAppSettings.CurrentFavId = mFavoStationData[*v].favId;
            } else {
                Log("[warn] TuneKnob: Click ignored ... too fast!");
            }
            last_click_time = ImGui::GetTime();
        }
    }

}; //class

