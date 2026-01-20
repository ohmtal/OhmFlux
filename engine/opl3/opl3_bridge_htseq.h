//-----------------------------------------------------------------------------
// Copyright (c) 2026 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#pragma once

#include "opl3.h"
#include <fstream>
#include <vector>
#include <string>
#include <array>
#include <cstdint>
#include <stdexcept> // For std::runtime_error

namespace opl3_bridge_htseq {

    // Unique file identifier and version
    const char FILE_IDENTIFIER[] = "HTSEQ\0";
    constexpr uint16_t FILE_VERSION = 1;

    // Helper to write fundamental types
    template <typename T>
    void write_binary(std::ofstream& ofs, const T& value) {
        ofs.write(reinterpret_cast<const char*>(&value), sizeof(T));
    }

    // Helper to read fundamental types
    template <typename T>
    void read_binary(std::ifstream& ifs, T& value) {
        ifs.read(reinterpret_cast<char*>(&value), sizeof(T));
    }

    // Helper to write std::string
    void write_string(std::ofstream& ofs, const std::string& str) {
        uint32_t length = str.length();
        write_binary(ofs, length);
        ofs.write(str.data(), length);
    }

    // Helper to read std::string
    void read_string(std::ifstream& ifs, std::string& str) {
        uint32_t length;
        read_binary(ifs, length);
        str.resize(length);
        ifs.read(str.data(), length);
    }

    // Helper to write std::vector of POD types
    template <typename T>
    void write_vector(std::ofstream& ofs, const std::vector<T>& vec) {
        uint32_t size = vec.size();
        write_binary(ofs, size);
        if (size > 0) {
            ofs.write(reinterpret_cast<const char*>(vec.data()), size * sizeof(T));
        }
    }

    // Helper to read std::vector of POD types
    template <typename T>
    void read_vector(std::ifstream& ifs, std::vector<T>& vec) {
        uint32_t size;
        read_binary(ifs, size);
        vec.resize(size);
        if (size > 0) {
            ifs.read(reinterpret_cast<char*>(vec.data()), size * sizeof(T));
        }
    }

    // Forward declarations for serialization functions
    void write_opl_instrument(std::ofstream& ofs, const opl3::OplInstrument& inst);
    void read_opl_instrument(std::ifstream& ifs, opl3::OplInstrument& inst);

    void write_pattern(std::ofstream& ofs, const opl3::Pattern& pat);
    void read_pattern(std::ifstream& ifs, opl3::Pattern& pat);

    void write_song_data(std::ofstream& ofs, const opl3::SongData& song);
    void read_song_data(std::ifstream& ifs, opl3::SongData& song);

    // Implementations for nested structs
    void write_opl_instrument(std::ofstream& ofs, const opl3::OplInstrument& inst) {
        write_string(ofs, inst.name);
        write_binary(ofs, inst.isFourOp);
        write_binary(ofs, inst.isDoubleVoice);
        write_binary(ofs, inst.fineTune);
        write_binary(ofs, inst.fixedNote);
        write_binary(ofs, inst.noteOffset);
        write_binary(ofs, inst.noteOffset2);
        write_binary(ofs, inst.velocityOffset);
        write_binary(ofs, inst.delayOn);
        write_binary(ofs, inst.delayOff);

        for (int i = 0; i < 2; ++i) {
            write_binary(ofs, inst.pairs[i].feedback);
            write_binary(ofs, inst.pairs[i].connection);
            write_binary(ofs, inst.pairs[i].panning);
            for (int j = 0; j < 2; ++j) {
                write_binary(ofs, inst.pairs[i].ops[j].ksl);
                write_binary(ofs, inst.pairs[i].ops[j].tl);
                write_binary(ofs, inst.pairs[i].ops[j].multi);
                write_binary(ofs, inst.pairs[i].ops[j].attack);
                write_binary(ofs, inst.pairs[i].ops[j].decay);
                write_binary(ofs, inst.pairs[i].ops[j].sustain);
                write_binary(ofs, inst.pairs[i].ops[j].release);
                write_binary(ofs, inst.pairs[i].ops[j].wave);
                write_binary(ofs, inst.pairs[i].ops[j].ksr);
                write_binary(ofs, inst.pairs[i].ops[j].egTyp);
                write_binary(ofs, inst.pairs[i].ops[j].vib);
                write_binary(ofs, inst.pairs[i].ops[j].am);
            }
        }
    }

