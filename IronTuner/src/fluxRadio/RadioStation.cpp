//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// RadioStation stuct
//-----------------------------------------------------------------------------
#include "RadioStation.h"

namespace FluxRadio {
    //--------------------------------------------------------------------------
    std::vector< std::string > RadioStation::dump(bool useLog) {
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
    //--------------------------------------------------------------------------
    void to_json(nlohmann::json_abi_v3_12_0::json& j, const RadioStation& s){
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
    //--------------------------------------------------------------------------
    void from_json(const nlohmann::json_abi_v3_12_0::json& j, RadioStation& s){
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
    //--------------------------------------------------------------------------
    RadioStation* getStationByFavId(std::vector< FluxRadio::RadioStation >* mFavList, uint32_t id) {
        for (auto& fav : *mFavList) {
            if (fav.favId == id) {
                return &fav;
            }
        }
        return nullptr;
    }
    //--------------------------------------------------------------------------
    int updateFavIds(std::vector< FluxRadio::RadioStation >* mFavList) {
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
};
