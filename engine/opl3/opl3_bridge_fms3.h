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
#include <stdexcept>
#include <type_traits>


namespace opl3_bridge_fms3 {

    // Unique file identifier and version
    const char FILE_IDENTIFIER[] = "Huehn Thomas FM OPL3 Song"; //<< this should be uniqe !!
    constexpr size_t ID_SIZE = sizeof(FILE_IDENTIFIER) - 1;
    constexpr uint16_t FILE_VERSION = 1;

    constexpr uint8_t DUMMYBYTE = 0; // a dummy for reserve bytes

    std::string errors = "";

    void addError(std::string error)
    {
        errors += error + "\n";
    }



    // Helper to write fundamental/trivial types
    template <typename T>
    void write_binary(std::ofstream& ofs, const T& value) {
        // Ensure we aren't accidentally writing complex objects with pointers
        static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable for binary write");

        ofs.write(reinterpret_cast<const char*>(&value), sizeof(T));
    }

    // Helper to read fundamental/trivial types
    template <typename T>
    void read_binary(std::ifstream& ifs, T& value) {
        // Ensure we aren't accidentally reading into complex objects
        static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable for binary read");

        ifs.read(reinterpret_cast<char*>(&value), sizeof(T));
    }


    // Helper to write std::string
    void write_string(std::ofstream& ofs, const std::string& str) {
        // 1. Determine the safe length to write
        uint32_t length = static_cast<uint32_t>(str.length());

        if (length > opl3::MAX_STRING_LENGTH) {
            length = static_cast<uint32_t>(opl3::MAX_STRING_LENGTH);
        }

        // 2. Write the (possibly truncated) length
        write_binary(ofs, length);

        // 3. Write exactly 'length' bytes from the string
        if (length > 0) {
            ofs.write(str.data(), length);
        }
    }

    // Helper to read std::string
    void read_string(std::ifstream& ifs, std::string& str) {
        uint32_t length;
        read_binary(ifs, length);

        // Safety check: prevent allocating gigabytes from a corrupted file
        // This now matches the limit enforced during saving.
        if (length > opl3::MAX_STRING_LENGTH) {
            throw std::runtime_error(std::format("String length {} exceeds limit {}",
                                                 length, opl3::MAX_STRING_LENGTH));
        }

        str.resize(length);
        if (length > 0) {
            ifs.read(str.data(), length);
        }
    }

    // Helper to write std::vector of POD/Trivial types
    template <typename T>
    void write_vector(std::ofstream& ofs, const std::vector<T>& vec) {
        // Compile-time safety: only allow types that can be copied as raw bytes
        static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable for binary I/O");

        uint32_t size = static_cast<uint32_t>(vec.size());

        // Safety check: prevent writing more data than i read, can't resize!
        if (size > opl3::MAX_VECTOR_ELEMENTS) {
            throw std::runtime_error(std::format("Vector size exceeds limit: read {} (max allowed: {})",
                                                 size, opl3::MAX_VECTOR_ELEMENTS));
        }

        write_binary(ofs, size);
        if (size > 0) {
            // ofs.write(reinterpret_cast<const char*>(vec.data()), size * sizeof(T));
            ofs.write(reinterpret_cast<const char*>(vec.data()), static_cast<size_t>(size) * sizeof(T));

        }
    }

    // Helper to read std::vector of POD/Trivial types
    template <typename T>
    void read_vector(std::ifstream& ifs, std::vector<T>& vec) {
        static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable for binary I/O");

        uint32_t size;
        read_binary(ifs, size);

        // Safety check: prevent memory exhaustion attacks
        if (size > opl3::MAX_VECTOR_ELEMENTS) {
            throw std::runtime_error(std::format("Vector size exceeds limit: read {} (max allowed: {})",
                                                 size, opl3::MAX_VECTOR_ELEMENTS));
        }

        vec.resize(size);
        if (size > 0) {
            // ifs.read(reinterpret_cast<char*>(vec.data()), size * sizeof(T));
            ifs.read(reinterpret_cast<char*>(vec.data()), static_cast<size_t>(size) * sizeof(T));
        }
    }

