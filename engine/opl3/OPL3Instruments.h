//-----------------------------------------------------------------------------
// Copyright (c) 2026 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#pragma once

#include "opl3.h"

using namespace opl3;

OplInstrument GetDefaultInstrument() {
    OplInstrument ins;
    ins.name = "Default FM Sine";
    ins.isFourOp = false; // Start with standard 2-Op OPL2 style
    ins.fineTune = 0;
    ins.fixedNote = 0;

    // Pointer to the first pair (Modulator and Carrier)
    auto& pair = ins.pairs[0];

    // Shared Channel Params (Register $C0)
    pair.feedback = 0;      // No self-modulation
    pair.connection = 0;    // 0 = FM mode (Modulator modulates Carrier)
    pair.panning = 3;       // 3 = Center (OPL3 Stereo)

    // Indices for clarity: 0 is Modulator, 1 is Carrier
    auto& mod = pair.ops[0];
    auto& car = pair.ops[1];

    // Modulator Settings (The "Timbre")
    mod.multi   = 0x01;
    mod.tl      = 0x10; // Medium output (the "brightness" of the FM)
    mod.attack  = 0x0F; // Instant start
    mod.decay   = 0x00;
    mod.sustain = 0x07;
    mod.release = 0x07;
    mod.wave    = 0x00; // Pure Sine
    mod.ksl     = 0x00;
    mod.ksr     = 0x00; // Envelope Scaling
    mod.egTyp   = 0x00; // 0 = Percussive, 1 = Sustaining
    mod.vib     = 0x00;
    mod.am      = 0x00;

    // Carrier Settings (The "Volume")
    car.multi   = 0x01;
    car.tl      = 0x00; // 0 is MAXIMUM volume in OPL hardware
    car.attack  = 0x0F;
    car.decay   = 0x00;
    car.sustain = 0x07;
    car.release = 0x07;
    car.wave    = 0x00;
    car.ksl     = 0x00;
    car.ksr     = 0x00;
    car.egTyp   = 0x00;
    car.vib     = 0x00;
    car.am      = 0x00;

    return ins;
}
