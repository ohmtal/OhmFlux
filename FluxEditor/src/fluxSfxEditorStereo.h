//-----------------------------------------------------------------------------
// Copyright (c) 2026 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#pragma once

#include <core/fluxBaseObject.h>
#include <imgui.h>
#include <SFXGeneratorStereo.h>
#include <gui/ImFlux.h>
#include <DSP.h>
#include <algorithm>

#ifdef __EMSCRIPTEN__
#ifdef __cplusplus
extern "C" {
#endif
    // Forward declaration
    void emscripten_trigger_download(const char* name);
#ifdef __cplusplus
}
#endif
#endif


class FluxSfxEditorStereo : public FluxBaseObject
{
public:
    enum SFXGEN_FILE_ACTION_TYPE :int {
        fa_none     = 0,
        fa_load     = 1,
        fa_save     = 2,
        fa_export   = 3
    };

private:
    SFXGeneratorStereo* mSFXGeneratorStereo = nullptr;

    struct FileDialogContext {
        SFXGeneratorStereo* generator;
        SFXGEN_FILE_ACTION_TYPE action;
        SDL_DialogFileFilter filters[2];
        char* basePath;
        char* filterStrings[3];
    };

    DSP::VisualAnalyzer* mVisualAnalyzer;
    // not DSP::Bitcrusher* mBitCrusher;

public:


    ~FluxSfxEditorStereo() { Deinitialize(); }

    SFXGeneratorStereo* getSFXGeneratorStereo() { return mSFXGeneratorStereo; }

    // void Execute() override;
    bool Initialize() override {
            mSFXGeneratorStereo = new SFXGeneratorStereo();
            if (!mSFXGeneratorStereo->initSDLAudio())
                return false;

            auto specAna = std::make_unique<DSP::VisualAnalyzer>(true);
            mVisualAnalyzer = specAna.get();
            mSFXGeneratorStereo->getDspEffects().push_back(std::move(specAna));

            // auto bitcrusher = std::make_unique<DSP::Bitcrusher>(false);
            // mBitCrusher = bitcrusher.get();
            // mSFXGeneratorStereo->getDspEffects().push_back(std::move(bitcrusher));


            return true;
    };
    void Deinitialize() override {
        SAFE_DELETE(mSFXGeneratorStereo);
    }


    void DrawGui()  {
        if (!mSFXGeneratorStereo)
            return;
        DrawSFXEditor();

    }
    //--------------------------------------------------------------------------

    //--------------------------------------------------------------------------
    bool SFXButton(std::string label, ImVec2 size, bool isSelected = false)
    {
        ImFlux::ButtonParams bp = ImFlux::SLATEDARK_BUTTON.WithSize(size);
        bp.selected = isSelected;
        if (isSelected) bp.color = ImFlux::ModifyRGB(bp.color, 2.f);
        return ImFlux::ButtonFancy(label, bp);
    }

