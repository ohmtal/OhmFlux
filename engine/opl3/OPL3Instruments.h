//-----------------------------------------------------------------------------
// Copyright (c) 2026 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#pragma once

#include "opl3_base.h"
#include "opl3_bridge_fm.h"


namespace OPL3InstrumentPresets {

    //------------------------------------------------------------------------------
    inline std::array< uint8_t, 24 > GetDefaultInstrument()
    {
        return
        {
            0x01, 0x01, // 0-1: Modulator/Carrier Frequency
            0x10, 0x00, // 2-3: Modulator/Carrier Output
            0x0F, 0x0F, // 4-5: Modulator/Carrier Attack
            0x00, 0x00, // 6-7: Modulator/Carrier Decay
            0x07, 0x07, // 8-9: Modulator/Carrier Sustain
            0x07, 0x07, // 10-11: Modulator/Carrier Release
            0x00, 0x00, // 12-13: Modulator/Carrier Waveform
            0x00, 0x00, // 14-15: Modulator/Carrier EG Typ
            0x00, 0x00, // 16-17: Modulator/Carrier Vibrato
            0x00, 0x00, // 18-19: Modulator/Carrier Amp Mod
            0x00, 0x00, // 20-21: Feedback / Modulation Mode
            0x00, 0x00  // 22-23: Modulator/Carrier Scaling
        };
    }

    //------------------------------------------------------------------------------
    inline std::array< uint8_t, 24 > GetDefaultLeadSynth()
    {
        return {
            0x01, 0x01, // 0-1: Multiplier
            0x12, 0x00, // 2-3: Output
            0x0F, 0x0F, // 4-5: Attack
            0x05, 0x02, // 6-7: Decay
            0x0F, 0x0F, // 8-9: Sustain
            0x05, 0x05, // 10-11: Release
            0x02, 0x00, // 12-13: Waveform
            0x01, 0x01, // 14-15: EG Typ
            0x00, 0x00, // 16-17: Vibrato
            0x00, 0x00, // 18-19: Amp Mod
            0x05, 0x00, // 20-21: Feedback  5  Mode: FM
            0x00, 0x00  // 22-23: Scaling
        };
    }
    //------------------------------------------------------------------------------
    inline std::array< uint8_t, 24 > GetDefaultOrgan()
    {
        return {
            0x01, 0x02, // 0-1: Multiplier (Mod=1 [Base], Car=2 [Octave/Harmonic])
            0x08, 0x00, // 2-3: Output (Both loud; Adjust Mod output to change 'drawbar' mix)
            0x0F, 0x0F, // 4-5: Attack (Instant for both)
            0x00, 0x00, // 6-7: Decay (None)
            0x0F, 0x0F, // 8-9: Sustain (Full - Organs don't fade)
            0x02, 0x02, // 10-11: Release (Very short - stops immediately)
            0x00, 0x00, // 12-13: Waveform (Pure Sines)
            0x01, 0x01, // 14-15: EG Typ (Sustain ON)
            0x01, 0x01, // 16-17: Vibrato (ON - Adds the 'Leslie Speaker' feel)
            0x00, 0x00, // 18-19: Amp Mod (OFF)
            0x00, 0x01, // 20-21: Feedback 0, Connection 1 (Additive Mode - CRITICAL)
            0x00, 0x00  // 22-23: Scaling (OFF)
        };
    }
    //------------------------------------------------------------------------------
    inline std::array< uint8_t, 24 > GetDefaultCowbell()
    {
        return {
            0x01, 0x04, // 0-1: Multiplier (Mod=1, Car=4: Creates a high, resonant 'clink')
            0x15, 0x00, // 2-3: Output (Mod=21 [Moderate FM bite], Car=0 [Loud])
            0x0F, 0x0F, // 4-5: Attack (Instant hit)
            0x06, 0x08, // 6-7: Decay (Modulator drops fast to clean the tone, Carrier lingers)
            0x00, 0x00, // 8-9: Sustain (Must be 0 for percussion)
            0x08, 0x08, // 10-11: Release (Fast fade-out)
            0x00, 0x00, // 12-13: Waveform (Pure Sine)
            0x00, 0x00, // 14-15: EG Typ (Decay mode)
            0x00, 0x00, // 16-17: Vibrato (Off)
            0x00, 0x00, // 18-19: Amp Mod (Off)
            0x07, 0x00, // 20-21: Feedback 7 (Max grit for 'metal' feel), Mode FM
            0x00, 0x00  // 22-23: Scaling
        };
    }

