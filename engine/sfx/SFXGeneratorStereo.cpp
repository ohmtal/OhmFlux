//-----------------------------------------------------------------------------
// Copyright (c) 2007 Tomas Pettersson
// Copyright (c) 2026 XXTH
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// 2026-02-03 File Version 103 (panning) FIXME export ok playback NOT !!!!
//                  need a bit sleep ;) maybe tomorrow
//-----------------------------------------------------------------------------

#include "SFXGeneratorStereo.h"
#include <SDL3/SDL.h>
#include <mutex>

#include <fstream>
#include <vector>
#include <string>
#include <stdexcept>
#include <type_traits>


#ifdef FLUX_ENGINE
#include <audio/fluxAudio.h>
#endif


#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif
//-----------------------------------------------------------------------------
// A helper function for random numbers
int SFXGeneratorStereo::rnd(int n) {
    if (n <= 0) return 0;
    // uniform_int_distribution includes both 0 and n
    std::uniform_int_distribution<int> dist(0, n);
    return dist(m_rand_engine);
}

float SFXGeneratorStereo::frnd(float range) {
    // 2. Define the distribution (uniform float between 0.0 and 1.0)
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    // 3. Generate the number and scale it
    return dist(m_rand_engine) * range;
}
//-----------------------------------------------------------------------------
SFXGeneratorStereo::SFXGeneratorStereo():
    m_rand_engine(std::random_device{}())
{

    SFXGeneratorStereo::mErrors = "";

    master_vol = 0.5f;
    // sound_vol = 0.5f;

    // Reset all sound parameters to default
    ResetParamsNoLock();
    // Initialize state variables
    mState.playing_sample = false;
    mState.phase = 0;
    mState.fperiod = 0.0;
    mState.fmaxperiod = 0.0;
    mState.fslide = 0.0;
    mState.fdslide = 0.0;
    mState.period = 0;
    mState.square_duty = 0.0f;
    mState.square_slide = 0.0f;
    mState.env_stage = 0;
    mState.env_time = 0;
    mState.env_length[0] = 0;
    mState.env_length[1] = 0;
    mState.env_length[2] = 0;
    mState.env_vol = 0.0f;
    mState.fphase = 0.0f;
    mState.fdphase = 0.0f;
    mState.iphase = 0;
    memset(mState.phaser_buffer, 0, sizeof(mState.phaser_buffer));
    mState.ipp = 0;
    memset(mState.noise_buffer, 0, sizeof(mState.noise_buffer));
    mState.fltp = 0.0f;
    mState.fltdp = 0.0f;
    mState.fltw = 0.0f;
    mState.fltw_d = 0.0f;
    mState.fltdmp = 0.0f;
    mState.fltphp = 0.0f;
    mState.flthp = 0.0f;
    mState.flthp_d = 0.0f;
    mState.vib_phase = 0.0f;
    mState.vib_speed = 0.0f;
    mState.vib_amp = 0.0f;
    mState.rep_time = 0;
    mState.rep_limit = 0;
    mState.arp_time = 0;
    mState.arp_limit = 0;
    mState.arp_mod = 0.0;
    //panning
    mState.pan = 0.f;
    mState.pan_ramp = 0.f;

}