    //--------------------------------------------------------------------------
    void DrawSFXEditor()
    {
        if (!mSFXGeneratorStereo)
            return;

        SFXGeneratorStereo::SFXParams& lParams = mSFXGeneratorStereo->mParams;
        SFXGeneratorStereo* lSfxGen = mSFXGeneratorStereo;

        // Persistent state for the Auto Play toggle
        static bool lAutoPlay = true;




        // Helper lambda to handle generation + auto-play logic
        auto TriggerGen = [&](std::function<void()> genFunc) {
            genFunc();
            if (lAutoPlay) lSfxGen->PlaySample();
        };
        // Helper to match your exact old parameters
        auto SFXKnob = [&](const char* label, float& value, bool is_signed) {
            float min_v = is_signed ? -1.0f : 0.0f;
            char format[64];
            snprintf(format, 64, "%s: %%.3f", label);
            return ImFlux::MiniKnobF(label, &value, min_v, 1.0f);
        };



        // minimum size
        ImGui::SetNextWindowSizeConstraints(ImVec2(600.0f, 650.f), ImVec2(FLT_MAX, FLT_MAX));

        ImGui::Begin("Sound Effects Generator Stereo");

        if (ImGui::BeginTable("EditorColumns", 3,
            ImGuiTableFlags_BordersInnerV
            // | ImGuiTableFlags_Resizable
            | ImGuiTableFlags_SizingStretchProp, // Better for mixed column types
            ImVec2(-FLT_MIN, 0.0f)))
        {
            // 3. Use WidthFixed for sides, WidthStretch for middle
            ImGui::TableSetupColumn("Presets", ImGuiTableColumnFlags_WidthFixed, 130.f);
            ImGui::TableSetupColumn("Sliders", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Actions", ImGuiTableColumnFlags_WidthFixed, 130.f);

            ImGui::TableNextRow();

            // --- COLUMN 1: GENERATORS (Left) ---
            ImGui::TableSetColumnIndex(0);

            // ImVec2 lButtonSize = ImVec2(120.f, 40.f);
            ImVec2 lButtonSize = ImVec2(-FLT_MIN, 40);



            if (SFXButton("Pickup/Coin", lButtonSize)) TriggerGen([&]{ lSfxGen->GeneratePickupCoin(); });
            if (SFXButton("Laser/Shoot", lButtonSize)) TriggerGen([&]{ lSfxGen->GenerateLaserShoot(); });
            if (SFXButton("Explosion",   lButtonSize)) TriggerGen([&]{ lSfxGen->GenerateExplosion(); });
            if (SFXButton("Powerup",     lButtonSize)) TriggerGen([&]{ lSfxGen->GeneratePowerup(); });
            if (SFXButton("Hit/Hurt",    lButtonSize)) TriggerGen([&]{ lSfxGen->GenerateHitHurt(); });
            if (SFXButton("Jump",        lButtonSize)) TriggerGen([&]{ lSfxGen->GenerateJump(); });
            if (SFXButton("Blip/Select", lButtonSize)) TriggerGen([&]{ lSfxGen->GenerateBlipSelect(); });

            ImFlux::SeparatorFancy();

            if (SFXButton("Randomize",   lButtonSize)) TriggerGen([&]{ lSfxGen->Randomize(); });
            ImFlux::Hint("Randomize Sample [F1]");
            if (SFXButton("Mutate",      lButtonSize)) TriggerGen([&]{ lSfxGen->Mutate(); });
            ImFlux::Hint("Mutate Sample [F2]");
            if (SFXButton("Panning Mutate",  lButtonSize)) TriggerGen([&]{ lSfxGen->AddPanning(true); });


            // --- COLUMN 2: PARAMETERS (Middle) ---
            ImGui::TableSetColumnIndex(1);

            // Force column stability (as we found in step #3)
            ImGui::Dummy(ImVec2(300.0f, 0.0f));

            // toggle buttons:
            // --- WAVE TYPE SELECTION (Top of Middle Column) ---

            // 1. Calculate the total width available for the row once
            float total_width = ImGui::GetContentRegionAvail().x;
            // 2. Subtract the spacing between buttons (default is Style.ItemSpacing.x)
            float spacing = ImGui::GetStyle().ItemSpacing.x;
            // 3. Divide by 4 (number of buttons)
            float button_width = (total_width - (spacing * 3)) / 4.0f;

            auto WaveButton = [&](const char* label, int type) {
                if (mSFXGeneratorStereo->DrawWaveButton(label,type)
                    &&   lAutoPlay
                ) lSfxGen->PlaySample();


            };


            if (ImGui::BeginChild("WAVE_TYPE_BOX", ImVec2(-FLT_MIN,70.f) )) {
                ImFlux::GradientBox(ImVec2(-FLT_MIN, -FLT_MIN),0.f);
                ImGui::Dummy(ImVec2(2,0)); ImGui::SameLine();
                ImGui::BeginGroup();

                ImFlux::ShadowText("WAVE TYPE", ImFlux::COL32_NEONCYAN);
                ImGui::Separator();
                WaveButton("SQUARE",   0); ImGui::SameLine();
                WaveButton("SAWTOOTH", 1); ImGui::SameLine();
                WaveButton("SINE",     2); ImGui::SameLine();
                WaveButton("NOISE",    3);

                ImGui::EndGroup();
            }
            ImGui::EndChild();


            // sliders .....
            if (ImGui::BeginChild("SliderRegion", ImVec2(0, 0), ImGuiChildFlags_None))
            {
                // 1. ENVELOPE
                if (ImGui::BeginChild("ENVELOPE_BOX", ImVec2(-FLT_MIN,65.f) )) {
                    ImFlux::GradientBox(ImVec2(-FLT_MIN, -FLT_MIN),0.f);

                    ImGui::Dummy(ImVec2(2,0)); ImGui::SameLine();
                    ImGui::BeginGroup();
                    ImFlux::ShadowText("ENVELOPE", ImFlux::COL32_NEONCYAN);
                    ImGui::Separator();
                    SFXKnob("ATTACK TIME", lParams.p_env_attack, false); ImGui::SameLine();
                    SFXKnob("SUSTAIN TIME", lParams.p_env_sustain, false); ImGui::SameLine();
                    SFXKnob("SUSTAIN PUNCH", lParams.p_env_punch, true); ImGui::SameLine();
                    SFXKnob("DECAY TIME", lParams.p_env_decay, false); ImGui::SameLine();
                    ImGui::EndGroup();
                }
                ImGui::EndChild();


                if (ImGui::BeginChild("FREQUENCY_VIBRATO_BOX", ImVec2(-FLT_MIN,65.f) )) {
                    ImFlux::GradientBox(ImVec2(-FLT_MIN, -FLT_MIN),0.f);
                    ImGui::Dummy(ImVec2(2,0)); ImGui::SameLine();
                    ImGui::BeginGroup();

                    ImFlux::ShadowText("FREQUENCY / VIBRATO", ImFlux::COL32_NEONCYAN);
                    ImGui::Separator();
                    SFXKnob("START FREQUENCY", lParams.p_base_freq, false); ImGui::SameLine();
                    SFXKnob("MIN FREQUENCY", lParams.p_freq_limit, false); ImGui::SameLine();
                    SFXKnob("SLIDE", lParams.p_freq_ramp, true); ImGui::SameLine();
                    SFXKnob("DELTA SLIDE", lParams.p_freq_dramp, true); ImGui::SameLine();
                    ImFlux::SeparatorVertical(0.f, 16.f);
                    SFXKnob("VIBRATO DEPTH", lParams.p_vib_strength, true); ImGui::SameLine();
                    SFXKnob("VIBRATO SPEED", lParams.p_vib_speed, true); ImGui::SameLine();
                    SFXKnob("VIBRATO DELAY", lParams.p_vib_delay, true); ImGui::SameLine();

                    ImGui::EndGroup();
                }
                ImGui::EndChild();

                if (ImGui::BeginChild("ARPEGGIATOR_BOX", ImVec2(-FLT_MIN,65.f) )) {
                    ImFlux::GradientBox(ImVec2(-FLT_MIN, -FLT_MIN),0.f);
                    ImGui::Dummy(ImVec2(2,0)); ImGui::SameLine();
                    ImGui::BeginGroup(/*1*/);
                    ImFlux::ShadowText("ARPEGGIATOR", ImFlux::COL32_NEONCYAN);
                    ImGui::Separator();
                    SFXKnob("CHANGE AMOUNT", lParams.p_arp_mod, true);ImGui::SameLine();
                    SFXKnob("CHANGE SPEED", lParams.p_arp_speed, true);ImGui::SameLine();
                    ImGui::EndGroup(/*1*/);


                    ImFlux::SeparatorVertical(0.f, 16.f);

                    ImGui::BeginGroup(/*2*/);
                    ImFlux::ShadowText("SQUARE DUTY", ImFlux::COL32_NEONCYAN);
                    ImGui::Dummy(ImVec2(0.f,4.f));
                    SFXKnob("SQUARE DUTY", lParams.p_duty, true); ImGui::SameLine();
                    SFXKnob("DUTY SWEEP", lParams.p_duty_ramp, true); ImGui::SameLine();
                    ImGui::EndGroup(/*2*/);

                }
                ImGui::EndChild();


                // if (ImGui::BeginChild("DUTYCYCLE_BOX", ImVec2(-FLT_MIN,65.f) )) {
                //     ImFlux::GradientBox(ImVec2(-FLT_MIN, -FLT_MIN),0.f);
                //     ImFlux::ShadowText("SQUARE DUTY", ImFlux::COL32_NEONCYAN);
                //     ImGui::Separator();
                //     SFXKnob("SQUARE DUTY", lParams.p_duty, false); ImGui::SameLine();
                //     SFXKnob("DUTY SWEEP", lParams.p_duty_ramp, true); ImGui::SameLine();
                // }
                // ImGui::EndChild();

                if (ImGui::BeginChild("REPEAT_BOX", ImVec2(-FLT_MIN,65.f) )) {
                    ImFlux::GradientBox(ImVec2(-FLT_MIN, -FLT_MIN),0.f);
                    ImGui::Dummy(ImVec2(2,0)); ImGui::SameLine();
                    ImGui::BeginGroup(/*1*/);
                    ImFlux::ShadowText("REPEAT", ImFlux::COL32_NEONCYAN);
                    ImGui::Separator();
                    SFXKnob("REPEAT SPEED", lParams.p_repeat_speed, true);
                    ImGui::EndGroup(/*1*/);

                    ImFlux::SeparatorVertical(0.f, 16.f);

                    ImGui::BeginGroup(/*2*/);
                    ImFlux::ShadowText("PHASER", ImFlux::COL32_NEONCYAN);
                    ImGui::Dummy(ImVec2(0.f,4.f));
                    SFXKnob("PHASER OFFSET", lParams.p_pha_offset, true); ImGui::SameLine();
                    SFXKnob("PHASER SWEEP", lParams.p_pha_ramp, true);
                    ImGui::EndGroup(/*2*/);



                }
                ImGui::EndChild();

                // if (ImGui::BeginChild("PHASER_BOX", ImVec2(-FLT_MIN,65.f) )) {
                //
                //     ImFlux::GradientBox(ImVec2(-FLT_MIN, -FLT_MIN),0.f);
                //     ImFlux::ShadowText("PHASER", ImFlux::COL32_NEONCYAN);
                //     SFXKnob("PHASER OFFSET", lParams.p_pha_offset, true); ImGui::SameLine();
                //     SFXKnob("PHASER SWEEP", lParams.p_pha_ramp, true);
                //     ImGui::Separator();
                // }
                // ImGui::EndChild();

                if (ImGui::BeginChild("FILTERS_BOX", ImVec2(-FLT_MIN,65.f) )) {
                    ImFlux::GradientBox(ImVec2(-FLT_MIN, -FLT_MIN),0.f);
                    ImGui::Dummy(ImVec2(2,0)); ImGui::SameLine();
                    ImGui::BeginGroup();
                    ImFlux::ShadowText("FILTERS", ImFlux::COL32_NEONCYAN);
                    ImGui::Separator();
                    SFXKnob("LP FILTER CUTOFF", lParams.p_lpf_freq, false);     ImGui::SameLine();
                    SFXKnob("LP FILTER CUTOFF SWEEP", lParams.p_lpf_ramp, true); ImGui::SameLine();
                    SFXKnob("LP FILTER RESONANCE", lParams.p_lpf_resonance, true); ImGui::SameLine();
                    SFXKnob("HP FILTER CUTOFF", lParams.p_hpf_freq, false); ImGui::SameLine();
                    SFXKnob("HP FILTER CUTOFF SWEEP", lParams.p_hpf_ramp, true); ImGui::SameLine();
                    ImGui::EndGroup();
                }
                ImGui::EndChild();

                if (ImGui::BeginChild("PANNING_BOX", ImVec2(-FLT_MIN,65.f) )) {
                    ImFlux::GradientBox(ImVec2(-FLT_MIN, -FLT_MIN),0.f);
                    ImGui::Dummy(ImVec2(2,0)); ImGui::SameLine();
                    ImGui::BeginGroup();
                    ImFlux::ShadowText("PANNING", ImFlux::COL32_NEONCYAN);
                    ImGui::Separator();
                    SFXKnob("PANNING LEFT | RIGHT", lParams.p_pan, true); ImGui::SameLine();
                    SFXKnob("PANNING SWEEP", lParams.p_pan_ramp, true); ImGui::SameLine();
                    SFXKnob("PANNING SPEED", lParams.p_pan_speed, false); ImGui::SameLine();
                    ImGui::EndGroup();
                }
                ImGui::EndChild();

            }

            // mBitCrusher->renderUI();

            ImGui::EndChild(); //<<< must be outside the if ...


            // --- COLUMN 3: OUTPUT (Right) ---
            ImGui::TableSetColumnIndex(2);

            // Main Play Button
            if (SFXButton("PLAY", lButtonSize)) {
                lSfxGen->PlaySample();
            }
            ImFlux::Hint("Play Sample [SPACE]");

            // Auto Play Checkbox placed directly under PLAY button
            // ImGui::Checkbox("Auto Play", &lAutoPlay);
            ImFlux::LEDCheckBox("AUTO PLAY", &lAutoPlay,ImColor(32, 128, 128));

            ImGui::Separator();

            // ---- Volume ----
            // ImGui::PushItemWidth(-FLT_MIN);
            // SFXSlider("Volume", lSfxGen->sound_vol, false);
            // ImGui::PopItemWidth();

//>>>>>>>>>>
            ImGui::PushItemWidth(-FLT_MIN);

            // 1. Background of the whole slider (The "Unfilled" part)
            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.15f, 0.15f, 0.15f, 1.0f));

            // 2. Background when you hover or click (The "Bar" usually lightens here)
            ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_FrameBgActive,  ImVec4(0.3f, 0.3f, 0.3f, 1.0f));

