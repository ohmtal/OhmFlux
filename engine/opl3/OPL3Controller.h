//-----------------------------------------------------------------------------
// Copyright (c) 2026 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#pragma once
#include <SDL3/SDL.h>

#include "ymfm.h"
#include "ymfm_opl.h"
#include "opl3.h"
#include "ymfmGlue.h"

#include <fstream>
#include <vector>
#include <cstdint>
#include <string>
#include <string_view>
#include <algorithm>
#include <array>
#include <mutex>
#include <errorlog.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

using namespace opl3;

//------------------------------------------------------------------------------
// class OPL3Controller
//------------------------------------------------------------------------------
class OPL3Controller
{
private:
    // ---------- OPL/YMFM ----------------
    // using OplChip = ymfm::ym3812; //OPL2
    using OplChip = ymfm::ymf262; //OPL3
    // using OplChip = ymfm::ymf289b; //OPL3L
    OplChip* mChip; //OPL
    YMFMInterface mInterface;

    double m_pos = 0.0;
    double m_step = 49716.0 / 44100.0; // Ratio of OPL rate to SDL rate
    OplChip::output_data mOutput;

    std::vector<OplInstrument> mInstruments;

    // ---------- SDL3 ----------------
    SDL_AudioStream* mStream = nullptr;

    // ---------- RenderMode -----------------
    RenderMode mRenderMode = RenderMode::RAW;
    float mRenderAlpha = 1.0f;
    bool mRenderUseBlending = false;
    float mRenderGain = 1.0f; // Global gain to simulate hot Sound Blaster output
    // Render State Members
    float mRender_lpf_l = 0.0f, mRender_lpf_r = 0.0f;
    float mCurrentFiltered_l = 0.0f, mCurrentFiltered_r = 0.0f; // New
    int32_t mRender_prev_l = 0, mRender_prev_r = 0;             // Changed to int32 for OPL3 summing

    // ---------- ThreadSafety ---------------
    std::recursive_mutex mDataMutex;

    // --------- SequencerState --------------
    struct SequencerState {
        bool playing = false;
        bool loop = false;

        // Position Tracking
        uint16_t orderIdx = 0;   // Current index in song.orderList
        uint16_t rowIdx = 0;     // Current row index in the active pattern

        // Limits (replaces song_startAt/stopAt for Pattern logic)
        uint16_t orderStartAt = 0;
        uint16_t orderStopAt = 0;

        // Timing (Using double prevents tempo drift over long songs)
        double samples_per_tick = 0.0;
        double sample_accumulator = 0.0;

        const SongData* current_song = nullptr;

        // UI Feedback for OPL3 (18 channels)
        uint8_t last_notes[18] = {0};
        bool note_updated = false;
    };
    const SequencerState& getSequencerState() const { return mSeqState; }

    uint8_t mShadowRegs[512] = {0}; // register for read

    //------------
    void fillBuffer(int16_t* buffer, int total_frames);
    virtual void tickSequencer();

    uint16_t get_modulator_offset(uint8_t channel);
    uint16_t get_carrier_offset(uint8_t channel);



    // ------ import -------------
    // ------ export -------------
    // FIXME bool saveWavFile(const std::string& filename, const std::vector<int16_t>& data, int sampleRate);

public:
    // ----------   Init ---------------
    OPL3Controller();
    ~OPL3Controller();
    static void SDLCALL audio_callback(void* userdata, SDL_AudioStream* stream, int additional_amount, int total_amount);
    bool initController();
    bool shutDownController();

    // ---------- SDL3 ----------------
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

    // --------- SequencerState --------------
    void setPlaying(bool value, bool hardStop = false);
    void togglePause();


    bool applyInstrument(uint8_t channel, uint8_t instrumentIndex);
    bool playNote(uint8_t channel, SongStep songStep);
    void stopNote(uint8_t channel);
    void setChannelVolume(uint8_t channel, uint8_t oplVolume);
    void setChannelPanning(uint8_t channel, uint8_t pan);




    // --------- RenderMode ------------
    void setRenderMode(RenderMode mode);
    RenderMode getRenderMode() { return mRenderMode; }

    // ---------  ------------

    OplChip* getChip() { return mChip; }
    OplChip::output_data& getOutPut() { return mOutput; }

    double getPos() const { return m_pos; }
    void setPos(double val) { m_pos = val; }
    double getStep() const { return m_step; }

    void silenceAll(bool hardStop);
    //FIXME ?! void set_speed(uint8_t songspeed);
    void reset();
    void write(uint16_t reg, uint8_t val);
    uint8_t readShadow(uint16_t reg);
    bool isChannelAdditive(uint8_t channel);
    void initDefaultBank();


    void setChannelOn(uint8_t channel); // required for setFrequency!
    void setFrequency(uint8_t channel, uint16_t fnum, uint8_t octave);
    void setFrequencyLinear(uint8_t channel, float linearFreq);


    void TESTChip() {
        SDL_PauseAudioStreamDevice(mStream);
        SDL_SetAudioStreamGetCallback(mStream, NULL, NULL);

        // 1. Hard Reset
        // mChip->reset();


        // 2. Enable OPL3 (Bank 2, Reg 0x05, Bit 0)
        write(0x105, 0x01);

        // 3. Setup a Channel (Channel 0)
        // $20: Multiplier/Vibrato ($20=Mod, $23=Car)
        write(0x20, 0x01); write(0x23, 0x01);
        // $40: Total Level ($40=Mod, $43=Car). 0 is LOUDEST.
        write(0x40, 0x10); write(0x43, 0x00);
        // $60: Attack/Decay. 0xF is INSTANT ATTACK.
        write(0x60, 0xF0); write(0x63, 0xF0);
        // $80: Sustain/Release.
        write(0x80, 0xFF); write(0x83, 0xFF);
        // $C0: Panning (Bits 4-5). 0x30 is BOTH (Center).
        write(0xC0, 0x31); // 0x30 (Pan) | 0x01 (Connection)

        // 4. Trigger Note ($A0/$B0)
        write(0xA0, 0x6B); // F-Number Low
        write(0xB0, 0x31); // 0x20 (Key-On) | 0x10 (Octave 4) | 0x01 (F-Num High)

        // 5. Generate
        mChip->generate(&mOutput);
        // mOutput.data[0...3] should now contain non-zero values.
        LogFMT("TEST OPL3 Chip data: {} {} {} {}",
               mOutput.data[0],
               mOutput.data[1],
               mOutput.data[2],
               mOutput.data[3]);

        // rebind the audio stream!
        SDL_SetAudioStreamGetCallback(mStream, OPL3Controller::audio_callback, this);
        SDL_ResumeAudioStreamDevice(mStream);


    }

    // ------ import -------------
    // ------ export -------------
    //FIXME bool exportToWav(SongData &sd, const std::string& filename, float* progressOut = nullptr);







protected:
    SequencerState mSeqState;


};  //class OPL3Controller