SFXGeneratorStereo::~SFXGeneratorStereo()
{
    if (mStream)
    {
        SDL_FlushAudioStream(mStream);
        SDL_SetAudioStreamGetCallback(mStream, NULL, NULL);
        SDL_DestroyAudioStream(mStream);
        mStream = nullptr;
    }
}
//-----------------------------------------------------------------------------
void SFXGeneratorStereo::ResetParams()
{
    std::lock_guard<std::recursive_mutex> lock(mParamsMutex);
    ResetParamsNoLock();
}
void SFXGeneratorStereo::ResetParamsNoLock()
{

    mParams.setName("... no name ...");
    mParams.wave_type=0;

    mParams.p_base_freq=0.3f;
    mParams.p_freq_limit=0.0f;
    mParams.p_freq_ramp=0.0f;
    mParams.p_freq_dramp=0.0f;
    mParams.p_duty=0.0f;
    mParams.p_duty_ramp=0.0f;

    mParams.p_vib_strength=0.0f;
    mParams.p_vib_speed=0.0f;
    mParams.p_vib_delay=0.0f;

    mParams.p_env_attack=0.0f;
    mParams.p_env_sustain=0.3f;
    mParams.p_env_decay=0.4f;
    mParams.p_env_punch=0.0f;

    mParams.filter_on=false;
    mParams.p_lpf_resonance=0.0f;
    mParams.p_lpf_freq=1.0f;
    mParams.p_lpf_ramp=0.0f;
    mParams.p_hpf_freq=0.0f;
    mParams.p_hpf_ramp=0.0f;
    
    mParams.p_pha_offset=0.0f;
    mParams.p_pha_ramp=0.0f;

    mParams.p_repeat_speed=0.0f;

    mParams.p_arp_speed=0.0f;
    mParams.p_arp_mod=0.0f;


    mParams.p_pan = 0.f;       // Position: -1.0 (Left) to 1.0 (Right), 0.0 is Center
    mParams.p_pan_ramp  = 0.f;  // Change in panning over time
    mParams.p_pan_speed =  0.0001f;

}
//-----------------------------------------------------------------------------
bool SFXGeneratorStereo::LoadSettings(const char* filename, bool allowLegacy)
{

    mErrors = "";
    std::ifstream ifs; //(filePath, std::ios::binary);
    ifs.exceptions(std::ifstream::badbit | std::ifstream::failbit);

    try {
        ifs.open(filename, std::ios::binary);
        if (!ifs.is_open()) {
            addError(std::format("Can't open File {} for read.", filename));
            return false;
        }

        // // Read and verify identifier
        char identifierBuffer[FluxSFX::FILE_IDENTIFIER_SIZE];
        bool hasIdentifier = true;
        try {
            ifs.read(identifierBuffer, FluxSFX::FILE_IDENTIFIER_SIZE);
            if (std::memcmp(identifierBuffer, FluxSFX::FILE_IDENTIFIER, FluxSFX::FILE_IDENTIFIER_SIZE) != 0) {
                hasIdentifier = false;
            }
        } catch (const std::ios_base::failure&) {
            hasIdentifier = false;
        }

        if (!hasIdentifier) {
            if (!allowLegacy) {
                addError("File Identifier mismatch.");
                return false;
            }
            // Clear error state and rewind to the very beginning
            ifs.clear();
            ifs.seekg(0, std::ios::beg);

            // Peek at the version byte without advancing the pointer too far
            int lVersion = 0;
            ifs.read(reinterpret_cast<char*>(&lVersion), sizeof(int));

            if (lVersion < 100 || lVersion > 103) {
                addError(std::format("Invalid Legacy Version: {}", lVersion));
                return false;
            }
            std::lock_guard<std::recursive_mutex> lock(mParamsMutex);
            bool success = mParams.loadLegacy(ifs, lVersion);
            if (!success) addError("Failed to load legacy file!");
            return success;
        }

        std::lock_guard<std::recursive_mutex> lock(mParamsMutex);
        bool success = mParams.setBinary(ifs);
        if (!success) addError("Failed to load file!");
        return success;

    } catch (const std::ios_base::failure& e) {
        // If we catch this, it's because a read_binary or read_vector failed
        if (ifs.eof()) {
            addError("Unexpected End of File: The file is truncated.");
        } else {
            addError(std::format("I/O failure: {}", e.what()));
        }
        return false;
    } catch (const std::bad_alloc&) {
        addError("File requested too much memory (possible corruption).");
        return false;
    } catch (const std::exception& e) {
        addError(std::format("General error: {}", e.what()));
        return false;
    }

    // i should never get here ...
    return false;
}
//-----------------------------------------------------------------------------
bool SFXGeneratorStereo::SaveSettings(const char* filename)
{
    mErrors = "";
    std::ofstream ofs;
    ofs.exceptions(std::ofstream::badbit | std::ofstream::failbit);

    try {
        ofs.open(filename, std::ios::binary | std::ios::trunc);
        ofs.write(FluxSFX::FILE_IDENTIFIER, FluxSFX::FILE_IDENTIFIER_SIZE);
        std::lock_guard<std::recursive_mutex> lock(mParamsMutex);
        mParams.getBinary(ofs);
        ofs.flush();
        ofs.close();
        return true;

    } catch (const std::ios_base::failure& e) {
        // Catch I/O errors (e.g., disk full, write protection)
        addError(std::format("I/O failure while writing {}: {}", filename, e.what()));
        return false;
    } catch (const std::exception& e) {
        // Catch any other unexpected errors
        addError(std::format("General error during saving: {}", e.what()));
        return false;
    }
}
//-----------------------------------------------------------------------------
// obsolete
// bool SFXGeneratorStereo::loadSettings(std::istream& is, uint8_t version) {
//
//     std::lock_guard<std::recursive_mutex> lock(mParamsMutex);
//
//
//     FILE* file=fopen(filename, "rb");
//     if(!file)
//         return false;
//
//     int version=0;
//     fread(&version, 1, sizeof(int), file);
//     if( version != 100 && version != 101 && version != 102 && version != 103 )
//         return false;
//
//     fread(&mParams.wave_type, 1, sizeof(int), file);
//
//     mParams.sound_vol=0.5f;
//     if(version>=102)
//         fread(&mParams.sound_vol, 1, sizeof(float), file);
//
//     fread(&mParams.p_base_freq, 1, sizeof(float), file);
//     fread(&mParams.p_freq_limit, 1, sizeof(float), file);
//     fread(&mParams.p_freq_ramp, 1, sizeof(float), file);
//     if(version>=101)
//         fread(&mParams.p_freq_dramp, 1, sizeof(float), file);
//     fread(&mParams.p_duty, 1, sizeof(float), file);
//     fread(&mParams.p_duty_ramp, 1, sizeof(float), file);
//
//     fread(&mParams.p_vib_strength, 1, sizeof(float), file);
//     fread(&mParams.p_vib_speed, 1, sizeof(float), file);
//     fread(&mParams.p_vib_delay, 1, sizeof(float), file);
//
//     fread(&mParams.p_env_attack, 1, sizeof(float), file);
//     fread(&mParams.p_env_sustain, 1, sizeof(float), file);
//     fread(&mParams.p_env_decay, 1, sizeof(float), file);
//     fread(&mParams.p_env_punch, 1, sizeof(float), file);
//
//     fread(&mParams.filter_on, 1, sizeof(bool), file);
//     fread(&mParams.p_f_resonance, 1, sizeof(float), file);
//     fread(&mParams.p_lpf_freq, 1, sizeof(float), file);
//     fread(&mParams.p_lpf_ramp, 1, sizeof(float), file);
//     fread(&mParams.p_hpf_freq, 1, sizeof(float), file);
//     fread(&mParams.p_hpf_ramp, 1, sizeof(float), file);
//
//     fread(&mParams.p_pha_offset, 1, sizeof(float), file);
//     fread(&mParams.p_pha_ramp, 1, sizeof(float), file);
//
//     fread(&mParams.p_repeat_speed, 1, sizeof(float), file);
//
//     if(version>=101)
//     {
//         fread(&mParams.p_arp_speed, 1, sizeof(float), file);
//         fread(&mParams.p_arp_mod, 1, sizeof(float), file);
//     }
//
//     if (version >= 103 ) { //panning
//         dLog("reading Version 103 params!");
//         fread(&mParams.p_pan, sizeof(float), 1, file);
//         fread(&mParams.p_pan_ramp, sizeof(float), 1, file);
//         fread(&mParams.p_pan_speed, sizeof(float), 1, file);
//     }
//
//     fclose(file);
//     return true;
// }
//-----------------------------------------------------------------------------
void SFXGeneratorStereo::ResetSample(bool restart)
{
    std::lock_guard<std::recursive_mutex> lock(mParamsMutex);
    if(!restart)
        mState.phase=0;
    mState.fperiod=100.0/(mParams.p_base_freq*mParams.p_base_freq+0.001);
    mState.period=(int)mState.fperiod;
    mState.fmaxperiod=100.0/(mParams.p_freq_limit*mParams.p_freq_limit+0.001);
    mState.fslide=1.0-pow((double)mParams.p_freq_ramp, 3.0)*0.01;
    mState.fdslide=-pow((double)mParams.p_freq_dramp, 3.0)*0.000001;
    mState.square_duty=0.5f-mParams.p_duty*0.5f;
    mState.square_slide=-mParams.p_duty_ramp*0.00005f;
    if(mParams.p_arp_mod>=0.0f)
        mState.arp_mod=1.0-pow((double)mParams.p_arp_mod, 2.0)*0.9;
    else
        mState.arp_mod=1.0+pow((double)mParams.p_arp_mod, 2.0)*10.0;
    mState.arp_time=0;
    mState.arp_limit=(int)(pow(1.0f-mParams.p_arp_speed, 2.0f)*20000+32);
    if(mParams.p_arp_speed==1.0f)
        mState.arp_limit=0;


    // 2026-02-05 sustain time < 0.01 makes problems when attack > 0
    if (mParams.p_env_sustain < 0.01f)
        mParams.p_env_sustain = 0.01f;



    if(!restart)
    {
        // reset filter
        mState.fltp=0.0f;
        mState.fltdp=0.0f;
        mState.fltw=pow(mParams.p_lpf_freq, 3.0f)*0.1f;
        mState.fltw_d=1.0f+mParams.p_lpf_ramp*0.0001f;
        mState.fltdmp=5.0f/(1.0f+pow(mParams.p_lpf_resonance, 2.0f)*20.0f)*(0.01f+mState.fltw);
        if(mState.fltdmp>0.8f) mState.fltdmp=0.8f;
        mState.fltphp=0.0f;
        mState.flthp=pow(mParams.p_hpf_freq, 2.0f)*0.1f;
        mState.flthp_d=1.0+mParams.p_hpf_ramp*0.0003f;
        // reset vibrato
        mState.vib_phase=0.0f;
        mState.vib_speed=pow(mParams.p_vib_speed, 2.0f)*0.01f;
        mState.vib_amp=mParams.p_vib_strength*0.5f;
        // reset envelope
        mState.env_vol=0.0f;
        mState.env_stage=0;
        mState.env_time=0;
        mState.env_length[0]=(int)(mParams.p_env_attack*mParams.p_env_attack*100000.0f);
        mState.env_length[1]=(int)(mParams.p_env_sustain*mParams.p_env_sustain*100000.0f);
        mState.env_length[2]=(int)(mParams.p_env_decay*mParams.p_env_decay*100000.0f);

        mState.fphase=pow(mParams.p_pha_offset, 2.0f)*1020.0f;
        if(mParams.p_pha_offset<0.0f) mState.fphase=-mState.fphase;
        mState.fdphase=pow(mParams.p_pha_ramp, 2.0f)*1.0f;
        if(mParams.p_pha_ramp<0.0f) mState.fdphase=-mState.fdphase;
        mState.iphase=abs((int)mState.fphase);
        if(mState.iphase>1023) mState.iphase=1023;
        mState.ipp=0;
        for(int i=0;i<1024;i++)
            mState.phaser_buffer[i]=0.0f;

        for(int i=0;i<32;i++)
            mState.noise_buffer[i]=frnd(2.0f)-1.0f;

        mState.rep_time=0;
        mState.rep_limit=(int)(pow(1.0f-mParams.p_repeat_speed, 2.0f)*20000+32);
        if(mParams.p_repeat_speed==0.0f)
            mState.rep_limit=0;

        //panning
        mState.pan = mParams.p_pan;
        // Scale the ramp to be meaningful per sample (adjust multiplier if needed)
        float speedMultiplier = pow(mParams.p_pan_speed, 2.0f) * 0.01f;
        // dLog("SPEED: %10.8f", speedMultiplier);
        mState.pan_ramp = mParams.p_pan_ramp * speedMultiplier;

    }
}
//-----------------------------------------------------------------------------
void SFXGeneratorStereo::PlaySample()
{
    ResetSample(false);
    mState.playing_sample=true;
}
//-----------------------------------------------------------------------------
// Returns a single mono sample (after supersampling, filters, and envelope)
float SFXGeneratorStereo::generateMonoTick() {
    float ssample = 0.0f;
    for (int si = 0; si < 8; si++) { // 8x supersampling
        float sample = 0.0f;
        mState.phase++;
        if (mState.phase >= mState.period) {
            mState.phase %= mState.period;
            if (mParams.wave_type == 3) {
                for (int n = 0; n < 32; n++)
                    mState.noise_buffer[n] = frnd(2.0f) - 1.0f;
            }
        }

        // --- Waveform Generation ---
        float fp = (float)mState.phase / mState.period;
        switch (mParams.wave_type) {
            case 0: sample = fp < mState.square_duty ? 0.5f : -0.5f; break;
            case 1: sample = 1.0f - fp * 2; break;
            case 2: sample = (float)sin(fp * 2 * M_PI); break;
            case 3: sample = mState.noise_buffer[mState.phase * 32 / mState.period]; break;
        }

        // --- Filters ---
        float pp = mState.fltp;
        mState.fltw *= mState.fltw_d;
        if (mState.fltw < 0.0f) mState.fltw = 0.0f;
        if (mState.fltw > 0.1f) mState.fltw = 0.1f;
        if (mParams.p_lpf_freq != 1.0f) {
            mState.fltdp += (sample - mState.fltp) * mState.fltw;
            mState.fltdp -= mState.fltdp * mState.fltdmp;
        } else {
            mState.fltp = sample;
            mState.fltdp = 0.0f;
        }
        mState.fltp += mState.fltdp;
        mState.fltphp += mState.fltp - pp;
        mState.fltphp -= mState.fltphp * mState.flthp;
        sample = mState.fltphp;

        // --- Phaser ---
        mState.phaser_buffer[mState.ipp & 1023] = sample;
        sample += mState.phaser_buffer[(mState.ipp - mState.iphase + 1024) & 1023];
        mState.ipp = (mState.ipp + 1) & 1023;

        ssample += sample * mState.env_vol;
    }
    return (ssample / 8.0f);
}
//-----------------------------------------------------------------------------
void SFXGeneratorStereo::updateSystemState() {
    mState.rep_time++;
    if (mState.rep_limit != 0 && mState.rep_time >= mState.rep_limit) {
        mState.rep_time = 0;
        ResetSample(true);
    }

    // Frequency envelopes / Arpeggios
    mState.arp_time++;
    if (mState.arp_limit != 0 && mState.arp_time >= mState.arp_limit) {
        mState.arp_limit = 0;
        mState.fperiod *= mState.arp_mod;
    }
    mState.fslide += mState.fdslide;
    mState.fperiod *= mState.fslide;
    if (mState.fperiod > mState.fmaxperiod) {
        mState.fperiod = mState.fmaxperiod;
        if (mParams.p_freq_limit > 0.0f) mState.playing_sample = false;
    }

    // Vibrato
    float rfperiod = mState.fperiod;
    if (mState.vib_amp > 0.0f) {
        mState.vib_phase += mState.vib_speed;
        rfperiod = mState.fperiod * (1.0 + sin(mState.vib_phase) * mState.vib_amp);
    }
    mState.period = (int)rfperiod;
    if (mState.period < 8) mState.period = 8;

    mState.square_duty += mState.square_slide;
    if (mState.square_duty < 0.0f) mState.square_duty = 0.0f;
    if (mState.square_duty > 0.5f) mState.square_duty = 0.5f;

    // Volume envelope
    mState.env_time++;
    if (mState.env_time > mState.env_length[mState.env_stage]) {
        mState.env_time = 0;
        mState.env_stage++;
        if (mState.env_stage == 3) mState.playing_sample = false;
    }
    if (mState.env_stage == 0) mState.env_vol = (float)mState.env_time / mState.env_length[0];
    if (mState.env_stage == 1) mState.env_vol = 1.0f + pow(1.0f - (float)mState.env_time / mState.env_length[1], 1.0f) * 2.0f * mParams.p_env_punch;
    if (mState.env_stage == 2) mState.env_vol = 1.0f - (float)mState.env_time / mState.env_length[2];

    // Other updates
    mState.fphase += mState.fdphase;
    mState.iphase = abs((int)mState.fphase);
    if (mState.iphase > 1023) mState.iphase = 1023;
    if (mState.flthp_d != 0.0f) {
        mState.flthp *= mState.flthp_d;
        if (mState.flthp < 0.00001f) mState.flthp = 0.00001f;
        if (mState.flthp > 0.1f) mState.flthp = 0.1f;
    }
}
//-----------------------------------------------------------------------------
void SFXGeneratorStereo::SynthSample(int length, float* stereoBuffer) {

    if (!mState.playing_sample)  {
        std::memset(stereoBuffer, 0, length * sizeof(float) * 2);
        return;
    }


    // double lock !! std::lock_guard<std::recursive_mutex> lock(mParamsMutex);

    for (int i = 0; i < length; i++) {
        if (!mState.playing_sample) break;

        // Part 1: Update the simulation
        updateSystemState();

        // Part 2: Generate the mono core signal
        float ssample = generateMonoTick();

        // Part 3: Global Volume & Panning
        ssample *= master_vol * 2.0f * mParams.sound_vol;
        if (ssample > 1.0f) ssample = 1.0f;
        if (ssample < -1.0f) ssample = -1.0f;

        // Panning movement
        mState.pan += mState.pan_ramp;

        if (mState.pan >= 1.0f) {
            mState.pan = 1.0f;
            mState.pan_ramp = -mState.pan_ramp; // back
        }
        else if (mState.pan <= -1.0f) {
            mState.pan = -1.0f;
            mState.pan_ramp = -mState.pan_ramp; // back
        }

        // mState.pan += mState.pan_ramp;
        // if (mState.pan < -1.0f) mState.pan = -1.0f;
        // if (mState.pan > 1.0f)  mState.pan = 1.0f;


        float panL = 0.5f * (1.0f - mState.pan);
        float panR = 0.5f * (1.0f + mState.pan);

        // Part 4: Output
        if (stereoBuffer != nullptr) {
            stereoBuffer[i * 2]     = ssample * panL;
            stereoBuffer[i * 2 + 1] = ssample * panR;
        }
    }
}


