//-----------------------------------------------------------------------------
// Copyright (c) 1993 T.Huehn (XXTH)
// Copyright (c) 2026 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// FIXME need dome cleanup
// TODO OPL3 addons:
// 1. Key Technical Advantages of OPL3
// Switching gives you access to specific hardware capabilities that OPL2 (YM3812)
// lacks:
//
// - Stereo Output & Panning: OPL3 supports stereo sound with per-channel panning
// (Left, Right, or Center/Both). In OPL2, everything is strictly mono.
// - 4-Operator Synthesis: You can combine two channels into a single 4-operator
// voice. This allows for much more complex, "modern" FM sounds that are
// difficult or impossible to achieve with standard 2-operator OPL2 instruments.
// - Double Polyphony: OPL3 features 18 melodic channels (36 operators total),
// exactly twice the capacity of OPL2's 9 channels.
// - More Waveforms: OPL3 adds 4 additional waveforms (8 total), including a
// Square Wave, which helps create "harder" and "thicker" sounds.
//
// 2. Implementation Differences
// If you switch your emulation core, your code will need small but important
// adjustments:
//
// - Register Sets: OPL3 uses two register banks ($0xx and $1xx). Your current
// offsets (0x20, 0x40, etc.) will only control the first 9 channels.
// - Reduced Delays: Genuine OPL3 hardware (and high-end emulators) requires
// almost zero delay between writes compared to the strict microsecond wait
// times of OPL2.
// - Compatibility: You can still use your existing INSTRUMENT_METADATA and
// setInstrument logic for the first 9 channels without any changes.
//
// 3. Verdict: Is it "Better" Sound?
//
// - If you play OPL2 songs: It will sound identical, though sometimes slightly
// louder depending on the emulation core.
// - If you write NEW music: It is much better. The ability to use stereo panning
// and 4-operator instruments makes the difference between a "thin" retro
// sound and a "full" professional FM synthesizer.
//-----------------------------------------------------------------------------

#pragma once
#include <SDL3/SDL.h>

#include "ymfm.h"
#include "ymfm_opl.h"

#include "OplInterface.h"
#include "errorlog.h"

// for load:
#include <fstream>
#include <vector>

// for songs:
#include <cstdint>
#include <string>

// always good ;)
#include <algorithm>

//FIXME ? rewrite instrument data to std::array<uint8_t, 24>
#include <array>

#include <mutex>
//------------------------------------------------------------------------------
const float PLAYBACK_FREQUENCY = 90.0f;

#define FMS_MAX_CHANNEL 8

//------------------------------------------------------------------------------
// class OplController
//------------------------------------------------------------------------------
// The Controller: Encapsulates the OPL chip and Music Logic
class OplController
{
public:
    struct SongData {
        // Actual_ins: array[1..9] of string [256 bytes each]
        // Total: 9 * 256 = 2304 bytes
        uint8_t actual_ins[10][256];

        // Ins_set: array[1..9, 0..23] of byte
        // Total: 9 * 24 = 216 bytes
        uint8_t ins_set[10][24];

        // songspeed: byte [1 byte]
        uint8_t song_speed;

        // songlaenge: word [2 bytes]
        uint16_t song_length;

        // song: array[1..1000, 1..9] of integer
        // Pascal 'integer' is 16-bit signed. 1000 * 9 * 2 = 18000 bytes
        int16_t song[1001][10];
    };

    struct InsParam {
        std::string name;
        uint8_t maxValue;
    };

    static const std::vector<InsParam> INSTRUMENT_METADATA; //Defined below

    std::recursive_mutex mDataMutex;

private:
    using OplChip = ymfm::ymf262; // = OPL3 // ymfm::ym3812 = OPL2

    OplChip* mChip; //OPL

    OplInterface mInterface;

    SDL_AudioStream* mStream = nullptr;




    // OPL RATIO
    double m_pos = 0.0;
    double m_step = 49716.0 / 44100.0; // Ratio of OPL rate to SDL rate
    OplChip::output_data mOutput;

    // 9 channels, each holding 24 instrument parameters
    uint8_t m_instrument_cache[9][24];

    // F-Numbers for the Chromatic Scale (C, C#, D, D#, E, F, F#, G, G#, A, A#, B)
    static constexpr uint16_t f_numbers[] = {
        0x157, 0x16B, 0x181, 0x198, 0x1B0, 0x1CA,
        0x1E5, 0x202, 0x220, 0x241, 0x263, 0x287
    };