            // 3. THE ACTUAL KNOB (The "Bar" color in standard ImGui)
            // To make it look like a "Bar", we make the Grab very wide or bright
            if ( lSfxGen->sound_vol <= 0.75 ) {
                ImGui::PushStyleColor(ImGuiCol_SliderGrab,  ImColor4F(cl_Yellow));
                ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, ImVec4(1.0f, 0.9f, 0.2f, 1.0f));
            }
            else {
                ImGui::PushStyleColor(ImGuiCol_SliderGrab,  ImColor4F(cl_Red));
                ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, ImColor4F(cl_Red));
            }

            SFXKnob("Volume", lSfxGen->sound_vol, false);

            ImGui::PopStyleColor(5);
            ImGui::PopItemWidth();

//<<<<<<<<<<

            // ---- more buttons ----
            ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();


#if !defined(__EMSCRIPTEN__) && !defined(__ANDROID__)
            if (SFXButton("Load", lButtonSize)) FileDialog(fa_load);
            if (SFXButton("Save", lButtonSize)) FileDialog(fa_save);
            ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();
#endif

#if defined(__EMSCRIPTEN__)
            ImGui::PushItemWidth(-FLT_MIN);
            ImGui::InputTextWithHint("##fname", "enter filename...", wav_file_name, IM_ARRAYSIZE(wav_file_name));
            ImGui::PopItemWidth();