//-----------------------------------------------------------------------------
void SFXGeneratorStereo::GeneratePickupCoin(){
    std::lock_guard<std::recursive_mutex> lock(mParamsMutex);
    ResetParamsNoLock();
    mParams.p_base_freq =  0.4f + frnd(0.5f);
    mParams.p_env_attack = 0.0f;
    mParams.p_env_sustain = frnd(0.1f);
    mParams.p_env_decay = 0.1f + frnd(0.4f);
    mParams.p_env_punch = 0.3f + frnd(0.3f);
    if (rnd(1)) {
        mParams.p_arp_speed = 0.5f + frnd(0.2f);
        mParams.p_arp_mod = 0.2f + frnd(0.4f);
    }
    mParams.setName("Pickup Coin");
}
//-----------------------------------------------------------------------------
void SFXGeneratorStereo::GenerateLaserShoot() {
     std::lock_guard<std::recursive_mutex> lock(mParamsMutex);
    ResetParamsNoLock();

    mParams.setName("Laser Shoot");

    mParams.wave_type = rnd(2);
    if (mParams.wave_type == 2 && rnd(1))
        mParams.wave_type = rnd(1);

    mParams.p_base_freq = 0.5f + frnd(0.5f);
    mParams.p_freq_limit = mParams.p_base_freq - 0.2f - frnd(0.6f);
    if (mParams.p_freq_limit < 0.2f) mParams.p_freq_limit = 0.2f;
    mParams.p_freq_ramp = -0.15f - frnd(0.2f);

    if (rnd(2) == 0) {
        mParams.p_base_freq = 0.3f + frnd(0.6f);
        mParams.p_freq_limit = frnd(0.1f);
        mParams.p_freq_ramp = -0.35f - frnd(0.3f);
    }

    if (rnd(1)) {
        mParams.p_duty = frnd(0.5f);
        mParams.p_duty_ramp = frnd(0.2f);
    } else {
        mParams.p_duty = 0.4f + frnd(0.5f);
        mParams.p_duty_ramp = -frnd(0.7f);
    }

    mParams.p_env_attack = 0.0f;
    mParams.p_env_sustain = 0.1f + frnd(0.2f);
    mParams.p_env_decay = frnd(0.4f);
    if (rnd(1)) mParams.p_env_punch = frnd(0.3f);

    if (rnd(2) == 0) {
        mParams.p_pha_offset = frnd(0.2f);
        mParams.p_pha_ramp = -frnd(0.2f);
    }
    if (rnd(1)) mParams.p_hpf_freq = frnd(0.3f);
}
//-----------------------------------------------------------------------------
void SFXGeneratorStereo::GenerateExplosion() {
     std::lock_guard<std::recursive_mutex> lock(mParamsMutex);
    ResetParamsNoLock();
    mParams.setName("Explosion");
    mParams.wave_type = 3;
    if (rnd(1)) {
        mParams.p_base_freq = 0.1f + frnd(0.4f);
        mParams.p_freq_ramp = -0.1f + frnd(0.4f);
    } else {
        mParams.p_base_freq = 0.2f + frnd(0.7f);
        mParams.p_freq_ramp = -0.2f - frnd(0.2f);
    }
    mParams.p_base_freq *= mParams.p_base_freq;

    if (rnd(4) == 0) mParams.p_freq_ramp = 0.0f;
    if (rnd(2) == 0) mParams.p_repeat_speed = 0.3f + frnd(0.5f);

    mParams.p_env_attack = 0.0f;
    mParams.p_env_sustain = 0.1f + frnd(0.3f);
    mParams.p_env_decay = frnd(0.5f);

    if (rnd(1) == 0) {
        mParams.p_pha_offset = -0.3f + frnd(0.9f);
        mParams.p_pha_ramp = -frnd(0.3f);
    }
    mParams.p_env_punch = 0.2f + frnd(0.6f);

    if (rnd(1)) {
        mParams.p_vib_strength = frnd(0.7f);
        mParams.p_vib_speed = frnd(0.6f);
    }
    if (rnd(2) == 0) {
        mParams.p_arp_speed = 0.6f + frnd(0.3f);
        mParams.p_arp_mod = 0.8f - frnd(1.6f);
    }
}
//-----------------------------------------------------------------------------
void SFXGeneratorStereo::GeneratePowerup() {
     std::lock_guard<std::recursive_mutex> lock(mParamsMutex);
    ResetParamsNoLock();
    mParams.setName("Powerup");

    if (rnd(1)) mParams.wave_type = 1;
    else mParams.p_duty = frnd(0.6f);

    if (rnd(1)) {
        mParams.p_base_freq = 0.2f + frnd(0.3f);
        mParams.p_freq_ramp = 0.1f + frnd(0.4f);
        mParams.p_repeat_speed = 0.4f + frnd(0.4f);
    } else {
        mParams.p_base_freq = 0.2f + frnd(0.3f);
        mParams.p_freq_ramp = 0.05f + frnd(0.2f);
        if (rnd(1)) {
            mParams.p_vib_strength = frnd(0.7f);
            mParams.p_vib_speed = frnd(0.6f);
        }
    }
    mParams.p_env_attack = 0.0f;
    mParams.p_env_sustain = frnd(0.4f);
    mParams.p_env_decay = 0.1f + frnd(0.4f);
}
//-----------------------------------------------------------------------------
void SFXGeneratorStereo::GenerateHitHurt() {
     std::lock_guard<std::recursive_mutex> lock(mParamsMutex);
    ResetParamsNoLock();
    mParams.setName("HitHurt");
    mParams.wave_type = rnd(2);
    if (mParams.wave_type == 2) mParams.wave_type = 3;
    if (mParams.wave_type == 0) mParams.p_duty = frnd(0.6f);

    mParams.p_base_freq = 0.2f + frnd(0.6f);
    mParams.p_freq_ramp = -0.3f - frnd(0.4f);
    mParams.p_env_attack = 0.0f;
    mParams.p_env_sustain = frnd(0.1f);
    mParams.p_env_decay = 0.1f + frnd(0.2f);
    if (rnd(1)) mParams.p_hpf_freq = frnd(0.3f);
}
//-----------------------------------------------------------------------------
void SFXGeneratorStereo::GenerateJump(){
     std::lock_guard<std::recursive_mutex> lock(mParamsMutex);
    ResetParamsNoLock();
    mParams.setName("Jump");
    mParams.wave_type = 0;
    mParams.p_duty = frnd(0.6f);
    mParams.p_base_freq = 0.3f + frnd(0.3f);
    mParams.p_freq_ramp = 0.1f + frnd(0.2f);
    mParams.p_env_attack = 0.0f;
    mParams.p_env_sustain = 0.1f + frnd(0.3f);
    mParams.p_env_decay = 0.1f + frnd(0.2f);
    if (rnd(1)) mParams.p_hpf_freq = frnd(0.3f);
    if (rnd(1)) mParams.p_lpf_freq = 1.0f - frnd(0.6f);
}
//-----------------------------------------------------------------------------
void SFXGeneratorStereo::GenerateBlipSelect(){
     std::lock_guard<std::recursive_mutex> lock(mParamsMutex);
    ResetParamsNoLock();
    mParams.setName("Blip Select");
    mParams.wave_type = rnd(1);
    if (mParams.wave_type == 0) mParams.p_duty = frnd(0.6f);
    mParams.p_base_freq = 0.2f + frnd(0.4f);
    mParams.p_env_attack = 0.0f;
    mParams.p_env_sustain = 0.1f + frnd(0.1f);
    mParams.p_env_decay = frnd(0.2f);
    mParams.p_hpf_freq = 0.1f;
}
//-----------------------------------------------------------------------------
void SFXGeneratorStereo::Randomize() {
     std::lock_guard<std::recursive_mutex> lock(mParamsMutex);
     mParams.setName("Random");

    mParams.p_base_freq = pow(frnd(1.0f), 2.0f);
    // mParams.p_base_freq = pow(frnd(2.0f) - 1.0f, 2.0f);


    if (rnd(1)) {
        mParams.p_base_freq = pow(frnd(2.0f) - 1.0f, 3.0f) + 0.5f;
    }


    mParams.p_freq_limit = 0.0f;
    mParams.p_freq_ramp = pow(frnd(2.0f) - 1.0f, 5.0f);

    if (mParams.p_base_freq > 0.7f && mParams.p_freq_ramp > 0.2f)
        mParams.p_freq_ramp = -mParams.p_freq_ramp;
    if (mParams.p_base_freq < 0.2f && mParams.p_freq_ramp < -0.05f)
        mParams.p_freq_ramp = -mParams.p_freq_ramp;

    mParams.p_freq_dramp = pow(frnd(2.0f) - 1.0f, 3.0f);
    mParams.p_duty = frnd(2.0f) - 1.0f;
    mParams.p_duty_ramp = pow(frnd(2.0f) - 1.0f, 3.0f);
    mParams.p_vib_strength = pow(frnd(2.0f) - 1.0f, 3.0f);
    mParams.p_vib_speed = frnd(2.0f) - 1.0f;
    mParams.p_vib_delay = frnd(2.0f) - 1.0f;

    mParams.p_env_attack = pow(frnd(1.0f), 3.0f);
    mParams.p_env_sustain = pow(frnd(1.0f), 2.0f);
    mParams.p_env_decay = frnd(1.0f);
    // orig:
    // mParams.p_env_attack = pow(frnd(2.0f) - 1.0f, 3.0f);
    // mParams.p_env_sustain = pow(frnd(2.0f) - 1.0f, 2.0f);
    // mParams.p_env_decay = frnd(2.0f) - 1.0f;
    mParams.p_env_punch = pow(frnd(0.8f), 2.0f);

    if (mParams.p_env_attack + mParams.p_env_sustain + mParams.p_env_decay < 0.2f) {
        mParams.p_env_sustain += 0.2f + frnd(0.3f);
        mParams.p_env_decay += 0.2f + frnd(0.3f);
    }

    mParams.p_lpf_resonance = std::abs( frnd(2.0f) - 1.0f);
    mParams.p_lpf_freq = 1.0f - pow(frnd(1.0f), 3.0f);
    mParams.p_lpf_ramp = pow(frnd(2.0f) - 1.0f, 3.0f);

    if (mParams.p_lpf_freq < 0.1f && mParams.p_lpf_ramp < -0.05f)
        mParams.p_lpf_ramp = -mParams.p_lpf_ramp;

    mParams.p_hpf_freq = pow(frnd(1.0f), 5.0f);
    mParams.p_hpf_ramp = pow(frnd(2.0f) - 1.0f, 5.0f);
    mParams.p_pha_offset = pow(frnd(2.0f) - 1.0f, 3.0f);
    mParams.p_pha_ramp = pow(frnd(2.0f) - 1.0f, 3.0f);
    mParams.p_repeat_speed = frnd(2.0f) - 1.0f;
    mParams.p_arp_speed = frnd(2.0f) - 1.0f;
    mParams.p_arp_mod = frnd(2.0f) - 1.0f;

    AddPanning(false);

    ResetSample(false); // normalize values !!

}
//-----------------------------------------------------------------------------
void SFXGeneratorStereo::Mutate(){
     std::lock_guard<std::recursive_mutex> lock(mParamsMutex);
    if (rnd(1)) mParams.p_base_freq = std::abs(mParams.p_base_freq +  frnd(0.1f) - 0.05f);
    // if (rnd(1)) mParams.p_base_freq += frnd(0.1f) - 0.05f;
    if (rnd(1)) mParams.p_freq_ramp += frnd(0.1f) - 0.05f;
    if (rnd(1)) mParams.p_freq_dramp += frnd(0.1f) - 0.05f;
    if (rnd(1)) mParams.p_duty += frnd(0.1f) - 0.05f;
    if (rnd(1)) mParams.p_duty_ramp += frnd(0.1f) - 0.05f;
    if (rnd(1)) mParams.p_vib_strength += frnd(0.1f) - 0.05f;
    if (rnd(1)) mParams.p_vib_speed += frnd(0.1f) - 0.05f;
    if (rnd(1)) mParams.p_vib_delay += frnd(0.1f) - 0.05f;


    if (rnd(1)) mParams.p_env_attack  = std::abs(mParams.p_env_attack + frnd(0.1f) - 0.05f);
    if (rnd(1)) mParams.p_env_sustain = std::abs(mParams.p_env_sustain + frnd(0.1f) - 0.05f);
    if (rnd(1)) mParams.p_env_decay   = std::abs(mParams.p_env_decay + frnd(0.1f) - 0.05f);

    // if (rnd(1)) mParams.p_env_attack += frnd(0.1f) - 0.05f;
    // if (rnd(1)) mParams.p_env_sustain += frnd(0.1f) - 0.05f;
    // if (rnd(1)) mParams.p_env_decay += frnd(0.1f) - 0.05f;

    if (rnd(1)) mParams.p_env_punch += frnd(0.1f) - 0.05f;

    if (rnd(1)) mParams.p_lpf_resonance = std::abs( mParams.p_lpf_resonance + frnd(0.1f) - 0.05f);
    // if (rnd(1)) mParams.p_lpf_resonance += frnd(0.1f) - 0.05f;
    if (rnd(1)) mParams.p_lpf_freq += frnd(0.1f) - 0.05f;
    if (rnd(1)) mParams.p_lpf_ramp += frnd(0.1f) - 0.05f;
    if (rnd(1)) mParams.p_hpf_freq += frnd(0.1f) - 0.05f;
    if (rnd(1)) mParams.p_hpf_ramp += frnd(0.1f) - 0.05f;
    if (rnd(1)) mParams.p_pha_offset += frnd(0.1f) - 0.05f;
    if (rnd(1)) mParams.p_pha_ramp += frnd(0.1f) - 0.05f;
    if (rnd(1)) mParams.p_repeat_speed += frnd(0.1f) - 0.05f;
    if (rnd(1)) mParams.p_arp_speed += frnd(0.1f) - 0.05f;
    if (rnd(1)) mParams.p_arp_mod += frnd(0.1f) - 0.05f;

    // Panning Randomization
    // use extra function !! if (rnd(1)) AddPanning(false);

    ResetSample(false); // normalize values !!

}
//------------------------------------------------------------------------------
// PANNING _D
void SFXGeneratorStereo::AddPanning(bool doLock) {
    std::unique_lock<std::recursive_mutex> lock(mParamsMutex, std::defer_lock);
    if (doLock) {
        lock.lock();
    }

    // --- Panning Randomization ---
    if (frnd(1.0f) > 0.5f) {
        mParams.p_pan = frnd(2.0f) - 1.0f;
    } else {
        mParams.p_pan = 0.0f;
    }

    if (frnd(1.0f) > 0.7f) {
        mParams.p_pan_ramp = frnd(2.0f) - 1.0f;
        // If we have a ramp, we need some speed, otherwise it stays static
        mParams.p_pan_speed = frnd(0.8f) + 0.2f;
    } else {
        mParams.p_pan_ramp = 0.0f;
        mParams.p_pan_speed = 0.0f;
    }
}

