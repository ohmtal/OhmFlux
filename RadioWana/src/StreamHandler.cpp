//-----------------------------------------------------------------------------
// Copyright (c) 2012 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#include "StreamHandler.h"

#include <curl/curl.h>
#include "utils/errorlog.h"
#include "net/CurlGlue.h"

#include "StreamInfo.h"

#include <iostream>
#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <functional>
#include <map>
namespace FluxRadio {

    //--------------------------------------------------------------------------
    size_t StreamHandler::HeaderCallback(char* buffer, size_t size, size_t nitems, void* userdata) {
        size_t totalSize = size * nitems;
        auto* self = static_cast<StreamHandler*>(userdata);
        if (!self) {
            Log("[error]HeaderCallback self in nil!!! ");
            return totalSize;
        }

        std::string header(buffer, totalSize);
        {
            std::lock_guard<std::mutex> lock(self->mHeaderMutex);
            self->mFullHeader += header;
        }

        if (header.find("icy-metaint:") == 0) {
            size_t pos = header.find(":");
            self->mMetaInt = std::atol(header.substr(pos + 1).c_str());
            // self->mBytesToRead = self->mMetaInt;
            dLog("[info] METAINT: %s (metaint:%d)", header.c_str(), self->mMetaInt);

        } else if (header == "\r\n" || header == "\n") {

            long http_code = 0;
            if (self->mCurlHandle) {
                curl_easy_getinfo(self->mCurlHandle, CURLINFO_RESPONSE_CODE, &http_code);
                if (http_code == 200) {
                    self->mState = StreamState::AUDIO;
                    self->mBytesToRead = self->mMetaInt;
                    self->mStreamInfo.parseHeader(self->mFullHeader);

                    if (self->OnConnected) {
                        self->OnConnected();
                    }
                    self->mConnected.store(true);
                } else if (http_code >= 300 && http_code < 400) {
                    self->mFullHeader.clear();
                }
            }
            dLog("[info] HTTP COMPLETE: %s (metaint:%d) HTTP CODE: %d", header.c_str(), self->mMetaInt, (int)http_code);

        } else {
            dLog("[info] header: %s", header.c_str());
        }
        return totalSize;
    }
    //--------------------------------------------------------------------------
    void StreamHandler::parseMeta() {
        if (mMetaString.empty()) return;

        dLog("[warn] META DATA: %s", mMetaString.c_str());

        size_t start = mMetaString.find("StreamTitle='");
        if (start != std::string::npos) {
            start += 13;
            size_t end = mMetaString.find("';", start);
            if (end != std::string::npos) {
                //only update if it's different
                if (mStreamTitle.compare(mMetaString) != 0) {
                    mStreamTitle = mMetaString.substr(start, end - start);
                    // dLog("New StreamTitle: %s", mStreamTitle.c_str());
                    mStreamTitleAudioByteNeedle = mTotalAudioBytesSent;
                    if (OnStreamTitleUpdate) OnStreamTitleUpdate( mStreamTitle, mStreamTitleAudioByteNeedle);
                }
            }
        }
    }
    //--------------------------------------------------------------------------
    size_t StreamHandler::WriteCallback(void* buffer, size_t size, size_t nmemb, void* userdata) {
        auto* self = static_cast<StreamHandler*>(userdata);
        size_t totalSize = size * nmemb;

        const uint8_t* data = static_cast<const uint8_t*>(buffer);
        size_t handled = 0;

        while (handled < totalSize) {
            // ~~~~~~~~~~~~ AUDIO ~~~~~~~~~~~~~~~
            if (self->mState == StreamState::AUDIO) {
                size_t canRead = std::min(self->mBytesToRead, totalSize - handled);

                //AUDIO CALLBACK .....
                if ( self->OnAudioChunk ) self->OnAudioChunk(data + handled, canRead);

                self->mTotalAudioBytesSent += canRead;
                self->mBytesToRead -= canRead;
                handled += canRead;
                if (self->mBytesToRead == 0) {
                    self->mMetaString.clear();
                    self->mState = StreamState::META_LENGTH;
                }
            }
            // ~~~~~~~~~~~~ META_LENGTH ~~~~~~~~~~~~~~~
            else if (self->mState == StreamState::META_LENGTH) {
                // Log("Switching to META_LENGTH at total handled: %llu", self->mTotalBytesProcessed);

                self->mCurrentMetaSize = data[handled] * 16;
                handled++;
                if (self->mCurrentMetaSize > 0) {
                    self->mState = StreamState::META_DATA;
                    self->mBytesToRead = self->mCurrentMetaSize;
                } else {
                    self->mState = StreamState::AUDIO; // no meta data avail
                    self->mBytesToRead = self->mMetaInt;
                }
            }
            // ~~~~~~~~~~~~ META_DATA ~~~~~~~~~~~~~~~
            else if (self->mState == StreamState::META_DATA) {
                size_t canRead = std::min(self->mBytesToRead, totalSize - handled);
                self->mMetaString.append((char*)data + handled, canRead);

                self->mBytesToRead -= canRead;
                handled += canRead;
                if (self->mBytesToRead == 0) {

                    // METADATA (JSON/Text)
                    self->parseMeta();

                    self->mState = StreamState::AUDIO;
                    self->mBytesToRead = self->mMetaInt;
                }
            }
        }
        return totalSize;
    }
    //--------------------------------------------------------------------------
    int StreamHandler::ProgressCallback(void* clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow){
        std::atomic<bool>* stopRequested = static_cast<std::atomic<bool>*>(clientp);
        if (stopRequested && stopRequested->load()) {
            return 1;
        }
        return 0; // 0 ==> continue
    }
    //--------------------------------------------------------------------------
    void StreamHandler::reset(){
        mRunning.store( false );
        mConnected.store( false );
        mStreamInfo = StreamInfo();
        mFullHeader = "";

        mMetaInt    = 0;
        mMetaString = "";

        mStreamTitle = "";
        mStreamTitleAudioByteNeedle = 0;

        mState = StreamState::AUDIO;
        mBytesToRead = 0;
        mCurrentMetaSize = 0;

        mTotalAudioBytesSent = 0;
        mCurlHandle = nullptr;

    }
    //--------------------------------------------------------------------------
    void StreamHandler::Execute(std::string url) {
        if (mRunning.load()) {
            Log("Request is running - stop first ! ");
        }
        reset();

        if (mThread.joinable()) {
            mStopRequested.store(true);
            mThread.join();
        }
        if (!url.empty()) mUrl = url; //overwrite by parameter

        mStreamInfo.streamUrl = url;


        mStopRequested.store(false);
        FluxNet::initCurl();

        mThread = std::thread([this]() {
            if (OnConnecting) OnConnecting();
            mRunning.store(true);
            struct curl_slist *headers = nullptr;
            headers = curl_slist_append(headers, "Icy-MetaData: 1");

            CURLcode res;
            mCurlHandle = curl_easy_init();
            if(mCurlHandle) {

                curl_easy_setopt(mCurlHandle, CURLOPT_URL, mUrl.c_str());
                curl_easy_setopt(mCurlHandle, CURLOPT_USERAGENT, "RadioWana/2.0");
                curl_easy_setopt(mCurlHandle, CURLOPT_HTTPHEADER, headers);
                curl_easy_setopt(mCurlHandle, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);

                curl_easy_setopt(mCurlHandle, CURLOPT_WRITEFUNCTION, WriteCallback);
                curl_easy_setopt(mCurlHandle, CURLOPT_WRITEDATA, this);

                curl_easy_setopt(mCurlHandle, CURLOPT_HEADERFUNCTION, HeaderCallback);
                curl_easy_setopt(mCurlHandle, CURLOPT_HEADERDATA, this);

                curl_easy_setopt(mCurlHandle, CURLOPT_FOLLOWLOCATION, 1L);
                curl_easy_setopt(mCurlHandle, CURLOPT_NOPROGRESS, 0L);
                curl_easy_setopt(mCurlHandle, CURLOPT_XFERINFOFUNCTION, ProgressCallback);
                curl_easy_setopt(mCurlHandle, CURLOPT_XFERINFODATA, &mStopRequested);
                curl_easy_setopt(mCurlHandle, CURLOPT_NOSIGNAL, 1L);
                curl_easy_setopt(mCurlHandle, CURLOPT_CONNECTTIMEOUT, 10L);

                // Low speed limit to detect connection is lost
                curl_easy_setopt(mCurlHandle, CURLOPT_LOW_SPEED_LIMIT, 1L);
                curl_easy_setopt(mCurlHandle, CURLOPT_LOW_SPEED_TIME, (long)mLowSpeedTimeOut);
                // curl_easy_setopt(mCurlHandle, CURLOPT_LOW_SPEED_TIME, 2L);

                // SSL-Verify
                curl_easy_setopt(mCurlHandle, CURLOPT_SSL_VERIFYPEER, 1L);
                curl_easy_setopt(mCurlHandle, CURLOPT_SSL_VERIFYHOST, 2L);

                // buffering
                curl_easy_setopt(mCurlHandle, CURLOPT_BUFFERSIZE, 16384L);
                curl_easy_setopt(mCurlHandle, CURLOPT_UPLOAD_BUFFERSIZE, 16384L);

                res = curl_easy_perform(mCurlHandle);

                if(res != CURLE_OK && res != CURLE_ABORTED_BY_CALLBACK) {
                    Log("[error] HttpStream error: %s", curl_easy_strerror(res));
                    if (OnError) OnError(res, curl_easy_strerror(res));
                }
                curl_easy_cleanup(mCurlHandle);
                mCurlHandle = nullptr;
            }
            if (headers) curl_slist_free_all(headers);
            stop();
            mRunning.store(false);
            mConnected.store(false);
            if (onDisConnected) {
                onDisConnected();
            }
            reset();
        });
    }
    //--------------------------------------------------------------------------

};

