//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Station Handler - NOT Thread Save!
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
            .clickcount  = 0,
            .isLocalFavo = true
        },
        // {"ok":true,"message":"retrieved station url","stationuuid":"92556f58-20d3-44ae-8faa-322ce5f256c0",
        // "name":"Radio BOB!","url":"http://streams.radiobob.de/bob-national/mp3-192/mediaplayer"},
        {
            .stationuuid = "92556f58-20d3-44ae-8faa-322ce5f256c0",
            .name= "BOB! - Radio Bob",
            .url = "http://streams.radiobob.de/bob-national/mp3-192/mediaplayer",
            .countrycode = "DE",
            .clickcount  = 0,
            .isLocalFavo = true
        },
        // not my music but can be used for a demo: NCS Radio (NoCopyrightSounds)
        // {
        //     .stationuuid = "",
        //     .name= "NCS Radio (NoCopyrightSounds)",
        //     .url = "https://stream.zeno.fm/ez4m4918n98uv",
        //     .clickcount  = 0,
        //     .isLocalFavo = true
        // }
    };
    //--------------------------------------------------------------------------
    class StationHandler {
    private:
        // Radio browser
        std::vector<FluxRadio::RadioStation> mQueryStationData;
        std::string mQueryString = "";
        // std::string mSelectedStationUuid = "";

        int mSelectedIndex = -1; //not the id the index in the list

        std::vector<FluxRadio::RadioStation> mLocalStations;
        std::vector<const FluxRadio::RadioStation*> mSortedStations; //cache sorted by click

    public:
        //Query
        const std::string& getQueryString() const;
        std::string& getQueryStringMutable();
        const std::vector<FluxRadio::RadioStation>& getQueryStationData() const { return  mQueryStationData; };
        std::vector<FluxRadio::RadioStation>& getQueryStationDataMutable() { return  mQueryStationData; };
        void DumpQueryStations();


        // local Stations list
        const std::vector<FluxRadio::RadioStation>& getStations() const;

        // get a local station pointer by station data
        FluxRadio::RadioStation* getStation(const FluxRadio::RadioStation* station);
        bool getSelectedStation(FluxRadio::RadioStation& station);
        FluxRadio::RadioStation* getStation(size_t index);

        bool isLocalStation(const FluxRadio::RadioStation* station);
        bool isFavoStation(const FluxRadio::RadioStation* station);

        // add a station if not in list ...update favo if setFavo
        bool addStation(const FluxRadio::RadioStation* station, bool setFavo = false);
        bool setFavo(const FluxRadio::RadioStation* station, bool value);
        bool incClick(const FluxRadio::RadioStation* station);

        // remove non favo stations
        bool cleanup();

        int  getSize() const;
        void dumpStations();


        // FavIndex
        int getIndex() const { return mSelectedIndex; }
        void setIndex(int value) { mSelectedIndex = value; }
        int& getIndexMutable()  { return mSelectedIndex; }
        void setSelectedIndex();

        // sorted cache
        const std::vector<const FluxRadio::RadioStation*>& getSortedStations() const { return mSortedStations;}
        void updateSortedCache();


        // -----
        void Load();
        void Save();
        void update();




    };
}
