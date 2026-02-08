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


class SfxStereoModule : public FluxBaseObject
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

    // DSP::RingModulator* mRingMod;
    // DSP::VoiceModulator* mVoiceMod;
    // DSP::Delay* mDelay;
    // DSP::Chorus* mChorus;
    DSP::VisualAnalyzer* mVisualAnalyzer;
    DSP::Limiter* mLimiter;

public:


    ~SfxStereoModule() { Deinitialize(); }

    SFXGeneratorStereo* getSFXGeneratorStereo() { return mSFXGeneratorStereo; }

    // void Execute() override;
    bool Initialize() override {

        mSFXGeneratorStereo = new SFXGeneratorStereo();
        if (!mSFXGeneratorStereo->initSDLAudio())
            return false;



        // mVoiceMod = DSP::addEffectToChain<DSP::VoiceModulator>(mSFXGeneratorStereo->getDspEffects(), false);
        // mRingMod  = DSP::addEffectToChain<DSP::RingModulator>(mSFXGeneratorStereo->getDspEffects(), false);
        // mChorus   = DSP::addEffectToChain<DSP::Chorus>(mSFXGeneratorStereo->getDspEffects(), false);
        // mDelay    = DSP::addEffectToChain<DSP::Delay>(mSFXGeneratorStereo->getDspEffects(), false);
        mLimiter = DSP::addEffectToChain<DSP::Limiter>(mSFXGeneratorStereo->getDspEffects(), true);
        mVisualAnalyzer = DSP::addEffectToChain<DSP::VisualAnalyzer>(mSFXGeneratorStereo->getDspEffects(), true);



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

            // --- WAVE TYPE SELECTION  ---
            auto WaveButton = [&](const char* label, int type) {
                if (mSFXGeneratorStereo->DrawWaveButton(label,type)
                    &&   lAutoPlay
                ) lSfxGen->PlaySample();


            };

            if (ImGui::BeginChild("NAME_BOX", ImVec2(-FLT_MIN,34.f) )) {
                ImFlux::GradientBox(ImVec2(-FLT_MIN, -FLT_MIN),0.f);
                ImGui::Dummy(ImVec2(2,5)); //
                ImGui::Dummy(ImVec2(2,0)); ImGui::SameLine();

                ImGui::BeginGroup();

                ImFlux::ShadowText("Name:", ImFlux::COL32_NEON_CYAN);
                ImGui::SameLine();

                ImGui::SetNextItemWidth(240.f);
                ImGui::PushStyleColor(ImGuiCol_FrameBg, ImFlux::DEFAULT_GRADIENTBOX.col_bot);
                ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImFlux::DEFAULT_GRADIENTBOX.col_top);
                ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImFlux::DEFAULT_GRADIENTBOX.col_top);
                ImGui::PushStyleColor(ImGuiCol_Border, ImFlux::DEFAULT_GRADIENTBOX.col_top);
                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
                ImGui::InputText("##SFXNAME", lParams.name, sizeof(lParams.name));
                ImGui::PopStyleVar();
                ImGui::PopStyleColor(4);

                ImGui::EndGroup();
            }
            ImGui::EndChild();


            if (ImGui::BeginChild("WAVE_TYPE_BOX", ImVec2(-FLT_MIN,70.f) )) {
                ImFlux::GradientBox(ImVec2(-FLT_MIN, -FLT_MIN),0.f);
                ImGui::Dummy(ImVec2(2,0)); ImGui::SameLine();
                ImGui::BeginGroup();

                ImFlux::ShadowText("WAVE TYPE", ImFlux::COL32_NEON_CYAN);
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
                    ImFlux::ShadowText("ENVELOPE", ImFlux::COL32_NEON_CYAN);
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

                    ImGui::BeginGroup(/*1*/);
                    ImFlux::ShadowText("FREQUENCY", ImFlux::COL32_NEON_CYAN);
                    ImGui::Separator();
                    SFXKnob("START FREQUENCY", lParams.p_base_freq, false); ImGui::SameLine();
                    SFXKnob("MIN FREQUENCY", lParams.p_freq_limit, false); ImGui::SameLine();
                    SFXKnob("SLIDE", lParams.p_freq_ramp, true); ImGui::SameLine();
                    SFXKnob("DELTA SLIDE", lParams.p_freq_dramp, true); ImGui::SameLine();
                    ImGui::EndGroup(/*1*/);

                    ImFlux::SeparatorVertical(0.f, 16.f);

                    ImGui::BeginGroup(/*2*/);
                    ImFlux::ShadowText("VIBRATO", ImFlux::COL32_NEON_CYAN);
                    ImGui::Dummy(ImVec2(0.f,3.5f));
                    SFXKnob("VIBRATO DEPTH", lParams.p_vib_strength, true); ImGui::SameLine();
                    SFXKnob("VIBRATO SPEED", lParams.p_vib_speed, true); ImGui::SameLine();
                    SFXKnob("VIBRATO DELAY", lParams.p_vib_delay, true); ImGui::SameLine();
                    ImGui::EndGroup(/*2*/);

                }
                ImGui::EndChild();

                if (ImGui::BeginChild("ARPEGGIATOR_BOX", ImVec2(-FLT_MIN,65.f) )) {
                    ImFlux::GradientBox(ImVec2(-FLT_MIN, -FLT_MIN),0.f);
                    ImGui::Dummy(ImVec2(2,0)); ImGui::SameLine();
                    ImGui::BeginGroup(/*1*/);
                    ImFlux::ShadowText("ARPEGGIATOR", ImFlux::COL32_NEON_CYAN);
                    ImGui::Separator();
                    SFXKnob("CHANGE AMOUNT", lParams.p_arp_mod, true);ImGui::SameLine();
                    SFXKnob("CHANGE SPEED", lParams.p_arp_speed, true);ImGui::SameLine();
                    ImGui::EndGroup(/*1*/);


                    ImFlux::SeparatorVertical(0.f, 16.f);

                    ImGui::BeginGroup(/*2*/);
                    ImFlux::ShadowText("SQUARE DUTY", ImFlux::COL32_NEON_CYAN);
                    ImGui::Dummy(ImVec2(0.f,3.5f));
                    SFXKnob("SQUARE DUTY", lParams.p_duty, true); ImGui::SameLine();
                    SFXKnob("DUTY SWEEP", lParams.p_duty_ramp, true); ImGui::SameLine();
                    ImGui::EndGroup(/*2*/);

                }
                ImGui::EndChild();


                // if (ImGui::BeginChild("DUTYCYCLE_BOX", ImVec2(-FLT_MIN,65.f) )) {
                //     ImFlux::GradientBox(ImVec2(-FLT_MIN, -FLT_MIN),0.f);
                //     ImFlux::ShadowText("SQUARE DUTY", ImFlux::COL32_NEON_CYAN);
                //     ImGui::Separator();
                //     SFXKnob("SQUARE DUTY", lParams.p_duty, false); ImGui::SameLine();
                //     SFXKnob("DUTY SWEEP", lParams.p_duty_ramp, true); ImGui::SameLine();
                // }
                // ImGui::EndChild();

                if (ImGui::BeginChild("REPEAT_BOX", ImVec2(-FLT_MIN,65.f) )) {
                    ImFlux::GradientBox(ImVec2(-FLT_MIN, -FLT_MIN),0.f);
                    ImGui::Dummy(ImVec2(2,0)); ImGui::SameLine();
                    ImGui::BeginGroup(/*1*/);
                    ImFlux::ShadowText("REPEAT", ImFlux::COL32_NEON_CYAN);
                    ImGui::Separator();
                    SFXKnob("REPEAT SPEED", lParams.p_repeat_speed, true);
                    ImGui::EndGroup(/*1*/);

                    ImFlux::SeparatorVertical(0.f, 16.f);

                    ImGui::BeginGroup(/*2*/);
                    ImFlux::ShadowText("PHASER", ImFlux::COL32_NEON_CYAN);
                    ImGui::Dummy(ImVec2(0.f,4.f));
                    SFXKnob("PHASER OFFSET", lParams.p_pha_offset, true); ImGui::SameLine();
                    SFXKnob("PHASER SWEEP", lParams.p_pha_ramp, true);
                    ImGui::EndGroup(/*2*/);



                }
                ImGui::EndChild();

                // if (ImGui::BeginChild("PHASER_BOX", ImVec2(-FLT_MIN,65.f) )) {
                //
                //     ImFlux::GradientBox(ImVec2(-FLT_MIN, -FLT_MIN),0.f);
                //     ImFlux::ShadowText("PHASER", ImFlux::COL32_NEON_CYAN);
                //     SFXKnob("PHASER OFFSET", lParams.p_pha_offset, true); ImGui::SameLine();
                //     SFXKnob("PHASER SWEEP", lParams.p_pha_ramp, true);
                //     ImGui::Separator();
                // }
                // ImGui::EndChild();

                if (ImGui::BeginChild("FILTERS_BOX", ImVec2(-FLT_MIN,65.f) )) {
                    ImFlux::GradientBox(ImVec2(-FLT_MIN, -FLT_MIN),0.f);
                    ImGui::Dummy(ImVec2(2,0)); ImGui::SameLine();
                    ImGui::BeginGroup();
                    ImFlux::ShadowText("FILTERS", ImFlux::COL32_NEON_CYAN);
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
                    ImFlux::ShadowText("PANNING", ImFlux::COL32_NEON_CYAN);
                    ImGui::Separator();
                    SFXKnob("PANNING LEFT | RIGHT", lParams.p_pan, true); ImGui::SameLine();
                    SFXKnob("PANNING SWEEP", lParams.p_pan_ramp, true); ImGui::SameLine();
                    SFXKnob("PANNING SPEED", lParams.p_pan_speed, false); ImGui::SameLine();
                    ImGui::EndGroup();
                }
                ImGui::EndChild();

                if (ImGui::BeginChild("PREAMP_BOX", ImVec2(-FLT_MIN,65.f) )) {
                    ImFlux::GradientBox(ImVec2(-FLT_MIN, -FLT_MIN),0.f);
                    ImGui::Dummy(ImVec2(2,0)); ImGui::SameLine();
                    ImGui::BeginGroup(/*1*/);
                    ImFlux::ShadowText("PREAMP VOLUME", ImFlux::COL32_NEON_CYAN);
                    ImGui::Separator();
                    SFXKnob("Volume", lParams.sound_vol, false);
                    ImFlux::SameLineCentered(12.f);
                    ImFlux::VUMeter80th(lParams.sound_vol, 20,ImVec2(2.f, 12.f) );
                    ImGui::EndGroup(/*1*/);

                    ImFlux::SeparatorVertical(0.f, 16.f);

                    ImGui::BeginGroup(/*2*/);
                    bool changed = false;
                    DSP::LimiterSettings& currentSettings = mLimiter->getSettings();
                    float reduction = mLimiter->getGainReduction();
                    ImFlux::ShadowText("LIMITER", ImFlux::COL32_NEON_CYAN);
                    ImGui::Dummy(ImVec2(0.f,1.5f));
                    ImGui::BeginGroup(/*2.2*/);
                     changed |= ImFlux::MiniKnobF("Threshold", &currentSettings.Threshold, 0.01f, 1.f);
                     if (changed) mLimiter->setSettings(currentSettings);
                     ImGui::SameLine();
                     ImGui::BeginGroup(/*2.1*/);
                      ImGui::TextDisabled("Reduction: %4.1f%%", reduction * 100.f);
                      ImFlux::PeakMeter(reduction,ImVec2(125.f, 8.f));
                     ImGui::EndGroup(/*2.1*/);
                    ImGui::EndGroup(/*2.2*/);
                    ImGui::EndGroup(/*2*/);
                }
                ImGui::EndChild();



            } //<<<< SLIDER REGION




            // mBitCrusher->renderUI();
            // mRingMod->renderUIWide();
            // mVoiceMod->renderUIWide();
            // mChorus->renderUIWide();
            // mDelay->renderUIWide();
            // mLimiter->renderUIWide();
            // mVisualAnalyzer->renderPeakTest(); //TEST VU's

            ImGui::EndChild(); //<<< must be outside the if ...


            // --- COLUMN 3: OUTPUT (Right) ---
            ImGui::TableSetColumnIndex(2);

            // background << too streched
            // ImFlux::GradientBox(ImVec2(-FLT_MIN, -FLT_MIN),0.f);


            // Main Play Button
            if (SFXButton("PLAY", lButtonSize)) {
                lSfxGen->PlaySample();
            }
            ImFlux::Hint("Play Sample [SPACE]");

            // Auto Play Checkbox placed directly under PLAY button
            // ImGui::Checkbox("Auto Play", &lAutoPlay);
            ImFlux::LEDCheckBox("AUTO PLAY", &lAutoPlay,ImColor(32, 128, 128));

            // ImGui::Separator();

            // ---- Volume ----
            // ImGui::PushItemWidth(-FLT_MIN);
            // SFXSlider("Volume", lSfxGen->sound_vol, false);
            // ImGui::PopItemWidth();

