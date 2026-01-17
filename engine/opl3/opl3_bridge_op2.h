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


/*
 C orrected 16-Byte Voice Mappi*ng
 Each voice in a GENMIDI entry (175 instruments total) consists of two operators (Modulator and Carrier). The 16-byte structure per voice follows this specific pattern:
 Offset
 Parameter	Hardware Reg	Logic
 0	Tremolo/Vibrato/EG/KSR/Multi	0x20
 Combined as bitfield
 1	Key Scale Level (KSL)	0x40	Separate byte (0-3)
 2	Total Level (TL)	0x40	Separate byte (0-63)
 3	Attack Rate	0x60	Separate byte (0-15)
 4	Decay Rate	0x60	Separate byte (0-15)
 5	Sustain Level	0x80	Separate byte (0-15)
 6	Release Rate	0x80	Separate byte (0-15)
 7	Waveform	0xE0	(0-7)
 8	Feedback / Connection	0xC0	Bits 3-1: FB, Bit 0: Conn
 9-15	Reserved / Padding	-	Usually 0

*/


using namespace opl3;

namespace opl3_bridge_op2 {

    //--------------------- IMPORT -----------------------------
    // // for 44KHz
    inline void FillOpParams(OplInstrument::OpPair::OpParams& op, const uint8_t* data, int offset) {
        op.am      = (data[offset + 0] >> 7) & 0x01;
        op.vib     = (data[offset + 0] >> 6) & 0x01;
        op.egTyp   = (data[offset + 0] >> 5) & 0x01;
        op.ksr     = (data[offset + 0] >> 4) & 0x01;
        op.multi   = data[offset + 0] & 0x0F;

        op.ksl     = (data[offset + 1] >> 6) & 0x03;
        op.tl      = data[offset + 1] & 0x3F;

        op.attack  = (data[offset + 2] >> 4) & 0x0F;
        op.decay   = data[offset + 2] & 0x0F;

        op.sustain = (data[offset + 3] >> 4) & 0x0F;
        op.release = data[offset + 3] & 0x0F;

        op.wave    = data[offset + 4] & 0x07;
    }

    inline void FillOpParamsOP2(OplInstrument::OpPair::OpParams& op, const uint8_t* data) {
        // Offset 0: Char (AM, VIB, EG, KSR, Multi)
        op.am      = (data[0] >> 7) & 0x01;
        op.vib     = (data[0] >> 6) & 0x01;
        op.egTyp   = (data[0] >> 5) & 0x01;
        op.ksr     = (data[0] >> 4) & 0x01;
        op.multi   = data[0] & 0x0F;

        // Offsets 1 & 2: KSL and TL are separate in OP2!
        op.ksl     = data[1] & 0x03;
        op.tl      = data[2] & 0x3F;

        // Offsets 3 & 4: Attack and Decay are separate!
        op.attack  = data[3] & 0x0F;
        op.decay   = data[4] & 0x0F;

        // Offsets 5 & 6: Sustain and Release are separate!
        op.sustain = data[5] & 0x0F;
        op.release = data[6] & 0x0F;

        // Offset 7: Waveform
        op.wave    = data[7] & 0x07;
    }


    inline bool ImportOP2(const std::string& filename, std::vector<OplInstrument>& bank, bool isGENMIDI = false) {
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

            inst.isFourOp = false; // GENMIDI is NEVER hardware 4-Op
            inst.isDoubleVoice = (flags & 0x04) != 0; // Custom flag for your player
            inst.fineTune = (int8_t)fineTune;
            inst.fixedNote = fixedNote;

            // Note Offset is stored at byte 14 of the voice record, NOT byte 10
            // (Byte 10 is Carrier Waveform)
            inst.noteOffset = (int8_t)(voice1[14] | (voice1[15] << 8));

            // --- Voice 1 (Pair 0) ---
            if (isGENMIDI)
            {
                FillOpParamsOP2(inst.pairs[0].ops[0], voice1); // Modulator (Uses bytes 0-7)
                FillOpParamsOP2(inst.pairs[0].ops[1], voice1 + 8); // Carrier starts after FB byte? NO.
            } else {
                FillOpParams(inst.pairs[0].ops[0], voice1, 0); // Modulator (0-5)
                FillOpParams(inst.pairs[0].ops[1], voice1, 7); // Carrier (7-12)
            }

            inst.pairs[0].feedback   = (voice1[6] >> 1) & 0x07;
            inst.pairs[0].connection = voice1[6] & 0x01;

            // --- Voice 2 (Pair 1) ---
            if (inst.isDoubleVoice) {
                if (isGENMIDI)
                {
                    FillOpParamsOP2(inst.pairs[0].ops[0], voice1); // Modulator (Uses bytes 0-7)
                    FillOpParamsOP2(inst.pairs[0].ops[1], voice1 + 8); // Carrier starts after FB byte? NO.
                } else {
                    FillOpParams(inst.pairs[1].ops[0], voice2, 0);
                    FillOpParams(inst.pairs[1].ops[1], voice2, 7);
                }
                inst.pairs[1].feedback   = (voice2[6] >> 1) & 0x07;
                inst.pairs[1].connection = voice2[6] & 0x01;
            }

            bank.push_back(inst);
        }

