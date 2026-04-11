//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Station Handler - NOT Thread Save!
//-----------------------------------------------------------------------------
#include "StationHandler.h"
#include "appMain.h"
#include "utils/fluxSettingsManager.h"

namespace IronTuner {


    //--------------------------------------------------------------------------
    const std::string& StationHandler::getQueryString() const{
        return mQueryString;
    }

    std::string& StationHandler::getQueryStringMutable(){
        return mQueryString;
    }

    void StationHandler::DumpQueryStations(){
        Log("QUERY:%s", mQueryString.c_str());
        for (auto& s: mQueryStationData ) {
            Log("%s %s (%s)", s.stationuuid.c_str(),  s.name.c_str(), s.url.c_str());
        }
    }
    //--------------------------------------------------------------------------
    const std::vector< FluxRadio::RadioStation >& StationHandler::getStations() const{
        return  mLocalStations;
    }
    //--------------------------------------------------------------------------
    FluxRadio::RadioStation* StationHandler::getStation(const FluxRadio::RadioStation* station) {
        if ( !station ) return nullptr;
        auto it = std::find_if(mLocalStations.begin(), mLocalStations.end(),
                               [&station](const FluxRadio::RadioStation& s) {
                                   return (
                                       (s.stationuuid != "" && s.stationuuid == station->stationuuid)
                                       || (s.url != "" &&  s.url == station->url)
                                   );
                               });
        if (it == mLocalStations.end()) {
            return nullptr;
        }
        return &(*it);
    }
    //--------------------------------------------------------------------------
    bool StationHandler::getSelectedStation(FluxRadio::RadioStation& station){
        if (mSelectedIndex >= 0 && mLocalStations.size() > mSelectedIndex) {
            station = mLocalStations[mSelectedIndex];
            return true;
        }
        return false;
    }
    //--------------------------------------------------------------------------
    FluxRadio::RadioStation* StationHandler::getStation(size_t index){
        if ( index >= mLocalStations.size() ) return nullptr;
        return &mLocalStations[index];
    }
    //--------------------------------------------------------------------------
    bool StationHandler::isLocalStation(const FluxRadio::RadioStation* station){
        if ( !station ) return false;
        return getStation(station) != nullptr;
    }
    //--------------------------------------------------------------------------
    bool StationHandler::isFavoStation(const FluxRadio::RadioStation* station){
        if ( !station ) return false;
        FluxRadio::RadioStation* s = getStation(station);
        return (s != nullptr && s->isLocalFavo  );
    }
    //--------------------------------------------------------------------------
    bool StationHandler::addStation(const FluxRadio::RadioStation* station, bool setFavo ) {
        if ( !station ) return false;
        FluxRadio::RadioStation* s = getStation(station);
        if ( s == nullptr ) {
            dLog("[info] Add new station to list:%s", station->name.c_str());
            mLocalStations.push_back(*station);
            s = &mLocalStations.back();
            s->clickcount = 0;
        }
        if ( s == nullptr ) return false;
        if (setFavo) {
            s->isLocalFavo = true;
        }


        return true;
    }
    //--------------------------------------------------------------------------
    bool StationHandler::setFavo(const FluxRadio::RadioStation* station, bool value) {
        if ( !station ) return false;
        FluxRadio::RadioStation* s = getStation(station);
        if ( s == nullptr ) return false;
        s->isLocalFavo = value;
        return true;
    }
    //--------------------------------------------------------------------------
    bool StationHandler::incClick(const FluxRadio::RadioStation* station)   {
        if ( !station ) return false;
        FluxRadio::RadioStation* s = getStation(station);
        if ( s == nullptr ) return false;
        s->clickcount++;
        return true;
    }
    //--------------------------------------------------------------------------
    int StationHandler::getSize() const{
        return (int)mLocalStations.size();
    }
    //--------------------------------------------------------------------------
    void StationHandler::dumpStations(){
        for (auto& s: mLocalStations ) {
            Log("%s(%d), %s, %s", s.name.c_str(), s.isLocalFavo , s.url.c_str(), s.stationuuid.c_str());
        }
    }
    //--------------------------------------------------------------------------
    void StationHandler::setSelectedIndex() {
        // we check against URL OR UUID!
        const auto& station = getMain()->getAppSettings().CurrentStation;
        for (size_t i =0 ; i < mLocalStations.size(); i++) {
            const auto& s = mLocalStations[i];

            if (
                (s.stationuuid != "" && s.stationuuid == station.stationuuid)
                || (s.url != "" &&  s.url == station.url)
            ) {
                mSelectedIndex = i;
                return;
            }
        }
        mSelectedIndex = -1; //nothing found!
    }
    //--------------------------------------------------------------------------
    bool StationHandler::cleanup()  {
        std::vector<FluxRadio::RadioStation> stations;
        stations = mLocalStations;
        mLocalStations.clear();
        for (auto s : stations) {
            if ( s.isLocalFavo || s.url == getMain()->getAppSettings().CurrentStation.url ) {
                 mLocalStations.push_back(s);
            }
        }
        mSelectedIndex = -1; //setup new index
        return true;
    }
    //--------------------------------------------------------------------------
    void StationHandler::update() {
        if (mSelectedIndex < 0) {
            if (!isLocalStation(&getMain()->getAppSettings().CurrentStation)) {
                addStation(&getMain()->getAppSettings().CurrentStation);
            }
            setSelectedIndex();
            updateSortedCache();
        }
    }
    //--------------------------------------------------------------------------
    void StationHandler::updateSortedCache(){
        mSortedStations.clear();
        for (const auto& s : mLocalStations) {
            mSortedStations.push_back(&s);
        }
        std::sort(mSortedStations.begin(), mSortedStations.end(), [](auto* a, auto* b) {
            return a->clickcount > b->clickcount;
        });
    }
    //--------------------------------------------------------------------------
    void StationHandler::Load(){
        mLocalStations = SettingsManager().get("Radio::Stations", DefaultFavo);
        updateSortedCache();
    }
    //--------------------------------------------------------------------------
    void StationHandler::Save(){
        SettingsManager().set("Radio::Stations", mLocalStations);
    }
    //--------------------------------------------------------------------------


}; //namespace
