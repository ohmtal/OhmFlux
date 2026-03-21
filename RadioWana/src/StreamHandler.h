//-----------------------------------------------------------------------------
// Copyright (c) 2012 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#pragma once
#include <curl/curl.h>
#include "utils/errorlog.h"
#include "StreamInfo.h"

#include <iostream>
#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <functional>
#include <map>


namespace FluxRadio {


    class StreamHandler {
    private:
        std::string mUrl;
        std::atomic<bool> mStopRequested{false}; // remember : .store .load  :P
        std::thread mThread;
        std::atomic<bool> mRunning = false;
        StreamInfo mStreamInfo;

        std::string mFullHeader;
        std::mutex mHeaderMutex;


        enum class StreamState { AUDIO, META_LENGTH, META_DATA };
        uint32_t mMetaInt = 0;
        uint32_t mMetaCounter = 0;
        std::string mMetaString = "";

        std::string mStreamTitle = "";
        size_t mStreamTitleAudioByteNeedle = 0;


        StreamState mState = StreamState::AUDIO;
        size_t mBytesToRead = mMetaInt;
        size_t mCurrentMetaSize = 0;

        size_t mTotalAudioBytesSent = 0; //counter for all bytes to sync meta data


        CURL* mCurlHandle = nullptr;


    public:
        std::function<void()> OnConnected = nullptr;
        std::function<void()> onDisConnected = nullptr;

        std::function<void(const std::string, const size_t)> OnStreamTitleUpdate = nullptr;
        std::function<void(const void*, const size_t)> OnAudioChunk = nullptr;



        StreamHandler() = default;
        ~StreamHandler() {
            stop();
            if (mThread.joinable()) mThread.join();
        }

        bool isRunning() {return mRunning.load();}
        StreamInfo* getStreamInfo() { return &mStreamInfo; };
        void dumpInfo() { mStreamInfo.dump(); }



        void stop() {
            mStopRequested.store( true ) ;
        };


    private:
        static size_t HeaderCallback(char* buffer, size_t size, size_t nitems, void* userdata);
        void parseMeta();
        static size_t WriteCallback(void* buffer, size_t size, size_t nmemb, void* userdata);
        static int ProgressCallback(void* clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow);
        void reset();

    public:
        void Execute(std::string url = "");
    }; //class


}
