//-----------------------------------------------------------------------------
// Copyright (c) 2025 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#include "fluxParticleEmitter.h"
#include <algorithm>
#include <cmath>
#include "fluxGlobals.h"
//-----------------------------------------------------------------------------
FluxParticleEmitter::FluxParticleEmitter(const EmitterProperties& props)
: mProperties(props),
mSpawnTimer(0.0f)
{
    mParticles.reserve(props.maxParticles);
    mActive = mProperties.autoActivate;
}
//-----------------------------------------------------------------------------
FluxParticleEmitter::~FluxParticleEmitter() {}
//-----------------------------------------------------------------------------
void FluxParticleEmitter::update(F32 dt)
{
    // 1. Update and Clean up particles using Swap-and-Pop (O(1) deletion)
    for (size_t i = 0; i < mParticles.size(); )
    {
        mParticles[i].update(dt);

        if (mParticles[i].lifeRemaining <= 0.0f)
        {
            // Move last element to current position and shrink
            mParticles[i] = mParticles.back();
            mParticles.pop_back();
            // Don't increment i; check the swapped particle next iteration
        }
        else
        {
            ++i;
        }
    }

    if ( !mActive ) {
        return;
    }

    // 2. Spawn new particles
    if (mParticles.size() < mProperties.maxParticles)
    {
        mSpawnTimer += dt;
        F32 spawnInterval = 1.0f / mProperties.spawnRate;

        while (mSpawnTimer >= spawnInterval)
        {
            if (mParticles.size() >= mProperties.maxParticles) {
                if (mProperties.playOnce)
                    mActive = false;
                break;
            }

            emitParticle();
            mSpawnTimer -= spawnInterval;
        }
    }
}
//-----------------------------------------------------------------------------
void FluxParticleEmitter::render()
{
    for (const auto& particle : mParticles)
    {
        // Particle is guaranteed alive by the update loop cleanup
        if (particle.texture)
        {
            // Use the lerped color based on life
            Color4F currentColor = particle.getCurrentColor();

            Render2D.drawWithTransform(
                particle.texture,
                particle.position,
                particle.rotation,
                particle.scale,
                currentColor
            );
        }
    }
}
//-----------------------------------------------------------------------------
void FluxParticleEmitter::emitParticle()
{
    // Re-check bounds to be safe
    if (mParticles.size() < mProperties.maxParticles)
    {
        mParticles.emplace_back();
        initializeParticle(mParticles.back());
    }
}
//-----------------------------------------------------------------------------
void FluxParticleEmitter::initializeParticle(FluxParticle& particle)
{

    particle.position = mProperties.position;

    particle.lifetime = RandInRange(mProperties.minLifetime, mProperties.maxLifetime);
    particle.lifeRemaining = particle.lifetime;

    F32 speed = RandInRange(mProperties.minSpeed, mProperties.maxSpeed);
    F32 angle = RandInRange(mProperties.minAngle, mProperties.maxAngle);

    // Use cosf/sinf for F32 precision and performance
    particle.velocity = Point2F{ cosf(angle), sinf(angle) } * speed;
    particle.acceleration = { 0.0f, 0.0f };



    particle.rotation = particle.rotation = mProperties.rotation; // RandFloat() * 2.0f * FLUX_PI ;
    particle.rotationSpeed = RandInRange(mProperties.minRotationSpeed, mProperties.maxRotationSpeed); //(RandFloat() - 0.5f) * 2.0f;

    particle.scale = RandInRange(mProperties.minScale / 10.f , mProperties.maxScale / 10.f );

    // Initializing Colors
    particle.startColor = {
        RandInRangeF(mProperties.startColorMin.r, mProperties.startColorMax.r),
        RandInRangeF(mProperties.startColorMin.g, mProperties.startColorMax.g),
        RandInRangeF(mProperties.startColorMin.b, mProperties.startColorMax.b),
        RandInRangeF(mProperties.startColorMin.a, mProperties.startColorMax.a)
    };

    particle.endColor = {
        RandInRangeF(mProperties.endColorMin.r, mProperties.endColorMax.r),
        RandInRangeF(mProperties.endColorMin.g, mProperties.endColorMax.g),
        RandInRangeF(mProperties.endColorMin.b, mProperties.endColorMax.b),
        RandInRangeF(mProperties.endColorMin.a, mProperties.endColorMax.a)
    };

    particle.texture = mProperties.texture;
}
//-----------------------------------------------------------------------------
void FluxParticleEmitter::play()
{
    mSpawnTimer = 1.0f; // Force instant burst for explosions
    mActive = true;
};
//-----------------------------------------------------------------------------
void FluxParticleEmitter::stop()
{
    mActive = false;
}
//-----------------------------------------------------------------------------
void FluxParticleEmitter::reset()
{
    // 1. Clear existing particles so the new effect starts fresh
    mParticles.clear();

    // 2. Reset the timer.
    // Setting this to 1.0f ensures that 'while (mSpawnTimer >= spawnInterval)'
    // triggers immediately on the next update call for a burst effect.
    mSpawnTimer = 1.0f;

    // 3. Re-activate the emitter
    mActive = true;
}
//-----------------------------------------------------------------------------
void FluxParticleEmitter::setProperties(const EmitterProperties& props)
{
    mProperties = props;
    // Always call reset when changing properties to ensure
    // playOnce and spawnTimer are in the correct state
    reset();

    // overwrite the reset activation if the Properties dont want autostart
    mActive = props.autoActivate;
}

