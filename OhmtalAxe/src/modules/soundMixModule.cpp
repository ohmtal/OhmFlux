
#include <SDL3/SDL.h>
#include <mutex>

#include <fstream>
#include <vector>
#include <string>
#include <stdexcept>
#include <type_traits>
#include <filesystem>

#include <src/appMain.h>

#include <audio/fluxAudio.h>
#include "soundMixModule.h"

#include "../appGlobals.h"



//------------------------------------------------------------------------------
void SDLCALL FinalMixCallback(void *userdata, const SDL_AudioSpec *spec, float *buffer, int buflen) {
    if (!userdata || !spec || !buffer || buflen < 1) return;
    auto* soundMix = static_cast<SoundMixModule*>(userdata);

    if (!soundMix )
        return;

    float vol = soundMix->getMasterVolume();

    //FIXME ALL EFFECTS NEED TO HANDLE CHANNELS spec->channels ....;
    //    ------- i added an example in limiter :D --------------

    if ( spec->format == SDL_AUDIO_F32 )
    {
        soundMix->getEffectsManager()->lock();
        if (buflen > 0 && soundMix->getEffectsManager()->getEffects().size() > 0)
        {
            int numSamples = buflen / sizeof(float);

            soundMix->getEffectsManager()->checkFrequence(spec->freq);
            soundMix->getEffectsManager()->process(buffer, numSamples, spec->channels);

            soundMix->getDrumManager()->checkFrequence(spec->freq);
            soundMix->getDrumManager()->process(buffer, numSamples, spec->channels);


            soundMix->mDrumKitLooper.process(buffer, numSamples, spec->channels);


            // master Volume
            for (int i = 0; i < numSamples; i++) {
                buffer[i] *= vol;
            }

            // analyzer
            soundMix->mSpectrumAnalyzer->process(buffer, numSamples, spec->channels);
            soundMix->mVisualAnalyzer->process(buffer, numSamples, spec->channels);


            // for (auto& effect : soundMix->getEffectsManager()->getEffects()) {
            //     effect->process(buffer, numSamples, spec->channels);
            // }


        }
        soundMix->getEffectsManager()->unlock();


    }

    // NOTE: EXAMPLE CODE:
    // if (is_recording && recording_stream) {
    //     SDL_PutAudioStreamData(recording_stream, buffer, buflen);
    // }
}

void SoundMixModule::DrawEffectManagerPresetListWindow(bool* p_enabled){
    if (!mInitialized ||  mEffectsManager == nullptr || !*p_enabled) return;
    ImGui::SetNextWindowSizeConstraints(ImVec2(200.0f, 400.f), ImVec2(FLT_MAX, FLT_MAX));
    ImGui::Begin("Rack Presets", p_enabled);
    DrawPresetList(mEffectsManager.get());
    ImGui::End(/*"Rack Presets", p_enabled*/);

}



//------------------------------------------------------------------------------
void SoundMixModule::DrawVisualAnalyzer(bool* p_enabled) {
    if (!mInitialized ||  mEffectsManager == nullptr || !*p_enabled) return;


        ImGui::SetNextWindowSizeConstraints(ImVec2(600.0f, 650.f), ImVec2(FLT_MAX, FLT_MAX));
        ImGui::Begin("Visualizer", p_enabled);

        {
            ImGui::PushID("SpectrumAnalyzer_Effect_Row");
            ImGui::BeginGroup();
            bool isEnabled = mSpectrumAnalyzer->isEnabled();
            if (ImFlux::LEDCheckBox(mSpectrumAnalyzer->getName(), &isEnabled, mSpectrumAnalyzer->getColor()))
                mSpectrumAnalyzer->setEnabled(isEnabled);
            float fullWidth = ImGui::GetContentRegionAvail().x;
            mSpectrumAnalyzer->DrawSpectrumAnalyzer(ImVec2(fullWidth, 80.0f));
            ImGui::EndGroup();
            ImGui::PopID();
            ImGui::Spacing();
        }

        {
            ImGui::PushID("VisualAnalyzer_Effect_Row");
            ImGui::BeginGroup();
            bool isEnabled = mVisualAnalyzer->isEnabled();
            if (ImFlux::LEDCheckBox(mVisualAnalyzer->getName(), &isEnabled, mVisualAnalyzer->getColor()))
                mVisualAnalyzer->setEnabled(isEnabled);
            float fullWidth = ImGui::GetContentRegionAvail().x;
            mVisualAnalyzer->renderPeakTest(); //FIXME
            ImGui::EndGroup();
            ImGui::PopID();
            ImGui::Spacing();
        }

        ImGui::End();

    }

