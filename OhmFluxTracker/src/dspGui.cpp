#include "otGui.h"
#include "otMain.h"
#include <imgui_internal.h>

#include <algorithm>
#include <string>
#include <cctype>



//------------------------------------------------------------------------------
void OTGui::ShowDSPWindow(){
    if (!mSettings.ShowDSP) return;
    ImGui::SetNextWindowSize(ImVec2(320, 600), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Digital Sound Processing", &mSettings.ShowDSP))
    {
        ImGui::End();
        return;
    }

    if (ImGui::BeginPopupContextItem())
    {
        if (ImGui::MenuItem("Close"))
            mSettings.ShowDSP = false;
        ImGui::EndPopup();
    }

    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 12.0f);

    // Sound Renderer Combo box
    RenderSpectrumAnalyzer();
    RenderBitCrusherUI();
    RenderWarmthUI();
    RenderChorusUI();
    RenderReverbUI();
    RenderEquilizer9BandUI();
    RenderLimiterUI();

    ImGui::PopStyleVar();

    ImGui::End();

}
//------------------------------------------------------------------------------
void OTGui::RenderBitCrusherUI() {

    // FIXME HAPPY MOVING ALL OTHERER !!!!!
    getMain()->getController()->getDSPBitCrusher()->renderUI();

    // // 1. Use PushID to prevent name collisions with other effects (e.g., if multiple have a "Wet" slider)
    // ImGui::PushID("BitCrusher_Effect_Row");
    //
    // // 2. Start a Group to treat this whole section as a single unit
    // ImGui::BeginGroup();
    //
    // bool isEnabled = getMain()->getController()->getDSPBitCrusher()->isEnabled();
    //
    // // if (ImGui::Checkbox("##Active", &isEnabled))
    // //     getMain()->getController()->getDSPBitCrusher()->setEnabled(isEnabled);
    // //
    // // ImGui::SameLine();
    // // ImGui::TextColored(ImVec4(0.8f, 0.4f, 0.5f, 1.0f), "BITCRUSHER");
    //
    // if (ImFlux::LEDCheckBox("BITCRUSHER", &isEnabled, ImVec4(0.8f, 0.4f, 0.5f, 1.0f)))
    //     getMain()->getController()->getDSPBitCrusher()->setEnabled(isEnabled);
    //
    //
    // // 3. Create a Child window for the "Box" look.
    // // Width 0 = use parent width. Height 140 is enough for your controls.
    // if (isEnabled)
    // {
    //     if (ImGui::BeginChild("BC_Box", ImVec2(0, 110), ImGuiChildFlags_Borders)) {
    //
    //         DSP::BitcrusherSettings currentSettings = getMain()->getController()->getDSPBitCrusher()->getSettings();
    //         bool changed = false;
    //
    //         int currentIdx = 0; // Standard: "Custom"
    //
    //         for (int i = 1; i < DSP::BITCRUSHER_PRESETS.size(); ++i) {
    //             if (currentSettings == DSP::BITCRUSHER_PRESETS[i]) {
    //                 currentIdx = i;
    //                 break;
    //             }
    //         }
    //         int displayIdx = currentIdx;  //<< keep currentIdx clean
    //
    //
    //         // Preset Selection
    //         // ImGui::SameLine();
    //         const char* presetNames[] = { "Custom", "Amiga (8-bit)", "NES (4-bit)", "Phone (Lo-Fi)", "Extreme" };
    //         ImGui::SetNextItemWidth(150);
    //         if (ImFlux::ValueStepper("##Preset", &displayIdx, presetNames, IM_ARRAYSIZE(presetNames))) {
    //             if (displayIdx > 0 && displayIdx < DSP::BITCRUSHER_PRESETS.size()) {
    //                 currentSettings =  DSP::BITCRUSHER_PRESETS[displayIdx];
    //                 changed = true;
    //             }
    //         }
    //         ImGui::SameLine(ImGui::GetWindowWidth() - 60); // Right-align reset button
    //
    //         if (ImFlux::FaderButton("Reset", ImVec2(40.f, 20.f)))  {
    //             currentSettings = DSP::AMIGA_BITCRUSHER; //DEFAULT
    //             changed = true;
    //         }
    //         ImGui::Separator();
    //
    //         // Control Sliders
    //         changed |= ImFlux::FaderHWithText("Bits", &currentSettings.bits, 1.0f, 16.0f, "%.1f");
    //         changed |= ImFlux::FaderHWithText("Rate", &currentSettings.sampleRate, 1000.0f, 44100.0f, "%.0f Hz");
    //         changed |= ImFlux::FaderHWithText("Mix", &currentSettings.wet, 0.0f, 1.0f, "%.2f");
    //
    //         // Engine Update
    //         if (changed) {
    //             if (isEnabled) {
    //                 getMain()->getController()->getDSPBitCrusher()->setSettings(currentSettings);
    //             }
    //         }
    //
    //     }
    //     ImGui::EndChild();
    // } else {
    //     ImGui::Separator();
    // }
    //
    // ImGui::EndGroup();
    // ImGui::PopID();
    // ImGui::Spacing(); // Add visual gap before the next effect
}
//------------------------------------------------------------------------------

