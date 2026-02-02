#include "sequencerGui.h"
#include "sequencerMain.h"
#include <imgui_internal.h>

#include <algorithm>
#include <string>
#include <cctype>



//------------------------------------------------------------------------------
void OhmfluxTrackerGui::ShowDSPWindow(){
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
    RenderSoundCardEmuUI();
    RenderWarmthUI();
    RenderChorusUI();
    RenderReverbUI();
    RenderEquilizer9BandUI();
    RenderLimiterUI();

    ImGui::PopStyleVar();

    ImGui::End();

}
//------------------------------------------------------------------------------
void OhmfluxTrackerGui::RenderSoundCardEmuUI() {
    auto* lEmu = getMain()->getController()->getSoundCardEmulation();

    ImGui::PushID("RenderSoundCardEmu_Effect_Row");
    ImGui::BeginGroup();

    bool isEnabled = lEmu->isEnabled();

    if (ImFlux::LEDCheckBox("SOUND RENDERING", &isEnabled, ImVec4(0.2f, 0.7f, 0.5f, 1.0f)))
        lEmu->setEnabled(isEnabled);


    if (isEnabled) {
        if (ImGui::BeginChild("BC_Box", ImVec2(0, 35), ImGuiChildFlags_Borders)) {

            DSP::RenderMode currentMode = lEmu->getMode();

            // Mapping for display names
            const char* modeNames[] = {
                "Blended (Smooth)", "Modern LPF (Warm)",
                "Sound Blaster Pro", "Sound Blaster", "AdLib Gold", "Sound Blaster Clone"
            };

            int lRenderModeInt  = (int)currentMode;

            if (ImFlux::ValueStepper("##Preset", &lRenderModeInt, modeNames, IM_ARRAYSIZE(modeNames))) {
                lEmu->setSettings({ (DSP::RenderMode)lRenderModeInt });
            }
        }
    ImGui::EndChild();
    } else {
        ImGui::Separator();
    }

    ImGui::EndGroup();
    ImGui::PopID();
    ImGui::Spacing(); // Add visual gap before the next effect
}
//------------------------------------------------------------------------------
void OhmfluxTrackerGui::RenderBitCrusherUI() {
    // 1. Use PushID to prevent name collisions with other effects (e.g., if multiple have a "Wet" slider)
    ImGui::PushID("BitCrusher_Effect_Row");

    // 2. Start a Group to treat this whole section as a single unit
    ImGui::BeginGroup();

    bool isEnabled = getMain()->getController()->getDSPBitCrusher()->isEnabled();

    // if (ImGui::Checkbox("##Active", &isEnabled))
    //     getMain()->getController()->getDSPBitCrusher()->setEnabled(isEnabled);
    //
    // ImGui::SameLine();
    // ImGui::TextColored(ImVec4(0.8f, 0.4f, 0.5f, 1.0f), "BITCRUSHER");

    if (ImFlux::LEDCheckBox("BITCRUSHER", &isEnabled, ImVec4(0.8f, 0.4f, 0.5f, 1.0f)))
        getMain()->getController()->getDSPBitCrusher()->setEnabled(isEnabled);


    // 3. Create a Child window for the "Box" look.
    // Width 0 = use parent width. Height 140 is enough for your controls.
    if (isEnabled)
    {
        if (ImGui::BeginChild("BC_Box", ImVec2(0, 110), ImGuiChildFlags_Borders)) {

            DSP::BitcrusherSettings currentSettings = getMain()->getController()->getDSPBitCrusher()->getSettings();
            bool changed = false;

            int currentIdx = 0; // Standard: "Custom"

            for (int i = 1; i < DSP::BITCRUSHER_PRESETS.size(); ++i) {
                if (currentSettings == DSP::BITCRUSHER_PRESETS[i]) {
                    currentIdx = i;
                    break;
                }
            }
            int displayIdx = currentIdx;  //<< keep currentIdx clean


            // Preset Selection
            // ImGui::SameLine();
            const char* presetNames[] = { "Custom", "Amiga (8-bit)", "NES (4-bit)", "Phone (Lo-Fi)", "Extreme" };
            ImGui::SetNextItemWidth(150);
            if (ImFlux::ValueStepper("##Preset", &displayIdx, presetNames, IM_ARRAYSIZE(presetNames))) {
                if (displayIdx > 0 && displayIdx < DSP::BITCRUSHER_PRESETS.size()) {
                    currentSettings =  DSP::BITCRUSHER_PRESETS[displayIdx];
                    changed = true;
                }
            }
            ImGui::SameLine(ImGui::GetWindowWidth() - 60); // Right-align reset button

            if (ImFlux::FaderButton("Reset", ImVec2(40.f, 20.f)))  {
                currentSettings = DSP::AMIGA_BITCRUSHER; //DEFAULT
                changed = true;
            }
            ImGui::Separator();

            // Control Sliders
            changed |= ImFlux::FaderHWithText("Bits", &currentSettings.bits, 1.0f, 16.0f, "%.1f");
            changed |= ImFlux::FaderHWithText("Rate", &currentSettings.sampleRate, 1000.0f, 44100.0f, "%.0f Hz");
            changed |= ImFlux::FaderHWithText("Mix", &currentSettings.wet, 0.0f, 1.0f, "%.2f");

            // Engine Update
            if (changed) {
                if (isEnabled) {
                    getMain()->getController()->getDSPBitCrusher()->setSettings(currentSettings);
                }
            }

        }
        ImGui::EndChild();
    } else {
        ImGui::Separator();
    }

    ImGui::EndGroup();
    ImGui::PopID();
    ImGui::Spacing(); // Add visual gap before the next effect
}
//------------------------------------------------------------------------------

