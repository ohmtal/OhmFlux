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



using namespace opl3;

namespace opl3_bridge_op2 {

    //--------------------- IMPORT -----------------------------

    inline void FillOpParams(OplInstrument::OpPair::OpParams& op, const uint8_t* data, int offset) {
        // Standard OPL register order in OP2: 20, 40, 60, 80, E0
        op.am      = (data[offset + 0] >> 7) & 0x01;
        op.vib     = (data[offset + 0] >> 6) & 0x01;
        op.egTyp   = (data[offset + 0] >> 5) & 0x01;
        op.ksr     = (data[offset + 0] >> 4) & 0x01;
        op.multi   = data[offset + 0] & 0x0F;

        op.ksl     = (data[offset + 1] >> 6) & 0x03;
        op.tl      = data[offset + 1] & 0x3F; // Total Level is index 1

        op.attack  = (data[offset + 2] >> 4) & 0x0F;
        op.decay   = data[offset + 2] & 0x0F;

        op.sustain = (data[offset + 3] >> 4) & 0x0F;
        op.release = data[offset + 3] & 0x0F;

        op.wave    = data[offset + 4] & 0x07; // Waveform is index 4
    }


    inline bool ImportOP2(const std::string& filename, std::vector<OplInstrument>& bank) {
        std::ifstream file(filename, std::ios::binary);
        if (!file) return false;

        char magic[8];
        file.read(magic, 8);
        if (std::string(magic, 8) != "#OPL_II#") return false;

        bank.clear();
        for (int i = 0; i < 175; ++i) {
            OplInstrument inst;
            uint16_t flags;
            uint8_t fineTune, fixedNote;
            uint8_t voice1[16], voice2[16];

            file.read((char*)&flags, 2);
            file.read((char*)&fineTune, 1);
            file.read((char*)&fixedNote, 1);
            file.read((char*)voice1, 16);
            file.read((char*)voice2, 16);

            // Map Voice 1 to Pair 0
            FillOpParams(inst.pairs[0].ops[0], voice1, 0); // Modulator
            FillOpParams(inst.pairs[0].ops[1], voice1, 6); // Carrier
            inst.pairs[0].feedback   = (voice1[5] >> 1) & 0x07;
            inst.pairs[0].connection = voice1[5] & 0x01;

            inst.fineTune = (int8_t)fineTune; // as signed
            inst.fixedNote = fixedNote;
            inst.noteOffset = (int8_t)voice1[10];

            // Check if double-voice (Pseudo 4-Op)
            if (flags & 0x04) {
                inst.isFourOp = true;
                FillOpParams(inst.pairs[1].ops[0], voice2, 0);
                FillOpParams(inst.pairs[1].ops[1], voice2, 6);
                inst.pairs[1].feedback   = (voice2[5] >> 1) & 0x07;
                inst.pairs[1].connection = voice2[5] & 0x01;
            }

            bank.push_back(inst);
        }

        // Optional: Read names at the end of the file
        for (int i = 0; i < 175; ++i) {
            char nameBuf[32];
            file.read(nameBuf, 32);
            bank[i].name = std::string(nameBuf);
        }

        return true;
    }


    //--------------------- EXPORT -----------------------------

    // inline void PackVoice(const OplInstrument::OpPair& pair, uint8_t* out) {
    //     std::memset(out, 0, 16);
    //
    //     // Modulator (Operator 0)
    //     out[0] = (pair.ops[0].am << 7) | (pair.ops[0].vib << 6) | (pair.ops[0].egTyp << 5) |
    //     (pair.ops[0].ksr << 4) | (pair.ops[0].multi & 0x0F);
    //     out[1] = (pair.ops[0].attack << 4) | (pair.ops[0].decay & 0x0F);
    //     out[2] = (pair.ops[0].sustain << 4) | (pair.ops[0].release & 0x0F);
    //     out[3] = pair.ops[0].wave & 0x07;
    //     out[4] = (pair.ops[0].ksl << 6) | (pair.ops[0].tl & 0x3F);
    //
    //     // Feedback / Connection (Register 0xC0)
    //     out[5] = ((pair.feedback & 0x07) << 1) | (pair.connection & 0x01);
    //
    //     // Carrier (Operator 1)
    //     out[6] = (pair.ops[1].am << 7) | (pair.ops[1].vib << 6) | (pair.ops[1].egTyp << 5) |
    //     (pair.ops[1].ksr << 4) | (pair.ops[1].multi & 0x0F);
    //     out[7] = (pair.ops[1].attack << 4) | (pair.ops[1].decay & 0x0F);
    //     out[8] = (pair.ops[1].sustain << 4) | (pair.ops[1].release & 0x0F);
    //     out[9] = pair.ops[1].wave & 0x07;
    //     out[10] = (pair.ops[1].ksl << 6) | (pair.ops[1].tl & 0x3F);
    //
    //     // Bytes 11-15 sind meist Padding oder Note-Offset (hier 0 für Standard)
    // }
    //
    // inline bool ExportOP2(const std::string& filename, const std::vector<OplInstrument>& bank) {
    //     std::ofstream file(filename, std::ios::binary);
    //     if (!file) return false;
    //
    //     // 1. Header schreiben
    //     file.write("#OPL_II#", 8);
    //
    //     // Es müssen immer genau 175 Instrumente sein (Standard GENMIDI)
    //     size_t count = std::min(bank.size(), (size_t)175);
    //
    //     // 2. Instrument-Daten (36 Bytes pro Eintrag)
    //     for (size_t i = 0; i < 175; ++i) {
    //         uint16_t flags = 0;
    //         uint8_t fineTune = 0;
    //         uint8_t fixedNote = 0;
    //         uint8_t voice1[16] = {0};
    //         uint8_t voice2[16] = {0};
    //
    //         if (i < count) {
    //             const auto& inst = bank[i];
    //             if (inst.isFourOp) flags |= 0x04; // Double-Voice Flag
    //
    //             PackVoice(inst.pairs[0], voice1);
    //             if (inst.isFourOp) {
    //                 PackVoice(inst.pairs[1], voice2);
    //             }
    //         }
    //
    //         file.write(reinterpret_cast<const char*>(&flags), 2);
    //         file.write(reinterpret_cast<const char*>(&fineTune), 1);
    //         file.write(reinterpret_cast<const char*>(&fixedNote), 1);
    //         file.write(reinterpret_cast<const char*>(voice1), 16);
    //         file.write(reinterpret_cast<const char*>(voice2), 16);
    //     }
    //
    //     // 3. Instrument-Namen (32 Bytes pro Eintrag, Null-terminated)
    //     for (size_t i = 0; i < 175; ++i) {
    //         char nameBuf[32] = {0};
    //         if (i < count) {
    //             std::strncpy(nameBuf, bank[i].name.c_str(), 31);
    //         }
    //         file.write(nameBuf, 32);
    //     }
    //
    //     return true;
    // }

};
