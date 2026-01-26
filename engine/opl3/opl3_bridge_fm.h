//-----------------------------------------------------------------------------
// Copyright (c) 2026 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#pragma once

#include "opl3_base.h"
#include <array>
#include <fstream>
#include <string>

using namespace opl3;


namespace opl3_bridge_fm {




    //--------------------------------------------------------------------------
    inline bool loadInstrumentData(const std::string& filename, std::array<uint8_t, 24>& instrumentData)
    {
        std::ifstream file(filename, std::ios::binary);
        if (!file.is_open()) return false;

        file.read(reinterpret_cast<char*>(instrumentData.data()), instrumentData.size());

        return file.gcount() == 24;
    }

    //--------------------------------------------------------------------------
    inline Instrument toInstrument(const std::string name, const std::array<uint8_t, 24>& instrumentData) {
        Instrument newIns;
        newIns.name = name;
        newIns.isFourOp = false; // Old format is always 2-Op


        // Channel-level parameters (Pair 0)
        newIns.pairs[0].feedback = instrumentData[20];
        newIns.pairs[0].connection = instrumentData[21]; //Modulation Mode
        newIns.pairs[0].panning = 3; // Default to Center for OPL3

        // Modulator (Op 0) mapping
        auto& mod = newIns.pairs[0].ops[0];
        mod.multi   = instrumentData[0]; //Freqency
        mod.tl      = instrumentData[2]; //Output
        mod.attack  = instrumentData[4];
        mod.decay   = instrumentData[6];
        mod.sustain = instrumentData[8];
        mod.release = instrumentData[10];
        mod.wave    = instrumentData[12];
        mod.ksr     = instrumentData[14]; // Sustain mode / EG Type
        mod.vib     = instrumentData[16];
        mod.am      = instrumentData[18];
        mod.ksl     = instrumentData[22]; //scaling

        // Carrier (Op 1) mapping
        auto& car = newIns.pairs[0].ops[1];
        car.multi   = instrumentData[1];
        car.tl      = instrumentData[3];
        car.attack  = instrumentData[5];
        car.decay   = instrumentData[7];
        car.sustain = instrumentData[9];
        car.release = instrumentData[11];
        car.wave    = instrumentData[13];
        car.ksr     = instrumentData[15];
        car.vib     = instrumentData[17];
        car.am      = instrumentData[19];
        car.ksl     = instrumentData[23];

        return newIns;
    }
    //--------------------------------------------------------------------------
    inline bool loadInstrument(const std::string& filename,  Instrument& inst, std::string instrumentName = "") {

        std::array<uint8_t, 24> instrumentData;
        if (!loadInstrumentData(filename,instrumentData))
            return false;

        inst = toInstrument(instrumentName, instrumentData);
        return true;
    }
    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    #define FMS_MIN_CHANNEL 0
    #define FMS_MAX_CHANNEL 8
    #define FMS_MAX_SONG_LENGTH 1000

    struct SongDataFMS {
        // Pascal: array[1..9] of string[255] -> 10 slots to allow 1-based indexing
        // Index [channel][0] is the length byte.
        uint8_t actual_ins[10][256];

        // Pascal: array[1..9, 0..23] of byte
        uint8_t ins_set[10][24];

        uint8_t song_delay;   // Pascal: byte
        uint16_t song_length; // Pascal: word (16-bit)

        // Pascal: array[1..1000, 1..9] of integer (16-bit signed)
        int16_t song[FMS_MAX_SONG_LENGTH + 1][FMS_MAX_CHANNEL + 1];

