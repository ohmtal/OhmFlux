//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// ResourceManagerBase - at the moment it only handle the blacklist
//-----------------------------------------------------------------------------
#pragma once
#include <vector>
#include <string>
#include <algorithm>

// adding OhmFlux namespace better late than never ;)
namespace OhmFlux {

    class ResourceManagerBase {
    private:
        std::vector<std::string> mBlacklist; //blacklisted filename
    public:
         ResourceManagerBase() = default;

         bool isBlackListed(std::string fileName) {
             return std::find(mBlacklist.begin(), mBlacklist.end(), fileName) != mBlacklist.end();
         }

         void blacklist(std::string fileName) {
             if (!isBlackListed(fileName))
                 mBlacklist.push_back(fileName);
         }
    };
};