//------------------------------------------------------------------------------
// --- SDL
//------------------------------------------------------------------------------
// void SDLCALL SFXGeneratorStereo::audio_callback(void* userdata, SDL_AudioStream* stream, int additional_amount, int total_amount)
// {
//     if (!userdata) return;
//     auto* gen = static_cast<SFXGeneratorStereo*>(userdata);
//
//     if (!gen)
//         return;
//
//     int frames_needed = additional_amount / (sizeof(float) * 2);
//
//
//     if (frames_needed > 0)
//     {
//         std::lock_guard<std::recursive_mutex> lock(gen->mParamsMutex);
//         if (gen->mState.playing_sample) {
//             std::vector<float> stereoBuffer(frames_needed * 2, 0.0f);
//             gen->SynthSample(frames_needed, stereoBuffer.data());
//
//             for (auto& effect : gen->mDspEffects) {
//                 effect->process(stereoBuffer.data(), frames_needed * 2);
//             }
//
//             SDL_PutAudioStreamData(stream, stereoBuffer.data(), additional_amount);
//         }
//     }
// }
void SDLCALL SFXGeneratorStereo::audio_callback(void* userdata, SDL_AudioStream* stream, int additional_amount, int total_amount)
{
    if (!userdata) return;
    auto* gen = static_cast<SFXGeneratorStereo*>(userdata);

    if (!gen)
        return;

    int frames_needed = additional_amount / (sizeof(float) * 2);

    if (frames_needed > 0)
    {
        std::lock_guard<std::recursive_mutex> lock(gen->mParamsMutex);
        // if (gen->mState.playing_sample)
        {
            std::vector<float> stereoBuffer(frames_needed * 2, 0.0f);
            gen->SynthSample(frames_needed, stereoBuffer.data());

            for (auto& effect : gen->mDspEffects) {
                effect->process(stereoBuffer.data(), frames_needed * 2);
            }

            // SDL_PutAudioStreamData(stream, stereoBuffer.data(), additional_amount);
            SDL_PutAudioStreamData(stream, stereoBuffer.data(), frames_needed * 2 * sizeof(float));
        }
    }
}

