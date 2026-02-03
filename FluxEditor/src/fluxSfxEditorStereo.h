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


            return mSFXGeneratorStereo->initSDLAudio();
    };
    void Deinitialize() override {
        SAFE_DELETE(mSFXGeneratorStereo);
    }
    // void Update(const double& dt) override {
    //     if ( mSFXGeneratorStereo )
    //     {
    //         // TEST:
    //         if (mSFXGeneratorStereo->mState.playing_sample) {
    //             int frames_to_generate = 512;
    //             std::vector<float> buffer(frames_to_generate * 2);
    //             mSFXGeneratorStereo->SynthSample(frames_to_generate, buffer.data());
    //             SDL_PutAudioStreamData(mSFXGeneratorStereo->getAudioStream(), buffer.data(), buffer.size() * sizeof(float));
    //         }
    //     }
    // }


    void DrawGui()  {
        if (!mSFXGeneratorStereo)
            return;
        DrawSFXEditor();

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
                    Log("Action: Load SFXR to [%s]", filename.c_str());
                    if (!c->generator->LoadSettings(filename.c_str()))
                        Log("ERROR: Failed to load SFXR [%s]", filename.c_str());
                }
                else
                if (c->action == fa_save)  {
                    Log("Action: Save SFXR to [%s]", filename.c_str());
                    if (!c->generator->SaveSettings(filename.c_str()))
                        Log("ERROR: Failed to save SFXR to [%s]", filename.c_str());
                }
                else
                if (c->action == fa_export) {
                    Log("Action: Export wav to [%s]", filename.c_str());
                    if (!c->generator->exportToWav(filename.c_str(), nullptr))
                        Log("ERROR: Failed to export wav to [%s]", filename.c_str());
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
    //--------------------------------------------------------------------------
    bool SFXButton(std::string label, ImVec2 size)
    {
        // return ImGui::Button(label.c_str(), size);

        ImFlux::ButtonParams bp = ImFlux::SLATE_BUTTON.WithSize(size);
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
        auto SFXSlider = [&](const char* label, float& value, bool is_signed) {
            float min_v = is_signed ? -1.0f : 0.0f;
            char format[64];
            snprintf(format, 64, "%s: %%.3f", label);

            // if returns true, we can trigger AutoPlay logic here if needed
            return ImGui::SliderFloat((std::string("##") + label).c_str(), &value, min_v, 1.0f, format);
        };



        // minimum size
        ImGui::SetNextWindowSizeConstraints(ImVec2(600.0f, 650.f), ImVec2(FLT_MAX, FLT_MAX));

        ImGui::Begin("Sound Effects Generator Stereo");

        if (ImGui::BeginTable("EditorColumns", 3,
            ImGuiTableFlags_Resizable |
            ImGuiTableFlags_BordersInnerV |
            ImGuiTableFlags_SizingStretchProp, // Better for mixed column types
            ImVec2(-FLT_MIN, 0.0f)))
        {                // 3. Use WidthFixed for sides, WidthStretch for middle
            ImGui::TableSetupColumn("Presets", ImGuiTableColumnFlags_WidthFixed, 130.0f);
            ImGui::TableSetupColumn("Sliders", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Actions", ImGuiTableColumnFlags_WidthFixed, 130.0f);

            ImGui::TableNextRow();

            // --- COLUMN 1: GENERATORS (Left) ---
            ImGui::TableSetColumnIndex(0);
            ImGui::TextDisabled("PRESETS");

            ImVec2 lButtonSize = ImVec2(120.f, 40.f);



            if (SFXButton("Pickup/Coin", lButtonSize)) TriggerGen([&]{ lSfxGen->GeneratePickupCoin(); });
            if (SFXButton("Laser/Shoot", lButtonSize)) TriggerGen([&]{ lSfxGen->GenerateLaserShoot(); });
            if (SFXButton("Explosion",   lButtonSize)) TriggerGen([&]{ lSfxGen->GenerateExplosion(); });
            if (SFXButton("Powerup",     lButtonSize)) TriggerGen([&]{ lSfxGen->GeneratePowerup(); });
            if (SFXButton("Hit/Hurt",    lButtonSize)) TriggerGen([&]{ lSfxGen->GenerateHitHurt(); });
            if (SFXButton("Jump",        lButtonSize)) TriggerGen([&]{ lSfxGen->GenerateJump(); });
            if (SFXButton("Blip/Select", lButtonSize)) TriggerGen([&]{ lSfxGen->GenerateBlipSelect(); });

            ImGui::Separator();
            if (SFXButton("Randomize",   lButtonSize)) TriggerGen([&]{ lSfxGen->Randomize(); });
            ImFlux::Hint("Randomize Sample [F1]");
            if (SFXButton("Mutate",      lButtonSize)) TriggerGen([&]{ lSfxGen->Mutate(); });
            ImFlux::Hint("Mutate Sample [F2]");
            if (SFXButton("PanningMutate",  lButtonSize)) TriggerGen([&]{ lSfxGen->AddPanning(true); });


            // --- COLUMN 2: PARAMETERS (Middle) ---
            ImGui::TableSetColumnIndex(1);

            // Force column stability (as we found in step #3)
            ImGui::Dummy(ImVec2(300.0f, 0.0f));

            // toggle buttons:
            // --- WAVE TYPE SELECTION (Top of Middle Column) ---
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 1.0f, 1.0f), "WAVE TYPE");

            // 1. Calculate the total width available for the row once
            float total_width = ImGui::GetContentRegionAvail().x;
            // 2. Subtract the spacing between buttons (default is Style.ItemSpacing.x)
            float spacing = ImGui::GetStyle().ItemSpacing.x;
            // 3. Divide by 4 (number of buttons)
            float button_width = (total_width - (spacing * 3)) / 4.0f;

            auto WaveButton = [&](const char* label, int type) {
                bool is_selected = (lParams.wave_type == type);

                if (is_selected) {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyle().Colors[ImGuiCol_ButtonActive]);
                }

                // Use the pre-calculated fixed width
                if (SFXButton(label, ImVec2(button_width, 30))) {
                    lParams.wave_type = type;
                    if (lAutoPlay) lSfxGen->PlaySample();
                }

                if (is_selected) ImGui::PopStyleColor();
            };

            WaveButton("SQUARE",   0); ImGui::SameLine();
            WaveButton("SAWTOOTH", 1); ImGui::SameLine();
            WaveButton("SINE",     2); ImGui::SameLine();
            WaveButton("NOISE",    3);

            ImGui::Spacing();
            ImGui::Separator();


            // sliders .....
            if (ImGui::BeginChild("SliderRegion", ImVec2(0, 0), ImGuiChildFlags_None))
            {
                ImGui::PushItemWidth(-FLT_MIN);


                // 1. ENVELOPE
                ImGui::TextColored(ImVec4(0.7f, 0.7f, 1.0f, 1.0f), "ENVELOPE");
                SFXSlider("ATTACK TIME", lParams.p_env_attack, false);
                SFXSlider("SUSTAIN TIME", lParams.p_env_sustain, false);
                SFXSlider("SUSTAIN PUNCH", lParams.p_env_punch, false);
                SFXSlider("DECAY TIME", lParams.p_env_decay, false);
                ImGui::Separator();

                // 2. FREQUENCY & VIBRATO
                ImGui::TextColored(ImVec4(0.7f, 0.7f, 1.0f, 1.0f), "FREQUENCY / VIBRATO");
                SFXSlider("START FREQUENCY", lParams.p_base_freq, false);
                SFXSlider("MIN FREQUENCY", lParams.p_freq_limit, false);
                SFXSlider("SLIDE", lParams.p_freq_ramp, true);
                SFXSlider("DELTA SLIDE", lParams.p_freq_dramp, true);
                SFXSlider("VIBRATO DEPTH", lParams.p_vib_strength, false);
                SFXSlider("VIBRATO SPEED", lParams.p_vib_speed, false);
                ImGui::Separator();

                // 3. ARPEGGIATOR
                ImGui::TextColored(ImVec4(0.7f, 0.7f, 1.0f, 1.0f), "ARPEGGIATOR");
                SFXSlider("CHANGE AMOUNT", lParams.p_arp_mod, true);
                SFXSlider("CHANGE SPEED", lParams.p_arp_speed, false);
                ImGui::Separator();

                // 4. DUTY CYCLE
                ImGui::TextColored(ImVec4(0.7f, 0.7f, 1.0f, 1.0f), "SQUARE DUTY");
                SFXSlider("SQUARE DUTY", lParams.p_duty, false);
                SFXSlider("DUTY SWEEP", lParams.p_duty_ramp, true);
                ImGui::Separator();

                // 5. REPEAT
                ImGui::TextColored(ImVec4(0.7f, 0.7f, 1.0f, 1.0f), "RETREIG");
                SFXSlider("REPEAT SPEED", lParams.p_repeat_speed, false);
                ImGui::Separator();

                // 6. PHASER
                ImGui::TextColored(ImVec4(0.7f, 0.7f, 1.0f, 1.0f), "PHASER");
                SFXSlider("PHASER OFFSET", lParams.p_pha_offset, true);
                SFXSlider("PHASER SWEEP", lParams.p_pha_ramp, true);
                ImGui::Separator();

                // 7. FILTERS
                ImGui::TextColored(ImVec4(0.7f, 0.7f, 1.0f, 1.0f), "FILTERS");
                SFXSlider("LP FILTER CUTOFF", lParams.p_lpf_freq, false);
                SFXSlider("LP FILTER CUTOFF SWEEP", lParams.p_lpf_ramp, true);
                SFXSlider("LP FILTER RESONANCE", lParams.p_lpf_resonance, false);
                SFXSlider("HP FILTER CUTOFF", lParams.p_hpf_freq, false);
                SFXSlider("HP FILTER CUTOFF SWEEP", lParams.p_hpf_ramp, true);


                ImGui::Separator();
                // 8. PANNING
                ImGui::TextColored(ImVec4(0.7f, 0.7f, 1.0f, 1.0f), "PANNING");
                SFXSlider("PANNING LEFT | RIGHT", lParams.p_pan, true);
                SFXSlider("PANNING RAMP", lParams.p_pan_ramp, true);
                SFXSlider("PANNING SPEED", lParams.p_pan_speed, false);


                ImGui::PopItemWidth();
            }
            ImGui::EndChild(); //<<< must be outside the if ...


            // --- COLUMN 3: OUTPUT (Right) ---
            ImGui::TableSetColumnIndex(2);

            // Main Play Button
            if (SFXButton("PLAY", lButtonSize)) {
                lSfxGen->PlaySample();
            }
            ImFlux::Hint("Play Sample [SPACE]");

            // Auto Play Checkbox placed directly under PLAY button
            ImGui::Checkbox("Auto Play", &lAutoPlay);

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

            SFXSlider("Volume", lSfxGen->sound_vol, false);

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


}; //class

