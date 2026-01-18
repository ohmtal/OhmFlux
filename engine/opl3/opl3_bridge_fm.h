//-----------------------------------------------------------------------------
// Copyright (c) 2026 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#pragma once

#include "opl3.h"
#include <array>
#include <fstream>
#include <string>

using namespace opl3;


namespace opl3_bridge_fm {


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



    inline bool loadInstrument(const std::string& filename, std::array<uint8_t, 24>& instrumentData)
    {
        std::ifstream file(filename, std::ios::binary);
        if (!file.is_open()) return false;

        file.read(reinterpret_cast<char*>(instrumentData.data()), instrumentData.size());

        return file.gcount() == 24;
    }

    inline OplInstrument toInstrument(const std::string name, const std::array<uint8_t, 24>& instrumentData) {
        OplInstrument newIns;
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
}
