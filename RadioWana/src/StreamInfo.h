//-----------------------------------------------------------------------------
// Copyright (c) 2012 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#pragma once

#include <iostream>
#include <string>
#include <map>
#include <sstream>
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
        std::string bitRate = "";       // icy-br: 192
        std::string description = "";     // icy-description: RADIO BOB - Power Metal
        std::string name = "";     //  icy-name: RADIO BOB - Power Metal
        std::string url = "";     //  icy-url: http://www.rockantenne.de



        void parseHeader ( const std::string line ) {
            std::string header = line;
            header.erase(header.find_last_not_of("\r\n ") + 1);

            size_t pos = 0;
            if (header.find("Content-Type:") == 0) {
                pos = header.find(": ");
                content_type = header.substr(pos + 2).c_str();
            }

            // [info] https got header: icy-audio-info: ice-channels=2;ice-samplerate=44100;ice-bitrate=128
            else if (header.find("ice-audio-info:") == 0) {
                pos = header.find(": ");
                audio_info = header.substr(pos + 2).c_str();
                ParseIcyAudioInfo(audio_info);
            }

            else if (header.find("icy-br") == 0) {
                pos = header.find(": ");
                bitRate = header.substr(pos + 2).c_str();
            }

            else if (header.find("icy-description:") == 0) {
                pos = header.find(": ");
                description = header.substr(pos + 2).c_str();

            } else if (header.find("icy-name:") == 0) {
                pos = header.find(": ");
                name = header.substr(pos + 2).c_str();
            } else if (header.find("icy-url:") == 0) {
                pos = header.find(": ");
                url = header.substr(pos + 2).c_str();
            }


        }


        void dump() {
            Log("Content Type: %s", content_type.c_str());
            Log("Audio: %d Hz, %d kbps, %d Channels", samplerate, bitrate, channels);
            Log("Bitrate: %s", bitRate.c_str());
            Log("Description: %s", description.c_str());
            Log("Name: %s", name.c_str());
            Log("Url: %s", url.c_str());
        }

    private:
        void ParseIcyAudioInfo(const std::string& info) {
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
    };
};
