#pragma once
#include <vector>
#include "fluxLight.h"
#include "core/fluxGlobals.h"

// MAX_LIGHTS is defined in fluxGlobals.h now

class FluxLightManager {
private:
    std::vector<FluxLight> mLights;

public:
    static FluxLightManager& getInstance() {
        static FluxLightManager instance;
        instance.mLights.reserve(MAX_LIGHTS);
        return instance;
    }

    FluxLightManager(const FluxLightManager&) = delete;
    void operator=(const FluxLightManager&) = delete;

    void addLight(const FluxLight& light)
    {
        auto it = std::find(mLights.begin(), mLights.end(), light);
        if (it == mLights.end()) mLights.push_back(light);
    }

    void removeLight(S32 id) {
        auto it = std::find_if(mLights.begin(), mLights.end(), [id](const FluxLight& light) {
            return light.id == id;
        });
        if ( it != mLights.end ()) mLights.erase(it);
    }

    void clearLights() {
        mLights.clear();
    }

    const std::vector<FluxLight>& getLights() const {
        return mLights;
    }

private:
    FluxLightManager() {}
};

#define LightManager FluxLightManager::getInstance()
