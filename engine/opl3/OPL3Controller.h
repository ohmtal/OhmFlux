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
    using OplChip = ymfm::ymf262; //OPL3
    OplChip* mChip; //OPL
    YMFMInterface mInterface;

    double m_pos = 0.0;
    double m_step = 49716.0 / 44100.0; // Ratio of OPL rate to SDL rate
    OplChip::output_data mOutput;


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
        // ... position tracking ...
        uint8_t current_tick = 0;
        uint8_t ticks_per_row = 6; // Standard tracker default

        double sample_accumulator = 0.0;
        double samples_per_tick = 0.0; // Calculate this: (SampleRate * 60) / (BPM * TPL)

        const SongData* current_song = nullptr;



        // Channel State (Used for UI and Effect Logic)

        SongStep last_steps[18] = {};
        bool ui_dirty = false;

    };
    const SequencerState& getSequencerState() const { return mSeqState; }

    uint8_t mShadowRegs[512] = {0}; // register for read

    //------------
    void generate(int16_t* out_buffer, int num_frames);
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
    bool initController();
    bool shutDownController();

    // ----------  ----------------
    std::vector<OplInstrument> mSoundBank;

    // ---------- SDL3 ----------------
    static void SDLCALL audio_callback(void* userdata, SDL_AudioStream* stream, int additional_amount, int total_amount);
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
    void processStepEffects(uint8_t channel, const SongStep& step);
    void modifyChannelPitch(uint8_t channel, int8_t amount);

    void playSong(SongData& songData) {
        std::lock_guard<std::recursive_mutex> lock(mDataMutex);

        // 1. Assign Song Data
        mSeqState.current_song = &songData;

        // 2. Reset Sequencer Positions
        mSeqState.orderIdx = 0;
        mSeqState.rowIdx = 0;
        mSeqState.current_tick = 0;
        mSeqState.sample_accumulator = 0.f;

        // 3. Reset Channel States using the new struct array
        // This clears all redundancy checks so the first row of the song
        // is guaranteed to write to the hardware registers.
        for (int i = 0; i < 18; ++i) {
            mSeqState.last_steps[i] = {}; // Resets note, vol, pan, effect
            mSeqState.last_steps[i].volume = 255; // Force update on first row
        }

        // 4. Calculate timing
        // In 2026, ensure hostRate is queried from your actual SDL audio device
        double hostRate = 44100.0;
        double ticks_per_sec = songData.bpm * 0.4;

        // 2. Calculate samples per tick
        if (ticks_per_sec > 0) {
            // At 125 BPM: 44100 / 50 = 882 samples per tick
            mSeqState.samples_per_tick = hostRate / ticks_per_sec;
        } else {
            mSeqState.samples_per_tick = hostRate;
        }


        // 5. Hard Reset Chip
        mChip->reset();
        write(0x105, 0x01); // RE-ENABLE OPL3 MODE (Mandatory after reset)

        // 6. Start Playback
        mSeqState.playing = true;
        mSeqState.ui_dirty = true;
    }



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



    // ------ import -------------
    // ------ export -------------
    //FIXME bool exportToWav(SongData &sd, const std::string& filename, float* progressOut = nullptr);


protected:
    SequencerState mSeqState;


    //------------------- TESTING ------------------------------------
