//-----------------------------------------------------------------------------
// Copyright (c) 1993 T.Huehn (XXTH)
// Copyright (c) 2026 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#pragma once

namespace OPL2Instruments {

    //------------------------------------------------------------------------------
    std::array< uint8_t, 24 > GetDefaultInstrument()
    {
        return {
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
    std::array< uint8_t, 24 > GetDefaultBassDrum(){
        return {
            0x01, 0x02, // Freq: Mod=1, Car=2 (gives a thicker sound)
            0x10, 0x00, // Output: Both loud
            0x0F, 0x0F, // Attack: Instant
            0x08, 0x06, // Decay: Rapid for punch
            0x00, 0x00, // Sustain: 0 (Drums shouldn't sustain)
            0x0A, 0x0A, // Release: Quick fade
            0x00, 0x00, // Waveform: Sine
            0x00, 0x00, // EG Typ: 0 (Drums always decay)
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        };
    }
    //------------------------------------------------------------------------------
    std::array< uint8_t, 24 > GetDefaultHiHat(){
        return {
            0x01, 0x01,
            0x12, 0x00, // Mod(HH) slightly quieter than Car(SD)
            0x0F, 0x0F, // Attack: Instant
            0x0D, 0x07, // Decay: HH is very short (D), SD is longer (7)
            0x00, 0x00, // Sustain: 0
            0x0D, 0x07, // Release: Match decay
            0x00, 0x00, // Waveform: Sine (OPL noise gen overrides this)
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        };
    }
    std::array< uint8_t, 24 > GetDefaultSnare() {
        return {
            0x01, 0x01, // 0-1: Multiplier
            0x00, 0x00, // 2-3: Output
            0x0F, 0x0F, // 4-5: Attack
            0x00, 0x07, // 6-7: Decay
            0x00, 0x00, // 8-9: Sustain
            0x00, 0x07, // 10-11: Release
            0x00, 0x00, // 12-13: Waveform (Sine 0x00 is best for OPL noise)
            0x00, 0x00, // Waveform: Sine (OPL noise gen overrides this)
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        };
    }
    //------------------------------------------------------------------------------
    std::array< uint8_t, 24 > GetDefaultCymbal() {
        return {
            0x02, 0x01, // 0-1: Multiplier (Mod/Tom: 2 for a hollower tone, Car/Cym: 1)
            0x12, 0x02, // 2-3: Output (Tom: 18, Cym: 2 [Loud])
            0x0F, 0x0F, // 4-5: Attack (Instant for both)
            0x07, 0x09, // 6-7: Decay (Tom: 7 [Snappy], Cym: 9 [Linger])
            0x00, 0x00, // 8-9: Sustain (Must be 0 for drums)
            0x07, 0x09, // 10-11: Release (Match Decay for clean trigger-off)
            0x00, 0x00, // 12-13: Waveform (Sine is standard)
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 14-21 (Flags/Feedback)
            0x00, 0x00  // 22-23: Scaling
        };
    }
    std::array< uint8_t, 24 > GetDefaultTom(){
        return {
            0x03, 0x00, // 0-1: Multiplier
            0x10, 0x00, // 2-3: Output Level
            0x0F, 0x00, // 4-5: Attack
            0x06, 0x00, // 6-7: Decay
            0x00, 0x00, // 8-9: Sustain (must be zero)
            0x06, 0x00, // 10-11: Release (same as  Decay )
            0x00, 0x00, // 12-13: Waveform
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 14-21: Flags
            0x00, 0x00  // 22-23: Scaling
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


} //namespace OPL2Instruments
