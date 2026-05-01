//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// known AudioTypes
//-----------------------------------------------------------------------------
#pragma once
#include <vector>

namespace FluxAudio {

    enum class AudioType { UNKNOWN, WAV, OGG, MP3, SFX };

    inline AudioType detectType(const std::vector<uint8_t>& data) {
        if (data.size() < 4) return AudioType::UNKNOWN;

        // WAV: "RIFF" .... "WAVE"
        if (data[0] == 'R' && data[1] == 'I' && data[2] == 'F' && data[3] == 'F') return AudioType::WAV;

        // OGG: "OggS"
        if (data[0] == 'O' && data[1] == 'g' && data[2] == 'g' && data[3] == 'S') return AudioType::OGG;

        // MP3: often has not "magic" but starts usually with 0xFF 0xFB (Frame Sync) or "ID3" (Metadata-Tag)
        if (data[0] == 'I' && data[1] == 'D' && data[2] == '3') return AudioType::MP3;
        if (data[0] == 0xFF && (data[1] & 0xE0) == 0xE0) return AudioType::MP3;


        // FluxSFX << SFXGenerator Stereo!
        if (data.size() < 7) return AudioType::UNKNOWN;

        if (data[0] == 'F' && data[1] == 'l' && data[2] == 'u' && data[3] == 'x'
            && data[4] == 'S' && data[5] == 'F' && data[6] == 'X'
        ) return AudioType::SFX;


        return AudioType::UNKNOWN;
    }



}
