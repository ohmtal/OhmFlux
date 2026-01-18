//-----------------------------------------------------------------------------
// Copyright (c) 1993 T.Huehn (XXTH)
// Copyright (c) 2026 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#pragma once

#include "opl3.h"
#include <fstream>
#include <vector>
#include <string>
#include <cstdint>
#include <cstring>

// FIXME this is NOT working !!!!


namespace opl3_bridge_wopl {

    // Helper to fill OplInstrument::OpPair::OpParams from Wopl instrument data (62 bytes)
    // op_idx: 0=Mod1, 1=Car1, 2=Mod2, 3=Car2
    inline void FillOpParamsFromWopl(opl3::OplInstrument::OpPair::OpParams& op, const uint8_t* instData, int op_idx) {
        // Offset mapping based on WOPL spec: Operators start at byte 10
        // Each operator block is 13 bytes: [20, 40, 60, 80, E0, BaseDetune]
        const uint8_t* o = instData + 10 + (op_idx * 13);

        op.am      = (o[0] >> 7) & 0x01;
        op.vib     = (o[0] >> 6) & 0x01;
        op.egTyp   = (o[0] >> 5) & 0x01;
        op.ksr     = (o[0] >> 4) & 0x01;
        op.multi   = o[0] & 0x0F;

        op.ksl     = (o[1] >> 6) & 0x03;
        op.tl      = o[1] & 0x3F;

        op.attack  = (o[2] >> 4) & 0x0F;
        op.decay   = o[2] & 0x0F;

        op.sustain = (o[3] >> 4) & 0x0F;
        op.release = o[3] & 0x0F;

        op.wave    = o[4] & 0x07;
    }

    // Helper to pack OpParams back into Wopl format
    inline void PackOpParamsToWopl(const opl3::OplInstrument::OpPair::OpParams& op, uint8_t* instData, int op_idx) {
        uint8_t* o = instData + 10 + (op_idx * 13);
        o[0] = (op.am << 7) | (op.vib << 6) | (op.egTyp << 5) | (op.ksr << 4) | (op.multi & 0x0F);
        o[1] = (op.ksl << 6) | (op.tl & 0x3F);
        o[2] = (op.attack << 4) | (op.decay & 0x0F);
        o[3] = (op.sustain << 4) | (op.release & 0x0F);
        o[4] = (op.wave & 0x07);
    }

    //--------------------- IMPORT -----------------------------
    inline bool importBank(const std::string& filename, std::vector<opl3::OplInstrument>& bank) {

        std::ifstream f(filename, std::ios::binary);
        if (!f) return false;

        char header[12];
        f.read(header, 12); // "WOPL3-BANK\0\0"
        if (std::strncmp(header, "WOPL3-BANK", 10) != 0) return false;

        f.seekg(19, std::ios::beg); // Skip version and metadata header

        // WOPL usually contains 128 melodic + 47 percussion = 175 total instruments
        bank.clear();
        for (int i = 0; i < 175; ++i) {
            uint8_t d[62];
            f.read(reinterpret_cast<char*>(d), 62);
            if (f.gcount() < 62) break;

            opl3::OplInstrument inst;
            inst.isFourOp = (d[0] & 0x01);
            // Pseudo 4-op (isDoubleVoice) is usually bit 0x02 in WOPL flag byte
            inst.isDoubleVoice = (d[0] & 0x02);

            inst.noteOffset = static_cast<int8_t>(d[1]);
            // WOPL 62-byte name starts at byte 30 (for 32 bytes)
            char nameBuf[33];
            std::memcpy(nameBuf, &d[30], 32);
            nameBuf[32] = '\0';
            inst.name = std::string(nameBuf);

            // Pair 0
            inst.pairs[0].feedback = (d[2] >> 1) & 0x07;
            inst.pairs[0].connection = d[2] & 0x01;
            FillOpParamsFromWopl(inst.pairs[0].ops[0], d, 0);
            FillOpParamsFromWopl(inst.pairs[0].ops[1], d, 1);

            // Pair 1 (if 4-op)
            inst.pairs[1].feedback = (d[3] >> 1) & 0x07;
            inst.pairs[1].connection = d[3] & 0x01;
            FillOpParamsFromWopl(inst.pairs[1].ops[0], d, 2);
            FillOpParamsFromWopl(inst.pairs[1].ops[1], d, 3);

            bank.push_back(inst);
        }

        return true;
    }

    //--------------------- EXPORT -----------------------------
    inline bool exportBank(const std::string& filename, const std::vector<opl3::OplInstrument>& bank) {
        std::ofstream f(filename, std::ios::binary);
        if (!f) return false;

        // Write Header
        f.write("WOPL3-BANK\0\0", 12);
        uint8_t ver[3] = {3, 0, 0}; // Version 3
        f.write(reinterpret_cast<char*>(ver), 3);
        uint16_t counts[2] = {128, 47}; // Melodic, Percussion
        f.write(reinterpret_cast<char*>(counts), 4);

        for (const auto& inst : bank) {
            uint8_t d[62];
            std::memset(d, 0, 62);

            d[0] |= inst.isFourOp ? 0x01 : 0x00;
            d[0] |= inst.isDoubleVoice ? 0x02 : 0x00;
            d[1] = static_cast<uint8_t>(inst.noteOffset);

            d[2] = (inst.pairs[0].feedback << 1) | (inst.pairs[0].connection & 0x01);
            d[3] = (inst.pairs[1].feedback << 1) | (inst.pairs[1].connection & 0x01);

            PackOpParamsToWopl(inst.pairs[0].ops[0], d, 0);
            PackOpParamsToWopl(inst.pairs[0].ops[1], d, 1);
            PackOpParamsToWopl(inst.pairs[1].ops[0], d, 2);
            PackOpParamsToWopl(inst.pairs[1].ops[1], d, 3);

            // Name (last 32 bytes)
            std::strncpy(reinterpret_cast<char*>(&d[30]), inst.name.c_str(), 31);

            f.write(reinterpret_cast<char*>(d), 62);
        }

        return true;
    }
};