        // Initialization function
        void init() {
            // 1. Zero out everything first for safety
            std::memset(this, 0, sizeof(SongDataFMS));

            // 2. Initialize Pascal Strings (actual_ins)
            // In Pascal, a blank string has a length of 0 at index [0].
            for (int ch = 0; ch <= FMS_MAX_CHANNEL; ++ch) {
                actual_ins[ch][0] = 0; // Length = 0
                // Optional: Initialize with a default name like "Empty"
                // SetPascalString(ch, "Empty");
            }

            // 3. Set standard defaults
            song_delay = 15;
            song_length = 0;

            // 4. Initialize Song Array
            // If "0" is not a valid empty note in your tracker,
            // initialize with -1 or your specific "empty" constant.
            for (int i = 0; i <= FMS_MAX_SONG_LENGTH; ++i) {
                for (int j = FMS_MIN_CHANNEL; j <= FMS_MAX_CHANNEL; ++j) {
                    song[i][j] = 0;
                }
            }
        }

        // Helper to get the Pascal string for C++ string (0..8 -> 1..9)
        std::string getInstrumentName(int channel)
        {
            // Safety check for your 0..8 range
            if (channel < FMS_MIN_CHANNEL || channel > FMS_MAX_CHANNEL)
                return "";

            // index [channel+1] selects the row (1..9)
            // index [0] is the Pascal length byte
            uint8_t len = actual_ins[channel + 1][0];

            // Data starts at [1]. Use reinterpret_cast for modern C++ standards.
            return std::string(reinterpret_cast<const char*>(&actual_ins[channel + 1][1]), len);
        }

        // Helper to set a Pascal string from a C-string (0..8 -> 1..9)
        bool setInstrumentName(int channel, const char* name) {
            if (channel < FMS_MIN_CHANNEL || channel > FMS_MAX_CHANNEL)
                return false;

            size_t len = std::strlen(name);
            if (len > 255) len = 255; // Turbo Pascal 6 limit

            // Update length byte at [0]
            actual_ins[channel + 1][0] = static_cast<uint8_t>(len);

            // Copy data starting at [1].
            // We don't null-terminate because Pascal doesn't use it.
            std::memcpy(&actual_ins[channel + 1][1], name, len);

            return true;
        }
    }; //stuct SongData


    //--------------------------------------------------------------------------

    inline uint8_t getMidiNoteIdFromFMSNoteId(int fmsNoteId) {
        if (fmsNoteId == 0 || fmsNoteId > 84) {
            return NONE_NOTE;
        } else if (fmsNoteId < 0) {
            return STOP_NOTE;
        }


        // 1. Your table's internal position (0-11)
        int positionInOctave = (fmsNoteId - 1) % 12;

        // 2. Map your "Document" order back to chromatic semitones (0=C, 1=C#, 2=D...)
        // This is the inverse of your documentToMusicalMap
        static const int musicalToMidiOffset[] = {
            2,  // 1 -> D (MIDI offset 2)
            4,  // 2 -> E (MIDI offset 4)
            5,  // 3 -> F (MIDI offset 5)
            7,  // 4 -> G (MIDI offset 7)
            9,  // 5 -> A (MIDI offset 9)
            11, // 6 -> B (MIDI offset 11)
            1,  // 7 -> C# (MIDI offset 1)
            3,  // 8 -> D# (MIDI offset 3)
            6,  // 9 -> F# (MIDI offset 6)
            8,  // 10 -> G# (MIDI offset 8)
            10, // 11 -> A# (MIDI offset 10)
            0   // 12 -> C (MIDI offset 0)
        };

        int semitone = musicalToMidiOffset[positionInOctave];

        // 3. Calculate Octave
        // Based on your getNoteNameFromId: IDs 1-12 = Octave 1
        int octave = (fmsNoteId  / 12) + 1;

        // 4. MIDI Calculation
        // MIDI Note 0 is C-0.
        // If your "Octave 1" C is meant to be C-1, the base is 12.
        // If your "Octave 1" C is meant to be C-2, the base is 24.
        // Standard MIDI Middle C (C-4) is 60.
        int midiBase = 0;
        return (octave * 12) + semitone + midiBase;
    }
    //--------------------------------------------------------------------------
    inline bool loadSongFMS(const std::string& filename, opl3::SongData& opl3SongData) {
        std::ifstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            Log("ERROR: Could not open song file: %s", filename.c_str());
            return false;
        }

        opl3SongData.init();

        SongDataFMS fmsSongData;
        fmsSongData.init();

