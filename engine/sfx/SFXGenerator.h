//-----------------------------------------------------------------------------
// Copyright (c) 2007 Tomas Pettersson
// Copyright (c) 2026 XXTH
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// based on: https://www.drpetter.se/project_sfxr.html
//-----------------------------------------------------------------------------
#pragma once
#ifndef SFXGENERATOR_H
#define SFXGENERATOR_H

#include <SDL3/SDL.h>
#include "errorlog.h"

#include <cstdio>
#include <cmath>
#include <random>
#include <chrono>
#include <cstring> // For memset()

// Forward declaration for FILE, as it's used in method signatures
// The actual definition comes from <cstdio> which is included above.

// Parameters that define the sound
struct SFXParams
{
    int wave_type;

    float p_base_freq;
    float p_freq_limit;
    float p_freq_ramp;
    float p_freq_dramp;
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

    float p_arp_speed;
    float p_arp_mod;
};

// State variables used during sound generation
struct SFXState
{
    bool playing_sample;
    bool mute_stream = false;
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
};

class SFXGenerator
{
public:
    SFXParams mParams;
    SFXState mState;

    float master_vol;
    float sound_vol;
    int wav_bits;
    int wav_freq;

    int file_sampleswritten;
    float filesample;
    int fileacc;

    SFXGenerator();

    void ResetParams();
    bool LoadSettings(const char* filename);
    bool SaveSettings(const char* filename);
    void ResetSample(bool restart);
    void PlaySample();
    void SynthSample(int length, float* buffer, FILE* file);
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

    //------------------------------------------------------------------------------
    // --- SDL
    //------------------------------------------------------------------------------
    static void SDLCALL audio_callback(void* userdata, SDL_AudioStream* stream, int additional_amount, int total_amount);
    bool initSDLAudio();

    void stop() {
        mState.playing_sample = false;
        SDL_ClearAudioStream(mStream);
    }



private:
    int rnd(int n);
    float frnd(float range);
    std::mt19937 m_rand_engine;
    SDL_AudioStream* mStream = nullptr;
};

#endif // SFXGENERATOR_H
