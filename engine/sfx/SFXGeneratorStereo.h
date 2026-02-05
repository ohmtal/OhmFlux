//-----------------------------------------------------------------------------
// Copyright (c) 2007 Tomas Pettersson
// Copyright (c) 2026 XXTH
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// based on: https://www.drpetter.se/project_sfxr.html
//-----------------------------------------------------------------------------
#pragma once
#ifndef SFXGENERATOR_STEREO_H
#define SFXGENERATOR_STEREO_H

#include <SDL3/SDL.h>
#include "errorlog.h"

#include <cstdio>
#include <cmath>
#include <random>
#include <chrono>
#include <cstring>
#include <mutex>

#include <DSP.h>

#ifdef FLUX_ENGINE
#include <imgui.h>
#include <imgui_internal.h>
#include <gui/ImFlux.h>
#endif


namespace FluxSFX {
    const char FILE_IDENTIFIER[] = "FluxSFX";
    constexpr size_t FILE_IDENTIFIER_SIZE = sizeof(FILE_IDENTIFIER) - 1;
} ;


class SFXGeneratorStereo
{


public:
    // Parameters that define the sound
    struct SFXParams
    {
        static const uint8_t CURRENT_VERSION = 104;
        char name[32] = {0};   // since version 104
        float sound_vol = 0.5; // since version 102

        int wave_type;  //0..3 if you change this update the verification in loader!

        float p_base_freq;
        float p_freq_limit;
        float p_freq_ramp;
        float p_freq_dramp; //since version 101
        float p_duty;
        float p_duty_ramp;

        float p_vib_strength;
        float p_vib_speed;
        float p_vib_delay;

        float p_env_attack;
        float p_env_sustain;
        float p_env_decay;
        float p_env_punch;

        bool filter_on;
        float p_lpf_resonance;
        float p_lpf_freq;
        float p_lpf_ramp;
        float p_hpf_freq;
        float p_hpf_ramp;

        float p_pha_offset;
        float p_pha_ramp;

        float p_repeat_speed;

        float p_arp_speed;  //since version 101
        float p_arp_mod;    //since version 101

        // stereo panning since Version 103
        float p_pan;       // Position: -1.0 (Left) to 1.0 (Right), 0.0 is Center
        float p_pan_ramp;  // Change in panning over time
        float p_pan_speed; // multiplier

        char DUMMY[64] = {0}; //playholder for furture use

        auto operator<=>(const SFXParams&) const = default; //C++20 lazy way

        void setName(std::string lName) {
            memset(name, 0, sizeof(name));
            strncpy(name, lName.c_str(), 31);
        }

        // new file format
        void getBinary(std::ostream& os) const {
            uint8_t ver = CURRENT_VERSION;
            os.write(reinterpret_cast<const char*>(&ver), sizeof(ver));
            os.write(reinterpret_cast<const char*>(this), sizeof(SFXParams));
        }

        bool setBinary(std::istream& is) {
            uint8_t fileVersion = 0;
            is.read(reinterpret_cast<char*>(&fileVersion), sizeof(fileVersion));
            if (fileVersion == CURRENT_VERSION) {
                is.read(reinterpret_cast<char*>(this), sizeof(SFXParams));
                if (!validate()) {
                    *this = SFXParams(); // reset to default
                    is.setstate(std::ios::failbit);
                    return false;
                }
            }
            return  is.good();
        }



        bool validate() {
            // Basic Range Checks
            if (wave_type < 0 || wave_type > 3) return false;
            if (sound_vol < 0.0f || sound_vol > 1.0f) return false;

            // Safety Checks for Audio Engine (Prevent NaN or Infinity)
            // Frequency should be positive (0.0 to 1.0 is your typical normalized range)
            // if (p_base_freq < 0.0f || p_base_freq > 1.0f) return false;

            // Filter Resonance should not be too high to avoid feedback loops/explosions
            // if (p_lpf_resonance < 0.0f || p_lpf_resonance > 1.0f) return false;

            // 3. Panning Safety (Ensure it stays within stereo bounds)
            // if (p_pan < -1.0f || p_pan > 1.0f) return false;

            // 4. Sanity check for strings (ensure name is null-terminated)
            // Even if corruption happened, this prevents string-reading crashes
            bool hasNull = false;
            for (int i = 0; i < 32; ++i) {
                if (name[i] == '\0') { hasNull = true; break; }
            }
            if (!hasNull) name[31] = '\0'; // Force termination

            return true;
        }

