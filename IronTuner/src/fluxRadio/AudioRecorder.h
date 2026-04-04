//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// file stream handling for mp3 recordings
//-----------------------------------------------------------------------------
#pragma once

#include "core/fluxGlobals.h"
#include "utils/errorlog.h"
#include "utils/fluxStr.h"

#include <iostream>
#include <filesystem>
#include <string>
#include <fstream>
#include <stdexcept>


namespace FluxRadio {

    class AudioRecorder{
    protected:
        std::string mPath = "";
        std::string mCurrentFilename = "";
        std::ofstream mFileStream;

        uintmax_t mMinRequiredFileSpace = 50 * 1024 * 1024; // 50 MB
        char mStreamBuffer[8192];

    public:
        AudioRecorder();

        std::string getPath() const { return mPath; }
        std::string getCurrentFilename() const { return mCurrentFilename; }
        bool isFileOpen() const { return mFileStream.is_open(); }
        void setPath(const std::string value) { mPath = value;}

        void OnStreamData(const uint8_t* buffer, size_t bufferSize);

        bool openFile(std::string streamTitle, bool append = false);
        void closeFile();

    private:
    };
};