void OhmfluxTrackerGui::RenderChorusUI() {
    ImGui::PushID("Chorus_Effect_Row");
    ImGui::BeginGroup();

    bool isEnabled = getMain()->getController()->getDSPChorus()->isEnabled();
    // if (ImGui::Checkbox("##Active", &isEnabled))
    //     getMain()->getController()->getDSPChorus()->setEnabled(isEnabled);
    // ImGui::SameLine();
    // ImGui::TextColored(ImVec4(0.6f, 0.4f, 1.0f, 1.0f), "CHORUS / ENSEMBLE");

    if (ImFlux::LEDCheckBox("CHORUS / ENSEMBLE", &isEnabled, ImVec4(0.6f, 0.4f, 1.0f, 1.0f)))
        getMain()->getController()->getDSPChorus()->setEnabled(isEnabled);


    if (isEnabled)
    {
        if (ImGui::BeginChild("Chorus_Box", ImVec2(0, 160), ImGuiChildFlags_Borders)) {

            DSP::ChorusSettings currentSettings = getMain()->getController()->getDSPChorus()->getSettings();
            bool changed = false;

            const char* presets[] = {"Custom", "Lush 80s", "Deep Ensemble", "Fast Leslie", "Juno-60 Style", "Vibrato", "Flanger" };

            int currentIdx = 0; // Standard: "Custom"

            for (int i = 1; i < DSP::CHROUS_PRESETS.size(); ++i) {
                if (currentSettings == DSP::CHROUS_PRESETS[i]) {
                    currentIdx = i;
                    break;
                }
            }
            int displayIdx = currentIdx;  //<< keep currentIdx clean

            ImGui::SetNextItemWidth(150);
            if (ImFlux::ValueStepper("##Preset", &displayIdx, presets, IM_ARRAYSIZE(presets))) {
                if (displayIdx > 0 && displayIdx < DSP::CHROUS_PRESETS.size()) {
                    currentSettings =  DSP::CHROUS_PRESETS[displayIdx];
                    changed = true;
                }
            }
            ImGui::SameLine(ImGui::GetWindowWidth() - 60);

            if (ImFlux::FaderButton("Reset", ImVec2(40.f, 20.f)))  {
                currentSettings = DSP::LUSH80s_CHORUS;
                changed = true;
            }


            ImGui::Separator();

            // Parameter Sliders
            changed |= ImFlux::FaderHWithText("Rate",  &currentSettings.rate, 0.1f, 2.5f, "%.2f Hz");
            changed |= ImFlux::FaderHWithText("Depth", &currentSettings.depth, 0.001f, 0.010f, "%.4f");
            changed |= ImFlux::FaderHWithText("Delay", &currentSettings.delayBase, 0.001f, 0.040f, "%.3f s");
            changed |= ImFlux::FaderHWithText("Phase", &currentSettings.phaseOffset, 0.0f, 1.0f, "Stereo %.2f");
            changed |= ImFlux::FaderHWithText("Mix",   &currentSettings.wet, 0.0f, 1.0f, "Wet %.2f");

            // Engine Update logic
            if (changed) {
                if (isEnabled) {
                    getMain()->getController()->getDSPChorus()->setSettings(currentSettings);
                }
            }
        }
        ImGui::EndChild();
    } else {
        ImGui::Separator();
    }
    ImGui::EndGroup();
    ImGui::PopID();
    ImGui::Spacing();
}
//------------------------------------------------------------------------------

