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

#include <DSP.h>

#include <fstream>
#include <vector>
#include <cstdint>
#include <string>
#include <string_view>
#include <algorithm>
#include <array>
#include <mutex>
#include <memory>

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
    uint32_t mOutputSampleRate;
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


    DSP::Bitcrusher* mDSPBitCrusher;
    DSP::Warmth* mDSPWarmth;
    DSP::Reverb* mDSPReverb;
    DSP::Chorus* mDSPChorus;
    DSP::Limiter* mLimiter;
    DSP::Equalizer9Band* mEquilzer9Band;

    std::vector<std::unique_ptr<DSP::Effect>> mDspEffects;

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

    void writeChannelReg(uint16_t baseReg, uint8_t channel, uint8_t value);

    void setOperatorRegisters(uint16_t opOffset, const OplInstrument::OpPair::OpParams& op);


    bool playNote(uint8_t channel, SongStep songStep);
    void playNote(uint8_t channel, uint16_t fnum, uint8_t octave);

    void stopNote(uint8_t channel);
    void setChannelVolume(uint8_t channel, uint8_t oplVolume);
    // Helper to keep KSL and update TL
    void updateOpVolume(uint8_t channel, bool isCarrier, uint8_t vol) {
        uint16_t offset = isCarrier ? get_carrier_offset(channel) : get_modulator_offset(channel);
        uint8_t reg = readShadow(0x40 + offset);
        write(0x40 + offset, (reg & 0xC0) | vol);
    }

    void setChannelPanning(uint8_t channel, uint8_t pan);
    void processStepEffects(uint8_t channel, const SongStep& step);
    void modifyChannelPitch(uint8_t channel, int8_t amount);
    void toggleDeepEffects(bool deepTremolo, bool deepVibrato);



    bool playSong(SongData& songData);



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
    void reset();
    void write(uint16_t reg, uint8_t val, bool doLog = false);
    uint8_t readShadow(uint16_t reg);
    bool isChannelAdditive(uint8_t channel);
    void initDefaultBank();
    void dumpInstrument(uint8_t instrumentIndex);

    void setChannelOn(uint8_t channel); // required for setFrequency!
    void setFrequency(uint8_t channel, uint16_t fnum, uint8_t octave);
    void setFrequencyLinear(uint8_t channel, float linearFreq);



    // ------ Console -----------------------
    void consoleSongOutput(bool useNumbers, uint8_t upToChannel = MAX_CHANNELS);


    // ------- DSP ---------------
    DSP::Bitcrusher* getDSPBitCrusher()  { return mDSPBitCrusher; }
    DSP::Chorus* getDSPChorus() {return mDSPChorus;}
    DSP::Reverb* getDSPReverb() {return mDSPReverb;}
    DSP::Warmth* getDSPWarmth() { return mDSPWarmth; }
    DSP::Limiter* getDSPLimiter() { return mLimiter; }
    DSP::Equalizer9Band* getDSPEquilzer9Band() { return mEquilzer9Band; }


    // ----- chords --------------
    // getMain()->getController()->playChord(mCurrentInstrument, 60, OPL3Controller::CHORD_MAJOR)

    void playChord(uint16_t instrument,  uint8_t rootNote, const std::vector<int>& offsets) {

        for (size_t i = 0; i < offsets.size(); i++) {
            // Assign each note to a unique OPL3 channel (1, 2, 3...)
            uint8_t channel = (uint8_t)(i + 1);
            uint8_t midiNote = rootNote + offsets[i];

            if (midiNote <= 127) {
                SongStep step{midiNote, instrument, 63};
                this->playNote(channel, step);
            }
        }
    }



    // ------ import -------------
    // ------ export -------------
    //FIXME bool exportToWav(SongData &sd, const std::string& filename, float* progressOut = nullptr);


protected:
    SequencerState mSeqState;

    double m_opl3_accumulator = 0.0;
    int16_t m_lastSampleL = 0;
    int16_t m_lastSampleR = 0;


};  //class OPL3Controller

