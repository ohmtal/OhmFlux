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
        return instance;
    }

    FluxLightManager(const FluxLightManager&) = delete;
    void operator=(const FluxLightManager&) = delete;

    void addLight(const FluxLight& light)
    {

        // Ignore MAX_LIGHTS handle this later
        mLights.push_back(light);
        // if (mLights.size() < MAX_LIGHTS)
        // {
        //     mLights.push_back(light);
        // } else {
        //     // Log or handle error: too many lights
        //     // For now, silently ignore or replace oldest light
        // }
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
