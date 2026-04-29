//-----------------------------------------------------------------------------
// Copyright (c) 2025 Thomas Hühn (XXTH) 
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#include "fluxParticleManager.h"

FluxParticleEmitter* FluxParticleManager::addEmitter(const EmitterProperties& props) {
    auto emitter = std::make_unique<FluxParticleEmitter>(props);
    FluxParticleEmitter* ptr = emitter.get();
    mEmitters.push_back(std::move(emitter));
    return ptr;
}

void FluxParticleManager::Update(F32 dt) {
    // Update all emitters
    for (auto it = mEmitters.begin(); it != mEmitters.end(); ) {
        (*it)->Update(dt);
        ++it;
    }
}

void FluxParticleManager::Draw() {
    for (const auto& emitter : mEmitters) {
        emitter->Draw();
    }
}

void FluxParticleManager::clear() {
    mEmitters.clear();
}

FluxParticleManager::~FluxParticleManager() {
    clear();
}
