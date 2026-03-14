//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#pragma once
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <fstream>
#include <algorithm>

struct SearchResult {
    std::string path;
    int line;
    std::string content;
};

class FileSearcher {
public:
    std::vector<SearchResult> results;
    std::mutex resultsMutex;
    bool isSearching = false;

    void startSearch(const std::vector<std::string>& files, std::string term, bool ignoreCase) {
        if (isSearching) return;

        results.clear();
        isSearching = true;

        mSearchThread = std::jthread([this, files, term, ignoreCase](std::stop_token st) {
            std::string termLower = term;
            if (ignoreCase) std::transform(termLower.begin(), termLower.end(), termLower.begin(), ::tolower);

            for (const auto& path : files) {
                if (st.stop_requested()) break; // Abbruch-Check

                std::ifstream file(path);
                std::string line;
                int lineNum = 1;

                while (std::getline(file, line)) {
                    std::string lineCheck = line;
                    if (ignoreCase) std::transform(lineCheck.begin(), lineCheck.end(), lineCheck.begin(), ::tolower);

                    if (lineCheck.find(termLower) != std::string::npos) {
                        std::lock_guard lock(resultsMutex);
                        results.push_back({path, lineNum, line});
                    }
                    lineNum++;
                }
            }
            isSearching = false;
        });
    }

private:
    std::jthread mSearchThread;
};
