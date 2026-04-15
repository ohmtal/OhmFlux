//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Internet Radio Stream Info
//-----------------------------------------------------------------------------
#pragma once

#include <iostream>
#include <string>
#include <map>
#include <sstream>
#include "net/NetTools.h"
#include "utils/errorlog.h"

namespace FluxRadio {

    struct MetaEvent {
        size_t byteOffset;
        std::string streamTitle;
    };


    struct StreamInfo {
        std::string streamUrl = "";
        std::string content_type = ""; // Content-Type: audio/mpeg
        std::string audio_info = "";   // ice-audio-info: samplerate=44100;bitrate=192;channels=2
        uint16_t samplerate = 44100;
        uint16_t bitrate    = 192;
        uint8_t  channels   = 2;
        std::string bitRateStr = "";       // icy-br: 192
        std::string description = "";     // icy-description: RADIO BOB - Power Metal
        std::string name = "";     //  icy-name: RADIO BOB - Power Metal
        std::string url = "";     //  icy-url: http://www.rockantenne.de


        void parseHeader ( const std::string headerData );
        void dump();

    private:
        void ParseIcyAudioInfo(const std::string& info);
    };
};
