//-----------------------------------------------------------------------------
// Copyright (c) 1993 T.Huehn (XXTH)
// Copyright (c) 2026 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#pragma once
#include <vector>
#include <string>
#include <array>
#include <cstdint>

namespace opl3 {
    // Standard OPL3 has 18 channels (0-17)
    // 0-8  = Bank 1 ($000)
    // 9-17 = Bank 2 ($100)
    const uint8_t MAX_CHANNELS = 18; // Total count
    const uint8_t BANK_LIMIT = 8; // Channels <= 8 are in the first bank

    // F-Numbers for a standard Chromatic Scale (C, C#, D, D#, E, F, F#, G, G#, A, A#, B)
    // These values are designed for the OPL hardware to produce accurate pitches
    static const uint16_t f_numbers[] = {
        0x16B, 0x181, 0x198, 0x1B0, 0x1CA, 0x1E5, 0x202, 0x220, 0x241, 0x263, 0x287, 0x2AE
    };

    //--------------------------------------------------------------------------
    enum OplEffect : uint8_t {
        EFF_NONE         = 0x0,
        EFF_PORTA_UP     = 0x1, // 1xx: Pitch Slide Up
        EFF_PORTA_DOWN   = 0x2, // 2xx: Pitch Slide Down
        EFF_VIBRATO      = 0x4, // 4xy: Vibrato
        EFF_SET_PANNING  = 0x8, // 8xx Set Panning
        EFF_VOL_SLIDE    = 0xA, // Axy: Volume Slide
        EFF_SET_VOLUME   = 0xC, // Cxx: Set Volume
        EFF_POSITION_JUMP= 0xB, // Bxx: Jump to Pattern
    };
    //--------------------------------------------------------------------------
    struct OplInstrument {
        std::string name = "New Instrument";
        bool isFourOp = false;  // OPL3 mode
        int8_t fineTune = 0;
        uint8_t fixedNote = 255;


        // Each pair (Modulator + Carrier) matches your existing UI loop
        struct OpPair {
            // Shared per-pair settings (Register $C0)
            uint8_t feedback = 0;   // 0-7
            uint8_t connection = 0; // 0: FM, 1: Additive
            uint8_t panning = 3;    // 0: Mute, 1: Left, 2: Right, 3: Center (OPL3)

            // Operator data (Indices 0=Modulator, 1=Carrier)
            struct OpParams {
                uint8_t ksl = 0, tl = 0, multi = 0;
                uint8_t attack = 0, decay = 0, sustain = 0, release = 0;
                uint8_t wave = 0, ksr = 0, egTyp = 0, vib = 0, am = 0;
            } ops[2];
        };

        OpPair pairs[2]; // Pair 0 (Ch A), Pair 1 (Ch B - only used if isFourOp is true)
    };
    //--------------------------------------------------------------------------
    // struct SongStep {
    //     uint8_t note = 0;        // 0=None, 1-96 (C-0 to B-7), 255=Off
    //     uint8_t instrument = 0;  // Index to Instrument table
    //     uint8_t volume = 63;     // 0-63 OPL range
    //     uint16_t effect = 0;     // Command (e.g., 0x0Axy for Volume Slide)
    // };

    struct SongStep {
        uint8_t note       = 0;  // 0=None, 1-127 (MIDI range), 255=Off
        uint8_t instrument = 0;  // 0=None, 1-255
        uint8_t volume     = 63; // 0-63 (Standard tracker range)
        uint8_t panning    = 32; // 0 (Left), 32 (Center), 64 (Right)
        uint8_t effectType = 0;  // High byte of effect (e.g., 'A')
        uint8_t effectVal  = 0;  // Low byte of effect (e.g., 0x0F)
    };
    //--------------------------------------------------------------------------
    struct Pattern {
        std::string name = "New Pattern";
        uint32_t color = 0xFFFFFFFF; // RGBA
        uint16_t rowCount;
        std::vector<SongStep> steps;

        // Use member initializer list for rowCount
        Pattern(uint16_t rows, int channels = 18) : rowCount(rows) {
            steps.resize(rows * channels);

            // Optional: Initialize steps to 'Empty' tracker state
            for (auto& step : steps) {
                step.note = 0;           // None
                step.instrument = 0;     // Default
                step.volume = 63;        // Max (Tracker Std)
                step.panning = 32;       // Center
                step.effectType = 0;
                step.effectVal = 0;
            }
        }
    };
    //--------------------------------------------------------------------------
    struct SongData {
        std::string title = "New OPL Song";
        float bpm = 125.0f;
        uint8_t speed = 6;      // Ticks per row

        // OPL3 max channels is 18. OPL2 is 9.
        static constexpr int CHANNELS = 18;

        std::vector<OplInstrument> instruments;
        std::vector<Pattern> patterns;
        std::vector<uint8_t> orderList; // The "playlist" of pattern indices

        // Utility: Calculate total song length in rows
        size_t getTotalRows() const {
            size_t total = 0;
            for (uint8_t patIdx : orderList) {
                total += patterns[patIdx].rowCount;
            }
            return total;
        }
    };
    //--------------------------------------------------------------------------
    // Returns note value 1-96 (C-0 to B-7)
    inline uint8_t NoteToValue(const std::string& name) {
        if (name == "...") return 0;
        if (name == "===") return 255;

        static const std::array<std::string, 12> noteNames = {
            "C-", "C#", "D-", "D#", "E-", "F-", "F#", "G-", "G#", "A-", "A#", "B-"
        };

        std::string notePart = name.substr(0, 2);
        int octave = name[2] - '0';

        for (int i = 0; i < 12; ++i) {
            if (notePart == noteNames[i]) return (octave * 12) + i + 1;
        }
        return 0;
    }
    //--------------------------------------------------------------------------
    inline std::string ValueToNote(uint8_t noteValue) {
        if (noteValue == 0) return "...";   // No note
        if (noteValue == 255) return "==="; // Note Off / Key Off

        static const std::array<const char*, 12> noteNames = {
            "C-", "C#", "D-", "D#", "E-", "F-", "F#", "G-", "G#", "A-", "A#", "B-"
        };

        // Assuming 1 = C-0
        int index = noteValue - 1;
        int noteIndex = index % 12;
        int octave = index / 12;

        if (octave >= 0 && octave <= 8) {
            return std::string(noteNames[noteIndex]) + std::to_string(octave);
        }

        return "???"; // Out of range
    }
    //--------------------------------------------------------------------------
    struct ParamMeta {
        std::string name;
        uint8_t maxValue;
        // Offset into an "OpParams" struct if you want to automate mapping
    };

    const std::vector<ParamMeta> OPL_OP_METADATA = {
        {"Frequency Multi", 0xF}, {"Output Level", 0x3F},
        {"Attack Rate", 0xF},     {"Decay Rate", 0xF},
        {"Sustain Level", 0xF},   {"Release Rate", 0xF},
        {"Waveform", 0x7},        {"KSR (Scaling)", 0x1},
        {"EG Type", 0x1},         {"Vibrato", 0x1},
        {"Amp Mod", 0x1},         {"KSL", 0x3}
    };
    //--------------------------------------------------------------------------
    enum class RenderMode {
        RAW,
        BLENDED,
        SBPRO,       // 3.2kHz
        SB_ORIGINAL, // 2.8kHz (Muffled+)
        ADLIB_GOLD,  // 16kHz (Hi-Fi)
        CLONE_CARD,  // 8kHz + No Blending
        MODERN_LPF   // 12kHz
    };



} //namespace opl3