    // Tonleiter: {FNum_Hi_KeyOn, FNum_Low}
    static constexpr uint8_t myDosScale[85][2] = {
        {0x00, 0x00}, // [0] Padding for 1-based indexing

        // Octave 1
        {0x21, 0x81}, {0x21, 0xB0}, {0x21, 0xCA}, {0x22, 0x02}, {0x22, 0x41}, {0x22, 0x87},
        {0x21, 0x6B}, {0x21, 0x98}, {0x21, 0xE5}, {0x22, 0x20}, {0x22, 0x63}, {0x22, 0xAE},

        // Octave 2
        {0x25, 0x81}, {0x25, 0xB0}, {0x25, 0xCA}, {0x26, 0x02}, {0x26, 0x41}, {0x26, 0x87},
        {0x25, 0x6B}, {0x25, 0x98}, {0x25, 0xE5}, {0x26, 0x20}, {0x26, 0x63}, {0x26, 0xAE},

        // Octave 3
        {0x29, 0x81}, {0x29, 0xB0}, {0x29, 0xCA}, {0x2A, 0x02}, {0x2A, 0x41}, {0x2A, 0x87},
        {0x29, 0x6B}, {0x29, 0x98}, {0x29, 0xE5}, {0x2A, 0x20}, {0x2A, 0x63}, {0x2A, 0xAE},

        // Octave 4
        {0x2D, 0x81}, {0x2D, 0xB0}, {0x2D, 0xCA}, {0x2E, 0x02}, {0x2E, 0x41}, {0x2E, 0x87},
        {0x2D, 0x6B}, {0x2D, 0x98}, {0x2D, 0xE5}, {0x2E, 0x20}, {0x2E, 0x63}, {0x2E, 0xAE},

        // Octave 5
        {0x31, 0x81}, {0x31, 0xB0}, {0x31, 0xCA}, {0x32, 0x02}, {0x32, 0x41}, {0x32, 0x87},
        {0x31, 0x6B}, {0x31, 0x98}, {0x31, 0xE5}, {0x32, 0x20}, {0x32, 0x63}, {0x32, 0xAE},

        // Octave 6
        {0x35, 0x81}, {0x35, 0xB0}, {0x35, 0xCA}, {0x36, 0x02}, {0x36, 0x41}, {0x36, 0x87},
        {0x35, 0x6B}, {0x35, 0x98}, {0x35, 0xE5}, {0x36, 0x20}, {0x36, 0x63}, {0x36, 0xAE},

        // Octave 7
        {0x39, 0x81}, {0x39, 0xB0}, {0x39, 0xCA}, {0x3A, 0x02}, {0x3A, 0x41}, {0x3A, 0x87},
        {0x39, 0x6B}, {0x39, 0x98}, {0x39, 0xE5}, {0x3A, 0x20}, {0x3A, 0x63}, {0x3A, 0xAE}
    };


    // Mapping for OPL2 Operator offsets (Channels 0-8)
     const uint8_t Adr_add[9] = { 0x00, 0x01, 0x02, 0x08, 0x09, 0x0A, 0x10, 0x11, 0x12 };


    // Base OPL2 Register addresses for operators
     // Index 3-7 correspond to the OPL register groups
     const uint8_t FM_adresses[9] = {
         0x00, // padding for pascal starts at index 1

         0xA0, // Set voices, control modulation, or adjust amplitude
         0xB0, // Configuring operator frequencies and modulation parameters

         0x20, // 3: Multiplier
         0x40, // 4: KSL / Level
         0x60, // 5: Attack / Decay
         0x80, // 6: Sustain / Release
         0xE0, // 7: Waveform
         0xC0  // 8: Feedback (Handled outside the loop)
     };
    // Array to store the last value written to registers 0xB0 through 0xB8
    uint8_t last_block_values[9] = { 0 };



    struct SequencerState {
        bool playing = false;
        bool loop = false;
        int song_counter = 0;
        int samples_per_tick = 0;
        int sample_accumulator = 0;
        const SongData* current_song = nullptr; // Pointer to the loaded song

        // see what it plays
        int16_t last_notes[10]; // Stores the notes for channels 1-9
        bool note_updated = false;
    };

    SequencerState mSeqState;



public:
    OplController();
    ~OplController();
    static void SDLCALL audio_callback(void* userdata, SDL_AudioStream* stream, int additional_amount, int total_amount);
    bool initController();
    bool shutDownController();


    const SequencerState& getSequencerState() const { return mSeqState; }


    // in my DosProgramm i used melodic only but is should be a flag;
    // Enables Deep Effects and Locks Melodic Mode
    // this is updated
    bool mMelodicMode = false; // else RhythmMode

    // need a lot of cleaning .. lol but for now it's here:
    void setPlaying(bool value, bool hardStop = false);
    void togglePause();


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



    /**
     * Returns the OPL2 register offset for the Carrier (Operator 2)
     * of a given channel (0-8).
     */
    uint8_t get_carrier_offset(uint8_t channel);
    /**
     * Returns the OPL2 register offset for the Modulator (Operator 1)
     */
    uint8_t get_modulator_offset(uint8_t channel);