//------------------------------------------------------------------------------
void SoundMixModule::DrawDrums(bool* p_enabled) {
    if (!mInitialized ||  mDrumManager == nullptr || !*p_enabled) return;

    ImGui::SetNextWindowSizeConstraints(ImVec2(600.0f, 650.f), ImVec2(FLT_MAX, FLT_MAX));
    ImGui::Begin("Drum Pads", p_enabled);
    mDrumManager->renderUI(3);
    ImGui::End();

}
//------------------------------------------------------------------------------
void SoundMixModule::DrawRack(bool* p_enabled)
{
    if (!mInitialized
        ||  mEffectsManager == nullptr
        || !*p_enabled) return;



    // if (!ImGui::Begin("Rack", p_enabled))
    // {
    //     ImGui::End();
    //     return;
    // }

    ImGui::SetNextWindowSizeConstraints(ImVec2(600.0f, 650.f), ImVec2(FLT_MAX, FLT_MAX));
    ImGui::Begin("Rack", p_enabled);

    //FIXME move the rack managemane to taskbar
    if (ImGui::BeginChild("RackManagement", ImVec2(0.f, 50.f))) {
        DSP::EffectsManager* lManager = getEffectsManager();

        int currentIdx = lManager->getActiveRackIndex();
        int count = lManager->getPresetsCount();



        if (count  > 0 && currentIdx >= 0)
        {
            ImGui::Text("Preset count: %d, Active Rack: [%d] %s (%d effects)",
                        count,
                        currentIdx,
                        lManager->getActiveRack()->getName().c_str(),
                        lManager->getActiveRack()->getEffectsCount()
            );

            if (ImFlux::ButtonFancy("<")) {
                currentIdx--;
                if (currentIdx < 0) currentIdx = count -1;
                lManager->setActiveRack(currentIdx);
            }
            ImGui::SameLine();
            ImFlux::LCDNumber(currentIdx , 3, 0, 24.0f);
            ImGui::SameLine();
            if (ImFlux::ButtonFancy(">")) {
                currentIdx++;
                if (currentIdx >= count) currentIdx = 0;
                lManager->setActiveRack(currentIdx);
            }
            ImGui::SameLine();

            char nameBuf[64];
            strncpy(nameBuf, lManager->getActiveRack()->getName().c_str(), sizeof(nameBuf));
            ImGui::Text("Name");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(200);
            if (ImGui::InputText("##Rack Name", nameBuf, sizeof(nameBuf))) {
                lManager->getActiveRack()->setName(nameBuf);
            }

            ImGui::SameLine();
            if (ImFlux::ButtonFancy("New")) {
                int newId = lManager->addRack();
                lManager->setActiveRack(newId);
                populateRack(lManager->getActiveRack());
            }
            ImGui::SameLine();
            if (ImFlux::ButtonFancy("Clone")) {
                int newId = lManager->cloneCurrent();
                lManager->setActiveRack(newId);
            }
            ImGui::SameLine();
            if (count < 2) ImGui::BeginDisabled();
            if (ImFlux::ButtonFancy("Delete")) {
                int newId = lManager->removeRack(currentIdx);
                lManager->setActiveRack(newId);
            }
            if (count < 2) ImGui::EndDisabled();

            ImGui::SameLine();
            if (ImFlux::ButtonFancy("restore Factory defaults", ImFlux::RED_BUTTON.WithSize(ImVec2(200.f,24.f)))) {

                if (!lManager->LoadPresets(mFactoryPresetFile)) {
                    showMessage("ERROR", "Failed to load Factory Presets.");

                }


                // mDeletePatternScheduleId = FluxSchedule.add(0.0f, nullptr, [&song, patternIndex]() {
                //     std::erase(song.orderList, patternIndex);
                //     for (auto& item : song.orderList) {
                //         if (item > patternIndex) {
                //             item--;
                //         }
                //     }
                //     song.deletePattern(patternIndex);
                // });

                // static FluxScheduler::TaskID loadFactorySchedule = 0;
                // if (!FluxSchedule.isPending(loadFactorySchedule))
                // {
                //     std::string fileName = mFactoryPresetFile;
                //     loadFactorySchedule = FluxSchedule.add(0.0f, nullptr, [fileName, lManager]() {
                //         if (!lManager->LoadPresets(fileName)) { showMessage("ERROR", "Failed to load Factory Presets."); }
                //     });
                //
                //
                // }


            }


        } // count > 0



    }
    ImGui::EndChild();
    //<<< rack management
    //FIXME save last selected in settings !
    if (ImGui::BeginTabBar("##tabs", ImGuiTabBarFlags_None))
    {
        //  ~~~~~~~~~~~ RENDERIU ~~~~~~~~~~~~~~~~~~~~~
        if (ImGui::BeginTabItem("Effects Rack 80th")){
            mEffectsManager->renderUI(0);
            ImGui::EndTabItem();
        }
        //  ~~~~~~~~~~~ RENDERIU_WIDE ~~~~~~~~~~~~~~~~~~~~~
        if (ImGui::BeginTabItem("Effects Rack"))
        {
            mEffectsManager->renderUI(1);
            ImGui::EndTabItem();
        }
        //  ~~~~~~~~~~~ RENDERPADDLE ~~~~~~~~~~~~~~~~~~~~~
        if (ImGui::BeginTabItem("Effect Paddles")) {
            mEffectsManager->renderUI(2);
            ImGui::EndTabItem();
        }
    } //tabBar ....
    ImGui::EndTabBar();
    ImGui::End();


}
//------------------------------------------------------------------------------
void SoundMixModule::populateRack(DSP::EffectsRack* lRack){
    std::vector<DSP::EffectType> types = {
        // DSP::EffectType::NoiseGate,
        // DSP::EffectType::ChromaticTuner,
        DSP::EffectType::DistortionBasic,
        DSP::EffectType::OverDrive,
        DSP::EffectType::Metal,
        // DSP::EffectType::Bitcrusher,
        DSP::EffectType::AnalogGlow,

        // DSP::EffectType::RingModulator,
        // DSP::EffectType::VoiceModulator,
        // DSP::EffectType::Warmth,

        DSP::EffectType::Chorus,
        DSP::EffectType::Reverb,
        DSP::EffectType::Delay,
        DSP::EffectType::Equalizer9Band,
        DSP::EffectType::ToneControl,
        DSP::EffectType::Limiter,
    };


    // manually !!
    // DSP::EffectType::DrumKit,
    // DSP::EffectType::SpectrumAnalyzer,
    // DSP::EffectType::VisualAnalyzer


    for (auto type : types) {
        auto fx = DSP::EffectFactory::Create(type);
        if (fx) {
            lRack->getEffects().push_back(std::move(fx));
        }
    }


}

