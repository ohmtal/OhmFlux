//-----------------------------------------------------------------------------
// Copyright (c) 2026 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// !! FIXME precussion added on bottom with redundant import code !!
//    and without names ... TODO......
//-----------------------------------------------------------------------------
// file format specification:
// https://github.com/Wohlstand/OPL3BankEditor/blob/master/Specifications/WOPL-and-OPLI-Specification.txt
//
//
//-----------------------------------------------------------------------------

#pragma once

#include "opl3.h"
#include "patch_names.h"

#include <fstream>
#include <vector>
#include <string>
#include <cstdint>
#include <cstring>
#include <format>
#include <string_view>
#include <algorithm>




namespace opl3_bridge_wopl {

    std::string error = "";
    std::string debug = "";

    // Helper to fill OplInstrument::OpPair::OpParams from Wopl instrument data (62 bytes)
    // op_idx: 0=Mod1, 1=Car1, 2=Mod2, 3=Car2
    inline void FillOpParamsFromWopl(opl3::OplInstrument::OpPair::OpParams& op, const uint8_t* d, int offset) {
        // Byte 0: AM(1) Vib(1) EGT(1) KSR(1) Multi(4)
        op.am    = (d[offset] >> 7) & 0x01;
        op.vib   = (d[offset] >> 6) & 0x01;
        op.egTyp = (d[offset] >> 5) & 0x01;
        op.ksr   = (d[offset] >> 4) & 0x01;
        op.multi = (d[offset] & 0x0F);

        // Byte 1: KSL(2) TL(6)
        op.ksl   = (d[offset + 1] >> 6) & 0x03;
        op.tl    = (d[offset + 1] & 0x3F);

        // Byte 2: Attack(4) Decay(4)
        op.attack = (d[offset + 2] >> 4) & 0x0F;
        op.decay  = (d[offset + 2] & 0x0F);

        // Byte 3: Sustain(4) Release(4)
        op.sustain = (d[offset + 3] >> 4) & 0x0F;
        op.release = (d[offset + 3] & 0x0F);

        // Byte 4: Waveform(3)
        op.wave = (d[offset + 4] & 0x07);
    }


    //--------------------- IMPORT -----------------------------

