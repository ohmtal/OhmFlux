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
void FluxParticleEmitter::update(F32 dt) {
    // 1. Bulk Update
    // Tight loop: maximizes L1 cache hits and allows auto-vectorization
    for (auto& p : mParticles) {
        p.lifeRemaining -= dt;
        p.position += p.velocity * dt;
    }

    // 2. Unstable Cleanup (O(1) deletion)
    // Faster than std::remove_if for large particle structs
    // because it avoids shifting the entire tail of the vector.
    for (size_t i = 0; i < mParticles.size(); ) {
        if (mParticles[i].lifeRemaining <= 0.0f) {
            mParticles[i] = std::move(mParticles.back());
            mParticles.pop_back();
        } else {
            ++i;
        }
    }

    if (!mActive) return;

    // 3. Batch Spawning
    mSpawnTimer += dt;

    // Optimization: Use multiplication instead of division where possible
    F32 invSpawnRate = 1.0f / mProperties.spawnRate;
    int numToSpawn = static_cast<int>(mSpawnTimer * mProperties.spawnRate);

    if (numToSpawn > 0) {
        // Call the batch function we optimized previously
        emitParticlesBatch(numToSpawn);

        // Subtract only the time accounted for by the particles actually spawned
        mSpawnTimer -= numToSpawn * invSpawnRate;
    }

    // 4. State Management
    if (mProperties.playOnce && mParticles.size() >= mProperties.maxParticles) {
        mActive = false;
    }
}

// void FluxParticleEmitter::update(F32 dt)
// {
//     // 1. Update and Clean up particles using Swap-and-Pop (O(1) deletion)
//     for (size_t i = 0; i < mParticles.size(); )
//     {
//         mParticles[i].update(dt);
//
//         if (mParticles[i].lifeRemaining <= 0.0f)
//         {
//             // Move last element to current position and shrink
//             mParticles[i] = mParticles.back();
//             mParticles.pop_back();
//             // Don't increment i; check the swapped particle next iteration
//         }
//         else
//         {
//             ++i;
//         }
//     }
//
//     if ( !mActive ) {
//         return;
//     }
//
//     // 2. Spawn new particles
//     if (mParticles.size() < mProperties.maxParticles)
//     {
//         mSpawnTimer += dt;
//         F32 spawnInterval = 1.0f / mProperties.spawnRate;
//
//         while (mSpawnTimer >= spawnInterval)
//         {
//             if (mParticles.size() >= mProperties.maxParticles) {
//                 if (mProperties.playOnce)
//                     mActive = false;
//                 break;
//             }
//
//             emitParticle();
//             mSpawnTimer -= spawnInterval;
//         }
//     }
// }
//-----------------------------------------------------------------------------
void FluxParticleEmitter::appendParticleVertices
(
    std::vector<Vertex2D>& buffer,
    FluxTexture* tex,
    const Point3F& pos,
    float rotation,
    float scale,
    const Color4F& color)
{
    // 1. Get UVs ONCE (In a real emitter, you'd pass these in as params to save even more time)
    // For now, let's assume full texture (0,0 to 1,1)
    float umin = 0.0f, vmin = 0.0f, umax = 1.0f, vmax = 1.0f;

    // 2. Vertex Positions
    float halfW = (tex->getWidth() * scale) * 0.5f;
    float halfH = (tex->getHeight() * scale) * 0.5f;

    float cosR = cosf(rotation);
    float sinR = sinf(rotation);

    // 3. Manual unrolling of the loop for speed
    size_t i = buffer.size();
    buffer.resize(i + 4);

    // Corner offsets
    float cornersX[4] = { -halfW,  halfW, halfW, -halfW };
    float cornersY[4] = { -halfH, -halfH, halfH,  halfH };
    float uvsU[4] = { umin, umax, umax, umin };
    float uvsV[4] = { vmin, vmin, vmax, vmax };

    for (int j = 0; j < 4; j++) {
        buffer[i + j].pos.x = (cornersX[j] * cosR - cornersY[j] * sinR) + pos.x;
        buffer[i + j].pos.y = (cornersX[j] * sinR + cornersY[j] * cosR) + pos.y;
        buffer[i + j].pos.z = -pos.z;
        buffer[i + j].color = color;
        buffer[i + j].uv.x    = uvsU[j];
        buffer[i + j].uv.y    = uvsV[j];
    }
}

