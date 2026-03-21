//-----------------------------------------------------------------------------
// Copyright (c) 2012 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// look at: https://de1.api.radio-browser.info/
// add a station: https://de1.api.radio-browser.info/#Add_radio_station
// vote: https://de1.api.radio-browser.info/#Vote_for_station
// click: https://de1.api.radio-browser.info/#Count_station_click
//-----------------------------------------------------------------------------
#pragma once

#include <curl/curl.h>
#include "utils/errorlog.h"
#include "core/fluxGlobals.h"
#include "net/CurlGlue.h"
#include "net/HttpsClient.h"

#include <functional>

#include <nlohmann/json.hpp>

namespace FluxRadio {
    using json = nlohmann::json;



    struct RadioStation {
        std::string stationuuid;
        std::string name;
        std::string url;
        std::string codec;
        int bitrate;
        std::string country;
        std::vector<std::string> tags;

        std::string homepage;
        std::string favicon;
        std::string countrycode;
        std::vector<std::string> languages; // 	string, multivalue, split by comma
        int clickcount;
        int clicktrend;

    };




    class RadioBrowser: public FluxNet::HttpsClient {
        typedef FluxNet::HttpsClient Parent;

    public:
        // ---------------------------------------------------------------------
        // ---------------------------------------------------------------------
        std::function<void(std::vector<RadioStation> stations)> OnStationResponse = nullptr;
        std::function<void()> OnStationResponseError = nullptr;
        // ---------------------------------------------------------------------
    protected:
        std::string mUserAgent = "RadioWana/2.0";
        //TODO later SRV - _api._tcp.radio-browser.info
        std::string mHostname = "de1.api.radio-browser.info";

        enum class RequestType { NONE, SEARCH, CLICK };
        RequestType mLastRequestType = RequestType::NONE;

        // ---------------------------------------------------------------------
        void processResponse(const std::string data) {
            try {
                auto j = json::parse(data);

                std::vector<RadioStation> stations;

                for (const auto& item : j) {
                    RadioStation s;
                    s.stationuuid = item.value("stationuuid", "");
                    s.name    = item.value("name", "Unknown");
                    s.url     = item.value("url_resolved", item.value("url", ""));
                    s.codec   = item.value("codec", "");
                    s.bitrate = item.value("bitrate", 0);
                    s.country = item.value("country", "");

                    std::string tagStr = item.value("tags", "");

                    for (auto sv : fluxStr::Tokenize(tagStr, ',')) {
                        s.tags.push_back(std::string(sv));
                    }

                    s.homepage      = item.value("homepage", "");
                    s.favicon       = item.value("favicon", "");
                    s.countrycode   = item.value("countrycode", "");
                    std::string strLanguages = item.value("language", "");
                    for (auto sv : fluxStr::Tokenize(strLanguages, ',')) {
                        s.languages.push_back(std::string(sv));
                    }

                    s.clickcount    = item.value("clickcount", 0);
                    s.clicktrend    = item.value("clicktrend", 0);


                    stations.push_back(s);
                }
                dLog("Found %zu stations", stations.size());

                if (OnStationResponse) OnStationResponse(std::move(stations));


            } catch (const json::parse_error& e) {
                if (OnStationResponseError) OnStationResponseError();
                Log("[error] JSON Parse Error: %s", e.what());
            }
        }


    public:
        RadioBrowser() {
            // Parent::HttpsClient();
            onDisConnected = [&]() {
                if (!mContentData.empty()) {
                    std::string cType =  getContentType();
                    dLog("[info] Content-Type %s", cType.c_str());
                    dLog("%s", mContentData.c_str());
                    if (mLastRequestType == RequestType::SEARCH &&  cType == "application/json") {
                        processResponse(mContentData);
                    }
                    if (mLastRequestType == RequestType::SEARCH &&  cType == "application/json") {
                        //FIXME click ... but what todo with it ?
                    }


                }
            };
        }
        //----------------------------------------------------------------------
        // bool Execute()
        void Execute(std::string url, std::string postFields, RequestType requestType) {
            mLastRequestType = requestType;
            Parent::Execute(url,postFields);
        }
        //----------------------------------------------------------------------
        // queries "Advanced station search" on https://de1.api.radio-browser.info/
        void searchStationsByNameAndTag(
            std::string name,
            std::string tag = "",
            uint8_t limit = 100,
            bool onlyMP3 = true
        ) {
            std::string url = "https://" + mHostname + "/json/stations/search";
            std::string postData = "name=" + name;
            if (!tag.empty() ) postData += "&tag=" + tag;
            postData += std::format ("&limit={}&hidebroken=true", limit);
            if (onlyMP3) postData += "&codec=mp3";
            Execute(url, postData, RequestType::SEARCH);
        }
        //----------------------------------------------------------------------
        void clickStation(std::string stationUuid) {
            std::string url = "https://" + mHostname + "/json/url/" + stationUuid;
            Execute(url, "", RequestType::CLICK);
        }

    };
};
