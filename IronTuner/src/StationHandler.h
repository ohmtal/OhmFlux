//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Station Handler
//-----------------------------------------------------------------------------
#pragma once

#include "fluxRadio/RadioStation.h"
#include <vector>
#include <string>

namespace IronTuner {

    // {"ok":true,"message":"retrieved station url","stationuuid":"960594a6-0601-11e8-ae97-52543be04c81","name":"Rock Antenne",
    // "url":"http://mp3channels.webradio.rockantenne.de/rockantenne"}
    static const std::vector<FluxRadio::RadioStation> DefaultFavo = {
        {
            .stationuuid = "960594a6-0601-11e8-ae97-52543be04c81",
            .name= "Rock Antenne",
            .url = "http://mp3channels.webradio.rockantenne.de/rockantenne",
            .countrycode = "DE",
            .favId = 1
        },
        // {"ok":true,"message":"retrieved station url","stationuuid":"92556f58-20d3-44ae-8faa-322ce5f256c0",
        // "name":"Radio BOB!","url":"http://streams.radiobob.de/bob-national/mp3-192/mediaplayer"},
        {
            .stationuuid = "92556f58-20d3-44ae-8faa-322ce5f256c0",
            .name= "BOB! - Radio Bob",
            .url = "http://streams.radiobob.de/bob-national/mp3-192/mediaplayer",
            .countrycode = "DE",
            .favId = 2
        },
        // not my music but can be used for a demo: NCS Radio (NoCopyrightSounds)
        // {
        //     .stationuuid = "92556f58-20d3-44ae-8faa-322ce5f256c0",
        //     .name= "NCS Radio (NoCopyrightSounds)",
        //     .url = "https://stream.zeno.fm/ez4m4918n98uv",
        //     .favId = 3
        // }
    };
    //--------------------------------------------------------------------------
    class StationHandler {
    private:
        // Radio browser
        std::vector<FluxRadio::RadioStation> mQueryStationData;
        std::string mQueryString = "";
        std::string mSelectedStationUuid = "";
        uint32_t mSelectedFavId = 0;
        int mSelectedFavIndex = -1; //not the id the index in the list


        std::vector<FluxRadio::RadioStation> mFavoStationData;
        std::vector<FluxRadio::RadioStation> mStationCache;
    public:
        //Query
        std::string getQueryString() const { return mQueryString; }
        std::string& getQueryStringMutable() { return mQueryString; }
        std::vector<FluxRadio::RadioStation> getQueryStationData() const { return  mQueryStationData; };
        std::vector<FluxRadio::RadioStation>& getQueryStationDataMutable() { return  mQueryStationData; };


        //UUID
        std::string getUuid() const { return mSelectedStationUuid; }
        void setUuid(std::string value)  { mSelectedStationUuid = value; }

        // favo
        std::vector<FluxRadio::RadioStation> getFavoStationData() const { return  mFavoStationData; };

        bool isFavoStation(const FluxRadio::RadioStation* station);
        FluxRadio::RadioStation* getFavoStation(const FluxRadio::RadioStation* station);
        bool AddFavo(const FluxRadio::RadioStation* station);
        bool RmvFavoByFavId(const FluxRadio::RadioStation* station);
        bool RmvFavoByUUID(const FluxRadio::RadioStation* station);
        void setFavId ( const int id) { mSelectedFavId = id; }
        uint32_t getFavId () const { return mSelectedFavId; }

        FluxRadio::RadioStation* getSelectedFavStation() {
            return getStationByFavId(mSelectedFavId);
        }

        FluxRadio::RadioStation* getStationByFavId(uint32_t id) {
            return FluxRadio::getStationByFavId(&mFavoStationData, id);
        }
        int updateFavIds() {
            return FluxRadio::updateFavIds(&mFavoStationData);
        }

        // cache
        int getCacheSize() const { return (int)mStationCache.size();}
        void DumpStationCache();
        bool isCacheStation(const FluxRadio::RadioStation* station);
        void addCache(const FluxRadio::RadioStation station ) {
            if (!isCacheStation(&station)) {
                mStationCache.push_back(station);
            }
        }
        std::vector<FluxRadio::RadioStation> getStationCache() const { return  mStationCache; };


        bool cachedStationBySelectedIndex(FluxRadio::RadioStation& station) {
            if (mSelectedFavIndex >= 0 && mStationCache.size() > mSelectedFavIndex) {
                station = mStationCache[mSelectedFavIndex];
                return true;
            }
            return false;
        }

        FluxRadio::RadioStation* getCachedStation(size_t id)  {
            if ( id >= mStationCache.size() ) return nullptr;
            return &mStationCache[id];
        }

        // FavIndex
        int getFavIndex() const { return mSelectedFavIndex; }
        void setFavIndex(int value) { mSelectedFavIndex = value; }
        int& getFavIndexMutable()  { return mSelectedFavIndex; }
        void setSelectedFavIndex();


        // -----
        void Load();
        void Save();
        void update();




    };
}