#endif

            if (SFXButton("EXPORT .WAV", lButtonSize)) {
                FileDialog(fa_export);
            }


            DSP::DrawVisualAnalyzerOszi(mVisualAnalyzer, ImVec2(lButtonSize.x,lButtonSize.y * 2.f));



            ImGui::EndTable();
        }

        ImGui::End();

        // KEYS
        if (ImGui::IsKeyPressed(ImGuiKey_Space))  lSfxGen->PlaySample();
        else
        if (ImGui::IsKeyPressed(ImGuiKey_F1))  TriggerGen([&]{ lSfxGen->Randomize(); });
        else
        if (ImGui::IsKeyPressed(ImGuiKey_F2))  TriggerGen([&]{ lSfxGen->Mutate(); });

    }

    //--------------------------------------------------------------------------
    void onEvent(SDL_Event event) {

    }
    //--------------------------------------------------------------------------

    #if defined(__ANDROID__)
    void FileDialog(SFXGEN_FILE_ACTION_TYPE action)
    {
        //FIXME not implemented on android Dialog
        return ;
    }
    #elif defined(__EMSCRIPTEN__)
    static inline char wav_file_name[256] = "export.wav"; // Buffer to hold the filename
    void FileDialog(SFXGEN_FILE_ACTION_TYPE action) {
        if (action == fa_export)
        {
            if (!mSFXGeneratorStereo->ExportWAV(wav_file_name))
                Log("ERROR: Failed to export wav to [%s]", wav_file_name);
            else
                emscripten_trigger_download(wav_file_name);

        } else {
            //FIXME not implemented on WebGL Dialog
        }
        return ;
    }
    #else //Desktop

    //FIXME THIS SUCKS !!!!!

    void FileDialog(SFXGEN_FILE_ACTION_TYPE action) {
        if (!mSFXGeneratorStereo) return;


        auto* ctx = new FileDialogContext();
        ctx->generator = mSFXGeneratorStereo;
        ctx->action = action;
        ctx->basePath = const_cast<char*>(SDL_GetBasePath());

        const char* ext = (action == fa_export) ? "wav" : "sfx";
        ctx->filters[0] = { ext, ext };
        ctx->filters[1] = { "All files", "*" };


        const char* defaultName = (action == fa_export) ? "untitled.wav" : "settings.sfx";
        char* fullPath = nullptr;

        const char* base = SDL_GetCurrentDirectory();
        if (base) {
            SDL_asprintf(&fullPath, "%s/%s", base, defaultName);
        }
        ctx->basePath = fullPath;

        const char* extStr = (action == fa_export) ? "wav" : "sfx";
        ctx->filterStrings[0] = SDL_strdup(extStr);        // Extension Name
        ctx->filterStrings[1] = SDL_strdup(extStr);        // Pattern
        ctx->filterStrings[2] = SDL_strdup("All files");   // Label

        ctx->filters[0] = { ctx->filterStrings[0], ctx->filterStrings[1] };
        ctx->filters[1] = { ctx->filterStrings[2], "*" };


        auto callback = [](void* userdata, const char* const* filelist, int filter) {
            auto* c = static_cast<FileDialogContext*>(userdata);
            if (filelist && filelist[0]) {
                std::string filename = *filelist;
                const char* extension = (c->action == fa_export) ? ".wav" : ".sfx";
                std::string lowerFilename = filename;
                std::transform(lowerFilename.begin(), lowerFilename.end(), lowerFilename.begin(), ::tolower);
                if (!lowerFilename.ends_with(extension)) {
                    filename += extension;
                }

                if (c->action == fa_load) {
                    dLog("Action: Load SFXR to [%s]", filename.c_str());
                    if (!c->generator->LoadSettings(filename.c_str()))
                        Log("[error] Failed to load SFXR [%s]", filename.c_str());
                }
                else if (c->action == fa_save)  {
                    dLog("Action: Save SFXR to [%s]", filename.c_str());
                    if (!c->generator->SaveSettings(filename.c_str()))
                        Log("[error] Failed to save SFXR to [%s]", filename.c_str());
                }
                else if (c->action == fa_export) {
                    dLog("Action: Export wav to [%s]", filename.c_str());
                    if (!c->generator->exportToWav(filename.c_str(), nullptr))
                        Log("[error] Failed to export wav to [%s]", filename.c_str());
                }
            }
            SDL_free(c->basePath);
            SDL_free(c->filterStrings[0]);
            SDL_free(c->filterStrings[1]);
            SDL_free(c->filterStrings[2]);
            delete c;
        };

        SDL_Window* window = SDL_GL_GetCurrentWindow();
        if (action == fa_save || action == fa_export) {
            SDL_ShowSaveFileDialog(callback, ctx, window, ctx->filters, 2, ctx->basePath);
        } else {
            SDL_ShowOpenFileDialog(callback, ctx, window, ctx->filters, 2, ctx->basePath, false);
        }

    }
    #endif // Desktop



}; //class

