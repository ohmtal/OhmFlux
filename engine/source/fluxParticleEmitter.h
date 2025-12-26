//-----------------------------------------------------------------------------
// Copyright (c) 2025 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#pragma once
#ifndef _FLUX_PARTICLEEMITTER_H
#define _FLUX_PARTICLEEMITTER_H

#include <vector>

#include "fluxParticle.h"
#include "fluxRender2D.h" // For rendering particles

class FluxTexture;

struct EmitterProperties
{
    // Position in 2D space
    Point3F position = { 0.f, 0.f, 0.f };

    // Emission Logic
    F32 spawnRate = 10.0f;           // 10 particles per second
    U32 maxParticles = 100;          // Buffer limit
    bool playOnce = false;           // Default to looping
    bool autoActivate = true;        // after create it plays or not

    // Physical Behavior
    F32 minLifetime = 1.0f;          // 1 second
    F32 maxLifetime = 1.0f;
    F32 minSpeed = 50.0f;            // 50 units per second
    F32 maxSpeed = 50.0f;
    F32 minAngle = 0.0f;             // 0 radians (Right)
    F32 maxAngle = 6.283185f;        // 2 * PI (Full Circle)


    // FIXME it's good but can be better
    F32 rotation         = 0.0f; //degree
    F32 minRotationSpeed = 0.1f;
    F32 maxRotationSpeed = 2.0f;

    // Visuals
    F32 minScale = 1.0f;
    F32 maxScale = 1.0f;
    Color4F startColorMin = cl_White; // Changed to white so they are visible
    Color4F startColorMax = cl_White;
    Color4F endColorMin = cl_Black;   // Fade to black/transparency
    Color4F endColorMax = cl_Black;

    FluxTexture* texture = nullptr;

    // with this you can create a evil magic line to update
    // parameters from a Template
    // see also fluxParticlePresets.h
    EmitterProperties& setSpawnRate( F32 lValue ) { spawnRate = lValue; return *this;}
    EmitterProperties& setMaxParticle( F32 lValue ) { maxParticles = lValue; return *this;}
    EmitterProperties& setPlayOnce( bool lValue ) { playOnce = lValue; return *this;}
    EmitterProperties& setAutoActivate( bool lValue ) { autoActivate = lValue; return *this;}
    EmitterProperties& setLifeTimeMinMax( F32 lMin, F32 lMax ) { minLifetime = lMin;  maxLifetime = lMax; return *this;}
    EmitterProperties& setSpeedMinMax( F32 lMin, F32 lMax ) { minSpeed = lMin;  maxSpeed = lMax; return *this;}
    EmitterProperties& setAngleMinMax( F32 lMin, F32 lMax ) { minAngle = lMin;  maxAngle = lMax; return *this;}
    EmitterProperties& setRotation( F32 lValue ) { rotation = lValue; return *this;}
    EmitterProperties& setRotationSpeedMinMax( F32 lMin, F32 lMax ) { minRotationSpeed = lMin;  maxRotationSpeed = lMax; return *this;}
    EmitterProperties& setStartEndColorMinMax( Color4F lStartMin, Color4F lStartMax,Color4F lEndMin, Color4F lEndMax ) {
        startColorMin = lStartMin;
        startColorMax = lStartMax;
        endColorMin   = lEndMin;
        endColorMax   = lEndMax;
        return *this;
    }
    EmitterProperties& setScaleMinMax( F32 lMin, F32 lMax ) { minScale = lMin;  maxScale = lMax; return *this;}
    EmitterProperties& setTexture( FluxTexture* lTexture ) { texture = lTexture; return *this;}

};

class FluxParticleEmitter
{
public:
    FluxParticleEmitter(const EmitterProperties& props);
    ~FluxParticleEmitter();

    void update(F32 dt);
    void render();

    FluxParticleEmitter*  setPosition( Point3F lPosition ) { mProperties.position = lPosition; return this; };
    Point3F getPosition() const { return mProperties.position; }
    F32 getLayer() const { return mProperties.position.z; }

    void play();        //activate
    void stop();        // deactivate like pause
    void reset();       // like play but reset the particles
    void setProperties(const EmitterProperties& props); // update the props
    bool getActive() { return mActive; };
private:
    void emitParticle();
    void initializeParticle(FluxParticle& particle);
    void appendParticleVertices ( std::vector<Vertex2D>& buffer,
                                FluxTexture* tex,
                                const Point3F& pos,
                                float rotation,
                                float scale,
                                const Color4F& color);

    std::vector<Vertex2D> _VertexBuffer;

    EmitterProperties mProperties;
    std::vector<FluxParticle> mParticles;
    F32 mSpawnTimer;


    bool mActive = true; // enable disable
public:
    EmitterProperties& getProperties()  { return  mProperties; }
    const EmitterProperties& getProperties() const { return mProperties; }
};

#endif // _FLUX_PARTICLEEMITTER_H