void OhmfluxTrackerGui::RenderReverbUI() {
    ImGui::PushID("Reverb_Effect_Row");
    ImGui::BeginGroup();

    bool isEnabled = getMain()->getController()->getDSPReverb()->isEnabled();
    // if (ImGui::Checkbox("##Active", &isEnabled))
    //     getMain()->getController()->getDSPReverb()->setEnabled(isEnabled);
    // ImGui::SameLine();
    // ImGui::TextColored(ImVec4(0.2f, 0.9f, 0.5f, 1.0f), "REVERB / SPACE");

    if (ImFlux::LEDCheckBox("REVERB / SPACE", &isEnabled, ImVec4(0.2f, 0.9f, 0.5f, 1.0f)))
        getMain()->getController()->getDSPReverb()->setEnabled(isEnabled);


    // Height set to 180px to fit 4 parameters and preset selection
    if (isEnabled)
    {
        if (ImGui::BeginChild("Reverb_Box", ImVec2(0, 140), ImGuiChildFlags_Borders)) {

            DSP::ReverbSettings currentSettings = getMain()->getController()->getDSPReverb()->getSettings();
            bool changed = false;

            const char* presets[] = { "Custom", "Concert Hall", "Massive Cave", "Small Room", "Haunted Corridor" };

            int currentIdx = 0; // Standard: "Custom"

            for (int i = 1; i < DSP::REVERB_PRESETS.size(); ++i) {
                if (currentSettings == DSP::REVERB_PRESETS[i]) {
                    currentIdx = i;
                    break;
                }
            }
            int displayIdx = currentIdx;  //<< keep currentIdx clean

            ImGui::SetNextItemWidth(150);
            if (ImFlux::ValueStepper("##Preset", &displayIdx, presets, IM_ARRAYSIZE(presets))) {
                if (displayIdx > 0 && displayIdx < DSP::REVERB_PRESETS.size()) {
                    currentSettings =  DSP::REVERB_PRESETS[displayIdx];
                    changed = true;
                }
            }

            ImGui::SameLine(ImGui::GetWindowWidth() - 60);
            // if (ImGui::SmallButton("Reset")) {
            //     currentSettings = DSP::ROOM_REVERB;
            //     selectedPreset = 2;
            //     changed = true;
            // }
            if (ImFlux::FaderButton("Reset", ImVec2(40.f, 20.f)))  {
                currentSettings = DSP::ROOM_REVERB;
                changed = true;
            }


            ImGui::Separator();

            // ImFlux::FaderHWithText

            // Decay Slider (Float 0.0 to 1.0)
            changed |= ImFlux::FaderHWithText("Decay", &currentSettings.decay, 0.1f, 0.98f, "%.2f");

            // Size L/R Sliders (Int range based on provided presets)
            // Max range set to 35,000 samples to accommodate the CAVE_REVERB preset

            float sizeL = (float) currentSettings.sizeL;
            float sizeR = (float) currentSettings.sizeR;

            changed |= ImFlux::FaderHWithText("Size L", &sizeL, 500, 35000, "%5.0f smp");
            changed |= ImFlux::FaderHWithText("Size R", &sizeR, 500, 35000, "%5.0f smp");

            currentSettings.sizeL = (int) sizeL;
            currentSettings.sizeR = (int) sizeR;

            // Wet/Dry Mix Slider
            changed |= ImFlux::FaderHWithText("Mix", &currentSettings.wet, 0.0f, 1.0f, "Wet %.2f");

            // Engine Update logic
            if (changed) {
                if (isEnabled) {
                    getMain()->getController()->getDSPReverb()->setSettings(currentSettings);
                }
            }
        }
        ImGui::EndChild();
    } else {
        ImGui::Separator();
    }

    ImGui::EndGroup();
    ImGui::PopID();
    ImGui::Spacing();
}
//------------------------------------------------------------------------------

