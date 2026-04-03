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
#include <vector>

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

        // internal fav id
        uint32_t    favId = 0;




        std::vector<std::string> dump(bool useLog = true) {
            std::vector<std::string> result ;

            auto output = [&](const char* fmt, ...) {
                char buf[512];
                va_list args;
                va_start(args, fmt);
                vsnprintf(buf, sizeof(buf), fmt, args);
                va_end(args);

                if (useLog) {
                    Log("%s", buf);
                } /*else {
                    ImGui::Text("%s", buf);
                }*/
                result.push_back(buf);
            };

            if (useLog) output("------------- STATION --------------");
            output("UUID     :%s", stationuuid.c_str());
            output("Name     :%s", name.c_str());
            output("URL      :%s", url.c_str());
            output("CODEC    :%s", codec.c_str());
            output("BITRATE  :%d", bitrate);
            output("COUNTRY  :%s / %s", country.c_str(), countrycode.c_str());
            output("HOMEPAGE :%s", homepage.c_str());
            output("FAVICON  :%s", favicon.c_str());
            output("CLICKS   :%d / %d", clickcount, clicktrend);
            //FIXME TAGS
            //FIXME languages
            if (useLog) output("------------------------------------");
            if (useLog) output("FAVID    :%d", favId);
            if (useLog) output("------------------------------------");
            return result;
        }
    };

    inline void to_json(nlohmann::json& j, const RadioStation& s) {
        j = nlohmann::json{
            {"stationuuid", s.stationuuid},
            {"name",        s.name},
            {"url",         s.url},
            {"codec",       s.codec},
            {"bitrate",     s.bitrate},
            {"country",     s.country},
            {"tags",        s.tags},
            {"homepage",    s.homepage},
            {"favicon",     s.favicon},
            {"countrycode", s.countrycode},
            {"languages",   s.languages},
            {"clickcount",  s.clickcount},
            {"clicktrend",  s.clicktrend},
            {"favId",       s.favId}
        };
    }

    inline void from_json(const nlohmann::json& j, RadioStation& s) {
        // .value() nimmt den Wert aus dem JSON, oder den Default (2. Parameter), falls der Key fehlt
        s.stationuuid = j.value("stationuuid", "");
        s.name        = j.value("name", "");
        s.url         = j.value("url", "");
        s.codec       = j.value("codec", "");
        s.bitrate     = j.value("bitrate", 0);
        s.country     = j.value("country", "");
        s.tags        = j.value("tags", std::vector<std::string>{});
        s.homepage    = j.value("homepage", "");
        s.favicon     = j.value("favicon", "");
        s.countrycode = j.value("countrycode", "");
        s.languages   = j.value("languages", std::vector<std::string>{});
        s.clickcount  = j.value("clickcount", 0);
        s.clicktrend  = j.value("clicktrend", 0);
        s.favId       = j.value("favId", (uint32_t)0);
    }



    // -----------------------------------------------------------------------------
    inline RadioStation* getStationByFavId(std::vector<FluxRadio::RadioStation>* mFavList, uint32_t id){
        for (auto& fav : *mFavList) {
            if (fav.favId == id) {
                return &fav;
            }
        }
        return nullptr;
    }


    inline int updateFavIds(std::vector<FluxRadio::RadioStation>* mFavList) {
        if (!mFavList) return -1;

        int maxId = 0;
        for (const auto& fav : *mFavList) {
            if (fav.favId > maxId) maxId = fav.favId;
        }

        for (auto& fav : *mFavList) {
            if (fav.favId == 0) {
                fav.favId = ++maxId;
            }
        }
        return maxId;
    }



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
        std::string mProto = "https://";

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
            uint8_t limit = 200,
            bool onlyMP3 = true
        );
        //----------------------------------------------------------------------
        void clickStation(std::string stationUuid);

    };
};