    // Forward declarations for serialization functions
    void write_opl_instrument(std::ofstream& ofs, const opl3::OplInstrument& inst);
    void read_opl_instrument(std::ifstream& ifs, opl3::OplInstrument& inst);

    void write_pattern(std::ofstream& ofs, const opl3::Pattern& pat);
    void read_pattern(std::ifstream& ifs, opl3::Pattern& pat);

    void write_song_data(std::ofstream& ofs, const opl3::SongData& song);
    void read_song_data(std::ifstream& ifs, opl3::SongData& song);

    //--------------------------------------------------------------------------
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

        // write 16 Byte dummy for future use
        // i guess i will never fill that all up but
        // even if we have 256 Instruments * 16 = 4096 come on
        for (uint8_t dummy = 0 ; dummy < 16; dummy++)
            write_binary(ofs, DUMMYBYTE);


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

    //--------------------------------------------------------------------------
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

        // skip 16 Byte dummy
        ifs.seekg(16,  std::ios::cur);


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

    //--------------------------------------------------------------------------
    void write_pattern(std::ofstream& ofs, const opl3::Pattern& pat) {
        write_string(ofs, pat.name);
        write_binary(ofs, pat.color);
        write_binary(ofs, pat.rowCount);
        // SongStep is a POD-like struct, so we can write the vector directly
        write_vector(ofs, pat.steps);

        // write 64 Byte dummy
        for (uint8_t dummy = 0 ; dummy < 64; dummy++)
            write_binary(ofs, DUMMYBYTE);


    }

    //--------------------------------------------------------------------------
    void read_pattern(std::ifstream& ifs, opl3::Pattern& pat) {
        read_string(ifs, pat.name);
        read_binary(ifs, pat.color);
        read_binary(ifs, pat.rowCount);
        read_vector(ifs, pat.steps);

        // skip 64 Byte dummy
        ifs.seekg(64,  std::ios::cur);

    }

    //--------------------------------------------------------------------------
    // Main serialization function
    bool saveSong(const std::string& filePath, const opl3::SongData& song) {
        errors = "";

        if (song.instruments.size() > opl3::MAX_INSTRUMENTS)
        {
            addError(std::format("Instrument count exceed the maximum: read:{} max:{}",
                                 song.instruments.size(),opl3::MAX_INSTRUMENTS ));
            return false;
        }
        if (song.patterns.size() > opl3::MAX_PATTERN)
        {
            addError(std::format("Pattern count exceed the maximum: read:{} max:{}",
                                 song.patterns.size(),opl3::MAX_PATTERN ));
            return false;
        }


        std::ofstream ofs(filePath, std::ios::binary);
        ofs.exceptions(std::ofstream::badbit | std::ofstream::failbit);

        try {
            if (!ofs.is_open()) {
                addError(std::format("Can't open File {} for write.", filePath));
                return false;
            }

            // Write identifier and version
            ofs.write(FILE_IDENTIFIER, ID_SIZE);
            write_binary(ofs, FILE_VERSION);

            // Write SongData members
            write_string(ofs, song.title);
            write_binary(ofs, song.bpm);
            write_binary(ofs, song.ticksPerSecond);

            // 18 byte default instruments for a channel
            for (uint8_t ch = 0; ch < song.CHANNELS; ch ++)
                write_binary(ofs, song.channelInstrument[ch]);

            // write 64 Byte dummy
            for (uint8_t dummy = 0 ; dummy < 64; dummy++)
                write_binary(ofs, DUMMYBYTE);


            // Write instruments
            uint32_t numInstruments = static_cast<uint32_t>(song.instruments.size());
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

            // we also write the Ident on the END to verify end is reached
            ofs.write(FILE_IDENTIFIER, ID_SIZE);


            ofs.close();
            return true;
        } catch (const std::ios_base::failure& e) {
            // Detailed system error (e.g., "No space left on device")
            addError(std::format("Disk I/O Error: {}", e.what()));
            return false;
        } catch (const std::exception& e) {
            addError(std::format("General write exception: {}", e.what()));
            return false;
        }

        return false;
    } // saveSong