void OhmfluxTrackerGui::RenderWarmthUI() {
    ImGui::PushID("Warmth_Effect_Row");
    ImGui::BeginGroup();

    bool isEnabled = getMain()->getController()->getDSPWarmth()->isEnabled();

    // if (ImGui::Checkbox("##Active", &isEnabled))
    //     getMain()->getController()->getDSPWarmth()->setEnabled(isEnabled);
    // ImGui::SameLine();
    // ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.4f, 1.0f), "ANALOG WARMTH / SATURATION");

    if (ImFlux::LEDCheckBox("ANALOG WARMTH / SATURATION", &isEnabled, ImVec4(1.0f, 0.6f, 0.4f, 1.0f)))
        getMain()->getController()->getDSPWarmth()->setEnabled(isEnabled);

    if (isEnabled)
    {
        if (ImGui::BeginChild("Warmth_Box", ImVec2(0, 115), ImGuiChildFlags_Borders)) {

            DSP::WarmthSettings currentSettings = getMain()->getController()->getDSPWarmth()->getSettings();
            bool changed = false;

            const char* presets[] = { "Custom", "Gentle Warmth", "Analog Desk", "Tube Amp", "Extreme" };

            int currentIdx = 0; // Standard: "Custom"

            for (int i = 1; i < DSP::WARMTH_PRESETS.size(); ++i) {
                if (currentSettings == DSP::WARMTH_PRESETS[i]) {
                    currentIdx = i;
                    break;
                }
            }
            int displayIdx = currentIdx;  //<< keep currentIdx clean

            ImGui::SetNextItemWidth(150);
            if (ImFlux::ValueStepper("##Preset", &displayIdx, presets, IM_ARRAYSIZE(presets))) {
                if (displayIdx > 0 && displayIdx < DSP::WARMTH_PRESETS.size()) {
                    currentSettings =  DSP::WARMTH_PRESETS[displayIdx];
                    changed = true;
                }
            }


            // ImGui::SetNextItemWidth(150);
            // if (ImGui::Combo("##Presets", &selectedPreset, presets, IM_ARRAYSIZE(presets))) {
            //     switch (selectedPreset) {
            //         case 0: currentSettings = DSP::GENTLE_WARMTH;     break;
            //         case 1: currentSettings = DSP::ANALOGDESK_WARMTH; break;
            //         case 2: currentSettings = DSP::TUBEAMP_WARMTH;    break;
            //         case 3: currentSettings = DSP::EXTREME_WARMTH;    break;
            //     }
            //     changed = true;
            // }

            ImGui::SameLine(ImGui::GetWindowWidth() - 60);

            if (ImFlux::FaderButton("Reset", ImVec2(40.f, 20.f)))  {
                currentSettings = DSP::GENTLE_WARMTH;
                changed = true;
            }


            ImGui::Separator();

            // Parameter Sliders
            // Cutoff (Filter): Lower is "muddier/warmer"
            changed |= ImFlux::FaderHWithText("Cutoff", &currentSettings.cutoff, 0.0f, 1.0f, "%.2f (Filter)");

            // Drive (Saturation): Range 1.0 to 2.0 per struct definition
            changed |= ImFlux::FaderHWithText("Drive", &currentSettings.drive, 1.0f, 2.0f, "%.2f (Gain)");

            // Wet/Dry Mix
            changed |= ImFlux::FaderHWithText("Mix", &currentSettings.wet, 0.0f, 1.0f, "Wet %.2f");

            // Engine Update logic
            if (changed) {
                if (isEnabled) {
                    getMain()->getController()->getDSPWarmth()->setSettings(currentSettings);
                }
            }


        }
        ImGui::EndChild();
    } else {
        ImGui::Separator();
    }

    ImGui::EndGroup();
    ImGui::PopID();
    ImGui::Spacing();
}

//------------------------------------------------------------------------------