    //------------------------------------------------------------------------------
    inline std::array< uint8_t, 24 > GetTomsStrings() // MÖNCH.FMI
    {
        return {
            0x01, 0x00, 0x20, 0x00, 0x0f, 0x01, 0x00, 0x00, 0x07, 0x0f, 0x07, 0x07,
            0x00, 0x03, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x07, 0x00, 0x00, 0x00
        };
    }
    //------------------------------------------------------------------------------
    inline std::array< uint8_t, 24 > GetTomsGuitar() // SYSN1.FMI
    {
        return {
            0x00, 0x00, 0x0a, 0x00, 0x0f, 0x0f, 0x02, 0x00, 0x0f, 0x0f, 0x0f, 0x0f,
            0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        };
    }
    //------------------------------------------------------------------------------
    inline std::array< uint8_t, 24 > GetTomsGuitar2() // GUITAR01_FMI
    {
        return {
            0x01, 0x00, 0x20, 0x00, 0x01, 0x0f, 0x05, 0x00, 0x0f, 0x0f, 0x00, 0x0f,
            0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        };
    }
    //------------------------------------------------------------------------------
    inline std::array< uint8_t, 24 > GetTomsHiHat() // HIHAT_FMI
    {
        return {
            0x0f, 0x01, 0x00, 0x00, 0x0F, 0x09, 0x0f, 0x08, 0x00, 0x04, 0x00, 0x0b,
            0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        };
    }

    //------------------------------------------------------------------------------
    inline std::array< uint8_t, 24 > GetTomsBassDrum() // BASDRUM2_FMI
    {
        return {
            0x00, 0x00, 0x0f, 0x00, 0x0f, 0x0f, 0x08, 0x04, 0x06, 0x0f, 0x08, 0x0f,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        };
    }
    //------------------------------------------------------------------------------
    inline std::array< uint8_t, 24 > GetTomsBell() // BELL_FMI
    {
        return {
            0x01, 0x00, 0x0a, 0x00, 0x0f, 0x0f, 0x05, 0x05, 0x05, 0x05, 0x0b, 0x0b,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00

        };
    }


    //------------------------------------------------------------------------------
    //------------------------------------------------------------------------------
   inline const int Count  = 12;

   inline const char* names[Count] = {
         "Basic Piano",
         "Bass",
         "Strings",
         "Lead Synth",
         "Organ",
         "Cow Bell",

         "Tom's Strings",
         "Tom's Guitar",
         "Tom's Guitar II",
         "Tom's HiHat",
         "Tom's Bass Drum",
         "Tom's Bell",

    };

    inline opl3::Instrument  GetMelodicDefault(uint8_t index) {


        std::array< uint8_t, 24 > data = GetDefaultInstrument(); // Start with your basic Sine template
        std::string insName = "Melodic default Instrument " + std::to_string(index);

        int idx = index % Count;

        insName = names[idx];

        switch (idx) {
            case 0: // Basic Piano
                data[4] = 0xF; data[5] = 0xF; // Fast Attack
                data[6] = 0x4; data[7] = 0x4; // Moderate Decay
                data[8] = 0x2; data[9] = 0x2; // Low Sustain

                break;
            case 1: // FM Bass
                data[20] = 0x5;               // High Feedback (Index 20)
                data[2] = 0x15;               // Higher Modulator Output for grit
                break;
            case 2: // Strings
                data[4] = 0x3; data[5] = 0x2; // Slow Attack
                data[10] = 0x5; data[11] = 0x5; // Slower Release
                // insName = "strings";
                break;
            case  3: data = GetDefaultLeadSynth();/*insName = "Lead Synth";*/ break;
            case  4: data = GetDefaultOrgan();/*insName = "Organ";*/ break;
            case  5: data = GetDefaultCowbell();/*insName = "Cow Bell"; */ break;
            case  6: data = GetTomsStrings(); /*insName = "Tom's Strings";*/ break;
            case  7: data = GetTomsGuitar(); break;
            case  8: data = GetTomsGuitar2(); break;
            case  9: data = GetTomsHiHat(); break;
            case 10: data = GetTomsBassDrum(); break;
            case 11: data = GetTomsBell(); break;
        }

        return  opl3_bridge_fm::toInstrument(insName, data);

    }

} // namespace OPL3Instruments