//------------------------------------------------------------------------------
bool SFXGeneratorStereo::exportToWav(const std::string& filename, float* progressOut, bool applyEffects) {
    detachAudio();
    std::lock_guard<std::recursive_mutex> lock(mParamsMutex);

    int sampleRate = 44100;
    int chunkSize = 1024;

    // 1. Synth Duration
    int attack = (int)(mParams.p_env_attack * mParams.p_env_attack * 100000.0f);
    int sustain = (int)(mParams.p_env_sustain * mParams.p_env_sustain * 100000.0f);
    int decay = (int)(mParams.p_env_decay * mParams.p_env_decay * 100000.0f);
    int synthFrames = attack + sustain + decay + 100;

    // 2. Calculate Tail Duration (Dynamic)
    int tailFrames = 0;
    if (applyEffects) {
        float maxTailSec = 0.0f;
        for (auto& effect : mDspEffects) {
            maxTailSec = std::max(maxTailSec, effect->getTailLengthSeconds());
        }
        maxTailSec = std::min(maxTailSec, 10.0f); // Cap at 10s
        tailFrames = static_cast<int>(maxTailSec * sampleRate);
    }

    int totalFrames = synthFrames + tailFrames;

    // 3. Allocate Buffer (Pre-filled with 0.0f)
    std::vector<float> f32ExportBuffer(totalFrames * 2, 0.0f);

    // 4. Reset Generator
    ResetSample(false);
    mState.playing_sample = true;

    // 5. Processing Loop
    int framesProcessed = 0;
    while (framesProcessed < totalFrames) {
        int toWrite = std::min(chunkSize, totalFrames - framesProcessed);

        // Only call SynthSample as long as the synth is actually playing
        if (framesProcessed < synthFrames && mState.playing_sample) {
            this->SynthSample(toWrite, &f32ExportBuffer[framesProcessed * 2]);
        } else {
            // We are in the Tail area: just "process" silence (do nothing, buffer is already 0)
        }

        framesProcessed += toWrite;

        if (progressOut) {
            float progressScale = applyEffects ? 0.70f : 1.0f;
            *progressOut = ((float)framesProcessed / (float)totalFrames) * progressScale;
        }
    }

    // 6. Apply Effects to the WHOLE buffer (including the silent tail)
    if (applyEffects) {
        if (progressOut) *progressOut = 0.75f;

        for (auto& effect : this->mDspEffects) {
            // This now processes the tail where the echoes will appear
            effect->process(f32ExportBuffer.data(), totalFrames * 2);
        }

        if (progressOut) *progressOut = 0.85f;
        DSP::normalizeBuffer(f32ExportBuffer.data(), f32ExportBuffer.size(), 0.98f);
        if (progressOut) *progressOut = 0.90f;
    }

    if (progressOut) *progressOut = 1.0f;

    attachAudio();
    return saveWavFile(filename, f32ExportBuffer, sampleRate);
}

