//-----------------------------------------------------------------------------
// Copyright (c) 2012 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// String Helpers
//-----------------------------------------------------------------------------
#pragma once

#include <string>

namespace FluxStr {


    //--------------------------------------------------------------------------
    inline std::string truncate(std::string str, size_t width, bool addPoints = true) {
        if (addPoints && width < 4) return str;
        if (str.length() > width) {
            if (addPoints) return str.substr(0, width - 3) + "...";
            else return str.substr(0, width);
        }
        return str;
    }
    //--------------------------------------------------------------------------
    inline std::string removePart(std::string s, const std::string& part) {
        if (part.empty()) return s;
        size_t pos = 0;
        while ((pos = s.find(part, pos)) != std::string::npos) {
            s.erase(pos, part.length());
        }
        return s;
    }

    //--------------------------------------------------------------------------
    inline std::string toUpper(std::string s) {
        std::ranges::transform(s, s.begin(), [](unsigned char c){ return std::toupper(c); });
        return s;
    }
    //--------------------------------------------------------------------------
    inline std::string toLower(std::string s) {
        std::ranges::transform(s, s.begin(), [](unsigned char c){ return std::tolower(c); });
        return s;
    }
    //--------------------------------------------------------------------------
    // Returns the number of words separated by spaces
    inline int getWordCount(std::string_view str) {
        int count = 0;
        size_t pos = str.find_first_not_of(' ');

        while (pos != std::string_view::npos) {
            count++;
            pos = str.find(' ', pos);             // Find next space
            pos = str.find_first_not_of(' ', pos); // Skip extra spaces
        }
        return count;
    }
    //--------------------------------------------------------------------------
    // Helper to find the start/end bounds of a word index
    inline std::string getWords(std::string_view text, int index, int endIndex = 999999) {
        size_t startPos = std::string_view::npos;
        size_t endPos = std::string_view::npos;
        int count = 0;

        size_t cur = text.find_first_not_of(' ');
        while (cur != std::string_view::npos) {
            if (count == index) startPos = cur;

            size_t nextSpace = text.find(' ', cur);
            if (count == endIndex) {
                endPos = nextSpace;
                break;
            }

            count++;
            cur = text.find_first_not_of(' ', nextSpace);
            if (cur == std::string_view::npos) endPos = text.size(); // End of string
        }

        if (startPos == std::string_view::npos) return "";
        if (endPos == std::string_view::npos) endPos = text.size();

        return std::string(text.substr(startPos, endPos - startPos));
    }
    //--------------------------------------------------------------------------
    inline std::string setWord(std::string text, int index, std::string_view replace) {
        int count = 0;
        size_t start = text.find_first_not_of(' ');

        while (start != std::string::npos) {
            size_t end = text.find(' ', start);
            if (count == index) {
                // Replace the segment [start, end) with replace string
                text.replace(start, (end == std::string::npos ? text.size() : end) - start, replace);
                return text;
            }
            count++;
            start = text.find_first_not_of(' ', end);
        }
        return text; // Return original if index not found
    }
    //--------------------------------------------------------------------------
    inline std::string removeWord(std::string text, int index) {
        int count = 0;
        size_t start = text.find_first_not_of(' ');

        while (start != std::string::npos) {
            size_t nextSpace = text.find(' ', start);
            size_t nextWord = text.find_first_not_of(' ', nextSpace);

            if (count == index) {
                // Remove from start of this word to start of next word (to clean up spaces)
                size_t length = (nextWord == std::string::npos) ? text.size() - start : nextWord - start;
                text.erase(start, length);
                return text;
            }
            count++;
            start = nextWord;
        }
        return text;
    }
    //--------------------------------------------------------------------------
    // Returns the word at index 'no' (0-based). Returns empty string if not found.
    inline std::string getWord(std::string_view str, int no) {
        int count = 0;
        size_t start = str.find_first_not_of(' ');

        while (start != std::string_view::npos) {
            size_t end = str.find(' ', start);

            if (count == no) {
                // Found the index; return as a new std::string
                return std::string(str.substr(start, end - start));
            }

            count++;
            start = str.find_first_not_of(' ', end);
        }
        return "";
    }
    //--------------------------------------------------------------------------
    inline std::vector<std::string_view> Tokenize(std::string_view str, char delimiter = ' ') {
        std::vector<std::string_view> tokens;
        size_t start = 0;
        size_t end = str.find(delimiter);

        while (end != std::string_view::npos) {
            if (end > start) { // Skip empty tokens (multiple spaces)
                tokens.push_back(str.substr(start, end - start));
            }
            start = end + 1;
            end = str.find(delimiter, start);
        }

        // Add the final token
        if (start < str.size()) {
            tokens.push_back(str.substr(start));
        }

        return tokens;
    }
    //--------------------------------------------------------------------------
    inline std::optional<int> safeStoi(const std::string& str) {
        try {
            return std::stoi(str);
        } catch (...) {
            return std::nullopt;
        }
    }
    //--------------------------------------------------------------------------
    // Wrapper mit Default-Wert
    inline int strToInt(const std::string& str, int defaultValue) {
        return safeStoi(str).value_or(defaultValue);
    }
    //--------------------------------------------------------------------------
    // ----- filename tools but string only -----
    //--------------------------------------------------------------------------
    inline std::string_view extractFilename(std::string_view path) {
        if (path.empty())
            return "";
        size_t lastSlash = path.find_last_of("/\\");
        if (lastSlash == std::string_view::npos) {
            return path;
        }
        return path.substr(lastSlash + 1);
    }
    //--------------------------------------------------------------------------
    inline std::string sanitizeFilenameWithUnderScores(std::string name)
    {
        std::string result;
        for (unsigned char c : name) {
            if (std::isalnum(c)) {
                result += c;
            } else if (std::isspace(c)) {
                result += '_';
            }
            // Special characters (like '.') are ignored/dropped here
        }
        return result;
    }
    //--------------------------------------------------------------------------
    inline std::string addTrailingSlash(std::string path) {
        if (path.empty()) return "/";

        char lastChar = path.back();
        if (lastChar == '/' || lastChar == '\\') {
            return path;
        }
        return path + "/";
    }
    //--------------------------------------------------------------------------


} //namespace
