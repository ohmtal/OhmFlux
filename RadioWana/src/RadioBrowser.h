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
        std::string stationuuid = "";
        std::string name = "";
        std::string url = "";
        std::string codec = "";
        int bitrate = 0;
        std::string country = "";
        std::vector<std::string> tags = {};

        std::string homepage = "";
        std::string favicon = "";
        std::string countrycode = "";
        std::vector<std::string> languages = {}; // 	string, multivalue, split by comma
        int clickcount = 0;
        int clicktrend = 0;

        // maybe longer compiletime because of header but i ve a fast machine ;)
        NLOHMANN_DEFINE_TYPE_INTRUSIVE(RadioStation,
                                       stationuuid, name, url, codec, bitrate, country, tags,
                                       homepage, favicon, countrycode, languages, clickcount, clicktrend)


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
        void processResponse(const std::string data);


    public:
        RadioBrowser();
        //----------------------------------------------------------------------
        // bool Execute()
        void Execute(std::string url, std::string postFields, RequestType requestType);
        //----------------------------------------------------------------------
        // queries "Advanced station search" on https://de1.api.radio-browser.info/
        void searchStationsByNameAndTag(
            std::string name,
            std::string tag = "",
            uint8_t limit = 100,
            bool onlyMP3 = true
        );
        //----------------------------------------------------------------------
        void clickStation(std::string stationUuid);

    };
};