//------------------------------------------------------------------------------
bool SoundMixModule::Initialize() {

    mPresetsFile =
    getGame()->mSettings.getPrefsPath()
        .append(getGame()->mSettings.getSafeCaption())
        .append(".rack.presets");

    mFactoryPresetFile = getGamePath().append("assets/Factory.rack.presets");

    mDrumKitFile =
    getGame()->mSettings.getPrefsPath()
    .append(getGame()->mSettings.getSafeCaption())
    .append(".drum");



    mEffectsManager = std::make_unique<DSP::EffectsManager>(true);
    populateRack(mEffectsManager->getActiveRack());


    // for (auto type : types) {
    //     auto fx = DSP::EffectFactory::Create(type);
    //     if (fx) {
    //         // use defaults ! fx->setEnabled(false);
    //         mEffectsManager->addEffect(std::move(fx));
    //     }
    // }

    mDrumManager = std::make_unique<DSP::EffectsManager>(true);

    std::vector<DSP::EffectType> drumTypes = {
        DSP::EffectType::KickDrum,
    };
    for (auto type : drumTypes) {
        auto fx = DSP::EffectFactory::Create(type);
        if (fx) {
            fx->setEnabled(true); //drums on be default
            mDrumManager->addEffect(std::move(fx));
        }
    }





    mSpectrumAnalyzer = cast_unique<DSP::SpectrumAnalyzer>(DSP::EffectFactory::Create(DSP::EffectType::SpectrumAnalyzer));
    mVisualAnalyzer = cast_unique<DSP::VisualAnalyzer>(DSP::EffectFactory::Create(DSP::EffectType::VisualAnalyzer));



    for (const auto& fx : mEffectsManager->getEffects()) {
        Log("[info] Effect %s (Type: %d) is loaded.", fx->getName().c_str(), fx->getType());
    }

    // Setup PostMix
    if (!SDL_SetAudioPostmixCallback(AudioManager.getDeviceID(), FinalMixCallback, this)) {
        dLog("[error] can NOT open PostMix Device !!! %s", SDL_GetError());
    } else {
        Log("[info] SoundMixModule: PostMix Callback installed.");
    }


    Log("[info] SoundMixModule init done.");


    bool presetExits = std::filesystem::exists(mPresetsFile);


    if (!mEffectsManager->LoadPresets(presetExits ? mPresetsFile : mFactoryPresetFile)) {
        LogFMT("[error] "+mEffectsManager->getErrors());
    }

    mInitialized = true;
    return true;

}
//------------------------------------------------------------------------------
// preset List FIXME to class
//------------------------------------------------------------------------------
void SoundMixModule::DrawPresetList(DSP::EffectsManager* lManager) {
    lManager->DrawPresetList();

    // // default size
    // ImVec2 controlSize = {0,0};
    // // magic pointer movement
    // static int move_from = -1, move_to = -1;
    // int delete_idx = -1, insert_idx = -1;
    // int clone_idx = -1;
    // // init pointers
    //
    // // --- Compact Header ---
    //
    // // Start a child region for scrolling if the list gets long
    // ImGui::BeginChild("PresetListScroll", controlSize, false);
    //
    // int currentIdx = lManager->getActiveRackIndex();
    //
    // for (int rackIdx = 0; rackIdx < lManager->getPresetsCount(); rackIdx++) {
    //     const bool is_selected = (currentIdx == rackIdx);
    //     ImGui::PushID(lManager);ImGui::PushID(rackIdx);
    //
    //     // 1. Draw Index
    //     ImGui::AlignTextToFramePadding();
    //     // ImGui::TextDisabled("%02d", rackIdx);
    //     ImGui::TextDisabled("%02X", rackIdx);
    //     ImGui::SameLine();
    //
    //     // 2. Button Dimensions & Interaction
    //     ImVec2 pos = ImGui::GetCursorScreenPos();
    //     ImVec2 size = ImVec2(ImGui::GetContentRegionAvail().x - 10.0f, ImGui::GetFrameHeight());
    //
    //
    //     float coloredWidth = 30.0f;
    //     ImVec2 coloredSize(coloredWidth, size.y);
    //
    //
    //     // InvisibleButton acts as the interaction hit-box for DragDrop and Clicks
    //     ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
    //     bool pressed = ImGui::InvisibleButton("rack_btn", size);
    //     if (pressed) lManager->setActiveRack(rackIdx);
    //
    //     bool is_hovered = ImGui::IsItemHovered();
    //     bool is_active = ImGui::IsItemActive();
    //     ImDrawList* draw_list = ImGui::GetWindowDrawList();
    //
    //     // ----------- rendering
    //
    //     // 1. Logic for enhanced selection visibility
    //     ImU32 col32 = ImFlux::getColorByIndex(rackIdx);
    //     ImU32 colMiddle32 =  IM_COL32(20, 20, 20, 255);
    //     ImU32 border_col = IM_COL32_WHITE;
    //     float border_thickness = 1.0f;
    //
    //     if (is_selected) {
    //         col32 = (col32 & 0x00FFFFFF) | 0xFF000000;
    //         border_col = IM_COL32(255, 255, 0, 255); // Neon Yellow
    //         border_thickness = 2.0f;
    //         colMiddle32 = IM_COL32(40, 40, 40, 255);
    //
    //     } else {
    //         col32 = (col32 & 0x00FFFFFF) | 0x66000000;
    //     }
    //
    //     // left
    //     draw_list->AddRectFilled(pos, pos + coloredSize, col32, 3.0f);
    //
    //     // Middle part: Starts after left bar, width is (total - 2 * side bars)
    //     ImVec2 midPos = ImVec2(pos.x + coloredWidth, pos.y);
    //     ImVec2 midSize = ImVec2(size.x - (2.0f * coloredWidth), size.y);
    //     draw_list->AddRectFilled(midPos, midPos + midSize, colMiddle32, 0.0f); // No rounding for middle to avoid gaps
    //
    //     // Right part: Starts at the end minus the side bar width
    //     ImVec2 rightPos = ImVec2(pos.x + size.x - coloredWidth, pos.y);
    //     draw_list->AddRectFilled(rightPos, rightPos + coloredSize, col32, 3.0f);
    //
    //
    //     // Selection "Glow" / Outline
    //     if (is_selected) {
    //         // Outer Glow Effect: Draw a slightly larger, transparent rect behind/around
    //         // draw_list->AddRect(pos - ImVec2(2, 2), pos + size + ImVec2(2, 2),
    //         //                    IM_COL32(255, 255, 0, 100), 3.0f, 0, 4.0f);
    //
    //
    //
    //         // Solid Inner Border
    //         draw_list->AddRect(pos, pos + size, border_col, 3.0f, 0, border_thickness);
    //
    //         // OPTIONAL: Add a small white "active" indicator circle on the left
    //         draw_list->AddCircleFilled(ImVec2(pos.x - 5, pos.y + size.y * 0.5f), 3.0f, border_col);
    //     } else if (is_hovered) {
    //         draw_list->AddRect(pos, pos + size, IM_COL32(255, 255, 255, 180), 3.0f);
    //     }
    //
    //     // Text Contrast
    //     // For the selected item, use Black text if the background is very bright
    //     // (or stay with White+Shadow for consistency)
    //     std::string rackNameStr = lManager->getRackByIndex(rackIdx)->getName();
    //     const char* rackName = rackNameStr.c_str();
    //     ImVec2 text_size = ImGui::CalcTextSize(rackName);
    //     ImVec2 text_pos = ImVec2(pos.x + (size.x - text_size.x) * 0.5f, pos.y + (size.y - text_size.y) * 0.5f);
    //
    //     ImU32 text_col = is_selected ? IM_COL32_WHITE : IM_COL32(200, 200, 200, 255);
    //     draw_list->AddText(text_pos + ImVec2(1, 1), IM_COL32(0, 0, 0, 255), rackName);
    //     draw_list->AddText(text_pos, text_col, rackName);
    //
    //
    //     // 6. Context Menu
    //     if (ImGui::BeginPopupContextItem("row_menu")) {
    //         if (ImGui::MenuItem("Clone")) clone_idx = rackIdx;
    //         if (ImGui::MenuItem("Insert Above")) insert_idx = rackIdx;
    //         if (ImGui::MenuItem("Remove")) delete_idx = rackIdx;
    //         ImGui::EndPopup();
    //     }
    //
    //     // 7. Drag and Drop (Attached to the InvisibleButton)
    //     if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
    //         ImGui::SetDragDropPayload("DND_ORDER", &rackIdx, sizeof(int));
    //     ImGui::Text("Moving %s", rackName);
    //     ImGui::EndDragDropSource();
    //     }
    //     if (ImGui::BeginDragDropTarget()) {
    //         if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_ORDER")) {
    //             move_from = *(const int*)payload->Data;
    //             move_to = rackIdx;
    //         }
    //         ImGui::EndDragDropTarget();
    //     }
    //
    //     ImGui::PopStyleVar();
    //     ImGui::PopID();ImGui::PopID();
    // }
    // ImGui::EndChild();
    //
    // // Deferred move/delete
    // if (delete_idx != -1) lManager->removeRack(delete_idx);
    // if ( clone_idx != -1 ) lManager->cloneRack(clone_idx);
    // if (insert_idx != -1) lManager->insertRackAbove(insert_idx);
    // if (move_from != -1 && move_to != -1) {
    //     dLog("[info] move rack from %d to %d", move_from, move_to) ;
    //     lManager->reorderRack(move_from, move_to);
    //     move_from = -1;
    //     move_to = -1;
    // }
} //DrawPresetList

