//-----------------------------------------------------------------------------
// Copyright (c) 2012 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#include "RadioBrowser.h"


namespace FluxRadio {

    // -----------------------------------------------------------------------------

    void RadioBrowser::clickStation(std::string stationUuid){
        if (stationUuid.empty()) return;
        std::string url = "https://" + mHostname + "/json/url/" + stationUuid;
        Execute(url, "", RequestType::CLICK);
    }
    // -----------------------------------------------------------------------------

    void RadioBrowser::processResponse(const std::string data) {
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
    // -----------------------------------------------------------------------------
    RadioBrowser::RadioBrowser() {
        // Parent::HttpsClient();
        onDisConnected = [&]() {
            if (!mContentData.empty()) {
                std::string cType =  getContentType();
                dLog("[info] Content-Type %s", cType.c_str());
                dLog("%s", mContentData.c_str());
                if (mLastRequestType == RequestType::SEARCH &&  cType == "application/json") {
                    processResponse(mContentData);
                }
                if (mLastRequestType == RequestType::CLICK &&  cType == "application/json") {
                    //FIXME click ... but what todo with it ?
                }


            }
        };
    }
    // -----------------------------------------------------------------------------
    void RadioBrowser::Execute(std::string url, std::string postFields, RequestType requestType){
        mLastRequestType = requestType;
        Parent::Execute(url,postFields);
    }
    // -----------------------------------------------------------------------------
    void RadioBrowser::searchStationsByNameAndTag(std::string name, std::string tag, uint8_t limit, bool onlyMP3){
        std::string url = "https://" + mHostname + "/json/stations/search";
        std::string postData = "name=" + name;
        if (!tag.empty() ) postData += "&tag=" + tag;
        postData += std::format ("&limit={}&hidebroken=true&order=clickcount", limit);
        if (onlyMP3) postData += "&codec=mp3";
        Execute(url, postData, RequestType::SEARCH);
    }
    // -----------------------------------------------------------------------------
}// namespace