    //--------------------------------------------------------------------------
    // Main deserialization function
    bool loadSong(const std::string& filePath, opl3::SongData& song) {
        errors = "";
        std::ifstream ifs(filePath, std::ios::binary);
        ifs.exceptions(std::ifstream::badbit | std::ifstream::failbit);

        try {
            if (!ifs.is_open()) {
                addError(std::format("Can't open File {} for read.", filePath));
                return false;
            }

            // Read and verify identifier
            char identifierBuffer[ID_SIZE];
            ifs.read(identifierBuffer, ID_SIZE);
            if (std::memcmp(identifierBuffer, FILE_IDENTIFIER, ID_SIZE) != 0) {
                addError("File Identifier mismatch");
                return false;
            }

            // Read and verify version
            uint16_t version;
            read_binary(ifs, version);
            if (version != FILE_VERSION) {
                addError(std::format("File Version missmatch read:{} expect:{}", version, FILE_VERSION));
                return false;
            }

            // Read SongData members
            read_string(ifs, song.title);
            read_binary(ifs, song.bpm);
            read_binary(ifs, song.ticksPerSecond);

            // 18 byte default instruments for a channel
            for (uint8_t ch = 0; ch < song.CHANNELS; ch ++)
                read_binary(ifs, song.channelInstrument[ch]);


            // skip 64 Byte dummy
            ifs.seekg(64,  std::ios::cur);

            // Read instruments
            uint32_t numInstruments;
            read_binary(ifs, numInstruments);
            if ( numInstruments > opl3::MAX_INSTRUMENTS )
            {
                addError(std::format("Instrument count exceed the maximum: read:{} max:{}", numInstruments,opl3::MAX_INSTRUMENTS ));
                return false;
            }
            song.instruments.resize(numInstruments);
            for (uint32_t i = 0; i < numInstruments; ++i) {
                read_opl_instrument(ifs, song.instruments[i]);
            }

            // Read patterns
            uint32_t numPatterns;
            read_binary(ifs, numPatterns);
            if ( numPatterns > opl3::MAX_PATTERN ) {
                addError(std::format("Pattern count exceed the maximum: read:{} max:{}", numPatterns,opl3::MAX_PATTERN ));
                return false;

            }
            song.patterns.resize(numPatterns);
            for (uint32_t i = 0; i < numPatterns; ++i) {
                read_pattern(ifs, song.patterns[i]);
            }

            // Read orderList
            read_vector(ifs, song.orderList);


            // Final Identifier Read (verification)
            ifs.read(identifierBuffer, ID_SIZE);
            if (std::memcmp(identifierBuffer, FILE_IDENTIFIER, ID_SIZE) != 0) {
                addError("Trailing FILE_IDENTIFIER is missing or corrupted.");
                return false;
            }

            //find out there is data left so i guess it's a coruppted file
            ifs.exceptions(std::ifstream::badbit);
            ifs.clear();
            ifs.get();
            if (!ifs.eof()) {
                addError("File too long (unexpected trailing data)!");
                return false;
            }

            return true;

        } catch (const std::ios_base::failure& e) {
            // If we catch this, it's because a read_binary or read_vector failed
            if (ifs.eof()) {
                addError("Unexpected End of File: The file is truncated.");
            } else {
                addError(std::format("I/O failure: {}", e.what()));
            }
            return false;
        } catch (const std::bad_alloc&) {
            addError("File requested too much memory (possible corruption).");
            return false;
        } catch (const std::exception& e) {
            addError(std::format("General error: {}", e.what()));
            return false;
        }

        // i should never get here ...
        return false;
    } //<< loadSong


} // namespace opl3_bridge_htseq