        // Read names (same as your original, this is correct)
        for (int i = 0; i < 175; ++i) {
            char nameBuf[32] = {0};
            file.read(nameBuf, 32);
            bank[i].name = std::string(nameBuf);
        }
        return true;
    }

    // inline void FillOpParams(OplInstrument::OpPair::OpParams& op, const uint8_t* data, int offset) {
    //     // Standard OPL register order in OP2: 20, 40, 60, 80, E0
    //     op.am      = (data[offset + 0] >> 7) & 0x01;
    //     op.vib     = (data[offset + 0] >> 6) & 0x01;
    //     op.egTyp   = (data[offset + 0] >> 5) & 0x01;
    //     op.ksr     = (data[offset + 0] >> 4) & 0x01;
    //     op.multi   = data[offset + 0] & 0x0F;
    //
    //     op.ksl     = (data[offset + 1] >> 6) & 0x03;
    //     op.tl      = data[offset + 1] & 0x3F; // Total Level is index 1
    //
    //     op.attack  = (data[offset + 2] >> 4) & 0x0F;
    //     op.decay   = data[offset + 2] & 0x0F;
    //
    //     op.sustain = (data[offset + 3] >> 4) & 0x0F;
    //     op.release = data[offset + 3] & 0x0F;
    //
    //     op.wave    = data[offset + 4] & 0x07; // Waveform is index 4
    // }


    // inline bool ImportOP2(const std::string& filename, std::vector<OplInstrument>& bank) {
    //     std::ifstream file(filename, std::ios::binary);
    //     if (!file) return false;
    //
    //     char magic[8];
    //     file.read(magic, 8);
    //     if (std::string(magic, 8) != "#OPL_II#") return false;
    //
    //     bank.clear();
    //     for (int i = 0; i < 175; ++i) {
    //         OplInstrument inst;
    //         uint16_t flags;
    //         uint8_t fineTune, fixedNote;
    //         uint8_t voice1[16], voice2[16];
    //
    //         file.read((char*)&flags, 2);
    //         file.read((char*)&fineTune, 1);
    //         file.read((char*)&fixedNote, 1);
    //         file.read((char*)voice1, 16);
    //         file.read((char*)voice2, 16);
    //
    //         // Map Voice 1 to Pair 0
    //         FillOpParams(inst.pairs[0].ops[0], voice1, 0); // Modulator
    //         FillOpParams(inst.pairs[0].ops[1], voice1, 6); // Carrier
    //         inst.pairs[0].feedback   = (voice1[5] >> 1) & 0x07;
    //         inst.pairs[0].connection = voice1[5] & 0x01;
    //
    //         //HACKFEST:
    //         // if (inst.pairs[0].ops[1].multi > 8) {
    //         //     inst.pairs[0].ops[1].multi = 1; // Force standard pitch tracking
    //         // }
    //
    //         inst.fineTune = (int8_t)fineTune; // as signed
    //         inst.fixedNote = fixedNote;
    //         inst.noteOffset = (int8_t)voice1[10];
    //
    //         // Check if double-voice (Pseudo 4-Op)
    //         if (flags & 0x04) {
    //             inst.isFourOp = true;
    //             FillOpParams(inst.pairs[1].ops[0], voice2, 0);
    //             FillOpParams(inst.pairs[1].ops[1], voice2, 6);
    //             inst.pairs[1].feedback   = (voice2[5] >> 1) & 0x07;
    //             inst.pairs[1].connection = voice2[5] & 0x01;
    //         }
    //
    //         bank.push_back(inst);
    //     }
    //
    //     // Optional: Read names at the end of the file
    //     for (int i = 0; i < 175; ++i) {
    //         char nameBuf[32];
    //         file.read(nameBuf, 32);
    //         bank[i].name = std::string(nameBuf);
    //     }
    //
    //     return true;
    // }
    //

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