    void read_opl_instrument(std::ifstream& ifs, opl3::OplInstrument& inst) {
        read_string(ifs, inst.name);
        read_binary(ifs, inst.isFourOp);
        read_binary(ifs, inst.isDoubleVoice);
        read_binary(ifs, inst.fineTune);
        read_binary(ifs, inst.fixedNote);
        read_binary(ifs, inst.noteOffset);
        read_binary(ifs, inst.noteOffset2);
        read_binary(ifs, inst.velocityOffset);
        read_binary(ifs, inst.delayOn);
        read_binary(ifs, inst.delayOff);

        for (int i = 0; i < 2; ++i) {
            read_binary(ifs, inst.pairs[i].feedback);
            read_binary(ifs, inst.pairs[i].connection);
            read_binary(ifs, inst.pairs[i].panning);
            for (int j = 0; j < 2; ++j) {
                read_binary(ifs, inst.pairs[i].ops[j].ksl);
                read_binary(ifs, inst.pairs[i].ops[j].tl);
                read_binary(ifs, inst.pairs[i].ops[j].multi);
                read_binary(ifs, inst.pairs[i].ops[j].attack);
                read_binary(ifs, inst.pairs[i].ops[j].decay);
                read_binary(ifs, inst.pairs[i].ops[j].sustain);
                read_binary(ifs, inst.pairs[i].ops[j].release);
                read_binary(ifs, inst.pairs[i].ops[j].wave);
                read_binary(ifs, inst.pairs[i].ops[j].ksr);
                read_binary(ifs, inst.pairs[i].ops[j].egTyp);
                read_binary(ifs, inst.pairs[i].ops[j].vib);
                read_binary(ifs, inst.pairs[i].ops[j].am);
            }
        }
    }

    void write_pattern(std::ofstream& ofs, const opl3::Pattern& pat) {
        write_string(ofs, pat.name);
        write_binary(ofs, pat.color);
        write_binary(ofs, pat.rowCount);
        // SongStep is a POD-like struct, so we can write the vector directly
        write_vector(ofs, pat.steps);
    }

    void read_pattern(std::ifstream& ifs, opl3::Pattern& pat) {
        read_string(ifs, pat.name);
        read_binary(ifs, pat.color);
        read_binary(ifs, pat.rowCount);
        read_vector(ifs, pat.steps);
    }

    // Main serialization function
    bool saveSongData(const std::string& filePath, const opl3::SongData& song) {
        std::ofstream ofs(filePath, std::ios::binary);
        if (!ofs.is_open()) {
            return false;
        }

        // Write identifier and version
        ofs.write(FILE_IDENTIFIER, sizeof(FILE_IDENTIFIER) -1); // -1 to not write the null terminator
        write_binary(ofs, FILE_VERSION);

        // Write SongData members
        write_string(ofs, song.title);
        write_binary(ofs, song.bpm);
        write_binary(ofs, song.speed);

        // Write instruments
        uint32_t numInstruments = song.instruments.size();
        write_binary(ofs, numInstruments);
        for (const auto& inst : song.instruments) {
            write_opl_instrument(ofs, inst);
        }

        // Write patterns
        uint32_t numPatterns = song.patterns.size();
        write_binary(ofs, numPatterns);
        for (const auto& pat : song.patterns) {
            write_pattern(ofs, pat);
        }

        // Write orderList
        write_vector(ofs, song.orderList);

        ofs.close();
        return true;
    }

    // Main deserialization function
    bool loadSongData(const std::string& filePath, opl3::SongData& song) {
        std::ifstream ifs(filePath, std::ios::binary);
        if (!ifs.is_open()) {
            return false;
        }

        // Read and verify identifier
        char identifierBuffer[sizeof(FILE_IDENTIFIER) -1];
        ifs.read(identifierBuffer, sizeof(FILE_IDENTIFIER) -1);
        if (std::string(identifierBuffer, sizeof(FILE_IDENTIFIER) -1) != std::string(FILE_IDENTIFIER, sizeof(FILE_IDENTIFIER) -1)) {
            // Identifier mismatch
            ifs.close();
            return false;
        }

        // Read and verify version
        uint16_t version;
        read_binary(ifs, version);
        if (version != FILE_VERSION) {
            // Version mismatch
            ifs.close();
            return false;
        }

        // Read SongData members
        read_string(ifs, song.title);
        read_binary(ifs, song.bpm);
        read_binary(ifs, song.speed);

        // Read instruments
        uint32_t numInstruments;
        read_binary(ifs, numInstruments);
        song.instruments.resize(numInstruments);
        for (uint32_t i = 0; i < numInstruments; ++i) {
            read_opl_instrument(ifs, song.instruments[i]);
        }

        // Read patterns
        uint32_t numPatterns;
        read_binary(ifs, numPatterns);
        song.patterns.resize(numPatterns);
        for (uint32_t i = 0; i < numPatterns; ++i) {
            read_pattern(ifs, song.patterns[i]);
        }

        // Read orderList
        read_vector(ifs, song.orderList);

        ifs.close();
        return true;
    }

} // namespace opl3_bridge_htseq