void OhmfluxTrackerGui::RenderLimiterUI() {
    ImGui::PushID("Limiter_Effect_Row");
    ImGui::BeginGroup();

    auto* lim = getMain()->getController()->getDSPLimiter();
    bool isEnabled = lim->isEnabled();

    if (ImFlux::LEDCheckBox("LIMITER", &isEnabled, ImVec4(1.0f, 0.4f, 0.4f, 1.0f)))
        getMain()->getController()->getDSPLimiter()->setEnabled(isEnabled);

    if (lim->isEnabled()) {
        const char* presetNames[] = { "CUSTOM", "DEFAULT", "EIGHTY", "FIFTY", "LOWVOL", "EXTREM" };
        bool changed = false;
        DSP::LimiterSettings& currentSettings = lim->getSettings();

        int currentIdx = 0; // Standard: "Custom"

        for (int i = 1; i < DSP::LIMITER_PRESETS.size(); ++i) {
            if (currentSettings == DSP::LIMITER_PRESETS[i]) {
                currentIdx = i;
                break;
            }
        }
        int displayIdx = currentIdx;  //<< keep currentIdx clean

        if (ImGui::BeginChild("EQ_Box", ImVec2(0, 35), ImGuiChildFlags_Borders)) {

            ImGui::SetNextItemWidth(150);

            if (ImFlux::ValueStepper("##Preset", &displayIdx, presetNames, IM_ARRAYSIZE(presetNames))) {
                if (displayIdx > 0 && displayIdx < DSP::LIMITER_PRESETS.size()) {
                            lim->setSettings(DSP::LIMITER_PRESETS[displayIdx]);
                }
            }

            // Quick Reset Button (Now using the FLAT_EQ preset)
            ImGui::SameLine(ImGui::GetWindowWidth() - 60);

            // if (ImGui::SmallButton("Reset")) {
            if (ImFlux::FaderButton("Reset", ImVec2(40.f, 20.f)))  {
                lim->setSettings(DSP::LIMITER_DEFAULT);
            }
            // ImGui::Separator();
            // changed |= ImFlux::FaderHWithText("Threshold", &currentSettings.Threshold, 0.01f, 1.f, "%.3f");
            // changed |= ImFlux::FaderHWithText("Depth", &currentSettings.Attack, 0.01f, 1.f, "%.4f");
            // changed |= ImFlux::FaderHWithText("Release", &currentSettings.Release, 0.0000005f, 0.0001f, "%.8f");
            //
            //
            // if (changed) {
            //     selectedPresetIdx = 0;
            //     lim->setSettings(currentSettings);
            // }

        } //box
        ImGui::EndChild();
    } //enabled

    ImGui::EndGroup();
    ImGui::PopID();
    ImGui::Spacing();
}
//------------------------------------------------------------------------------
void OhmfluxTrackerGui::RenderEquilizer9BandUI() {
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


        // ImGui::SetNextItemWidth(150);
        //
        //     // if (ImGui::Combo("##Preset", &selectedPresetIdx, presetNames, IM_ARRAYSIZE(presetNames))) {
        //     if (ImFlux::ValueStepper("##Preset", &selectedPresetIdx, presetNames, IM_ARRAYSIZE(presetNames))) {
        //             switch (selectedPresetIdx) {
        //             case 1: eq->setSettings(DSP::FLAT_EQ);       break;
        //             case 2: eq->setSettings(DSP::BASS_BOOST_EQ); break;
        //             case 3: eq->setSettings(DSP::SMILE_EQ);      break;
        //             case 4: eq->setSettings(DSP::RADIO_EQ);      break;
        //             case 5: eq->setSettings(DSP::CLARITY_EQ);    break;
        //             default: break; // "Custom" - do nothing, let sliders move
        //         }
        //     }

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

            // Layout constants
            // float sliderWidth = 25.f; //35.0f;
            // float sliderHeight = 100.f; //150.0f;
            // ImVec2 padding = ImVec2(10, 50);
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
    ImGui::Spacing();
}
//------------------------------------------------------------------------------
void OhmfluxTrackerGui::RenderSpectrumAnalyzer() {
    ImGui::PushID("SpectrumAnalyzer_Effect_Row");
    ImGui::BeginGroup();

    bool isEnabled = mSpectrumAnalyzer->isEnabled();
    // if (ImGui::Checkbox("##Active", &isEnabled))
    //     mSpectrumAnalyzer->setEnabled(isEnabled);
    //
    // ImGui::SameLine();
    // ImGui::TextColored(ImVec4(1.0f, 1.4f, 0.4f, 1.0f), "SPECTRUM ANALYSER");

    if (ImFlux::LEDCheckBox("SPECTRUM ANALYSER", &isEnabled, ImVec4(1.0f, 1.4f, 0.4f, 1.0f)))
        mSpectrumAnalyzer->setEnabled(isEnabled);



    float fullWidth = ImGui::GetContentRegionAvail().x;

    DSP::DrawSpectrumAnalyzer(mSpectrumAnalyzer, ImVec2(fullWidth, 80.0f));

    ImGui::EndGroup();
    ImGui::PopID();
    ImGui::Spacing();
}
