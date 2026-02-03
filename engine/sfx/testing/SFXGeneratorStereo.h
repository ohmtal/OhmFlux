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
#include <cstring>
#include <mutex>

#include <DSP.h>



class SFXGeneratorStereo
{
public:
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

        // stereo panning Version
        float p_pan;       // Position: -1.0 (Left) to 1.0 (Right), 0.0 is Center
        float p_pan_ramp;  // Change in panning over time
        float p_pan_speed; // multiplier


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

    // WAV
    bool saveWavFile(const std::string& filename, const std::vector<float>& data, int sampleRate);
    void attachAudio();
    void detachAudio();

    //Effects
    std::vector<std::unique_ptr<DSP::Effect>> mDspEffects;


public:
    SFXParams mParams;
    SFXState mState;

    std::recursive_mutex mParamsMutex;

    float master_vol;
    float sound_vol;
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
    bool LoadSettings(const char* filename);
    bool SaveSettings(const char* filename);
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

};

#endif // SFXGENERATOR_H