// bool SFXGeneratorStereo::exportToWav(const std::string& filename, float* progressOut, bool applyEffects ) {
//     detachAudio();
//     std::lock_guard<std::recursive_mutex> lock(mParamsMutex);
//
//
//     // 1. Calculate total duration from envelope
//     int attack = (int)(mParams.p_env_attack * mParams.p_env_attack * 100000.0f);
//     int sustain = (int)(mParams.p_env_sustain * mParams.p_env_sustain * 100000.0f);
//     int decay = (int)(mParams.p_env_decay * mParams.p_env_decay * 100000.0f);
//
//     int totalFrames = attack + sustain + decay + 100;
//
//     int sampleRate = 44100;
//     int chunkSize = 1024;
//
//     // 2. Allocate Stereo Float Buffer (Size: Frames * 2)
//     std::vector<float> f32ExportBuffer(totalFrames * 2, 0.0f);
//
//     // 3. Reset Generator State
//     ResetSample(false);
//     mState.playing_sample = true;
//
//     int framesProcessed = 0;
//     while (framesProcessed < totalFrames && mState.playing_sample) {
//         int toWrite = std::min(chunkSize, totalFrames - framesProcessed);
//
//         this->SynthSample(toWrite, &f32ExportBuffer[framesProcessed * 2]);
//         framesProcessed += toWrite;
//         if (progressOut) {
//             // *progressOut = (float)framesProcessed / (float)totalFrames;
//             float progressScale = applyEffects ? 0.70f : 1.0f;
//             *progressOut = (float)framesProcessed / (float)totalFrames * progressScale;
//         }
//     }
//
//     // 4. Effects
//     if (applyEffects) {
//         if (progressOut) *progressOut = 0.75f;
//
//         // Apply DSP directly to the master float buffer
//         for (auto& effect : this->mDspEffects) {
//             effect->process(f32ExportBuffer.data(), f32ExportBuffer.size());
//         }
//         if (progressOut) *progressOut = 0.85f;
//
//         // Normalize
//         DSP::normalizeBuffer(f32ExportBuffer.data(), f32ExportBuffer.size(), 0.98f);
//         if (progressOut) *progressOut = 0.90f;
//     }
//     if (progressOut) *progressOut = 1.0f;
//
//     attachAudio();
//     // Resize to actual frames written (multiplied by 2 for stereo)
//     f32ExportBuffer.resize(framesProcessed * 2);
//
//     return saveWavFile(filename, f32ExportBuffer, sampleRate);
// }
//------------------------------------------------------------------------------
bool SFXGeneratorStereo::saveWavFile(const std::string& filename, const std::vector<float>& data, int sampleRate)  {
    // Open the file for writing using SDL3's IO system
    SDL_IOStream* io = SDL_IOFromFile(filename.c_str(), "wb");
    if (!io) {
        LogFMT("ERROR:Failed to open file for writing: %s", SDL_GetError());
        return false;
    }

    dLog("[info] SFXGenerator::saveWavFile (32-bit Float).....");

    uint32_t numChannels = 2;
    uint32_t bitsPerSample = 32; // 32 bits for float
    uint32_t dataSize = (uint32_t)(data.size() * sizeof(float));
    uint32_t fileSize = 36 + dataSize;
    uint32_t byteRate = sampleRate * numChannels * (bitsPerSample / 8);
    uint16_t blockAlign = (uint16_t)(numChannels * (bitsPerSample / 8));

    // Write the WAV Header
    SDL_WriteIO(io, "RIFF", 4);
    SDL_WriteU32LE(io, fileSize);
    SDL_WriteIO(io, "WAVE", 4);
    SDL_WriteIO(io, "fmt ", 4);
    SDL_WriteU32LE(io, 16);

    // IMPORTANT: AudioFormat 3 is IEEE Float (instead of 1 for PCM)
    SDL_WriteU16LE(io, 3);

    SDL_WriteU16LE(io, (uint16_t)numChannels);
    SDL_WriteU32LE(io, (uint32_t)sampleRate);
    SDL_WriteU32LE(io, byteRate);
    SDL_WriteU16LE(io, blockAlign);
    SDL_WriteU16LE(io, (uint16_t)bitsPerSample);

    SDL_WriteIO(io, "data", 4);
    SDL_WriteU32LE(io, dataSize);

    // Write the raw float data directly - no conversion needed!
    SDL_WriteIO(io, data.data(), dataSize);

    SDL_CloseIO(io);
    LogFMT("Successfully exported WAV {}", filename);

    return true;
}
//------------------------------------------------------------------------------
void SFXGeneratorStereo::detachAudio(){
    dLog("SFXGeneratorStereo::detachAudio");
    SDL_PauseAudioStreamDevice(mStream);
    SDL_SetAudioStreamGetCallback(mStream, NULL, NULL);
}
void SFXGeneratorStereo::attachAudio() {
    dLog("SFXGeneratorStereo::attachAudio");
    SDL_SetAudioStreamGetCallback(mStream, SFXGeneratorStereo::audio_callback, this);
    SDL_ResumeAudioStreamDevice(mStream);
}


