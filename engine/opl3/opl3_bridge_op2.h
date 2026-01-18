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

/*
 s tr*uct OplInstrument {
 std::string name = "New Instrument";
 bool isFourOp = false;  // OPL3 mode
 bool isDoubleVoice = false; //pseudo 4OP not implemented so far (was the 0x105 stuff is disabled )
 int8_t fineTune = 0;
 uint8_t fixedNote = 0;
 int8_t noteOffset = 0;


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
} ops[2];
};

OpPair pairs[2]; // Pair 0 (Ch A), Pair 1 (Ch B - only used if isFourOp is true)
};
*/

    //--------------------- IMPORT -----------------------------
    // // for 44KHz
    inline void FillOpParams(OplInstrument::OpPair::OpParams& op, const uint8_t* data, int offset) {

        //--- 0
        op.am      = (data[offset + 0] >> 7) & 0x01;
        op.vib     = (data[offset + 0] >> 6) & 0x01;
        op.egTyp   = (data[offset + 0] >> 5) & 0x01;
        op.ksr     = (data[offset + 0] >> 4) & 0x01;
        op.multi   = data[offset +  0] & 0x0F;
        //--- 1
        op.attack  = (data[offset + 1] >> 4) & 0x0F;
        op.decay   = data[offset +  1] & 0x0F;
        //--- 2
        op.sustain = (data[offset + 2] >> 4) & 0x0F;
        op.release = data[offset +  2] & 0x0F;
        //--- 3
        op.wave    = data[offset +  3] & 0x07;
        //--- 4
        op.ksl     = (data[offset + 4] >> 6) & 0x03;
        //--- 5
        op.tl      = data[offset +  5] & 0x3F; //????



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

            inst.isFourOp = false; // GENMIDI is NEVER hardware 4-Op
            inst.isDoubleVoice = (flags & 0x04) != 0; // Custom flag for your player

            inst.fineTune = (int8_t)fineTune;
            inst.fixedNote = fixedNote;

            // Note Offset is stored at byte 14 of the voice record, NOT byte 10
            // (Byte 10 is Carrier Waveform)
            inst.noteOffset = (int8_t)(voice1[14] | (voice1[15] << 8));

            // --- Voice 1 (Pair 0) ---
            FillOpParams(inst.pairs[0].ops[0], voice1, 0); // Modulator (0-5)
            FillOpParams(inst.pairs[0].ops[1], voice1, 7); // Carrier (7-12)

            inst.pairs[0].feedback   = (voice1[6] >> 1) & 0x07;
            inst.pairs[0].connection = voice1[6] & 0x01;

            // --- Voice 2 (Pair 1) ---
            if (inst.isDoubleVoice) {
                FillOpParams(inst.pairs[1].ops[0], voice2, 0);
                FillOpParams(inst.pairs[1].ops[1], voice2, 7);
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


};
