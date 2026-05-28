//-----------------------------------------------------------------------------
// Copyright (c) 2025 Thomas Hühn (XXTH) 
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#pragma once
#ifndef _FLUX_PARTICLE_H
#define _FLUX_PARTICLE_H

#include "core/fluxMath.h"
#include "core/fluxTexture.h"

struct FluxParticle
{
    Point3F position;
    Point2F velocity;
    Point2F acceleration;
    F32 rotation;
    F32 rotationSpeed;
    F32 scale;
    F32 endScale;
    F32 lifetime; // Total lifetime
    F32 lifeRemaining; // Current life remaining
    Color4F startColor;
    Color4F endColor;
    FluxTexture* texture; // Optional texture for the particle

    void update(F32 dt)
    {
        if (lifeRemaining > 0.0f)
        {
            mLifeRatio =  1.0f - getNormalizedLife();
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

    F32 getCurrentScale() const
    {
        F32 t = mLifeRatio;
        F32 exponentialT = t * t * t; // Cubic curve for aggressive forward motion

        return scale * (1.0f - exponentialT) + endScale * exponentialT;
    }

    Color4F getCurrentColor() const
    {
        return startColor * (1.0f - mLifeRatio) + endColor * mLifeRatio;
    }

private:
    F32 mLifeRatio = 0.0f;
};

#endif // _FLUX_PARTICLE_H
