//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// RadioStation stuct
//-----------------------------------------------------------------------------
#pragma once

#include "utils/errorlog.h"
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

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

        std::vector<std::string> dump(bool useLog = true) const ;
    };
    // -----------------------------------------------------------------------------
    void to_json(nlohmann::json& j, const RadioStation& s);
    void from_json(const nlohmann::json& j, RadioStation& s);
    // -----------------------------------------------------------------------------
    RadioStation* getStationByFavId(std::vector<FluxRadio::RadioStation>* mFavList, uint32_t id);
    int updateFavIds(std::vector<FluxRadio::RadioStation>* mFavList);



}; //namespace