//------------------------------------------------------------------------------
bool SFXGeneratorStereo::initSDLAudio()
{
    if ( mStream ) {
        Log("[error] SFXGeneratorStereo::initSDLAudio stream is ready do NOT initSDLAudio again!!!!");
        return false;
    }

    SDL_AudioSpec spec;
    spec.format = SDL_AUDIO_F32;
    spec.channels = 2;
    spec.freq =  44100 ;
    mStream = SDL_CreateAudioStream(&spec, &spec);


    if (!mStream)
    {
        Log("SDL_OpenAudioDeviceStream failed: %s", SDL_GetError());
        return false;
    }

    SDL_SetAudioStreamGetCallback(mStream, SFXGeneratorStereo::audio_callback, this);



    #ifdef FLUX_ENGINE
    AudioManager.bindStream(mStream);
    #else
    SDL_AudioDeviceID dev = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, NULL);

    if (dev == 0) {
        Log("Failed to open audio device: %s", SDL_GetError());
        return false;
    }

    if (!SDL_BindAudioStream(dev, mStream)) {
        Log("SDL_OpenAudioDeviceStream failed to bind stream: %s", SDL_GetError());
        return false;
    }
    #endif

    SDL_ResumeAudioStreamDevice(mStream);
    Log("SFXGeneratorStereo initialized..");
    return true;
}

