#pragma once
#include <vector>
#include "fluxLight.h"
#include "core/fluxGlobals.h"

// MAX_LIGHTS is defined in fluxGlobals.h now

class FluxLightManager {
private:
   std::vector<FluxLight*> mLights;

public:
    static FluxLightManager& getInstance() {
        static FluxLightManager instance;
        instance.mLights.reserve(MAX_LIGHTS);
        return instance;
    }

    FluxLightManager(const FluxLightManager&) = delete;
    void operator=(const FluxLightManager&) = delete;

    void addLight(FluxLight* light, bool autoDelete = true)
    {
        auto it = std::find(mLights.begin(), mLights.end(), light);
        if (it == mLights.end()) {
            light->mAutoDelete = autoDelete;
            mLights.push_back(light);
        }
    }

    void removeLight(FluxLight* light) {
        auto it = std::find(mLights.begin(), mLights.end(), light);
        if ( it != mLights.end ()) mLights.erase(it);
    }

    void clearLights() {
        for (auto* light : mLights) {
            if (light->mAutoDelete) SAFE_DELETE(light);
        }
        mLights.clear();
    }

    const std::vector<FluxLight*> getLights()  {
        return mLights;
    }

    // void setAmbientColor(Color4F color) {
    //      Render2D.setAmbientColor(color);
    // }


private:
    FluxLightManager() {}
};

#define LightManager FluxLightManager::getInstance()
