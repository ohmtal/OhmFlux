//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#pragma once
#include <string>
#include <vector>
#include <thread>
#include <atomic>
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
    std::string lastSearchTerm = "";
    bool open = false;
    bool isSearching = false;
    std::atomic<bool> stopRequested{false};

    void startSearch(const std::vector<std::string>& files, std::string term, bool ignoreCase) {
        open = true;
        if (isSearching) {
            stopRequested = true;
            if (mSearchThread.joinable()) mSearchThread.join();
        }
        lastSearchTerm = term;
        results.clear();
        isSearching = true;

        mSearchThread = std::thread([this, files, term, ignoreCase]() {
            std::string termLower = term;
            if (ignoreCase) std::transform(termLower.begin(), termLower.end(), termLower.begin(), ::tolower);

            for (const auto& path : files) {
                if (stopRequested) break;

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
                    if (stopRequested) break;
                }
            }
            isSearching = false;
        });
    }

    ~FileSearcher() {
        stopRequested = true;
        if (mSearchThread.joinable()) mSearchThread.join();
    }

private:
    std::thread mSearchThread;
};
