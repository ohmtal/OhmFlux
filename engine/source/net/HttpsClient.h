//-----------------------------------------------------------------------------
// Copyright (c) 2012 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#pragma once

#include <curl/curl.h>
#include "utils/errorlog.h"
#include "CurlGlue.h"

#include <functional>
#include <thread>
#include <atomic>
#include <mutex>
#include <string>
#include <algorithm>
#include "NetTools.h"

namespace FluxNet {
    class HttpsClient {
    protected:
        std::atomic<bool> mStopRequested{false}; // remember : .store .load  :P
        std::thread mThread;
        std::atomic<bool> mRunning = false;

        std::mutex mHeaderMutex;
        std::mutex mContentMutex;

        CURL* mCurlHandle = nullptr;
        std::string mUrl = "";
        std::string mPostData = "";
        std::string mUserAgent = "Mozilla/5.0";
        std::vector<std::string> mHttpHeaders;

        std::string mHeaderData;
        std::string mContentData;

    public:
        //--------------------------------------------------------------------------
        std::function<void()> OnConnecting = nullptr;
        std::function<void()> OnConnected = nullptr;
        std::function<void()> onDisConnected = nullptr;
        std::function<void(const std::string)> OnError = nullptr;

        long mConnectTimeOut = 10;
        long mBufferSize = 16384;
        //--------------------------------------------------------------------------
        std::string getHeaderData() const { return mHeaderData;}
        std::string getContentData() const { return mContentData;}
        //--------------------------------------------------------------------------
        std::string getHeaderValue(std::string key) const {

            return NetTools::getHeaderValue(mHeaderData, key);

            // if (!key.empty() && key.back() == ':') key.pop_back();
            // std::string lowerKey = key;
            // std::transform(lowerKey.begin(), lowerKey.end(), lowerKey.begin(), ::tolower);
            //
            // std::string headerBlock = mHeaderData;
            // std::transform(headerBlock.begin(), headerBlock.end(), headerBlock.begin(), ::tolower);
            //
            // size_t keyPos = headerBlock.find(lowerKey + ":");
            // if (keyPos != std::string::npos) {
            //     size_t valueStart = keyPos + lowerKey.length() + 1;
            //     while (valueStart < mHeaderData.length() && std::isspace(mHeaderData[valueStart])) {
            //         valueStart++;
            //     }
            //     size_t valueEnd = mHeaderData.find("\r\n", valueStart);
            //     if (valueEnd == std::string::npos) valueEnd = mHeaderData.length();
            //     return mHeaderData.substr(valueStart, valueEnd - valueStart);
            // }
            // return "";
        }
        //--------------------------------------------------------------------------
        std::string getContentType() const {
            return getHeaderValue("Content-Type");
        }
        //--------------------------------------------------------------------------
        HttpsClient() = default;
        ~HttpsClient() {
            stop();
            if (mThread.joinable()) mThread.join();
        }
        //--------------------------------------------------------------------------
        void reset() {
            std::scoped_lock lock(mHeaderMutex, mContentMutex);
            mHeaderData = "";
            mContentData = "";
        }
        //--------------------------------------------------------------------------
        void stop() {
            mStopRequested.store( true ) ;
        };
        //--------------------------------------------------------------------------
        void Execute(std::string url, std::string postFields = "") {
            if (mRunning.load()) {
                dLog("[warn] Request is running - you should stop first !");
            }
            reset();

            if (mThread.joinable()) {
                mStopRequested.store(true);
                mThread.join();
            }
            if (!url.empty()) mUrl = url; //overwrite by parameter
            mPostData = postFields;


            mStopRequested.store(false);
            FluxNet::initCurl();

            mThread = std::thread([this]() {
                if (OnConnecting) OnConnecting();
                mRunning.store(true);

                struct curl_slist *headers = nullptr;
                for (const auto& entry: mHttpHeaders) {
                    headers = curl_slist_append(headers, entry.c_str());
                    if (!headers) break;
                }


                CURLcode res;
                mCurlHandle = curl_easy_init();
                if(mCurlHandle) {

                    curl_easy_setopt(mCurlHandle, CURLOPT_URL, mUrl.c_str());
                    curl_easy_setopt(mCurlHandle, CURLOPT_USERAGENT, mUserAgent.c_str());
                    if (headers) curl_easy_setopt(mCurlHandle, CURLOPT_HTTPHEADER, headers);

                    if (!mPostData.empty()) {
                        curl_easy_setopt(mCurlHandle, CURLOPT_POSTFIELDS, mPostData.c_str());
                        curl_easy_setopt(mCurlHandle, CURLOPT_POSTFIELDSIZE, (long)mPostData.length());
                    }

                    curl_easy_setopt(mCurlHandle, CURLOPT_WRITEFUNCTION, WriteCallback);
                    curl_easy_setopt(mCurlHandle, CURLOPT_WRITEDATA, this);

                    curl_easy_setopt(mCurlHandle, CURLOPT_HEADERFUNCTION, HeaderCallback);
                    curl_easy_setopt(mCurlHandle, CURLOPT_HEADERDATA, this);

                    curl_easy_setopt(mCurlHandle, CURLOPT_FOLLOWLOCATION, 1L);
                    curl_easy_setopt(mCurlHandle, CURLOPT_NOPROGRESS, 0L);
                    curl_easy_setopt(mCurlHandle, CURLOPT_XFERINFOFUNCTION, ProgressCallback);
                    curl_easy_setopt(mCurlHandle, CURLOPT_XFERINFODATA, &mStopRequested);
                    curl_easy_setopt(mCurlHandle, CURLOPT_NOSIGNAL, 1L);
                    curl_easy_setopt(mCurlHandle, CURLOPT_CONNECTTIMEOUT, mConnectTimeOut);

                    // SSL-Verify
                    curl_easy_setopt(mCurlHandle, CURLOPT_SSL_VERIFYPEER, 1L);
                    curl_easy_setopt(mCurlHandle, CURLOPT_SSL_VERIFYHOST, 2L);

                    // buffering
                    curl_easy_setopt(mCurlHandle, CURLOPT_BUFFERSIZE, mBufferSize);
                    curl_easy_setopt(mCurlHandle, CURLOPT_UPLOAD_BUFFERSIZE, mBufferSize);


                    res = curl_easy_perform(mCurlHandle);

                    if(res != CURLE_OK && res != CURLE_ABORTED_BY_CALLBACK) {
                        Log("[error] HttpStream error: %s", curl_easy_strerror(res));
                        if (OnError) OnError(curl_easy_strerror(res));
                    }
                    curl_easy_cleanup(mCurlHandle);
                     mCurlHandle = nullptr;
                }
                if (headers) curl_slist_free_all(headers);
                stop();
                mRunning.store(false);
                if (onDisConnected) onDisConnected();
            });
        }

