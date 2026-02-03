//-----------------------------------------------------------------------------
// Copyright (c) 2007 Tomas Pettersson
// Copyright (c) 2026 XXTH
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#include "SFXGenerator.h"
#include <SDL3/SDL.h>
#include <mutex>

#ifdef FLUX_ENGINE
#include <audio/fluxAudio.h>
#endif


#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif
//-----------------------------------------------------------------------------
// A helper function for random numbers
int SFXGenerator::rnd(int n) {
    if (n <= 0) return 0;
    // uniform_int_distribution includes both 0 and n
    std::uniform_int_distribution<int> dist(0, n);
    return dist(m_rand_engine);
}

float SFXGenerator::frnd(float range) {
    // 2. Define the distribution (uniform float between 0.0 and 1.0)
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    // 3. Generate the number and scale it
    return dist(m_rand_engine) * range;
}
//-----------------------------------------------------------------------------
SFXGenerator::SFXGenerator():
    m_rand_engine(std::random_device{}())
{


    master_vol = 0.5f; //0.05f;
    sound_vol = 0.5f;
    wav_bits = 16;
    wav_freq = 44100;

    file_sampleswritten = 0;
    filesample = 0.0f;
    fileacc = 0;

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
}

SFXGenerator::~SFXGenerator()
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
void SFXGenerator::ResetParams()
{
    std::lock_guard<std::recursive_mutex> lock(mParamsMutex);
    ResetParamsNoLock();
}
void SFXGenerator::ResetParamsNoLock()
{
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
}
//-----------------------------------------------------------------------------
bool SFXGenerator::LoadSettings(const char* filename)
{
    std::lock_guard<std::recursive_mutex> lock(mParamsMutex);

    FILE* file=fopen(filename, "rb");
    if(!file)
        return false;

    int version=0;
    fread(&version, 1, sizeof(int), file);
    if(version!=100 && version!=101 && version!=102)
        return false;

    fread(&mParams.wave_type, 1, sizeof(int), file);

    sound_vol=0.5f;
    if(version==102)
        fread(&sound_vol, 1, sizeof(float), file);

    fread(&mParams.p_base_freq, 1, sizeof(float), file);
    fread(&mParams.p_freq_limit, 1, sizeof(float), file);
    fread(&mParams.p_freq_ramp, 1, sizeof(float), file);
    if(version>=101)
        fread(&mParams.p_freq_dramp, 1, sizeof(float), file);
    fread(&mParams.p_duty, 1, sizeof(float), file);
    fread(&mParams.p_duty_ramp, 1, sizeof(float), file);

    fread(&mParams.p_vib_strength, 1, sizeof(float), file);
    fread(&mParams.p_vib_speed, 1, sizeof(float), file);
    fread(&mParams.p_vib_delay, 1, sizeof(float), file);

    fread(&mParams.p_env_attack, 1, sizeof(float), file);
    fread(&mParams.p_env_sustain, 1, sizeof(float), file);
    fread(&mParams.p_env_decay, 1, sizeof(float), file);
    fread(&mParams.p_env_punch, 1, sizeof(float), file);

    fread(&mParams.filter_on, 1, sizeof(bool), file);
    fread(&mParams.p_lpf_resonance, 1, sizeof(float), file);
    fread(&mParams.p_lpf_freq, 1, sizeof(float), file);
    fread(&mParams.p_lpf_ramp, 1, sizeof(float), file);
    fread(&mParams.p_hpf_freq, 1, sizeof(float), file);
    fread(&mParams.p_hpf_ramp, 1, sizeof(float), file);
    
    fread(&mParams.p_pha_offset, 1, sizeof(float), file);
    fread(&mParams.p_pha_ramp, 1, sizeof(float), file);

    fread(&mParams.p_repeat_speed, 1, sizeof(float), file);

    if(version>=101)
    {
        fread(&mParams.p_arp_speed, 1, sizeof(float), file);
        fread(&mParams.p_arp_mod, 1, sizeof(float), file);
    }

    fclose(file);
    return true;
}
//-----------------------------------------------------------------------------
bool SFXGenerator::SaveSettings(const char* filename)
{
    std::lock_guard<std::recursive_mutex> lock(mParamsMutex);

    FILE* file=fopen(filename, "wb");
    if(!file)
        return false;

    int version=102;
    fwrite(&version, 1, sizeof(int), file);

    fwrite(&mParams.wave_type, 1, sizeof(int), file);

    fwrite(&sound_vol, 1, sizeof(float), file);

    fwrite(&mParams.p_base_freq, 1, sizeof(float), file);
    fwrite(&mParams.p_freq_limit, 1, sizeof(float), file);
    fwrite(&mParams.p_freq_ramp, 1, sizeof(float), file);
    fwrite(&mParams.p_freq_dramp, 1, sizeof(float), file);
    fwrite(&mParams.p_duty, 1, sizeof(float), file);
    fwrite(&mParams.p_duty_ramp, 1, sizeof(float), file);

    fwrite(&mParams.p_vib_strength, 1, sizeof(float), file);
    fwrite(&mParams.p_vib_speed, 1, sizeof(float), file);
    fwrite(&mParams.p_vib_delay, 1, sizeof(float), file);

    fwrite(&mParams.p_env_attack, 1, sizeof(float), file);
    fwrite(&mParams.p_env_sustain, 1, sizeof(float), file);
    fwrite(&mParams.p_env_decay, 1, sizeof(float), file);
    fwrite(&mParams.p_env_punch, 1, sizeof(float), file);

    fwrite(&mParams.filter_on, 1, sizeof(bool), file);
    fwrite(&mParams.p_lpf_resonance, 1, sizeof(float), file);
    fwrite(&mParams.p_lpf_freq, 1, sizeof(float), file);
    fwrite(&mParams.p_lpf_ramp, 1, sizeof(float), file);
    fwrite(&mParams.p_hpf_freq, 1, sizeof(float), file);
    fwrite(&mParams.p_hpf_ramp, 1, sizeof(float), file);
    
    fwrite(&mParams.p_pha_offset, 1, sizeof(float), file);
    fwrite(&mParams.p_pha_ramp, 1, sizeof(float), file);

    fwrite(&mParams.p_repeat_speed, 1, sizeof(float), file);

    fwrite(&mParams.p_arp_speed, 1, sizeof(float), file);
    fwrite(&mParams.p_arp_mod, 1, sizeof(float), file);

    fclose(file);
    return true;
}
//-----------------------------------------------------------------------------
void SFXGenerator::ResetSample(bool restart)
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
    }
}
//-----------------------------------------------------------------------------
void SFXGenerator::PlaySample()
{
    ResetSample(false);
    mState.playing_sample=true;
}
//-----------------------------------------------------------------------------
void SFXGenerator::SynthSample(int length, float* buffer, FILE* file)
{
    //double lock!  std::lock_guard<std::recursive_mutex> lock(mParamsMutex);
    for(int i=0;i<length;i++)
    {
        if(!mState.playing_sample)
            break;

        mState.rep_time++;
        if(mState.rep_limit!=0 && mState.rep_time>=mState.rep_limit)
        {
            mState.rep_time=0;
            ResetSample(true);
        }

        // frequency envelopes/arpeggios
        mState.arp_time++;
        if(mState.arp_limit!=0 && mState.arp_time>=mState.arp_limit)
        {
            mState.arp_limit=0;
            mState.fperiod*=mState.arp_mod;
        }
        mState.fslide+=mState.fdslide;
        mState.fperiod*=mState.fslide;
        if(mState.fperiod>mState.fmaxperiod)
        {
            mState.fperiod=mState.fmaxperiod;
            if(mParams.p_freq_limit>0.0f)
                mState.playing_sample=false;
        }
        float rfperiod=mState.fperiod;
        if(mState.vib_amp>0.0f)
        {
            mState.vib_phase+=mState.vib_speed;
            rfperiod=mState.fperiod*(1.0+sin(mState.vib_phase)*mState.vib_amp);
        }
        mState.period=(int)rfperiod;
        if(mState.period<8) mState.period=8;
        mState.square_duty+=mState.square_slide;
        if(mState.square_duty<0.0f) mState.square_duty=0.0f;
        if(mState.square_duty>0.5f) mState.square_duty=0.5f;
        // volume envelope
        mState.env_time++;
        if(mState.env_time>mState.env_length[mState.env_stage])
        {
            mState.env_time=0;
            mState.env_stage++;
            if(mState.env_stage==3)
                mState.playing_sample=false;
        }
        if(mState.env_stage==0)
            mState.env_vol=(float)mState.env_time/mState.env_length[0];
        if(mState.env_stage==1)
            mState.env_vol=1.0f+pow(1.0f-(float)mState.env_time/mState.env_length[1], 1.0f)*2.0f*mParams.p_env_punch;
        if(mState.env_stage==2)
            mState.env_vol=1.0f-(float)mState.env_time/mState.env_length[2];

        // phaser step
        mState.fphase+=mState.fdphase;
        mState.iphase=abs((int)mState.fphase);
        if(mState.iphase>1023) mState.iphase=1023;

        if(mState.flthp_d!=0.0f)
        {
            mState.flthp*=mState.flthp_d;
            if(mState.flthp<0.00001f) mState.flthp=0.00001f;
            if(mState.flthp>0.1f) mState.flthp=0.1f;
        }

        float ssample=0.0f;
        for(int si=0;si<8;si++) // 8x supersampling
        {
            float sample=0.0f;
            mState.phase++;
            if(mState.phase>=mState.period)
            {
//				state.phase=0;
                mState.phase%=mState.period;
                if(mParams.wave_type==3)
                    for(int i=0;i<32;i++)
                        mState.noise_buffer[i]=frnd(2.0f)-1.0f;
            }
            // base waveform
            float fp=(float)mState.phase/mState.period;
            switch(mParams.wave_type)
            {
            case 0: // square
                if(fp<mState.square_duty)
                    sample=0.5f;
                else
                    sample=-0.5f;
                break;
            case 1: // sawtooth
                sample=1.0f-fp*2;
                break;
            case 2: // sine
                sample=(float)sin(fp*2*M_PI);
                break;
            case 3: // noise
                sample=mState.noise_buffer[mState.phase*32/mState.period];
                break;
            }
            // lp filter
            float pp=mState.fltp;
            mState.fltw*=mState.fltw_d;
            if(mState.fltw<0.0f) mState.fltw=0.0f;
            if(mState.fltw>0.1f) mState.fltw=0.1f;
            if(mParams.p_lpf_freq!=1.0f)
            {
                mState.fltdp+=(sample-mState.fltp)*mState.fltw;
                mState.fltdp-=mState.fltdp*mState.fltdmp;
            }
            else
            {
                mState.fltp=sample;
                mState.fltdp=0.0f;
            }
            mState.fltp+=mState.fltdp;
            // hp filter
            mState.fltphp+=mState.fltp-pp;
            mState.fltphp-=mState.fltphp*mState.flthp;
            sample=mState.fltphp;
            // phaser
            mState.phaser_buffer[mState.ipp&1023]=sample;
            sample+=mState.phaser_buffer[(mState.ipp-mState.iphase+1024)&1023];
            mState.ipp=(mState.ipp+1)&1023;
            // final accumulation and envelope application
            ssample+=sample*mState.env_vol;
        }
        ssample=ssample/8*master_vol;

        ssample*=2.0f*sound_vol;

        if(buffer!=NULL)
        {
            if(ssample>1.0f) ssample=1.0f;
            if(ssample<-1.0f) ssample=-1.0f;
            *buffer++=ssample;
        }
        if(file!=NULL)
        {
            // quantize depending on format
            // accumulate/count to accomodate variable sample rate?
            ssample*=4.0f; // arbitrary gain to get reasonable output volume...
            if(ssample>1.0f) ssample=1.0f;
            if(ssample<-1.0f) ssample=-1.0f;
            filesample+=ssample;
            fileacc++;
            if(wav_freq==44100 || fileacc==2)
            {
                filesample/=fileacc;
                fileacc=0;
                if(wav_bits==16)
                {
                    short isample=(short)(filesample*32000);
                    fwrite(&isample, 1, 2, file);
                }
                else
                {
                    unsigned char isample=(unsigned char)(filesample*127+128);
                    fwrite(&isample, 1, 1, file);
                }
                filesample=0.0f;
            }
            file_sampleswritten++;
        }
    }
}
//-----------------------------------------------------------------------------
bool SFXGenerator::ExportWAV(const char* filename)
{
     std::lock_guard<std::recursive_mutex> lock(mParamsMutex);
    FILE* foutput=fopen(filename, "wb");
    if(!foutput)
        return false;
    // write wav header
    // char string[32]; // Not used
    unsigned int dword=0;
    unsigned short word=0;
    fwrite("RIFF", 4, 1, foutput); // "RIFF"
    dword=0;
    fwrite(&dword, 1, 4, foutput); // remaining file size
    fwrite("WAVE", 4, 1, foutput); // "WAVE"

    fwrite("fmt ", 4, 1, foutput); // "fmt "
    dword=16;
    fwrite(&dword, 1, 4, foutput); // chunk size
    word=1;
    fwrite(&word, 1, 2, foutput); // compression code
    word=1;
    fwrite(&word, 1, 2, foutput); // channels
    dword=wav_freq;
    fwrite(&dword, 1, 4, foutput); // sample rate
    dword=wav_freq*wav_bits/8;
    fwrite(&dword, 1, 4, foutput); // bytes/sec
    word=wav_bits/8;
    fwrite(&word, 1, 2, foutput); // block align
    word=wav_bits;
    fwrite(&word, 1, 2, foutput); // bits per sample

    fwrite("data", 4, 1, foutput); // "data"
    dword=0;
    int foutstream_datasize=ftell(foutput);
    fwrite(&dword, 1, 4, foutput); // chunk size

    // write sample data
    mState.mute_stream=true;
    file_sampleswritten=0;
    filesample=0.0f;
    fileacc=0;
    PlaySample();
    while(mState.playing_sample)
        SynthSample(256, NULL, foutput);
    mState.mute_stream=false;


    //  Calculate the total audio data size
    unsigned int audio_bytes = file_sampleswritten * (wav_bits / 8);

    //  Update RIFF Chunk Size (Total File Size - 8)
    // Located at offset 4
    fseek(foutput, 4, SEEK_SET);
    unsigned int riff_size = 36 + audio_bytes; // 36 is the header size after this field
    fwrite(&riff_size, 4, 1, foutput);

    //  Update Data Chunk Size (Just the audio bytes)
    // In a standard WAV, this is located at offset 40
    fseek(foutput, 40, SEEK_SET);
    fwrite(&audio_bytes, 4, 1, foutput);

    //  MEMFS Safety: Seek to the absolute end before closing
    fseek(foutput, 0, SEEK_END);
    fflush(foutput);
    fclose(foutput);


    return true;
}
//-----------------------------------------------------------------------------
void SFXGenerator::GeneratePickupCoin(){
    std::lock_guard<std::recursive_mutex> lock(mParamsMutex);
    ResetParamsNoLock();
    mParams.p_base_freq = 0.4f + frnd(0.5f);
    mParams.p_env_attack = 0.0f;
    mParams.p_env_sustain = frnd(0.1f);
    mParams.p_env_decay = 0.1f + frnd(0.4f);
    mParams.p_env_punch = 0.3f + frnd(0.3f);
    if (rnd(1)) {
        mParams.p_arp_speed = 0.5f + frnd(0.2f);
        mParams.p_arp_mod = 0.2f + frnd(0.4f);
    }
}
//-----------------------------------------------------------------------------
void SFXGenerator::GenerateLaserShoot() {
     std::lock_guard<std::recursive_mutex> lock(mParamsMutex);
    ResetParamsNoLock();
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
void SFXGenerator::GenerateExplosion() {
     std::lock_guard<std::recursive_mutex> lock(mParamsMutex);
    ResetParamsNoLock();
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
void SFXGenerator::GeneratePowerup() {
     std::lock_guard<std::recursive_mutex> lock(mParamsMutex);
    ResetParamsNoLock();
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
void SFXGenerator::GenerateHitHurt() {
     std::lock_guard<std::recursive_mutex> lock(mParamsMutex);
    ResetParamsNoLock();
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
void SFXGenerator::GenerateJump(){
     std::lock_guard<std::recursive_mutex> lock(mParamsMutex);
    ResetParamsNoLock();
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
void SFXGenerator::GenerateBlipSelect(){
     std::lock_guard<std::recursive_mutex> lock(mParamsMutex);
    ResetParamsNoLock();
    mParams.wave_type = rnd(1);
    if (mParams.wave_type == 0) mParams.p_duty = frnd(0.6f);
    mParams.p_base_freq = 0.2f + frnd(0.4f);
    mParams.p_env_attack = 0.0f;
    mParams.p_env_sustain = 0.1f + frnd(0.1f);
    mParams.p_env_decay = frnd(0.2f);
    mParams.p_hpf_freq = 0.1f;
}
//-----------------------------------------------------------------------------
void SFXGenerator::Randomize() {
     std::lock_guard<std::recursive_mutex> lock(mParamsMutex);
    mParams.p_base_freq = pow(frnd(2.0f) - 1.0f, 2.0f);
    if (rnd(1))
        mParams.p_base_freq = pow(frnd(2.0f) - 1.0f, 3.0f) + 0.5f;

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
    mParams.p_env_attack = pow(frnd(2.0f) - 1.0f, 3.0f);
    mParams.p_env_sustain = pow(frnd(2.0f) - 1.0f, 2.0f);
    mParams.p_env_decay = frnd(2.0f) - 1.0f;
    mParams.p_env_punch = pow(frnd(0.8f), 2.0f);

    if (mParams.p_env_attack + mParams.p_env_sustain + mParams.p_env_decay < 0.2f) {
        mParams.p_env_sustain += 0.2f + frnd(0.3f);
        mParams.p_env_decay += 0.2f + frnd(0.3f);
    }

    mParams.p_lpf_resonance = frnd(2.0f) - 1.0f;
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
}
//-----------------------------------------------------------------------------
void SFXGenerator::Mutate(){
     std::lock_guard<std::recursive_mutex> lock(mParamsMutex);
    if (rnd(1)) mParams.p_base_freq += frnd(0.1f) - 0.05f;
    if (rnd(1)) mParams.p_freq_ramp += frnd(0.1f) - 0.05f;
    if (rnd(1)) mParams.p_freq_dramp += frnd(0.1f) - 0.05f;
    if (rnd(1)) mParams.p_duty += frnd(0.1f) - 0.05f;
    if (rnd(1)) mParams.p_duty_ramp += frnd(0.1f) - 0.05f;
    if (rnd(1)) mParams.p_vib_strength += frnd(0.1f) - 0.05f;
    if (rnd(1)) mParams.p_vib_speed += frnd(0.1f) - 0.05f;
    if (rnd(1)) mParams.p_vib_delay += frnd(0.1f) - 0.05f;
    if (rnd(1)) mParams.p_env_attack += frnd(0.1f) - 0.05f;
    if (rnd(1)) mParams.p_env_sustain += frnd(0.1f) - 0.05f;
    if (rnd(1)) mParams.p_env_decay += frnd(0.1f) - 0.05f;
    if (rnd(1)) mParams.p_env_punch += frnd(0.1f) - 0.05f;
    if (rnd(1)) mParams.p_lpf_resonance += frnd(0.1f) - 0.05f;
    if (rnd(1)) mParams.p_lpf_freq += frnd(0.1f) - 0.05f;
    if (rnd(1)) mParams.p_lpf_ramp += frnd(0.1f) - 0.05f;
    if (rnd(1)) mParams.p_hpf_freq += frnd(0.1f) - 0.05f;
    if (rnd(1)) mParams.p_hpf_ramp += frnd(0.1f) - 0.05f;
    if (rnd(1)) mParams.p_pha_offset += frnd(0.1f) - 0.05f;
    if (rnd(1)) mParams.p_pha_ramp += frnd(0.1f) - 0.05f;
    if (rnd(1)) mParams.p_repeat_speed += frnd(0.1f) - 0.05f;
    if (rnd(1)) mParams.p_arp_speed += frnd(0.1f) - 0.05f;
    if (rnd(1)) mParams.p_arp_mod += frnd(0.1f) - 0.05f;
}
//------------------------------------------------------------------------------
// --- SDL
//------------------------------------------------------------------------------
void SDLCALL SFXGenerator::audio_callback(void* userdata, SDL_AudioStream* stream, int additional_amount, int total_amount)
{
    if (!userdata)
        return;

    auto* gen = static_cast<SFXGenerator*>(userdata);

    if (!gen)
        return;

    // additional_amount ist in BYTES.
    // one float == 4 Bytes.
    int frames_to_generate = additional_amount / sizeof(float);

    if (frames_to_generate > 0)
    {
        std::lock_guard<std::recursive_mutex> lock(gen->mParamsMutex);
        std::vector<float> buffer(frames_to_generate, 0.0f);

        if (gen->mState.playing_sample && !gen->mState.mute_stream) {
            gen->SynthSample(frames_to_generate, buffer.data(), NULL);
        } else {
            std::fill(buffer.begin(), buffer.end(), 0.0f);
        }

        SDL_PutAudioStreamData(stream, buffer.data(), additional_amount);
    }
}
//------------------------------------------------------------------------------
bool SFXGenerator::initSDLAudio()
{
    SDL_AudioSpec spec;
    spec.format = SDL_AUDIO_F32; // not: spec.format = SDL_AUDIO_S16;
    spec.channels = 1; // mono
    spec.freq = 44100;


    mStream = SDL_CreateAudioStream(&spec, &spec);
    if (!mStream)
    {
        Log("SDL_OpenAudioDeviceStream failed: %s", SDL_GetError());
        return false;
    }

    SDL_SetAudioStreamGetCallback(mStream, SFXGenerator::audio_callback, this);

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
    return true;
}

