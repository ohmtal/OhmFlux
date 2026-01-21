//-----------------------------------------------------------------------------
// Copyright (c) 2026 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#pragma once

#include "opl3.h"
#include "opl3_bridge_fm.h"


namespace OPL3InstrumentPresets {

    // OplInstrument GetDefaultInstrument() {
    //     OplInstrument ins;
    //     ins.name = "Default FM Sine";
    //     ins.isFourOp = false; // Start with standard 2-Op OPL2 style
    //     ins.fineTune = 0;
    //     ins.fixedNote = 0;
    //
    //     // Pointer to the first pair (Modulator and Carrier)
    //     auto& pair = ins.pairs[0];
    //
    //     // Shared Channel Params (Register $C0)
    //     pair.feedback = 0;      // No self-modulation
    //     pair.connection = 0;    // 0 = FM mode (Modulator modulates Carrier)
    //     pair.panning = 3;       // 3 = Center (OPL3 Stereo)
    //
    //     // Indices for clarity: 0 is Modulator, 1 is Carrier
    //     auto& mod = pair.ops[0];
    //     auto& car = pair.ops[1];
    //
    //     // Modulator Settings (The "Timbre")
    //     mod.multi   = 0x01;
    //     mod.tl      = 0x10; // Medium output (the "brightness" of the FM)
    //     mod.attack  = 0x0F; // Instant start
    //     mod.decay   = 0x00;
    //     mod.sustain = 0x07;
    //     mod.release = 0x07;
    //     mod.wave    = 0x00; // Pure Sine
    //     mod.ksl     = 0x00;
    //     mod.ksr     = 0x00; // Envelope Scaling
    //     mod.egTyp   = 0x00; // 0 = Percussive, 1 = Sustaining
    //     mod.vib     = 0x00;
    //     mod.am      = 0x00;
    //
    //     // Carrier Settings (The "Volume")
    //     car.multi   = 0x01;
    //     car.tl      = 0x00; // 0 is MAXIMUM volume in OPL hardware
    //     car.attack  = 0x0F;
    //     car.decay   = 0x00;
    //     car.sustain = 0x07;
    //     car.release = 0x07;
    //     car.wave    = 0x00;
    //     car.ksl     = 0x00;
    //     car.ksr     = 0x00;
    //     car.egTyp   = 0x00;
    //     car.vib     = 0x00;
    //     car.am      = 0x00;
    //
    //     return ins;
    // }
    //

    //------------------------------------------------------------------------------
    std::array< uint8_t, 24 > GetDefaultInstrument()
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
    std::array< uint8_t, 24 > GetDefaultLeadSynth()
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
    std::array< uint8_t, 24 > GetDefaultOrgan()
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
    std::array< uint8_t, 24 > GetDefaultCowbell()
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

    opl3::OplInstrument  GetMelodicDefault(uint8_t index) {


        std::array< uint8_t, 24 > data = GetDefaultInstrument(); // Start with your basic Sine template
        std::string insName = "Melodic default Instrument " + std::to_string(index);

        switch (index % 6) {
            case 0: // Basic Piano
                data[4] = 0xF; data[5] = 0xF; // Fast Attack
                data[6] = 0x4; data[7] = 0x4; // Moderate Decay
                data[8] = 0x2; data[9] = 0x2; // Low Sustain
                insName = "Basic Piano";
                break;
            case 1: // FM Bass
                data[20] = 0x5;               // High Feedback (Index 20)
                data[2] = 0x15;               // Higher Modulator Output for grit
                insName = "Bass";
                break;
            case 2: // Strings
                data[4] = 0x3; data[5] = 0x2; // Slow Attack
                data[10] = 0x5; data[11] = 0x5; // Slower Release
                insName = "strings";
                break;
            case 3: data = GetDefaultLeadSynth();insName = "Lead Synth"; break;
            case 4: data = GetDefaultOrgan();insName = "Organ"; break;
            case 5: data = GetDefaultCowbell();insName = "Cow Bell"; break;
        }

        return  opl3_bridge_fm::toInstrument(insName, data);

    }

} // namespace OPL3Instruments