    inline bool importBank(const std::string& filename, std::vector<opl3::OplInstrument>& bank) {

        std::ifstream f(filename, std::ios::binary);
        if (!f) return false;

        error = "";
        debug = "";

        char buffer[64];
        // 11            | Magic number "WOPL3-BANK\0". Where '\0' is a zero byte  which termiates the string
        f.read(buffer, 11); // "WOPL3-BANK\0"
        if (std::strncmp(buffer, "WOPL3-BANK\0", 11) != 0) {
            error += std::format("Invalid Magic {}\n", buffer);
            return false;
        }
        debug += "* magic ok\n";

        // 2 | Version. Little endian Unsigned 16-bit integer. Latest version is 3
        f.read(buffer, 2);
        uint16_t version = static_cast<uint8_t>(buffer[0]) | (static_cast<uint8_t>(buffer[1]) << 8);

        debug += std::format("* found version {}\n", version);

        // 2 | [MBanks] Unsigned 16-bit BE integer, count of melodic MIDI banks
        f.read(buffer, 2);
        uint16_t MBanks = (static_cast<uint8_t>(buffer[0]) << 8) | static_cast<uint8_t>(buffer[1]);
        debug += std::format("* melodic bank count {}\n", MBanks);

        // 2  | [PBanks] Unsigned 16-bit BE integer, count of percussion  MIDI banks (every bank contains 128 instruments)
        f.read(buffer, 2);
        uint16_t PBanks = (static_cast<uint8_t>(buffer[0]) << 8) | static_cast<uint8_t>(buffer[1]);

        debug += std::format("* percussion bank count {}\n", PBanks);

        // skip 2 bytes
        // 1  8-bit unsigned integer, Global bank ...
        // 1 ADLMIDI's volume scaling model ...
        f.seekg(2,  std::ios::cur);

        // SKIP
        // (repeat MBanks times)
        // 32            | Name of melodic bank null-terminated string
        // 1             | LSB index of bank (unsigned char)
        // 1             | MSB index of bank (unsigned char)
        // --VERSION >= 2---Percussion bank meta-data--
        // (repeat PBanks times)
        // 32            | Name of melodic bank null-terminated string
        // 1             | LSB index of bank (unsigned char)
        // 1             | MSB index of bank (unsigned char)

        if (version >= 2) {
            size_t bytes_to_skip = 34ULL * (MBanks + PBanks);
            f.seekg(bytes_to_skip, std::ios::cur);
            debug += std::format("SKIP {} bytes (bank meta data)\n", bytes_to_skip);
        }

        // InsSize:
        // --62 bytes in up to version 2
        // --66 bytes since version 3 and later
        //
        // -----------Melodic Instruments--------------
        // InsSize * 128 * MBanks  | 128 [Single-instrument entries] per each bank,
        // |     look at top of this text file
        // ---------Percussion Instruments-------------
        // InsSize * 128 * PBanks  | 128 [Single-instrument entries] per each bank,
        // |     look at top of this text file
        // --------------------------------------------

        bank.clear();

        uint8_t bytes_to_read = version > 2 ? 66 : 62;
        uint8_t needle = 0;
        uint8_t d[66];

        for (uint16_t idx_bank = 0; idx_bank < MBanks; idx_bank++)
        {
            for (uint8_t idx_inst = 0; idx_inst < 128; idx_inst++)
            {
                f.read(reinterpret_cast<char*>(d), bytes_to_read);
                if (f.gcount() < bytes_to_read) {
                    // guess the file is currupted
                    bank.clear();
                    f.close();
                    return false;
                }
                opl3::OplInstrument inst;
                needle = 0;

                // 1. Name (Bytes 0-31)
                inst.name = std::string(reinterpret_cast<const char*>(&d[needle]),
                                        strnlen(reinterpret_cast<const char*>(&d[needle]), 32));


                if (inst.name.empty()) {
                     inst.name = GM_PATCH_NAMES[idx_inst];
                }

                needle += 32;

                // 2. Note Offsets (Bytes 32-35)
                // Master offset
                inst.noteOffset = (static_cast<int16_t>(d[needle]) << 8) | d[needle + 1];
                needle += 2;
                // Second voice offset
                inst.noteOffset2 = (static_cast<int16_t>(d[needle]) << 8) | d[needle + 1];
                needle += 2;

                // 3. Velocity and Detune (Bytes 36-37)
                int8_t velocityOffset = static_cast<int8_t>(d[needle++]);  //FIXME
                // int8_t detune = static_cast<int8_t>(d[needle++]); // //FIXME  Used if isDoubleVoice is true
                inst.fineTune = static_cast<int8_t>(d[needle++]); // Mapping Detune to your fineTune

                // 4. Percussion Key and Flags (Bytes 38-39)
                inst.fixedNote = d[needle++]; // Percussion key number
                uint8_t flags = d[needle++];

                inst.isFourOp      = (flags & 0x01) != 0;
                inst.isDoubleVoice = (flags & 0x02) != 0;
                // inst.isBlank    = (flags & 0x04) != 0; //FIXME

                // 5. Feedback / Connection (Bytes 40-41)
                // Byte 40: Feedback/Conn for Ops 1 & 2
                inst.pairs[0].feedback   = (d[needle] >> 1) & 0x07;
                inst.pairs[0].connection = (d[needle] & 0x01);
                needle++;

                // Byte 41: Feedback/Conn for Ops 3 & 4
                inst.pairs[1].feedback   = (d[needle] >> 1) & 0x07;
                inst.pairs[1].connection = (d[needle] & 0x01);
                needle++;

                // 6. Operator Data (Bytes 42-61)
                // We map WOPL's sequence to your Modulator/Carrier slots

                // --- Pair 0 (Ch A) ---
                // WOPL Op 1 is the CARRIER for Pair 0
                FillOpParamsFromWopl(inst.pairs[0].ops[1], d, needle);
                needle += 5;
                // WOPL Op 2 is the MODULATOR for Pair 0
                FillOpParamsFromWopl(inst.pairs[0].ops[0], d, needle);
                needle += 5;

                // --- Pair 1 (Ch B) ---
                // WOPL Op 3 is the CARRIER for Pair 1
                FillOpParamsFromWopl(inst.pairs[1].ops[1], d, needle);
                needle += 5;
                // WOPL Op 4 is the MODULATOR for Pair 1
                FillOpParamsFromWopl(inst.pairs[1].ops[0], d, needle);
                needle += 5;


                // 7. Version 3 Extra Fields (Bytes 62-65)
                if (version >= 3) {
                    inst.delayOn   = (static_cast<uint16_t>(d[needle]) << 8) | d[needle + 1]; //FIXME
                    needle += 2;
                    inst.delayOff = (static_cast<uint16_t>(d[needle]) << 8) | d[needle + 1]; //FIXME
                    needle += 2;
                    // Map these to your struct if you add delay fields later
                }
                bank.push_back(inst);
            }
        }

        // Basicly the same as before but i dont have the names :/
        for (uint16_t idx_bank = 0; idx_bank < PBanks; idx_bank++)
        {
            for (uint8_t idx_inst = 0; idx_inst < 128; idx_inst++)
            {
                f.read(reinterpret_cast<char*>(d), bytes_to_read);
                if (f.gcount() < bytes_to_read) {
                    // guess the file is currupted
                    bank.clear();
                    f.close();
                    return false;
                }


                // DEBUG:
                // for (size_t i = 0; i < bytes_to_read; ++i) {
                //     // Cast to unsigned char to avoid sign extension
                //     printf("%02x ", static_cast<unsigned char>(d[i]));
                // }
                // printf("\n");

                // check instrument is empty ....
                // checking the first 39 bytes
                bool is_empty = std::all_of(d, d + 39, [](char c) {
                     return c == 0;
                });
                if (is_empty)
                    continue;





                opl3::OplInstrument inst;
                needle = 0;

                // 1. Name (Bytes 0-31)
                inst.name = std::string(reinterpret_cast<const char*>(&d[needle]),
                                        strnlen(reinterpret_cast<const char*>(&d[needle]), 32));



                if (inst.name.empty()) {
                    inst.name = std::format("Percussion {:01}{:03}", idx_bank, idx_inst);
                }

                needle += 32;

                // 2. Note Offsets (Bytes 32-35)
                // Master offset
                inst.noteOffset = (static_cast<int16_t>(d[needle]) << 8) | d[needle + 1];
                needle += 2;
                // Second voice offset
                inst.noteOffset2 = (static_cast<int16_t>(d[needle]) << 8) | d[needle + 1];
                needle += 2;

                // 3. Velocity and Detune (Bytes 36-37)
                int8_t velocityOffset = static_cast<int8_t>(d[needle++]);  //FIXME
                // int8_t detune = static_cast<int8_t>(d[needle++]); // //FIXME  Used if isDoubleVoice is true
                inst.fineTune = static_cast<int8_t>(d[needle++]); // Mapping Detune to your fineTune

                // 4. Percussion Key and Flags (Bytes 38-39)
                inst.fixedNote = d[needle++]; // Percussion key number
                uint8_t flags = d[needle++];

                inst.isFourOp      = (flags & 0x01) != 0;
                inst.isDoubleVoice = (flags & 0x02) != 0;
                // inst.isBlank    = (flags & 0x04) != 0; //FIXME

                // 5. Feedback / Connection (Bytes 40-41)
                // Byte 40: Feedback/Conn for Ops 1 & 2
                inst.pairs[0].feedback   = (d[needle] >> 1) & 0x07;
                inst.pairs[0].connection = (d[needle] & 0x01);
                needle++;

                // Byte 41: Feedback/Conn for Ops 3 & 4
                inst.pairs[1].feedback   = (d[needle] >> 1) & 0x07;
                inst.pairs[1].connection = (d[needle] & 0x01);
                needle++;

                // 6. Operator Data (Bytes 42-61)
                // We map WOPL's sequence to your Modulator/Carrier slots

                // --- Pair 0 (Ch A) ---
                // WOPL Op 1 is the CARRIER for Pair 0
                FillOpParamsFromWopl(inst.pairs[0].ops[1], d, needle);
                needle += 5;
                // WOPL Op 2 is the MODULATOR for Pair 0
                FillOpParamsFromWopl(inst.pairs[0].ops[0], d, needle);
                needle += 5;

                // --- Pair 1 (Ch B) ---
                // WOPL Op 3 is the CARRIER for Pair 1
                FillOpParamsFromWopl(inst.pairs[1].ops[1], d, needle);
                needle += 5;
                // WOPL Op 4 is the MODULATOR for Pair 1
                FillOpParamsFromWopl(inst.pairs[1].ops[0], d, needle);
                needle += 5;


                // 7. Version 3 Extra Fields (Bytes 62-65)
                if (version >= 3) {
                    inst.delayOn   = (static_cast<uint16_t>(d[needle]) << 8) | d[needle + 1]; //FIXME
                    needle += 2;
                    inst.delayOff = (static_cast<uint16_t>(d[needle]) << 8) | d[needle + 1]; //FIXME
                    needle += 2;
                    // Map these to your struct if you add delay fields later
                }
                bank.push_back(inst);
            }
        }


        printf("%s", debug.c_str());;

        f.close();
        return true;
    }

    //--------------------- EXPORT -----------------------------

};
