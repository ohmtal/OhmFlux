//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// TODO: SRV lookup at  _api._tcp.radio-browser.info
// TODO: http is forced on ANDROID
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

#include "RadioStation.h"

#include <functional>
#include <vector>


namespace FluxRadio {

    class RadioBrowser: public FluxNet::HttpsClient {
        typedef FluxNet::HttpsClient Parent;

    public:
        // ---------------------------------------------------------------------
        // CallBacks:
        std::function<void(std::vector<RadioStation> stations)> OnStationResponse = nullptr;
        std::function<void()> OnStationResponseError = nullptr;
        // ---------------------------------------------------------------------
    protected:
        std::string mUserAgent = "FluxRadioClass/1.0";
        std::string mHostname = "de1.api.radio-browser.info";
        std::string mProto = "https://";

        enum class RequestType { NONE, SEARCH, CLICK };
        RequestType mLastRequestType = RequestType::NONE;

        void processResponse(const std::string data);


    public:
        RadioBrowser(std::string userAgent );
        void Execute(std::string url, std::string postFields, RequestType requestType);
        // queries "Advanced station search" on https://de1.api.radio-browser.info/
        void searchStationsByNameAndTag(
            std::string name,
            std::string tag = "",
            uint8_t limit = 200,
            bool onlyMP3 = true
        );
        //----------------------------------------------------------------------
        void clickStation(std::string stationUuid);
    };
};
