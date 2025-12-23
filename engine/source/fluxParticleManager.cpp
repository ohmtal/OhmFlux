//-----------------------------------------------------------------------------
// Copyright (c) 2025 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#include "fluxParticleManager.h"

FluxParticleEmitter* FluxParticleManager::addEmitter(const EmitterProperties& props) {
    auto emitter = std::make_unique<FluxParticleEmitter>(props);
    FluxParticleEmitter* ptr = emitter.get();
    mEmitters.push_back(std::move(emitter));
    return ptr;
}

void FluxParticleManager::update(F32 dt) {
    // Update all emitters
    for (auto it = mEmitters.begin(); it != mEmitters.end(); ) {
        (*it)->update(dt);

        // Optional: If you add an 'isDead' flag to emitters for one-shots
        // if ((*it)->isDead()) { it = mEmitters.erase(it); }
        // else { ++it; }
        ++it;
    }
}

void FluxParticleManager::render() {
    for (const auto& emitter : mEmitters) {
        emitter->render();
    }
}

void FluxParticleManager::clear() {
    mEmitters.clear();
}

FluxParticleManager::~FluxParticleManager() {
    clear();
}