//-----------------------------------------------------------------------------
//enhanced batch version:
void FluxParticleEmitter::render()
{
    if (mParticles.empty()) return;

    // 1. Clear local cache
    _VertexBuffer.clear();

    // 2. Performance: Pre-reserve memory to avoid reallocations during the loop
    if (_VertexBuffer.capacity() < mParticles.size() * 4) {
        _VertexBuffer.reserve(mParticles.size() * 4);
    }

    for (const auto& particle : mParticles)
    {
        // Use the lerped color based on life
        Color4F currentColor = particle.getCurrentColor();

        // Optimized: Bypass the heavy generateDrawParams if possible
        // and call the vertex-filling logic directly.
        appendParticleVertices(
            _VertexBuffer,
            particle.texture,
            particle.position,
            particle.rotation,
            particle.scale,
            currentColor
        );
    }
    // 3. Submit ONE command for the WHOLE system
    RenderCommand cmd;
    cmd.params.z = getLayer();
    cmd.textureHandle = mProperties.texture->getHandle();
    cmd.isGui = false;

    // This callback will be called by FluxRender2D::renderBatch
    cmd.userData = this;
    cmd.customRenderCallback = [](const RenderCommand& c) {
        auto* emitter = static_cast<FluxParticleEmitter*>(c.userData);
        // Direct GPU Draw Call:
        Render2D.renderCurrentBuffer(emitter->_VertexBuffer, c.textureHandle, false);
    };

    Render2D.submitCustomCommand(cmd);
}
//-----------------------------------------------------------------------------
void FluxParticleEmitter::emitParticlesBatch(int count)
{
    // 1. Strict Boundary Guard
    int spaceLeft = mProperties.maxParticles - static_cast<int>(mParticles.size());
    // int toSpawn = std::min(count, spaceLeft);
    int toSpawn = (std::min)(count, spaceLeft);

    if (toSpawn <= 0) return;

    // 2. Pre-allocate to prevent multiple reallocations
    // (Ideally, reserve is called once at emitter creation, but this is a safety net)
    if (mParticles.capacity() < mParticles.size() + toSpawn) {
        mParticles.reserve(mProperties.maxParticles);
    }

    // 3. Use emplace_back to avoid the "double-write" of resize()
    // Modern compilers (Clang 16+, GCC 13+) can often vectorize this loop
    // if initializeParticle is inlined.
    for (int i = 0; i < toSpawn; ++i)
    {
        // Construct directly in place
        auto& p = mParticles.emplace_back();
        initializeParticle(p);
    }
}

// void FluxParticleEmitter::emitParticle()
// {
//     // Re-check bounds to be safe
//     if (mParticles.size() < mProperties.maxParticles)
//     {
//         mParticles.emplace_back();
//         initializeParticle(mParticles.back());
//     }
// }
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

    particle.rotation = mProperties.rotation; // RandFloat() * 2.0f * FLUX_PI ;
    particle.rotationSpeed = RandInRange(mProperties.minRotationSpeed, mProperties.maxRotationSpeed); //(RandFloat() - 0.5f) * 2.0f;

    particle.scale = RandInRange(mProperties.minScale / 10.f , mProperties.maxScale / 10.f );

    // Initializing Colors
    if (mProperties.startColorMin == mProperties.startColorMax)
    {
            particle.startColor = mProperties.startColorMin;
    } else {
        particle.startColor = {
            RandInRange(mProperties.startColorMin.r, mProperties.startColorMax.r),
            RandInRange(mProperties.startColorMin.g, mProperties.startColorMax.g),
            RandInRange(mProperties.startColorMin.b, mProperties.startColorMax.b),
            RandInRange(mProperties.startColorMin.a, mProperties.startColorMax.a)
        };
    }

    if ( mProperties.endColorMin ==  mProperties.endColorMax )
    {
        particle.endColor = mProperties.endColorMin;
    } else {
        particle.endColor = {
            RandInRange(mProperties.endColorMin.r, mProperties.endColorMax.r),
            RandInRange(mProperties.endColorMin.g, mProperties.endColorMax.g),
            RandInRange(mProperties.endColorMin.b, mProperties.endColorMax.b),
            RandInRange(mProperties.endColorMin.a, mProperties.endColorMax.a)
        };
    }

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

