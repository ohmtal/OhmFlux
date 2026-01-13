//-----------------------------------------------------------------------------
// Copyright (c) 1993 T.Huehn (XXTH)
// Copyright (c) 2026 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------

#include "opl3.h"

using namespace opl3;

// void convertFmsNotesToSteps(const SongDataFMS& fms, Pattern& modernPattern) {
//     for (int r = 0; r < fms.song_length; ++r) {
//         for (int ch = 0; ch < 9; ++ch) {
//             int16_t legacyVal = fms.song[r][ch + 1];
//
//             // Your new SongStep inside the 18-channel pattern
//             SongStep& step = modernPattern.steps[r * 18 + ch];
//
//             if (legacyVal > 0 && legacyVal <= 84) {
//                 // VALID NOTE: Store the index (1-84) directly
//                 step.note = (uint8_t)legacyVal;
//                 step.instrument = ch; // Associate with the channel's instrument
//             }
//             else if (legacyVal == -1) {
//                 step.note = 255; // Note Off
//             }
//             else {
//                 step.note = 0;   // Empty Row
//             }
//         }
//     }
// }


OplInstrument ImportFMI(const uint8_t lIns[24]) {
    OplInstrument newIns;
    newIns.isFourOp = false; // Old format is always 2-Op

    // Channel-level parameters (Pair 0)
    newIns.pairs[0].feedback = lIns[20];
    newIns.pairs[0].connection = lIns[21];
    newIns.pairs[0].panning = 3; // Default to Center for OPL3

    // Modulator (Op 0) mapping
    auto& mod = newIns.pairs[0].ops[0];
    mod.multi   = lIns[0];
    mod.tl      = lIns[2];
    mod.attack  = lIns[4];
    mod.decay   = lIns[6];
    mod.sustain = lIns[8];
    mod.release = lIns[10];
    mod.wave    = lIns[12];
    mod.ksr     = lIns[14]; // Sustain mode / EG Type
    mod.vib     = lIns[16];
    mod.am      = lIns[18];
    mod.ksl     = lIns[22];

    // Carrier (Op 1) mapping
    auto& car = newIns.pairs[0].ops[1];
    car.multi   = lIns[1];
    car.tl      = lIns[3];
    car.attack  = lIns[5];
    car.decay   = lIns[7];
    car.sustain = lIns[9];
    car.release = lIns[11];
    car.wave    = lIns[13];
    car.ksr     = lIns[15];
    car.vib     = lIns[17];
    car.am      = lIns[19];
    car.ksl     = lIns[23];

    return newIns;
}
