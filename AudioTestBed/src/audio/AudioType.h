//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// known AudioTypes and detectType by data
//-----------------------------------------------------------------------------
#pragma once

#include <vector>
#include <cstdint>
#include <cstring>
#include <string>
#include <array>
#include <string_view>

namespace FluxAudio {

    enum class AudioType { UNKNOWN, WAV, MP3, OGG, FLAC, SFX };

    constexpr std::array<std::string_view, 6> AudioTypeNames = {
        "UNKNOWN", "WAV", "MP3", "OGG", "FLAC", "SFX"
    };
    constexpr std::string_view to_string(AudioType type) {
        return AudioTypeNames[static_cast<std::size_t>(type)];
    }

    // should be perfect now:
    inline AudioType detectType(const std::vector<uint8_t>& data) {
        if (data.size() < 4) return AudioType::UNKNOWN;

        // --- Unique Fixed Headers (High Priority) ---

        // FluxSFX: Custom format
        if (data.size() >= 7 && std::memcmp(data.data(), "FluxSFX", 7) == 0) {
            return AudioType::SFX;
        }

        // FLAC: "fLaC" (Checked before ID3 to avoid false positives)
        if (std::memcmp(data.data(), "fLaC", 4) == 0) {
            return AudioType::FLAC;
        }

        // OGG: "OggS"
        if (std::memcmp(data.data(), "OggS", 4) == 0) {
            return AudioType::OGG;
        }

        // --- RIFF/WAVE Container (Deep Inspection for wrapped MP3/OGG) ---
        if (data.size() >= 12 &&
            std::memcmp(&data[0], "RIFF", 4) == 0 &&
            std::memcmp(&data[8], "WAVE", 4) == 0) {

            size_t offset = 12;
        while (offset + 8 <= data.size()) {
            const char* chunkId = reinterpret_cast<const char*>(&data[offset]);
            uint32_t chunkSize = *reinterpret_cast<const uint32_t*>(&data[offset + 4]);

            // Search for "fmt " chunk
            if (std::memcmp(chunkId, "fmt ", 4) == 0) {
                if (offset + 10 <= data.size()) {
                    // Format tag is at offset 8 of the fmt chunk
                    uint16_t formatTag = data[offset + 8] | (data[offset + 9] << 8);

                    switch (formatTag) {
                        case 0x0055: return AudioType::MP3; // WAVE_FORMAT_MPEGLAYER3
                        case 0x674F:                        // Ogg Vorbis (Mode 1)
                        case 0x6750:                        // Ogg Vorbis (Mode 2)
                        case 0x6751: return AudioType::OGG; // Ogg Vorbis (Mode 3)
                        default:     return AudioType::WAV;
                    }
                }
                break;
            }
            offset += 8 + chunkSize + (chunkSize % 2); // Jump to next chunk
        }
        return AudioType::WAV; // Fallback for standard RIFF
        }

        // --- Fuzzy Headers (Lower Priority) ---

        // ID3 Skipping can precede MP3, OGG, FLAC or even RIFF!
        if (data.size() >= 10 && std::memcmp(data.data(), "ID3", 3) == 0) {

            // ID3v2 tag size is stored in bytes 6-9.
            // It's a "synchsafe integer" (7 bits per byte).
            uint32_t s0 = data[6];
            uint32_t s1 = data[7];
            uint32_t s2 = data[8];
            uint32_t s3 = data[9];
            uint32_t tagSize = (s0 << 21) | (s1 << 14) | (s2 << 7) | s3;
            size_t totalId3Size = tagSize + 10; // tag + header

            // Boundary check: ensure we don't jump out of the vector
            if (data.size() >= totalId3Size + 4) {
                const uint8_t* afterId3 = data.data() + totalId3Size;
                if (std::memcmp(afterId3, "fLaC", 4) == 0) return AudioType::FLAC;
                if (std::memcmp(afterId3, "OggS", 4) == 0) return AudioType::OGG;
                if (std::memcmp(afterId3, "RIFF", 4) == 0) return AudioType::WAV;
            }

            // Default for ID3 is usually MP3
            return AudioType::MP3;
        }

        // Pure MP3 Frame Sync: 0xFFE0
        if (data.size() >= 2 && data[0] == 0xFF && (data[1] & 0xE0) == 0xE0) {
            return AudioType::MP3;
        }

        return AudioType::UNKNOWN;
    }

}