        // Channel
        for (int ch = 1; ch <= 9; ++ch) {
            if (!file.read(reinterpret_cast<char*>(fmsSongData.actual_ins[ch]), 256)) {
                Log("ERROR: Failed reading Name for Channel %d at offset %ld", ch, (long)file.tellg());
                file.close();
                return false;
            }
            if (!file.read(reinterpret_cast<char*>(fmsSongData.ins_set[ch]), 24)) {
                Log("ERROR: Failed reading Settings for Channel %d at offset %ld", ch, (long)file.tellg());
                file.close();
                return false;
            }
            // i can set the default instrument here
            // FMS set the instruments bind on channel
            opl3SongData.channelInstrument[ch] = ch;
        }

        // Speed
        if (!file.read(reinterpret_cast<char*>(&fmsSongData.song_delay), 1)) {
            Log("ERROR: Failed reading song_speed");
            file.close();
            return false;
        }
        if (!file.read(reinterpret_cast<char*>(&fmsSongData.song_length), 2)) {
            Log("ERROR: Failed reading song_length");
            file.close();
            return false;
        }

        Log("INFO: Song Header Loaded. Speed: %u, Length: %u", fmsSongData.song_delay, fmsSongData.song_length);

        // Safety check for 2026 memory limits
        if (fmsSongData.song_length > FMS_MAX_SONG_LENGTH) {
            Log("ERROR: song_length (%u) exceeds maximum allowed (1000)", fmsSongData.song_length);
            file.close();
            return false;
        }

        // 3. Load Note Grid (Adjusted for 0-based C++ logic)
        for (int i = 0; i < fmsSongData.song_length; ++i) { // Start at 0
            for (int j = 0; j <= FMS_MAX_CHANNEL; ++j) {           // Start at 0
                int16_t temp_note;
                if (!file.read(reinterpret_cast<char*>(&temp_note), 2)) {
                    Log("ERROR: Failed reading Note at Tick %d, Channel %d", i, j);
                    file.close();
                    return false;
                }
                // Store it 0-based so sd.song[0][0] is the first note
                fmsSongData.song[i][j] = temp_note;
            }
        }

        Log("SUCCESS: Song '%s' loaded completely.", filename.c_str());

        // -------------  converting to opl3::SongData -----------------
        // -------- instruments
        for (int ch = FMS_MIN_CHANNEL; ch <= FMS_MAX_CHANNEL; ++ch) {
            opl3SongData.instruments.push_back(
                toInstrument(
                    fmsSongData.getInstrumentName(ch).c_str()
                    , std::to_array(fmsSongData.ins_set[ch + 1])
                )
            );
        }
        // -------- Pattern
        // Create a pattern with song_length rows
        int fmsNoteIndex  = -1;
        Pattern scalePat(fmsSongData.song_length);
        for (int row = 0; row < fmsSongData.song_length; row++ ) {
            // printf("#%04d ", row);
            for (int ch = 0; ch < FMS_MAX_CHANNEL; ch++)
            {
                SongStep& step = scalePat.getStep(row, ch);    //mSteps[row * 18 + ch];
                step.instrument = ch; //match the channel we
                step.volume = 63;
                step.note = NONE_NOTE;

                // tricky converting DosScale id's to current
                fmsNoteIndex = fmsSongData.song[row][ch];
                step.note = getMidiNoteIdFromFMSNoteId(fmsNoteIndex);
                // printf("| %2d => %3d ", fmsNoteIndex, step.note);

            }
            // printf("\n");
        }

        opl3SongData.patterns.push_back(scalePat);
        opl3SongData.orderList.push_back(0); // Play pattern 0


        // -------- BPM
        // it's a bit faster then OPLController class but this does
        // also not match the old dos composer 100%
        opl3SongData.bpm = (float)fmsSongData.song_delay * 6.f; // this match the dos composer !
        Log("Speed id %d setting bpm to %5.2f",fmsSongData.song_delay, opl3SongData.bpm );


        file.close();
        return true;
    }

}
