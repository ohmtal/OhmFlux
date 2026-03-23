//-----------------------------------------------------------------------------
// Copyright (c) 2012 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#pragma once

#include <algorithm>
#include <string>

namespace FluxNet::NetTools {


    // ------------------- URL handling  ----------------------------

    struct URLParts {
        std::string protocol;
        std::string hostname;
        std::string path;
    };

    inline URLParts parseURL(const std::string& url) {
        URLParts parts;
        size_t pos = url.find("://");

        size_t hostStart = 0;
        if (pos != std::string::npos) {
            parts.protocol = url.substr(0, pos);
            hostStart = pos + 3;
        }

        if (hostStart >= url.length()) return parts;

        size_t pathStart = url.find('/', hostStart);
        if (pathStart != std::string::npos) {
            parts.hostname = url.substr(hostStart, pathStart - hostStart);
            parts.path = url.substr(pathStart);
        } else {
            parts.hostname = url.substr(hostStart);
            parts.path = "/";
        }

        return parts;
    }

    inline bool isValidURL(const std::string& url) {
        if (url.empty()) return false;
        if (url.find(' ') != std::string::npos) return false;
        URLParts parts = parseURL(url);
        if (parts.hostname.empty()) return false;
        if (parts.hostname.find('.') == std::string::npos && parts.hostname != "localhost") {
            return false;
        }
        if (parts.protocol.empty()) {
            return false;
        }

        return true;
    }

    // ------------------- http header ----------------------------
    inline std::string getHeaderValue(std::string headerData, std::string key) {
        if (!key.empty() && key.back() == ':') key.pop_back();

        std::string lowerKey = key;
        std::transform(lowerKey.begin(), lowerKey.end(), lowerKey.begin(), ::tolower);

        std::string headerBlock = headerData;
        std::transform(headerBlock.begin(), headerBlock.end(), headerBlock.begin(), ::tolower);

        size_t keyPos = headerBlock.find(lowerKey + ":");
        if (keyPos != std::string::npos) {
            size_t valueStart = keyPos + lowerKey.length() + 1;

            size_t lineEnd = headerData.find("\r\n", valueStart);
            if (lineEnd == std::string::npos) lineEnd = headerData.length();

            while (valueStart < lineEnd && (headerData[valueStart] == ' ' || headerData[valueStart] == '\t')) {
                valueStart++;
            }

            return headerData.substr(valueStart, lineEnd - valueStart);
        }
        return "";
    }


}
