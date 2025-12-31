//-----------------------------------------------------------------------------
// Copyright (c) 2025 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#pragma once
#include <vector>
#include <memory>
#include "fluxParticleEmitter.h"

class FluxParticleManager {
public:
    // Singleton access (common for managers)
    static FluxParticleManager& get() {
        static FluxParticleManager instance;
        return instance;
    }

    // Creates and returns a pointer to an emitter managed by this class
    FluxParticleEmitter* addEmitter(const EmitterProperties& props);

    void update(F32 dt);
    void render();

    // Removes all emitters (useful for scene changes)
    void clear();

private:
    FluxParticleManager() = default;
    ~FluxParticleManager();

    // Using unique_ptr for automatic memory management
    std::vector<std::unique_ptr<FluxParticleEmitter>> mEmitters;
};

// Global helper for cleaner access
#define ParticleManager FluxParticleManager::get()