void OTGui::RenderChorusUI() {

    getMain()->getController()->getDSPChorus()->renderUI();
}
//------------------------------------------------------------------------------

void OTGui::RenderReverbUI() {

    getMain()->getController()->getDSPReverb()->renderUI();

}
//------------------------------------------------------------------------------

void OTGui::RenderWarmthUI() {
    getMain()->getController()->getDSPWarmth()->renderUI();

}

//------------------------------------------------------------------------------

void OTGui::RenderLimiterUI() {

    getMain()->getController()->getDSPLimiter()->renderUI();

}
//------------------------------------------------------------------------------
void OTGui::RenderEquilizer9BandUI() {
    auto* eq = getMain()->getController()->getDSPEquilzer9Band();
    eq->renderUI();
/*
    ImGui::PushID("EQ9_Effect_Row");
    ImGui::BeginGroup();

    auto* eq = getMain()->getController()->getDSPEquilzer9Band();

    bool isEnabled = eq->isEnabled();
    // if (ImGui::Checkbox("##Active", &isEnabled))
    //     eq->setEnabled(isEnabled);
    //
    // ImGui::SameLine();
    // ImGui::TextColored(ImVec4(0.2f, 0.7f, 1.0f, 1.0f), "9-BAND EQUALIZER");
    if (ImFlux::LEDCheckBox("9-BAND EQUALIZER", &isEnabled, ImVec4(0.2f, 0.7f, 1.0f, 1.0f)))
        eq->setEnabled(isEnabled);



    if (eq->isEnabled()) {
        const char* presetNames[] = { "Custom", "Flat", "Bass Boost", "Loudness", "Radio", "Clarity" };


        if (ImGui::BeginChild("EQ_Box", ImVec2(0, 180), ImGuiChildFlags_Borders)) {

            int currentIdx = 0; // Standard: "Custom"

            DSP::Equalizer9BandSettings currentSettings = eq->getSettings();

            for (int i = 1; i < DSP::EQ9BAND_PRESETS.size(); ++i) {
                if (currentSettings == DSP::EQ9BAND_PRESETS[i]) {
                    currentIdx = i;
                    break;
                }
            }
            int displayIdx = currentIdx;  //<< keep currentIdx clean

            ImGui::SetNextItemWidth(150);
            if (ImFlux::ValueStepper("##Preset", &displayIdx, presetNames, IM_ARRAYSIZE(presetNames))) {
                if (displayIdx > 0 && displayIdx < DSP::EQ9BAND_PRESETS.size()) {
                    currentSettings =  DSP::EQ9BAND_PRESETS[displayIdx];
                    eq->setSettings(currentSettings);
                }
            }

            // Quick Reset Button (Now using the FLAT_EQ preset)
            ImGui::SameLine(ImGui::GetWindowWidth() - 60);

            // if (ImGui::SmallButton("Reset")) {
            if (ImFlux::FaderButton("Reset", ImVec2(40.f, 20.f)))  {
                eq->setSettings(DSP::FLAT_EQ);
            }
            ImGui::Separator();


            const char* labels[] = { "63", "125", "250", "500", "1k", "2k", "4k", "8k", "16k" };
            const float minGain = -12.0f;
            const float maxGain = 12.0f;

            float sliderWidth = 20.f; //35.0f;
            float sliderHeight = 80.f; //150.0f;
            float sliderSpaceing = 12.f ; //12.f;
            ImVec2 padding = ImVec2(10, 50);


            ImGui::SetCursorPos(padding);

            for (int i = 0; i < 9; i++) {
                ImGui::PushID(i);

                // Get current gain from the DSP engine (you may need a getGain method in your EQ class)
                float currentGain = eq->getGain(i);

                // Draw the vertical slider
                ImGui::BeginGroup();


                // if (ImGui::VSliderFloat("##v", ImVec2(sliderWidth, sliderHeight), &currentGain, minGain, maxGain, "")) {
                if (ImFlux::FaderVertical("##v", ImVec2(sliderWidth, sliderHeight), &currentGain, minGain, maxGain)) {
                    eq->setGain(i, currentGain);
                }

                // Frequency label centered under slider
                float textWidth = ImGui::CalcTextSize(labels[i]).x;
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (sliderWidth - textWidth) * 0.5f);
                ImGui::TextUnformatted(labels[i]);
                ImGui::EndGroup();

                if (i < 8) ImGui::SameLine(0, sliderSpaceing); // Spacing between sliders

                ImGui::PopID();
            }
        }
        ImGui::EndChild();
    } else {
        ImGui::Separator();
    }


    ImGui::EndGroup();
    ImGui::PopID();
    ImGui::Spacing();*/
}
//------------------------------------------------------------------------------
void OTGui::RenderSpectrumAnalyzer() {
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
