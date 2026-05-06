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
        mLights.push_back(light);

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
