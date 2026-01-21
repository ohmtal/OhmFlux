//-----------------------------------------------------------------------------
// Copyright (c) 2026 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#pragma once

#include "opl3.h"
#include <array>
#include <fstream>
#include <string>
#include <cstring>


using namespace opl3;


namespace opl3_bridge_sbi {


    /**
     * @brief Converts raw SBI data to an OplInstrument.
     * @param nameData Pointer to the 32-byte name field in the SBI file.
     * @param regData Pointer to the register data (starting at SBI offset 36).
     * @param inst Output instrument structure.
     */
    inline void _toInstrument(const char* nameData, const uint8_t* regData, OplInstrument& inst) {
        // 1. Extract Name (SBI names are up to 32 chars, null-terminated or space-padded)
        // We use a helper to ensure we don't read past 32 bytes.
        inst.name = std::string(nameData, strnlen(nameData, 32));

        // 2. Operator 0 (Modulator) - Mapping from regData indices
        inst.pairs[0].ops[0].am      = (regData[0] >> 7) & 0x01;
        inst.pairs[0].ops[0].vib     = (regData[0] >> 6) & 0x01;
        inst.pairs[0].ops[0].egTyp   = (regData[0] >> 5) & 0x01;
        inst.pairs[0].ops[0].ksr     = (regData[0] >> 4) & 0x01;
        inst.pairs[0].ops[0].multi   = regData[0] & 0x0F;
        inst.pairs[0].ops[0].ksl     = (regData[2] >> 6) & 0x03;
        inst.pairs[0].ops[0].tl      = regData[2] & 0x3F;
        inst.pairs[0].ops[0].attack  = (regData[4] >> 4) & 0x0F;
        inst.pairs[0].ops[0].decay   = regData[4] & 0x0F;
        inst.pairs[0].ops[0].sustain = (regData[6] >> 4) & 0x0F;
        inst.pairs[0].ops[0].release = regData[6] & 0x0F;
        inst.pairs[0].ops[0].wave    = regData[8] & 0x07;

        // 3. Operator 1 (Carrier)
        inst.pairs[0].ops[1].am      = (regData[1] >> 7) & 0x01;
        inst.pairs[0].ops[1].vib     = (regData[1] >> 6) & 0x01;
        inst.pairs[0].ops[1].egTyp   = (regData[1] >> 5) & 0x01;
        inst.pairs[0].ops[1].ksr     = (regData[1] >> 4) & 0x01;
        inst.pairs[0].ops[1].multi   = regData[1] & 0x0F;
        inst.pairs[0].ops[1].ksl     = (regData[3] >> 6) & 0x03;
        inst.pairs[0].ops[1].tl      = regData[3] & 0x3F;
        inst.pairs[0].ops[1].attack  = (regData[5] >> 4) & 0x0F;
        inst.pairs[0].ops[1].decay   = regData[5] & 0x0F;
        inst.pairs[0].ops[1].sustain = (regData[7] >> 4) & 0x0F;
        inst.pairs[0].ops[1].release = regData[7] & 0x0F;
        inst.pairs[0].ops[1].wave    = regData[9] & 0x07;

        // 4. Global Pair settings (regData[10] corresponds to Register 0xC0)
        inst.pairs[0].feedback   = (regData[10] >> 1) & 0x07;
        inst.pairs[0].connection = regData[10] & 0x01;
        inst.pairs[0].panning    = 3; // OPL3 Center

        // SBI is 2-operator by definition
        inst.isFourOp = false;
    }

    inline bool loadInstrument(const std::string& filename, OplInstrument& inst)
    {
        std::ifstream file(filename, std::ios::binary);
        if (!file.is_open()) return false;

        // Buffer for the entire 51-byte standard SBI file
        // 4 (Sig) + 32 (Name) + 15 (Data) = 51 bytes
        std::array<uint8_t, 51> buffer;
        file.read(reinterpret_cast<char*>(buffer.data()), buffer.size());

        // 1. Verify file size and SBI Signature ("SBI\x1A")
        if (file.gcount() < 51 || std::memcmp(buffer.data(), "SBI\x1A", 4) != 0) {
            return false;
        }

        // 2. Map the data using your _toInstrument function
        // Pass pointer to name (offset 4) and pointer to register data (offset 36)
        _toInstrument(
            reinterpret_cast<const char*>(&buffer[4]),
                      &buffer[36],
                      inst
        );

        file.close();
        return true;
    }






    /**
     * @brief Exports an OplInstrument to a standard SBI file.
     *
     * @param filename The destination path (e.g., "piano.sbi").
     * @param inst The instrument structure to save.
     * @return true if successful, false on file error.
     */
    inline bool saveInstrument(const std::string& filename, const OplInstrument& inst) {
        std::ofstream file(filename, std::ios::binary);
        if (!file.is_open()) return false;

        // --- 1. Header: Signature (4 Bytes) ---
        // Standard SBI signature: "SBI" followed by EOF (0x1A)
        const char signature[] = {'S', 'B', 'I', 0x1A};
        file.write(signature, 4);

        // --- 2. Header: Instrument Name (32 Bytes) ---
        // Pad with null bytes to reach exactly 32 bytes
        std::array<char, 32> nameBuffer;
        nameBuffer.fill(0);
        std::strncpy(nameBuffer.data(), inst.name.c_str(), 31);
        file.write(nameBuffer.data(), 32);

        // --- 3. Data: Hardware Registers (11 Bytes) ---
        // Map the structure fields back into OPL hardware bitmasks
        const auto& mod = inst.pairs[0].ops[0];
        const auto& car = inst.pairs[0].ops[1];
        const auto& pair = inst.pairs[0];

        std::array<uint8_t, 11> regData;

        // 0x20/0x23: Characteristics (AM, VIB, EG, KSR, MULTI)
        regData[0] = (mod.am << 7) | (mod.vib << 6) | (mod.egTyp << 5) | (mod.ksr << 4) | (mod.multi & 0x0F);
        regData[1] = (car.am << 7) | (car.vib << 6) | (car.egTyp << 5) | (car.ksr << 4) | (car.multi & 0x0F);

        // 0x40/0x43: Levels (KSL, TL)
        regData[2] = (mod.ksl << 6) | (mod.tl & 0x3F);
        regData[3] = (car.ksl << 6) | (car.tl & 0x3F);

        // 0x60/0x63: Attack / Decay
        regData[4] = (mod.attack << 4) | (mod.decay & 0x0F);
        regData[5] = (car.attack << 4) | (car.decay & 0x0F);

        // 0x80/0x83: Sustain / Release
        regData[6] = (mod.sustain << 4) | (mod.release & 0x0F);
        regData[7] = (car.sustain << 4) | (car.release & 0x0F);

        // 0xE0/0xE3: Waveform
        regData[8] = (mod.wave & 0x07);
        regData[9] = (car.wave & 0x07);

        // 0xC0: Feedback / Connection
        // Note: SBI is OPL2 standard; Bits 4-5 (Panning) are usually ignored by older software.
        regData[10] = (pair.feedback << 1) | (pair.connection & 0x01);

        file.write(reinterpret_cast<const char*>(regData.data()), 11);

        // --- 4. Padding (5 Bytes) ---
        // Standard SBI files are 51 bytes total (4 + 32 + 15).
        // We wrote 11 bytes of OPL data, so we add 4 more bytes of padding + 1 unused.
        const uint8_t padding[5] = {0, 0, 0, 0, 0};
        file.write(reinterpret_cast<const char*>(padding), 5);

        file.close();
        return true;
    }



} //namespace
