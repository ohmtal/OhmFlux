//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Internet Radio Stream Info
//-----------------------------------------------------------------------------
#include "StreamInfo.h"

namespace FluxRadio {
    //--------------------------------------------------------------------------
    void StreamInfo::parseHeader(const std::string headerData) {

        content_type = FluxNet::NetTools::getHeaderValue(headerData, "Content-Type");
        audio_info   = FluxNet::NetTools::getHeaderValue(headerData, "ice-audio-info");
        if (audio_info.empty()) audio_info = FluxNet::NetTools::getHeaderValue(headerData, "icy-audio-info");
        if (!audio_info.empty()) ParseIcyAudioInfo(audio_info);
        bitRate      = FluxNet::NetTools::getHeaderValue(headerData, "icy-br");
        description  = FluxNet::NetTools::getHeaderValue(headerData, "icy-description");
        name         = FluxNet::NetTools::getHeaderValue(headerData, "icy-name");
        url          = FluxNet::NetTools::getHeaderValue(headerData, "icy-url");
    };
    //--------------------------------------------------------------------------
    void StreamInfo::dump(){
        Log("Content Type: %s", content_type.c_str());
        Log("Audio: %d Hz, %d kbps, %d Channels", samplerate, bitrate, channels);
        Log("Bitrate: %s", bitRate.c_str());
        Log("Description: %s", description.c_str());
        Log("Name: %s", name.c_str());
        Log("Url: %s", url.c_str());
    }
    //--------------------------------------------------------------------------
    void StreamInfo::ParseIcyAudioInfo(const std::string& info) {
        if (info.empty()) return;
        std::map<std::string, std::string> params;
        std::stringstream ss(info);
        std::string item;

        while (std::getline(ss, item, ';')) {
            size_t sep = item.find('=');
            if (sep != std::string::npos) {
                std::string key = item.substr(0, sep);
                std::string value = item.substr(sep + 1);

                key.erase(0, key.find_first_not_of(" \t\r\n"));
                key.erase(key.find_last_not_of(" \t\r\n") + 1);
                value.erase(0, value.find_first_not_of(" \t\r\n"));
                value.erase(value.find_last_not_of(" \t\r\n") + 1);

                params[key] = value;
            }
        }

        try {
            auto getParam = [&](const std::string& key1, const std::string& key2) -> int {
                if (params.count(key1)) return std::stoi(params[key1]);
                if (params.count(key2)) return std::stoi(params[key2]);
                return 0; // Fallback
            };

            int newSamplerate = getParam("samplerate", "ice-samplerate");
            int newBitrate    = getParam("bitrate",    "ice-bitrate");
            int newChannels   = getParam("channels",   "ice-channels");

            if (newSamplerate > 0) samplerate = newSamplerate;
            if (newBitrate > 0)    bitrate    = newBitrate;
            if (newChannels > 0)   channels   = newChannels;


            Log("Audio Config: %d Hz, %d kbps, %d Ch", samplerate, bitrate, channels);
        } catch (const std::exception& e) {
            Log("[error] ParseIcyAudioInfo failed: %s (String was: '%s')", e.what(), info.c_str());
        }
    }
    //--------------------------------------------------------------------------

};