//>>>>>>>>>>
            // ImGui::PushItemWidth(-FLT_MIN);
            //
            //
            // SFXKnob("Volume", lParams.sound_vol, false);
            // ImFlux::SameLineCentered(12.f);
            // ImFlux::VUMeter80th(lParams.sound_vol, 20,ImVec2(2.f, 12.f) );
            //
            // ImGui::PopItemWidth();

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


            ImVec2 lVisuSize = ImVec2(lButtonSize.x,lButtonSize.y * 2.f );
            if (ImGui::BeginChild("VisualAnalyzer_BOX", ImVec2(lVisuSize.x, lVisuSize.y * 3.2f ) )) {
                // invisible nearly ... ImFlux::GradientBox(ImVec2(-FLT_MIN, -FLT_MIN),0.f);

                mVisualAnalyzer->DrawVisualAnalyzerOszi(lVisuSize, 2);
                float levLeft = 0.f;
                float levRight = 0.f;

                levLeft = mVisualAnalyzer->getLevel(0);
                levRight = mVisualAnalyzer->getLevel(1);

                // mVisualAnalyzer->getLevels(levLeft, levRight);
                const ImU32 startCol = IM_COL32(70, 70, 70, 255);
                const ImU32 endCol   = IM_COL32(15, 15, 20, 255);

                ImFlux::VUMeter70th(lVisuSize, levLeft, "(L)", startCol, endCol);
                ImFlux::VUMeter70th(lVisuSize, levRight, "(R)", startCol, endCol);
                //  IM_COL32(75, 75, 85, 255), IM_COL32(15, 15, 20, 255)
                // ImFlux::VUMeter70th(lVisuSize, levRight, "(R)", IM_COL32(25, 25, 30, 255), IM_COL32(45, 45, 55, 255));

            }
            ImGui::EndChild();



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
                        Log("[error] Failed to load SFXR [%s]\n%s", filename.c_str(), c->generator->getErrors().c_str());
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