public:
    void consoleSongOutput(bool useNumbers)
    {
        if (!mSeqState.playing)
            return;
        // 2026 Update: Check if the UI state has changed
        if (mSeqState.ui_dirty) {
            // Print current position (Pattern Index : Row Index)
            printf("[%02d:%03d] ", mSeqState.orderIdx, mSeqState.rowIdx);

            // OPL3 has 18 channels. We'll print them in two rows of 9
            // or one long row depending on your terminal width.
            for (int ch = 0; ch < 18; ch++) {
                const SongStep& step = mSeqState.last_steps[ch];

                // 1. Note Column
                if (step.note == 255) {
                    printf("==="); // Note Off
                } else if (step.note == 0) {
                    printf("..."); // Empty
                } else {
                    if (useNumbers) {
                        printf("%02X", step.note);
                    } else {
                        // Assuming you have a helper that handles std::string_view or const char*
                        printf("%3s", opl3::ValueToNote(step.note).c_str());
                    }
                }

                // 2. Instrument & Volume Column (Visual: Ins Vol)
                // Example output: "01 40" (Instrument 1, Volume 40)
                printf(" %02X %02X", step.instrument, step.volume);

                // 3. Effect Column (Visual: TypeVal)
                // Example output: "A05" (Volume Slide 05)
                if (step.effectType > 0) {
                    printf("|%1X%02X ", step.effectType, step.effectVal);
                } else {
                    printf("|... ");
                }

                // Separator between channels
                if (ch == 8) printf(" | ");
            }

            printf("\n");
            mSeqState.ui_dirty = false;
        }
    }

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

    SongData createScaleSong() {
        SongData song;
        song.title = "OPL3 Scale Test";
        song.bpm = 125.0f;
        song.speed = 6;

        // 1. Create a pattern with 32 rows
        Pattern scalePat(32, 18);

        // 2. Define the scale (C-4 to C-5)
        // MIDI Notes: 48, 50, 52, 53, 55, 57, 59, 60
        uint8_t scale[] = { 48, 50, 52, 53, 55, 57, 59, 60 };

        for (int i = 0; i < 8; ++i) {
            // Place a note every 4th row on channel 0
            int row = i * 4;
            SongStep& step = scalePat.steps[row * 18 + 0];

            step.note = scale[i];
            step.instrument = 0; // Ensure your soundbank has at least one instrument
            step.volume = 63;
        }

        // 3. Add pattern and set order
        song.patterns.push_back(scalePat);
        song.orderList.push_back(0); // Play pattern 0

        return song;
    }

    SongData createEffectTestSong(uint8_t ins = 0) {
        SongData song;
        song.title = "OPL3 Effects Stress Test";
        song.bpm = 90.f; //125.0f;
        song.speed = 6;


        // Create a 32-row pattern
        Pattern testPat(64, 18);

        // --- Channel 0: Volume Slide Test ---
        // Trigger a long note on Row 0
        SongStep& startNote = testPat.steps[0 * 18 + 0];
        startNote.note = 48; // C-4
        startNote.instrument = ins;
        startNote.volume = 63;

        // Starting at Row 1, slide volume DOWN
        for (int r = 1; r < 16; ++r) {
            SongStep& s = testPat.steps[r * 18 + 0];
            s.effectType = EFF_VOL_SLIDE;
            s.effectVal  = 0x04; // Slide down by 4 per tick (0x0y)
        }

        // --- Channel 1: Panning Test ---
        // Trigger a note that jumps across speakers
        for (int i = 0; i < 4; ++i) {
            int row = i * 8;
            SongStep& s = testPat.steps[row * 18 + 1];
            s.note = 60; // C-5
            s.instrument = ins;
            s.volume = 50;

            // Alternate Panning: 0 (Hard Left), 64 (Hard Right)
            s.effectType = EFF_SET_PANNING;
            s.effectVal  = (i % 2 == 0) ? 0 : 64;
        }

        // --- Channel 2: Note-Off Test ---
        // Test if 255 correctly stops the OPL3 operators
        for (int i = 0; i < 4; ++i) {
            testPat.steps[(i * 8) * 18 + 2].note = 55;       // G-4 Key-On
            testPat.steps[(i * 8) * 18 + 2].instrument = 0;
            testPat.steps[(i * 8 + 4) * 18 + 2].note = 255;  // Key-Off 4 rows later
        }

        // --- Channel 3: Set Volume Test ---
        // Directly setting volume via effect Cxx
        for (int i = 0; i < 8; ++i) {
            SongStep& s = testPat.steps[(i * 4) * 18 + 3];
            s.note = 52; // E-4
            s.instrument = ins;
            s.effectType = EFF_SET_VOLUME;
            s.effectVal  = (i % 2 == 0) ? 20 : 60; // Alternate quiet/loud
        }


        for (int i = 0; i < MAX_CHANNELS; i++)
            testPat.steps[31 * 18 + i].note = 255;

        // indexing for Row 32, Channel 0
        SongStep& s = testPat.steps[32 * 18 + 0];
        s.note = 52;
        s.instrument = ins;
        s.volume = 63;           // Start at max volume
        s.effectType = EFF_VOL_SLIDE;
        s.effectVal  = 0x08;

        // SongStep& s2 = testPat.steps[36 * 18 + 0];
        // s2.note = 52;
        // s2.instrument = 0;
        // s2.volume = 0;
        // s2.effectType = EFF_VOL_SLIDE;
        // s2.effectVal  = 0x40; // Slides volume UP by 4 units per tick

        // testPat.steps[40 * 18 + 0].volume=0;



        song.patterns.push_back(testPat);
        song.orderList.push_back(0);

        return song;
    }


};  //class OPL3Controller