    protected:
        //--------------------------------------------------------------------------
        static size_t WriteCallback(void* buffer, size_t size, size_t nmemb, void* userdata) {
            auto* self = static_cast<HttpsClient*>(userdata);
            size_t totalSize = size * nmemb;

            const char* data = static_cast<const char*>(buffer);
            std::string content(data, totalSize);
            {
                std::lock_guard<std::mutex> lock(self->mContentMutex);
                self->mContentData += content;
            }

            return totalSize;
        }
        //--------------------------------------------------------------------------
        static size_t HeaderCallback(char* buffer, size_t size, size_t nitems, void* userdata) {
            size_t totalSize = size * nitems;
            auto* self = static_cast<HttpsClient*>(userdata);
            if (!self) {
                Log("[error]HeaderCallback self in nil!!! ");
                return totalSize;
            }

            std::string header(buffer, totalSize);
            {
                std::lock_guard<std::mutex> lock(self->mHeaderMutex);
                self->mHeaderData += header;
            }
            if (header == "\r\n" || header == "\n") {

                long http_code = 0;
                if (self->mCurlHandle) {
                    curl_easy_getinfo(self->mCurlHandle, CURLINFO_RESPONSE_CODE, &http_code);
                    if (http_code == 200) {
                        if (self->OnConnected) self->OnConnected();
                    }
                }
                dLog("[info] https got header: %s HTTP CODE: %d", header.c_str(), (int)http_code);

            } else {
                dLog("[info] https got header: %s", header.c_str());
            }
            return totalSize;
        }

        //--------------------------------------------------------------------------
        static int ProgressCallback(void* clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow){
            std::atomic<bool>* stopRequested = static_cast<std::atomic<bool>*>(clientp);
            if (stopRequested && stopRequested->load()) {
                return 1;
            }
            return 0; // 0 ==> continue
        }
    };
};
