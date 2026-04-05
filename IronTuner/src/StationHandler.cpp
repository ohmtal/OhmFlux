#include "StationHandler.h"
#include "appMain.h"
#include "utils/fluxSettingsManager.h"

namespace IronTuner {
    void StationHandler::DumpStationCache(){
        for (auto& s: mStationCache ) {
            Log("%03d %s (%s)", s.favId,  s.name.c_str(), s.stationuuid.c_str());
        }
    }


    bool StationHandler::isFavoStation(const FluxRadio::RadioStation* station){
        if ( !station ) return false;
        auto it = std::find_if(mFavoStationData.begin(), mFavoStationData.end(),
                               [&station](const FluxRadio::RadioStation& s) {
                                   return (
                                       (s.stationuuid != "" && s.stationuuid == station->stationuuid)
                                       || (s.url != "" &&  s.url == station->url)
                                   );
                               });
        return it != mFavoStationData.end();
    }

    FluxRadio::RadioStation* StationHandler::getFavoStation(const FluxRadio::RadioStation* station) {
        if ( !station ) return nullptr;
        auto it = std::find_if(mFavoStationData.begin(), mFavoStationData.end(),
                               [&station](const FluxRadio::RadioStation& s) {
                                   return (
                                       (s.stationuuid != "" && s.stationuuid == station->stationuuid)
                                       || (s.url != "" &&  s.url == station->url)
                                   );
                               });

        if (it != mFavoStationData.end()) {
            return &(*it);
        }
        return nullptr;

    }

    bool StationHandler::isCacheStation(const FluxRadio::RadioStation* station){
        if ( !station ) return false;
        auto it = std::find_if(mStationCache.begin(), mStationCache.end(),
                               [&station](const FluxRadio::RadioStation& s) {
                                   return (
                                       (s.favId != 0 && s.favId == station->favId)
                                       || (s.stationuuid != "" && s.stationuuid == station->stationuuid)
                                       || (s.url != "" &&  s.url == station->url)
                                   );
                               });
        return it != mStationCache.end();
    }

    bool StationHandler::AddFavo(const FluxRadio::RadioStation* station) {
        if ( !station ) return false;
        if ( !isFavoStation(station) ) {
            mFavoStationData.push_back(*station);
        }
        FluxRadio::updateFavIds(&mFavoStationData);

        FluxRadio::RadioStation* favStation =  getFavoStation(station);
        if (favStation) {
            if ( !isCacheStation(favStation) ) {
                mStationCache.push_back(*favStation);
            }
            // update current station
            if (
                (favStation->url  != "" && getMain()->getAppSettings().CurrentStation.url == favStation->url )
                || (favStation->stationuuid  != "" && getMain()->getAppSettings().CurrentStation.stationuuid == favStation->stationuuid )
            ) {
                getMain()->getAppSettings().CurrentStation.favId = favStation->favId;
            }
        }


        return true;
    }

    bool StationHandler::RmvFavoByFavId(const FluxRadio::RadioStation* station) {
        if (!station || station->favId < 1 ) return false;
        bool result = std::erase_if(mFavoStationData, [&](const FluxRadio::RadioStation& s) {
            return s.favId == station->favId;
        });

        if (result) {
            std::erase_if(mStationCache, [&](const FluxRadio::RadioStation& s) {
                return s.stationuuid == station->stationuuid;
            });

            // update current station
            if ( station->favId  ==  getMain()->getAppSettings().CurrentStation.favId) {
                getMain()->getAppSettings().CurrentStation.favId = 0;
            }

        }
        return result;
    }

    bool StationHandler::RmvFavoByUUID(const FluxRadio::RadioStation* station) {
        if (!station || station->stationuuid == "" ) return false;
        bool result = std::erase_if(mFavoStationData, [&](const FluxRadio::RadioStation& s) {
            return s.stationuuid == station->stationuuid;
        });

        if (result) {
            std::erase_if(mStationCache, [&](const FluxRadio::RadioStation& s) {
                return s.stationuuid == station->stationuuid;
            });
            // update current station
            if ( getMain()->getAppSettings().CurrentStation.stationuuid == station->stationuuid ) {
                getMain()->getAppSettings().CurrentStation.favId = 0;
            }
        }
        return result;
    }

    void StationHandler::setSelectedFavIndex() {
        // mCurStationIsFavo =
        for (int i =0 ; i < (int)mFavoStationData.size(); i++) {
            if ( mFavoStationData[i].favId == getMain()->getAppSettings().CurrentStation.favId ) {
                mSelectedFavIndex = i;
                return;
            }
        }
        mSelectedFavIndex = -1; //nothing found!
    }

    void StationHandler::update() {
        // set current favIndex ...
        if (mSelectedFavIndex < 0) {
            setSelectedFavIndex();

            mStationCache.clear();
            for ( auto& station : mFavoStationData ) {
                mStationCache.push_back(station);
            }

            if ( getMain()->getAppSettings().CurrentStation.favId < 1) {
                mStationCache.push_back( getMain()->getAppSettings().CurrentStation );
                mSelectedFavIndex = (int)mStationCache.size() - 1;
            }
        }
    }

    void StationHandler::Load(){
        mFavoStationData = SettingsManager().get("Radio::Favo", DefaultFavo);
        FluxRadio::updateFavIds(&mFavoStationData);
    }
    void StationHandler::Save(){
        SettingsManager().set("Radio::Favo", mFavoStationData);
    }


};