        // This is now a static helper inside the struct to handle old versions
        bool loadLegacy(std::istream& is, uint8_t version) {
            // Reset to defaults first
            *this = SFXParams();

            // legacy check for version number
            if (version < 100 || version > 103) {
                return false;
            }
            is.read(reinterpret_cast<char*>(&wave_type), sizeof(int));
            if (version >= 102) {
                is.read(reinterpret_cast<char*>(&sound_vol), sizeof(float));
            }

            is.read(reinterpret_cast<char*>(&p_base_freq), sizeof(float));
            is.read(reinterpret_cast<char*>(&p_freq_limit), sizeof(float));
            is.read(reinterpret_cast<char*>(&p_freq_ramp), sizeof(float));

            if (version >= 101) {
                is.read(reinterpret_cast<char*>(&p_freq_dramp), sizeof(float));
            }

            is.read(reinterpret_cast<char*>(&p_duty), sizeof(float));
            is.read(reinterpret_cast<char*>(&p_duty_ramp), sizeof(float));

            is.read(reinterpret_cast<char*>(&p_vib_strength), sizeof(float));
            is.read(reinterpret_cast<char*>(&p_vib_speed), sizeof(float));
            is.read(reinterpret_cast<char*>(&p_vib_delay), sizeof(float));

            is.read(reinterpret_cast<char*>(&p_env_attack), sizeof(float));
            is.read(reinterpret_cast<char*>(&p_env_sustain), sizeof(float));
            is.read(reinterpret_cast<char*>(&p_env_decay), sizeof(float));
            is.read(reinterpret_cast<char*>(&p_env_punch), sizeof(float));

            // Note: old bool might have been saved as 1 or 4 bytes depending on platform
            // If it was 'fread(&filter_on, 1, sizeof(bool), file)', this is fine:
            is.read(reinterpret_cast<char*>(&filter_on), sizeof(bool));

            is.read(reinterpret_cast<char*>(&p_lpf_resonance), sizeof(float));
            is.read(reinterpret_cast<char*>(&p_lpf_freq), sizeof(float));
            is.read(reinterpret_cast<char*>(&p_lpf_ramp), sizeof(float));
            is.read(reinterpret_cast<char*>(&p_hpf_freq), sizeof(float));
            is.read(reinterpret_cast<char*>(&p_hpf_ramp), sizeof(float));

            is.read(reinterpret_cast<char*>(&p_pha_offset), sizeof(float));
            is.read(reinterpret_cast<char*>(&p_pha_ramp), sizeof(float));

            is.read(reinterpret_cast<char*>(&p_repeat_speed), sizeof(float));

            if (version >= 101) {
                is.read(reinterpret_cast<char*>(&p_arp_speed), sizeof(float));
                is.read(reinterpret_cast<char*>(&p_arp_mod), sizeof(float));
            }

            if (version >= 103) {
                is.read(reinterpret_cast<char*>(&p_pan), sizeof(float));
                is.read(reinterpret_cast<char*>(&p_pan_ramp), sizeof(float));
                is.read(reinterpret_cast<char*>(&p_pan_speed), sizeof(float));
            }

            // version 104
            memset(name, 0, sizeof(name));
            strncpy(name, "Legacy SFX", 31);

            if (!validate()) {
                *this = SFXParams(); // reset to default
                is.setstate(std::ios::failbit);
                return false;
            }


            return is.good();
        }
    };

    // State variables used during sound generation
    struct SFXState
    {
        bool playing_sample;
        int phase;
        double fperiod;
        double fmaxperiod;
        double fslide;
        double fdslide;
        int period;
        float square_duty;
        float square_slide;
        int env_stage;
        int env_time;
        int env_length[3];
        float env_vol;
        float fphase;
        float fdphase;
        int iphase;
        float phaser_buffer[1024];
        int ipp;
        float noise_buffer[32];
        float fltp;
        float fltdp;
        float fltw;
        float fltw_d;
        float fltdmp;
        float fltphp;
        float flthp;
        float flthp_d;
        float vib_phase;
        float vib_speed;
        float vib_amp;
        int rep_time;
        int rep_limit;
        int arp_time;
        int arp_limit;
        double arp_mod;

        //panning:
        float pan;          // Current panning position (-1.0 to 1.0)
        float pan_ramp;     // Current panning speed/direction

    };


protected:
    // SynthSample parts:
    float generateMonoTick();
    void updateSystemState();

    // WAV
    bool saveWavFile(const std::string& filename, const std::vector<float>& data, int sampleRate);

    //Effects
    std::vector<std::unique_ptr<DSP::Effect>> mDspEffects;

    //error handling load / save
    std::string mErrors = "";
    void addError(std::string error) { mErrors += error + "\n"; }

public:
    SFXParams mParams;
    SFXState mState;

    std::recursive_mutex mParamsMutex;

    float master_vol;
    // float sound_vol;
    int wav_bits;
    int wav_freq;

    int file_sampleswritten;
    float filesample;
    int fileacc;

    SFXGeneratorStereo();
    ~SFXGeneratorStereo();

    void ResetParams();
    void ResetSample(bool restart);

    void SynthSample(int length, float* stereoBuffer);

    void PlaySample();
    bool LoadSettings(const char* filename, bool allowLegacy = true);
    bool SaveSettings(const char* filename);
    std::string getErrors() { return mErrors; }

    bool ExportWAV(const char* filename);

    void GeneratePickupCoin();
    void GenerateLaserShoot();
    void GenerateExplosion();
    void GeneratePowerup();
    void GenerateHitHurt();
    void GenerateJump();
    void GenerateBlipSelect();
    void Randomize();
    void Mutate();

