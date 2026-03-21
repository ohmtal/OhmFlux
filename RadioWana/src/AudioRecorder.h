//-----------------------------------------------------------------------------
// Copyright (c) 2012 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Audio Steam handling for mp3 recordings
//-----------------------------------------------------------------------------
#pragma once

#include "core/fluxGlobals.h"
#include "utils/errorlog.h"

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
        AudioRecorder(){
            mPath = fluxStr::addTrailingSlash(getMusicPath())+ "radioRecordings/";
            Log("[info] set recording path to: %s", mPath.c_str());

        }


        std::string getPath() const { return mPath; }
        std::string getCurrentFilename() const { return mCurrentFilename; }
        bool isFileOpen() const { return mFileStream.is_open(); }
        void setPath(const std::string value) { mPath = value;}

        void OnStreamData(const uint8_t* buffer, size_t bufferSize) {
            if (mFileStream.is_open()) {
                try {
                    mFileStream.write(reinterpret_cast<const char*>(buffer), bufferSize);
                } catch (const std::exception& e) {
                    Log("[error] Failed to write to file %s: %s", mCurrentFilename.c_str(), e.what());
                    closeFile(); //<< close the stream
                }
            }
        }


        void closeFile() {
            if (mFileStream.is_open()) mFileStream.close();
            mCurrentFilename = "";
        }

        // ---------------------------------------------------------------------
        bool openFile(std::string streamTitle, bool append = false) {

            // ~~~~ set current filename ~~~~~
            std::string newFileName = fluxStr::addTrailingSlash(mPath) + fluxStr::sanitizeFilenameWithUnderScores(streamTitle)+".mp3";
            if ( mCurrentFilename.compare(newFileName) == 0 && mFileStream.is_open()) {
                // who cares dLog("[warn] Recording: We are already on this file ....");
                return true;
            }

            // ~~~~ close current file stream ~~~~~
            if ( mFileStream.is_open() ) {
                dLog("[warn] File is open ! I'll close it here");
                closeFile();
            }

            // new file name after close check it reset it!
            mCurrentFilename = newFileName;

            // ~~~~~ check path exists / create path ~~~~~
            std::filesystem::path p(mPath);
            if (!std::filesystem::exists(p)) {
                std::error_code ec;
                if (std::filesystem::create_directories(p, ec)) {
                    Log("Created Export directory %s", mPath.c_str());
                } else if (ec) {
                    Log("[error] failed to create Export directory %s", mPath.c_str());
                    return false;
                }
            } else {
                if (!std::filesystem::is_directory(p)) {
                    Log("[error] can't use path %s it's not a directory!!", mPath.c_str());
                    return false;
                }
            }

            // ~~~~~ check File Space ~~~~~
            try {
                std::filesystem::space_info si = std::filesystem::space(mPath);
                if (si.available < mMinRequiredFileSpace) {
                    Log("[error] Not enough space on disk! Available: %d MB", static_cast<uint32_t>(si.available / 1024 / 1024));
                    return false;
                }
            } catch (const std::filesystem::filesystem_error& e) {
                Log("[error] Could not check disk space: %s", e.what());
                return false;
            }


            // ~~~~ open File Stream ~~~~
            mFileStream.exceptions(std::ofstream::failbit | std::ofstream::badbit);
            try {
                std::ios_base::openmode flags = std::ios::binary | std::ios::out;
                if (append)  flags |= std::ios::app;
                mFileStream.open(mCurrentFilename, flags);
                mFileStream.rdbuf()->pubsetbuf(mStreamBuffer, sizeof(mStreamBuffer));
            } catch (const std::exception& e) {
                Log("[error] Failed to open File %s: %s", mCurrentFilename.c_str(), e.what());
                return false;
            }

            return mFileStream.is_open();
        }

    private:
    };

};
