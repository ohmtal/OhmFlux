//-----------------------------------------------------------------------------
// Copyright (c) 2025 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#pragma once
#ifndef _FLUX_PARTICLE_H
#define _FLUX_PARTICLE_H

#include "fluxMath.h"
#include "fluxTexture.h"

struct FluxParticle
{
    Point3F position;
    Point2F velocity;
    Point2F acceleration;
    F32 rotation;
    F32 rotationSpeed;
    F32 scale;
    F32 lifetime; // Total lifetime
    F32 lifeRemaining; // Current life remaining
    Color4F startColor;
    Color4F endColor;
    FluxTexture* texture; // Optional texture for the particle

    void update(F32 dt)
    {
        if (lifeRemaining > 0.0f)
        {
            velocity += acceleration * dt;
            position += velocity * dt;
            rotation += rotationSpeed * dt;
            lifeRemaining -= dt;
        }
    }

    F32 getNormalizedLife() const
    {
        return lifeRemaining / lifetime;
    }

    Color4F getCurrentColor() const
    {
        F32 lifeRatio = 1.0f - getNormalizedLife();
        return startColor * (1.0f - lifeRatio) + endColor * lifeRatio;
    }
};

#endif // _FLUX_PARTICLE_H