    void AddPanning(bool doLock = true);

    // moderisation WAV
    bool exportToWav(const std::string& filename, float* progressOut, bool applyEffects = false);

    void attachAudio();
    void detachAudio();


public:

    //------------------------------------------------------------------------------
    // --- SDL
    //------------------------------------------------------------------------------
    static void SDLCALL audio_callback(void* userdata, SDL_AudioStream* stream, int additional_amount, int total_amount);
    bool initSDLAudio();

    void stop() {
        mState.playing_sample = false;
        SDL_ClearAudioStream(mStream);
    }

    SDL_AudioStream* getAudioStream() { return mStream; }
    float getVolume() {
        if (mStream) {
            float gain = SDL_GetAudioStreamGain(mStream);
            return (gain < 0.0f) ? 0.0f : gain;
        }
        return 0.f;
    }
    bool setVolume(const float value) {
        if (mStream)
            return SDL_SetAudioStreamGain(mStream, value);
        return false;
    }


    std::vector<std::unique_ptr<DSP::Effect>>& getDspEffects() {
        return mDspEffects;
    }


private:
    int rnd(int n);
    float frnd(float range);
    std::mt19937 m_rand_engine;
    SDL_AudioStream* mStream = nullptr;
    void ResetParamsNoLock();

public:
#ifdef FLUX_ENGINE
    void DrawWaveIcon(ImDrawList* draw_list, ImVec2 center, float size, int type, ImU32 color) {
        float h = size * 0.4f; // Half height
        float w = size * 0.6f; // Width of the icon

        if (type == 0) { // SQUARE
            ImVec2 pts[4] = {
                center + ImVec2(-w/2, h/2), center + ImVec2(-w/2, -h/2),
                center + ImVec2(0, -h/2), center + ImVec2(0, h/2)
            };
            draw_list->AddPolyline(pts, 4, color, 0, 2.0f);
            // Add the second half of the square wave
            ImVec2 pts2[3] = { center + ImVec2(0, h/2), center + ImVec2(w/2, h/2), center + ImVec2(w/2, -h/2) };
            draw_list->AddPolyline(pts2, 3, color, 0, 2.0f);
        }
        else if (type == 1) { // SAWTOOTH
            ImVec2 pts[3] = { center + ImVec2(-w/2, h/2), center + ImVec2(w/2, -h/2), center + ImVec2(w/2, h/2) };
            draw_list->AddPolyline(pts, 3, color, 0, 2.0f);
        }
        else if (type == 2) { // SINE
            const int segments = 16;
            for (int n = 0; n < segments; n++) {
                float t1 = (float)n / segments;
                float t2 = (float)(n + 1) / segments;
                ImVec2 p1 = center + ImVec2(-w/2 + t1*w, sinf(t1 * 6.28f) * -h);
                ImVec2 p2 = center + ImVec2(-w/2 + t2*w, sinf(t2 * 6.28f) * -h);
                draw_list->AddLine(p1, p2, color, 2.0f);
            }
        }
        else if (type == 3) { // NOISE (Random jagged lines)
            for (int i = 0; i < 8; i++) {
                float x1 = -w/2 + (w/8.0f)*i;
                float x2 = -w/2 + (w/8.0f)*(i+1);
                draw_list->AddLine(center + ImVec2(x1, (i%2==0?h:-h)*0.5f),
                            center + ImVec2(x2, (i%2==0?-h:h)*0.5f), color, 1.5f);
            }
        }
    }
    //--------------------------------------------------------------------------
    bool DrawWaveButton(const char* label, int wave_type) {

        bool is_selected = mParams.wave_type == wave_type;

        ImVec2 size = ImVec2(45, 45); // Fixed round size
        float rounding = size.x * 0.5f;

        // Use your ButtonFancy logic here...
        ImGui::PushID(wave_type);
        bool pressed = ImGui::InvisibleButton(label, size);
        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImRect bb(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());



        // Background & Bevel
        if (is_selected) {
            // Your "Deep" recessed effect
            dl->AddRectFilled(bb.Min, bb.Max, IM_COL32(20, 20, 25, 255), rounding);
            dl->AddRect(bb.Min, bb.Max, IM_COL32(0, 0, 0, 100), rounding, 0, 2.0f);

        } else {
            dl->AddRectFilled(bb.Min, bb.Max, IM_COL32(50, 50, 60, 255), rounding);
            // Outer light bevel
            dl->AddRect(bb.Min, bb.Max, IM_COL32(255, 255, 255, 30), rounding);
        }

        // Draw the icon
        ImU32 icon_col = is_selected ? IM_COL32(0, 255, 180, 255) : IM_COL32(200, 200, 200, 255);
        DrawWaveIcon(dl, bb.GetCenter(), size.x, wave_type, icon_col);

        if (pressed) mParams.wave_type = wave_type;

        ImFlux::Hint(label);

        ImGui::PopID();
        return pressed;
    }


#endif //FLUX_ENGINE
};

#endif // SFXGENERATOR_H
