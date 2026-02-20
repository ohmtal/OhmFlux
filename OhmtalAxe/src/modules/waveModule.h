#pragma once

#include <string>
#include <imgui.h>

#include <gui/fluxGuiGlue.h>
#include <gui/ImFlux.h>

#include <audio/fluxAudioStream.h>

#include <core/fluxBaseObject.h>

class WaveModule : public FluxBaseObject {
private:
    std::vector<std::unique_ptr<FluxAudioStream>> mAudioStreams;

    bool mInitialized = false;

public:
    WaveModule() = default;
    ~WaveModule() {}
    bool Initialize() override  {
        return true;
    }
    void Deinitialize() override {

    }

    virtual void Update(const double& dt) override {
        // for (auto& effect : soundMix->getEffectsManager()->getEffects()) {
        //     effect->process(buffer, numSamples, spec->channels);
        // }

        for (auto& ao : mAudioStreams )
            ao->Update(dt);
    };

    bool loadWave( std::string fileName )
    {
        std::unique_ptr<FluxAudioStream> as = std::make_unique<FluxAudioStream>(fileName.c_str());
        if ( as->IsInitialized() ) {
            mAudioStreams.push_back(std::move(as));
            Log("[info] Wave File %s loaded.", fileName.c_str());
            return true;
        }

        Log("[error] Failed to load Wave File %s!", fileName.c_str());
        return false;
    }

    void DrawWaveStreams( )
    {
        ImGui::SetNextWindowSize(ImVec2(200, 600), ImGuiCond_FirstUseEver);
        ImGui::Begin("Wave Streams");

        if (ImGui::BeginChild("WaveModule_BOX", ImVec2(0, 0), ImGuiChildFlags_Borders)) {
            //Buttons
            //FIXME buttons: RenderButtons(getMain()->getController());
            ImGui::Separator();

            // List
            std::string waveCaption;
            bool isSelected = false;

            if (ImGui::BeginListBox("##WaveList", ImVec2(-FLT_MIN, -FLT_MIN))) {
                for (int n = 0; n < (int)mAudioStreams.size(); n++) {
                    auto ao = mAudioStreams[n].get();

                    waveCaption = std::format("{:02X} {}", n, fluxStr::extractFilename( ao->getFileName()));

                    if (ao->isPlaying()) waveCaption += " *";

                    if (ImGui::Selectable(waveCaption.c_str(), isSelected)) {
                        //FIXME WE SELECT SOMETHING
                    }

                    if (ImGui::IsItemHovered()) {

                        if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                            //FIXME double click action
                            if (ao->isPlaying()) {
                                ao->stop();

                            } else {
                                ao->play();
                            }
                        }
                    }

                    if (isSelected) {
                        ImGui::SetItemDefaultFocus();

                        if (ImGui::BeginPopupContextItem("##WavePopup")) {
                            ImGui::TextColored(Color4FIm(cl_SkyBlue), "%s", waveCaption.c_str());
                            ImGui::TextDisabled("%s",mAudioStreams[n]->getFileName().c_str());

                            ImGui::Separator();
                            ImGui::EndPopup();
                        }
                    }
                }
                ImGui::EndListBox();
            }
        }
        ImGui::EndChild();
        ImGui::End();
    }

};