    // Return the pointer directly
    OplChip* getChip() { return mChip; }

    // Return by reference (&) so generate() writes to the REAL m_output
    OplChip::output_data& getOutPut() { return mOutput; }

    double getPos() const { return m_pos; }
    void setPos(double val) { m_pos = val; }

    double getStep() const { return m_step; }



    void silenceAll();
    void set_speed(uint8_t songspeed);
    void reset();
    void write(uint16_t reg, uint8_t val);
    void playNoteDOS(int channel, int noteIndex);
    void playNote(int channel, int noteIndex);
    void stopNote(int channel);
    void render(int16_t* buffer, int frames);

    void dumpInstrumentFromCache(uint8_t channel);
    void setInstrument(uint8_t channel, const uint8_t lIns[24]);
    const uint8_t* getInstrument(uint8_t channel) const;

    bool loadSongFMS(const std::string& filename, SongData& sd);
    bool saveSongFMS(const std::string& filename, const SongData& sd); //FIXME NOT TESTED !!!!

    void start_song(SongData& sd, bool loopit);

    bool loadInstrument(const std::string& filename, uint8_t channel);
    bool saveInstrument(const std::string& filename, const uint8_t* instrumentData); //FIXME NOT TESTED !!!!
    void TestInstrumentDOS(uint8_t channel, const uint8_t ins[24], int noteIndex = 48);

    void replaceSongNotes(SongData& sd, uint8_t targetChannel, int16_t oldNote, int16_t newNote);

    void fillBuffer(int16_t* buffer, int total_frames);
    void tickSequencer();

    // can be called in mail loop to output the playing song to console
    void consoleSongOutput();
    std::string getNoteNameFromId(int noteID);
    int getIdFromNoteName(std::string name);

    const char* GetChannelName(int index) {
        static const char* melodic[] = { "Channel 1", "Channel 2", "Channel 3", "Channel 4", "Channel 5", "Channel 6", "Channel 7", "Channel 8", "Channel 9" };
        static const char* rhythm[]  = { "Channel 1", "Channel 2", "Channel 3", "Channel 4", "Channel 5", "Channel 6", "Bass Drum", "Snare/HH", "Tom/Cym" };

        if (mMelodicMode)
            return melodic[index];

        return rhythm[index];
    }


    std::array<uint8_t, 24> GetDefaultInstrument();

    std::array<uint8_t, 24> GetDefaultBassDrum();
    std::array<uint8_t, 24> GetDefaultSnareHiHat();
    std::array<uint8_t, 24> GetDefaultTomCymbal();
    std::array<uint8_t, 24> GetDefaultLeadSynth();
    std::array<uint8_t, 24> GetDefaultOrgan();
    std::array<uint8_t, 24> GetDefaultCowbell();

    // Channel     Sound Type	Logic
    // 0	Grand Piano	Fast attack, medium decay, low sustain, fast release.
    // 1	FM Bass	Short attack, rapid decay, high feedback for "grit".
    // 2	Strings/Pad	Slow attack (1-2s), high sustain, slow release (~2s).
    // 3	Lead Synth	Fast attack, full sustain, pseudo-sawtooth waveform.
    // 4	Electric Organ	Fast attack, 100% sustain, no decay/release (on/off feel).
    // 5	Bell / Xylophone	Instant attack, rapid decay to zero sustain.
    std::array<uint8_t, 24> GetMelodicDefault(uint8_t index);


    void resetInstrument(uint8_t channel);
    void loadInstrumentPreset();

}; //class

inline const std::vector<OplController::InsParam> OplController::INSTRUMENT_METADATA = {
    {"Modulator Frequency", 0xF}, {"Carrier Frequency", 0xF},
    {"Modulator Output", 0x3F},   {"Carrier Output", 0x3F},
    {"Modulator Attack", 0xF},    {"Carrier Attack", 0xF},
    {"Modulator Decay", 0xF},     {"Carrier Decay", 0xF},
    {"Modulator Sustain", 0xF},   {"Carrier Sustain", 0xF},
    {"Modulator Release", 0xF},   {"Carrier Release", 0xF},
    {"Modulator Waveform", 0x3},  {"Carrier Waveform", 0x3},
    {"Modulator EG Typ", 0x1},    {"Carrier EG Typ", 0x1},
    {"Modulator Vibrato", 0x1},   {"Carrier Vibrato", 0x1},
    {"Modulator Amp Mod", 0x1},   {"Carrier Amp Mod", 0x1},
    {"Feedback", 0x7},            {"Modulation Mode", 0x1},
    {"Modulator Scaling", 0x3},   {"Carrier Scaling", 0x3}
};
