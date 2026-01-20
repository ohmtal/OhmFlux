#include "sequencerGui.h"
#include "sequencerMain.h"
#include <gui/ImFileDialog.h>
#include <imgui_internal.h>
#include <utils/fluxSettingsManager.h>

#include <algorithm>
#include <string>
#include <cctype>



//------------------------------------------------------------------------------
void SequencerGui::ShowDSPWindow(){
    if (!mGuiSettings.mShowDSP) return;
    ImGui::SetNextWindowSize(ImVec2(320, 600), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Digital Sound Processing", &mGuiSettings.mShowDSP))
    {
        ImGui::End();
        return;
    }

    if (ImGui::BeginPopupContextItem())
    {
        if (ImGui::MenuItem("Close"))
            mGuiSettings.mShowDSP = false;
        ImGui::EndPopup();
    }

    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 12.0f);

    RenderWarmthUI();
    RenderBitCrusherUI();
    RenderChorusUI();
    RenderReverbUI();
    RenderEquilizer9BandUI();
    RenderLimiterUI();

    ImGui::PopStyleVar();

    ImGui::End();

}

//------------------------------------------------------------------------------

void SequencerGui::RenderBitCrusherUI() {
    // 1. Use PushID to prevent name collisions with other effects (e.g., if multiple have a "Wet" slider)
    ImGui::PushID("BitCrusher_Effect_Row");

    // 2. Start a Group to treat this whole section as a single unit
    ImGui::BeginGroup();

    static bool isEnabled = getMain()->getController()->getDSPBitCrusher()->isEnabled();

    if (ImGui::Checkbox("##Active", &isEnabled))
        getMain()->getController()->getDSPBitCrusher()->setEnabled(isEnabled);

    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.8f, 0.4f, 0.5f, 1.0f), "BITCRUSHER");

    // 3. Create a Child window for the "Box" look.
    // Width 0 = use parent width. Height 140 is enough for your controls.
    if (isEnabled)
    {
        if (ImGui::BeginChild("BC_Box", ImVec2(0, 110), ImGuiChildFlags_Borders)) {

            static DSP::BitcrusherSettings currentSettings = getMain()->getController()->getDSPBitCrusher()->getSettings();
            static int selectedPreset = -1;
            bool changed = false;



            // Preset Selection
            // ImGui::SameLine();
            const char* presets[] = { "Amiga (8-bit)", "NES (4-bit)", "Phone (Lo-Fi)", "Extreme" };
            ImGui::SetNextItemWidth(150);
            if (ImGui::Combo("##Presets", &selectedPreset, presets, IM_ARRAYSIZE(presets))) {
                switch (selectedPreset) {
                    case 0: currentSettings = DSP::AMIGA_BITCRUSHER;   break;
                    case 1: currentSettings = DSP::NES_BITCRUSHER;     break;
                    case 2: currentSettings = DSP::PHONE_BITCRUSHER;   break;
                    case 3: currentSettings = DSP::EXTREME_BITCRUSHER; break;
                }
                changed = true;
            }

            ImGui::SameLine(ImGui::GetWindowWidth() - 60); // Right-align reset button
            if (ImGui::SmallButton("Reset")) {
                currentSettings = DSP::AMIGA_BITCRUSHER; //DEFAULT
                selectedPreset = 0;
                changed = true;
            }

            ImGui::Separator();

            // Control Sliders
            changed |= ImGui::SliderFloat("Bits", &currentSettings.bits, 1.0f, 16.0f, "%.1f");
            changed |= ImGui::SliderFloat("Rate", &currentSettings.sampleRate, 1000.0f, 44100.0f, "%.0f Hz");
            changed |= ImGui::SliderFloat("Mix", &currentSettings.wet, 0.0f, 1.0f, "%.2f");

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

void SequencerGui::RenderChorusUI() {
    ImGui::PushID("Chorus_Effect_Row");
    ImGui::BeginGroup();

    static bool isEnabled = getMain()->getController()->getDSPChorus()->isEnabled();
    if (ImGui::Checkbox("##Active", &isEnabled))
        getMain()->getController()->getDSPChorus()->setEnabled(isEnabled);
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.6f, 0.4f, 1.0f, 1.0f), "CHORUS / ENSEMBLE");

    if (isEnabled)
    {
        if (ImGui::BeginChild("Chorus_Box", ImVec2(0, 160), ImGuiChildFlags_Borders)) {

            static DSP::ChorusSettings currentSettings = getMain()->getController()->getDSPChorus()->getSettings();
            static int selectedPreset = -1;
            bool changed = false;

            const char* presets[] = { "Lush 80s", "Deep Ensemble", "Fast Leslie", "Juno-60 Style", "Vibrato", "Flanger" };
            ImGui::SetNextItemWidth(150);
            if (ImGui::Combo("##Presets", &selectedPreset, presets, IM_ARRAYSIZE(presets))) {
                switch (selectedPreset) {
                    case 0: currentSettings = DSP::LUSH80s_CHORUS;      break;
                    case 1: currentSettings = DSP::DEEPENSEMPLE_CHORUS; break;
                    case 2: currentSettings = DSP::FASTLESLIE_CHORUS;   break;
                    case 3: currentSettings = DSP::JUNO60_CHORUS;       break;
                    case 4: currentSettings = DSP::VIBRATO_CHORUS;      break;
                    case 5: currentSettings = DSP::FLANGER_CHORUS;      break;
                }
                changed = true;
            }
            ImGui::SameLine(ImGui::GetWindowWidth() - 60);
            if (ImGui::SmallButton("Reset")) {
                currentSettings = DSP::LUSH80s_CHORUS;
                selectedPreset = 0;
                changed = true;
            }

            ImGui::Separator();

            // Parameter Sliders based on your new struct
            changed |= ImGui::SliderFloat("Rate", &currentSettings.rate, 0.1f, 2.5f, "%.2f Hz");
            changed |= ImGui::SliderFloat("Depth", &currentSettings.depth, 0.001f, 0.010f, "%.4f");
            changed |= ImGui::SliderFloat("Delay", &currentSettings.delayBase, 0.001f, 0.040f, "%.3f s");

            // New Stereo Phase Slider
            changed |= ImGui::SliderFloat("Phase", &currentSettings.phaseOffset, 0.0f, 1.0f, "Stereo %.2f");

            changed |= ImGui::SliderFloat("Mix", &currentSettings.wet, 0.0f, 1.0f, "Wet %.2f");

            // Engine Update logic
            if (changed) {
                // If the user manually tweaks a slider, clear the preset label
                if (ImGui::IsItemActive()) selectedPreset = -1;

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

void SequencerGui::RenderReverbUI() {
    ImGui::PushID("Reverb_Effect_Row");
    ImGui::BeginGroup();

    static bool isEnabled = getMain()->getController()->getDSPReverb()->isEnabled();
    if (ImGui::Checkbox("##Active", &isEnabled))
        getMain()->getController()->getDSPReverb()->setEnabled(isEnabled);
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.2f, 0.9f, 0.5f, 1.0f), "REVERB / SPACE");

    // Height set to 180px to fit 4 parameters and preset selection
    if (isEnabled)
    {
        if (ImGui::BeginChild("Reverb_Box", ImVec2(0, 140), ImGuiChildFlags_Borders)) {

            static DSP::ReverbSettings currentSettings = getMain()->getController()->getDSPReverb()->getSettings();
            static int selectedPreset = -1;
            bool changed = false;

            const char* presets[] = { "Concert Hall", "Massive Cave", "Small Room", "Haunted Corridor" };
            ImGui::SetNextItemWidth(150);
            if (ImGui::Combo("##Presets", &selectedPreset, presets, IM_ARRAYSIZE(presets))) {
                switch (selectedPreset) {
                    case 0: currentSettings = DSP::HALL_REVERB;    break;
                    case 1: currentSettings = DSP::CAVE_REVERB;    break;
                    case 2: currentSettings = DSP::ROOM_REVERB;    break;
                    case 3: currentSettings = DSP::HAUNTED_REVERB; break;
                }
                changed = true;
            }

            ImGui::SameLine(ImGui::GetWindowWidth() - 60);
            if (ImGui::SmallButton("Reset")) {
                currentSettings = DSP::ROOM_REVERB;
                selectedPreset = 0;
                changed = true;
            }

            ImGui::Separator();

            // Decay Slider (Float 0.0 to 1.0)
            changed |= ImGui::SliderFloat("Decay", &currentSettings.decay, 0.1f, 0.98f, "%.2f");

            // Size L/R Sliders (Int range based on provided presets)
            // Max range set to 35,000 samples to accommodate the CAVE_REVERB preset
            changed |= ImGui::SliderInt("Size L", &currentSettings.sizeL, 500, 35000, "%d smp");
            changed |= ImGui::SliderInt("Size R", &currentSettings.sizeR, 500, 35000, "%d smp");

            // Wet/Dry Mix Slider
            changed |= ImGui::SliderFloat("Mix", &currentSettings.wet, 0.0f, 1.0f, "Wet %.2f");

            // Engine Update logic
            if (changed) {
                // Clear preset label if user manually moves sliders
                if (ImGui::IsItemActive()) selectedPreset = -1;

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

void SequencerGui::RenderWarmthUI() {
    ImGui::PushID("Warmth_Effect_Row");
    ImGui::BeginGroup();

    static bool isEnabled = getMain()->getController()->getDSPWarmth()->isEnabled();

    if (ImGui::Checkbox("##Active", &isEnabled))
        getMain()->getController()->getDSPWarmth()->setEnabled(isEnabled);
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.4f, 1.0f), "ANALOG WARMTH / SATURATION");

    if (isEnabled)
    {
        if (ImGui::BeginChild("Warmth_Box", ImVec2(0, 115), ImGuiChildFlags_Borders)) {

            static DSP::WarmthSettings currentSettings = getMain()->getController()->getDSPWarmth()->getSettings();
            static int selectedPreset = -1;
            bool changed = false;

            const char* presets[] = { "Gentle Warmth", "Analog Desk", "Tube Amp", "Extreme" };
            ImGui::SetNextItemWidth(150);
            if (ImGui::Combo("##Presets", &selectedPreset, presets, IM_ARRAYSIZE(presets))) {
                switch (selectedPreset) {
                    case 0: currentSettings = DSP::GENTLE_WARMTH;     break;
                    case 1: currentSettings = DSP::ANALOGDESK_WARMTH; break;
                    case 2: currentSettings = DSP::TUBEAMP_WARMTH;    break;
                    case 3: currentSettings = DSP::EXTREME_WARMTH;    break;
                }
                changed = true;
            }

            ImGui::SameLine(ImGui::GetWindowWidth() - 60);
            if (ImGui::SmallButton("Reset")) {
                currentSettings = DSP::TUBEAMP_WARMTH; // Default per 2026 spec
                selectedPreset = 2; // Tube Amp index
                changed = true;
            }


            ImGui::Separator();

            // Parameter Sliders
            // Cutoff (Filter): Lower is "muddier/warmer"
            changed |= ImGui::SliderFloat("Cutoff", &currentSettings.cutoff, 0.0f, 1.0f, "%.2f (Filter)");

            // Drive (Saturation): Range 1.0 to 2.0 per struct definition
            changed |= ImGui::SliderFloat("Drive", &currentSettings.drive, 1.0f, 2.0f, "%.2f (Gain)");

            // Wet/Dry Mix
            changed |= ImGui::SliderFloat("Mix", &currentSettings.wet, 0.0f, 1.0f, "Wet %.2f");

            // Engine Update logic
            if (changed) {
                // Desync preset if manual adjustment occurs
                if (ImGui::IsItemActive()) selectedPreset = -1;

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

void SequencerGui::RenderLimiterUI() {
    ImGui::PushID("Limiter_Effect_Row");
    ImGui::BeginGroup();

    static bool isEnabled = getMain()->getController()->getDSPLimiter()->isEnabled();

    if (ImGui::Checkbox("##Active", &isEnabled))
        getMain()->getController()->getDSPLimiter()->setEnabled(isEnabled);
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "LIMITER");


    ImGui::EndGroup();
    ImGui::PopID();
    ImGui::Spacing();
}
//------------------------------------------------------------------------------
void SequencerGui::RenderEquilizer9BandUI() {
    ImGui::PushID("EQ9_Effect_Row");
    ImGui::BeginGroup();

    auto* eq = getMain()->getController()->getDSPEquilzer9Band();
    bool isEnabled = eq->isEnabled();

    if (ImGui::Checkbox("##Active", &isEnabled))
        eq->setEnabled(isEnabled);



    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.2f, 0.7f, 1.0f, 1.0f), "9-BAND EQUALIZER");


    if (isEnabled) {
        const char* presetNames[] = { "Custom", "Flat", "Bass Boost", "Loudness", "Radio", "Clarity" };
        static int selectedPresetIdx = 1; // Default to "Flat"


        if (ImGui::BeginChild("EQ_Box", ImVec2(0, 180), ImGuiChildFlags_Borders)) {


            ImGui::SetNextItemWidth(150);
            if (ImGui::Combo("##Preset", &selectedPresetIdx, presetNames, IM_ARRAYSIZE(presetNames))) {
                switch (selectedPresetIdx) {
                    case 1: eq->setSettings(DSP::FLAT_EQ);       break;
                    case 2: eq->setSettings(DSP::BASS_BOOST_EQ); break;
                    case 3: eq->setSettings(DSP::SMILE_EQ);      break;
                    case 4: eq->setSettings(DSP::RADIO_EQ);      break;
                    case 5: eq->setSettings(DSP::CLARITY_EQ);    break;
                    default: break; // "Custom" - do nothing, let sliders move
                }
            }

            // Quick Reset Button (Now using the FLAT_EQ preset)
            ImGui::SameLine(ImGui::GetWindowWidth() - 60);
            if (ImGui::SmallButton("Reset")) {
                eq->setSettings(DSP::FLAT_EQ);
                selectedPresetIdx = 1; // Update combo selection to "Flat"
            }
            ImGui::Separator();


            const char* labels[] = { "63", "125", "250", "500", "1k", "2k", "4k", "8k", "16k" };
            const float minGain = -12.0f;
            const float maxGain = 12.0f;

            // Layout constants
            float sliderWidth = 25.f; //35.0f;
            float sliderHeight = 100.f; //150.0f;
            ImVec2 padding = ImVec2(10, 50);

            ImGui::SetCursorPos(padding);

            for (int i = 0; i < 9; i++) {
                ImGui::PushID(i);

                // Get current gain from the DSP engine (you may need a getGain method in your EQ class)
                float currentGain = eq->getGain(i);

                // Draw the vertical slider
                ImGui::BeginGroup();
                if (ImGui::VSliderFloat("##v", ImVec2(sliderWidth, sliderHeight), &currentGain, minGain, maxGain, "")) {
                    eq->setGain(i, currentGain);
                    selectedPresetIdx = 0;
                }

                // Frequency label centered under slider
                float textWidth = ImGui::CalcTextSize(labels[i]).x;
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (sliderWidth - textWidth) * 0.5f);
                ImGui::TextUnformatted(labels[i]);
                ImGui::EndGroup();

                if (i < 8) ImGui::SameLine(0, 12); // Spacing between sliders

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
