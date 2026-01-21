//-----------------------------------------------------------------------------
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
    constexpr uint8_t MAX_CHANNELS = 18; // Total count
    constexpr uint8_t BANK_LIMIT = 8; // Channels <= 8 are in the first bank

    constexpr uint8_t LAST_NOTE = 127; // G9
    constexpr uint8_t STOP_NOTE = 128; // Represents Note Off (===)
    constexpr uint8_t NONE_NOTE = 255; // Represents a Rest (...)
    constexpr uint8_t INVALID_NOTE = 192; // for note string to value when invalid

    constexpr uint8_t MAX_VOLUME = 63;

    static constexpr std::array<const char*, 12> NOTE_NAMES = {
        "C-", "C#", "D-", "D#", "E-", "F-", "F#", "G-", "G#", "A-", "A#", "B-"
    };



    // F-Numbers for a standard Chromatic Scale (C, C#, D, D#, E, F, F#, G, G#, A, A#, B)
    // F-Numbers mapped to the high range (0x200 - 0x3FF)
    // This provides the most "bits" for your fineTuneTable to work with.
    //TEST: XXTH
    // C-3 sounds like A-3
    // static constexpr uint16_t f_numbers[] = {
    //     517, 547, 580, 615, 651, 690, 731, 774, 820, 869, 921, 975
    // };
    // static constexpr uint16_t f_numbers[] = {
    //     0x16B, 0x181, 0x198, 0x1B0, 0x1CA, 0x1E5, 0x202, 0x220, 0x241, 0x263, 0x287, 0x2AE
    //
    // };
    /**
     * OPL3 F-Numbers for a standard chromatic scale.
     *
     * Formula used: f_num = (f_out * 2^20) / (fs * 2^(B-1))
     * Values are calculated for an equal-tempered scale where
     * F-Number 512 represents the midpoint of the phase increment.
     */
    // static constexpr uint16_t f_numbers[12] = {
    //     345, // C
    //     365, // C#
    //     387, // D
    //     410, // D#
    //     435, // E
    //     460, // F
    //     488, // F#
    //     517, // G
    //     547, // G#
    //     580, // A  (440Hz at Block 4)
    //     614, // A#
    //     651  // B
    // };
    // 44100 Hz =>
    static constexpr uint16_t f_numbers[12] = {
        389, // C
        412, // C#
        437, // D
        463, // D#
        490, // E
        519, // F
        550, // F#
        583, // G
        617, // G#
        654, // A  (Should be ~440Hz at Block 4)
        693, // A#
        734  // B
    };


    // Fine-tune multiplier table for -128 to +127 cents
    // Formula: pow(2.0, index / 1200.0)
    static constexpr float fineTuneTable[256] = {
        0.928477f, 0.929014f, 0.929551f, 0.930089f, 0.930626f, 0.931164f, 0.931703f, 0.932242f,
        0.932781f, 0.933320f, 0.933860f, 0.934400f, 0.934940f, 0.935481f, 0.936021f, 0.936563f,
        0.937104f, 0.937646f, 0.938188f, 0.938731f, 0.939274f, 0.939817f, 0.940360f, 0.940904f,
        0.941448f, 0.941992f, 0.942537f, 0.943082f, 0.943627f, 0.944173f, 0.944719f, 0.945265f,
        0.945811f, 0.946358f, 0.946905f, 0.947453f, 0.948000f, 0.948549f, 0.949097f, 0.949646f,
        0.950195f, 0.950744f, 0.951294f, 0.951844f, 0.952394f, 0.952945f, 0.953496f, 0.954047f,
        0.954598f, 0.955150f, 0.955702f, 0.956255f, 0.956807f, 0.957360f, 0.957914f, 0.958467f,
        0.959022f, 0.959576f, 0.960131f, 0.960686f, 0.961241f, 0.961797f, 0.962353f, 0.962909f,
        0.963466f, 0.964023f, 0.964580f, 0.965137f, 0.965695f, 0.966253f, 0.966812f, 0.967371f,
        0.967930f, 0.968489f, 0.970170f, 0.970731f, 0.971292f, 0.971854f, 0.972416f, 0.972978f,
        0.973541f, 0.974104f, 0.974667f, 0.975230f, 0.975794f, 0.976358f, 0.976922f, 0.977487f,
        0.978052f, 0.978617f, 0.979183f, 0.979749f, 0.980315f, 0.980881f, 0.981448f, 0.982015f,
        0.982583f, 0.983150f, 0.983718f, 0.984287f, 0.984855f, 0.985425f, 0.985994f, 0.986564f,
        0.987134f, 0.987704f, 0.988275f, 0.988846f, 0.989417f, 0.989989f, 0.990561f, 0.991133f,
        0.991705f, 0.992278f, 0.992851f, 0.993425f, 0.993998f, 0.994572f, 0.995147f, 0.995721f,
        0.996296f, 0.996871f, 0.997447f, 0.998023f, 0.998599f, 0.999175f, 0.999752f, 1.000329f,
        1.000000f, 1.000578f, 1.001156f, 1.001734f, 1.002313f, 1.002891f, 1.003471f, 1.004050f,
        1.004630f, 1.005210f, 1.005790f, 1.006371f, 1.006952f, 1.007533f, 1.008115f, 1.008697f,
        1.009279f, 1.009861f, 1.010444f, 1.011027f, 1.011611f, 1.012194f, 1.012778f, 1.013363f,
        1.013947f, 1.014532f, 1.015118f, 1.015703f, 1.016289f, 1.016876f, 1.017462f, 1.018049f,
        1.018637f, 1.019224f, 1.019812f, 1.020400f, 1.020989f, 1.021578f, 1.022167f, 1.022757f,
        1.023347f, 1.023937f, 1.024527f, 1.025118f, 1.025709f, 1.026300f, 1.026892f, 1.027484f,
        1.028076f, 1.028669f, 1.029262f, 1.029855f, 1.030449f, 1.031043f, 1.031637f, 1.032231f,
        1.032826f, 1.033421f, 1.034017f, 1.034612f, 1.035209f, 1.035805f, 1.036402f, 1.036999f,
        1.037597f, 1.038194f, 1.038793f, 1.039391f, 1.039990f, 1.040589f, 1.041189f, 1.041788f,
        1.042389f, 1.042989f, 1.043590f, 1.044191f, 1.044793f, 1.045394f, 1.045997f, 1.046599f,
        1.047202f, 1.047805f, 1.048409f, 1.049013f, 1.049617f, 1.050221f, 1.050826f, 1.051431f,
        1.052037f, 1.052643f, 1.053249f, 1.053855f, 1.054462f, 1.055069f, 1.055677f, 1.056285f,
        1.056893f, 1.057502f, 1.058111f, 1.058720f, 1.059329f, 1.059939f, 1.060550f, 1.061160f,
        1.061771f, 1.062383f, 1.062994f, 1.063606f, 1.064219f, 1.064831f, 1.065444f, 1.066058f,
        1.066671f, 1.067286f, 1.067900f, 1.068515f, 1.069130f, 1.069746f, 1.070362f, 1.070978f,
        1.071594f, 1.072211f, 1.072828f, 1.073446f, 1.074064f, 1.074682f, 1.075301f, 1.075920f
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
        bool isDoubleVoice = false; //pseudo 4OP not implemented so far (was the 0x105 stuff is disabled )
        int8_t fineTune = 0;
        uint8_t fixedNote = 0;
        int8_t noteOffset = 0;
        int8_t noteOffset2 = 0; //unused do far;
        uint8_t  velocityOffset = 0; //unused do far;
        uint16_t delayOn   = 0; //unused do far;
        uint16_t delayOff  = 0; //unused do far;


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

                bool operator==(const OpParams& other) const {
                    return ksl == other.ksl && tl == other.tl && multi == other.multi &&
                           attack == other.attack && decay == other.decay &&
                           sustain == other.sustain && release == other.release &&
                           wave == other.wave && ksr == other.ksr &&
                           egTyp == other.egTyp && vib == other.vib && am == other.am;
                }
            } ops[2];

            bool operator==(const OpPair& other) const {
                return feedback == other.feedback &&
                       connection == other.connection &&
                       panning == other.panning &&
                       ops[0] == other.ops[0] &&
                       ops[1] == other.ops[1];
            }
        };

        OpPair pairs[2]; // Pair 0 (Ch A), Pair 1 (Ch B - only used if isFourOp is true)

        bool operator==(const OplInstrument& other) const {
            return name == other.name &&
                   isFourOp == other.isFourOp &&
                   isDoubleVoice == other.isDoubleVoice &&
                   fineTune == other.fineTune &&
                   fixedNote == other.fixedNote &&
                   noteOffset == other.noteOffset &&
                   noteOffset2 == other.noteOffset2 &&
                   velocityOffset == other.velocityOffset &&
                   delayOn == other.delayOn &&
                   delayOff == other.delayOff &&
                   pairs[0] == other.pairs[0] &&
                   pairs[1] == other.pairs[1];
        }
    };
    //--------------------------------------------------------------------------
    // struct SongStep {
    //     uint8_t note = 0;        // 0=None, 1-96 (C-0 to B-7), 255=Off
    //     uint8_t instrument = 0;  // Index to Instrument table
    //     uint8_t volume = 63;     // 0-63 OPL range
    //     uint16_t effect = 0;     // Command (e.g., 0x0Axy for Volume Slide)
    // };

    struct SongStep {
        uint8_t note       = NONE_NOTE;  // 255=None, 1-127 (MIDI range), 128=Off
        uint16_t instrument = 0;  // up to 64k should be enough
        uint8_t volume     = 63; // 0-63 (Standard tracker range)
        uint8_t panning    = 32; // 0 (Left), 32 (Center), 64 (Right)
        uint8_t effectType = 0;  // High byte of effect (e.g., 'A')
        uint8_t effectVal  = 0;  // Low byte of effect (e.g., 0x0F)

        uint8_t dummy[32] = {0}; //space for future use



        bool operator==(const SongStep& other) const {
            return note == other.note &&
                   instrument == other.instrument &&
                   volume == other.volume &&
                   panning == other.panning &&
                   effectType == other.effectType &&
                   effectVal == other.effectVal;
        }
    };
    //--------------------------------------------------------------------------
    struct Pattern {
        std::string name = "New Pattern";
        uint32_t color = 0xFFFFFFFF; // RGBA
        uint16_t rowCount;
        std::vector<SongStep> steps;

         Pattern() = default;

        // Use member initializer list for rowCount
        Pattern(uint16_t rows, int channels = 18) : rowCount(rows) {
            steps.resize(rows * channels);

            // Initialize steps to 'Empty' tracker state
            for (auto& step : steps) {
                step.note = NONE_NOTE;           // None
                step.instrument = 0;     // Default
                step.volume     = MAX_VOLUME + 1; // Max (Tracker Std)
                step.panning    = 32;       // Center
                step.effectType = 0;
                step.effectVal  = 0;
            }
        }

        bool operator==(const Pattern& other) const {
            return name == other.name &&
                   color == other.color &&
                   rowCount == other.rowCount &&
                   steps == other.steps;
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

        void init() {
            // Reset basic info
            title = "New OPL Song";
            bpm = 125.0f;
            speed = 6;

            // Empty the collections
            instruments.clear();
            patterns.clear();
            orderList.clear();
        }

        // Utility: Calculate total song length in rows
        size_t getTotalRows() const {
            size_t total = 0;
            for (uint8_t patIdx : orderList) {
                total += patterns[patIdx].rowCount;
            }
            return total;
        }

        bool operator==(const SongData& other) const {
            return title == other.title &&
                   bpm == other.bpm &&
                   speed == other.speed &&
                   instruments == other.instruments &&
                   patterns == other.patterns &&
                   orderList == other.orderList;
        }
    };
    //--------------------------------------------------------------------------
    inline std::string ValueToNote(uint8_t noteValue) {
        if (noteValue == NONE_NOTE) return "...";
        if (noteValue == STOP_NOTE) return "===";
        if (noteValue > LAST_NOTE)  return "???";

        int noteIndex = noteValue % 12;
        int octave    = (noteValue / 12) - 1;

        std::string octStr = (octave == -1) ? "X" : std::to_string(octave);

        return std::string(NOTE_NAMES[noteIndex]) + octStr;
    }
    //--------------------------------------------------------------------------
    inline uint8_t NoteToValue(const std::string& name) {
        // 1. Precise Fast-Path for Sequencer Commands
        if (name == "...") return NONE_NOTE;
        if (name == "===") return STOP_NOTE;
        if (name == "???") return INVALID_NOTE;

        // 2. Length Validation
        // If it's "???", this catches it.
        if (name.length() != 3) return INVALID_NOTE;

        // 3. Octave Parsing
        int octave;
        if (name[2] == 'X') {
            octave = -1;
        } else if (std::isdigit(name[2])) {
            octave = name[2] - '0';
        } else {
            return INVALID_NOTE; // Catches strings like "C-?"
        }

        // 4. Note Name Validation
        std::string notePart = name.substr(0, 2);
        for (uint8_t i = 0; i < 12; ++i) {
            if (notePart == NOTE_NAMES[i]) {
                // 5. Final Range Calculation
                int result = (octave + 1) * 12 + i;

                // Ensure we don't return something in the 128-255 command range by mistake
                if (result < 0 || result > 127) return INVALID_NOTE;

                return static_cast<uint8_t>(result);
            }
        }

        // 6. Fallback for strings that don't match NOTE_NAMES (like "??X")
        return INVALID_NOTE;
    }

    //--------------------------------------------------------------------------
    struct ParamMeta {
        std::string name;
        uint8_t maxValue;
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
    //--------------------------------------------------------------------------
    const std::vector<int> CHORD_MAJOR     = {0, 4, 7};
    const std::vector<int> CHORD_MINOR     = {0, 3, 7};
    const std::vector<int> CHORD_AUGMENTED = {0, 4, 8};
    const std::vector<int> CHORD_DIMINISHED = {0, 3, 6};
    const std::vector<int> CHORD_MAJOR_7   = {0, 4, 7, 11};
    const std::vector<int> CHORD_MINOR_7   = {0, 3, 7, 10};
    const std::vector<int> CHORD_DOM_7     = {0, 4, 7, 10}; // Dominant 7th



} //namespace opl3
