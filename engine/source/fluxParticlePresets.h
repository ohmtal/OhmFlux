#pragma once
#include "fluxParticleEmitter.h"


namespace ParticlePresets
{
    // you can better do evil thinks like:
    /*
        mSparkEmitter = ParticleManager.addEmitter(
            ParticlePresets::sparkPreset
            .setTexture(loadTexture( "assets/particles/SnowFlake1.png" ))
            .setScaleMinMax( 0.01f, 0.1f)
        );
    */

    // so this is useless.....
    // EmitterProperties getPreset(const EmitterProperties& preset , FluxTexture* lTextrue )
    // {
    //     EmitterProperties result = preset;
    //     result.texture = lTextrue;
    //     return result;
    // }

    // ------------- playOnce particles --------------------
    EmitterProperties explosionPreset = {
        .spawnRate = 10000.0f, // High rate to ensure instant burst via your update loop
        .maxParticles = 80,
        .playOnce = true,   // use .play to play again
        .minLifetime = 0.4f,   .maxLifetime = 0.7f,
        .minSpeed = 300.0f,    .maxSpeed = 600.0f,
        .minAngle = 0.0f,      .maxAngle = 6.283f, // Full 360 degrees
        .minScale = 4.0f,      .maxScale = 8.0f,
        .startColorMin = {1.0f, 1.0f, 0.8f, 1.0f}, // White Flash
        .startColorMax = {1.0f, 0.7f, 0.0f, 1.0f}, // Orange Core
        .endColorMin = {0.1f, 0.1f, 0.1f, 0.0f},   // Dark Smoke
        .endColorMax = {0.3f, 0.3f, 0.3f, 0.0f},   // Light Smoke

    };

    EmitterProperties sparkPreset = {
        .spawnRate = 5000.0f,
        .maxParticles = 20,
        .playOnce = true,
        .minLifetime = 0.2f,   .maxLifetime = 0.4f,
        .minSpeed = 400.0f,    .maxSpeed = 800.0f,
        .minAngle = 0.0f,      .maxAngle = 6.283f,
        .minScale = 0.5f,      .maxScale = 1.2f,
        .startColorMin = {1.0f, 1.0f, 1.0f, 1.0f}, // White
        .startColorMax = {1.0f, 0.9f, 0.5f, 1.0f}, // Light Yellow
        .endColorMin = {1.0f, 0.5f, 0.0f, 0.0f},   // Orange
        .endColorMax = {1.0f, 1.0f, 0.0f, 0.0f},   // Yellow

    };

    // ------------- looping particles --------------------

    EmitterProperties firePreset = {
        .spawnRate = 60.0f,
        .maxParticles = 200,
        .playOnce = false,
        .minLifetime = 0.8f, .maxLifetime = 1.2f,
        .minSpeed = 60.0f,   .maxSpeed = 120.0f,
        .minAngle = -1.74f,  .maxAngle = -1.40f, // Narrow upward cone (~ -100 to -80 degrees)
        .minScale = 1.5f,    .maxScale = 3.0f,
        .startColorMin = {1.0f, 0.9f, 0.4f, 1.0f}, // Bright Yellow
        .startColorMax = {1.0f, 1.0f, 0.7f, 1.0f}, // Pale Yellow
        .endColorMin = {0.8f, 0.2f, 0.0f, 0.0f},   // Deep Red (Faded)
        .endColorMax = {1.0f, 0.4f, 0.0f, 0.0f},   // Orange (Faded)
    };

    EmitterProperties soulFirePreset = {
        .spawnRate = 45.0f,
        .maxParticles = 150,
        .playOnce = false,
        .minLifetime = 0.6f,   .maxLifetime = 1.0f,
        .minSpeed = 80.0f,    .maxSpeed = 150.0f,
        .minAngle = -1.65f,    .maxAngle = -1.48f, // Steady upward jet
        .minScale = 1.0f,      .maxScale = 2.0f,
        .startColorMin = {0.0f, 0.7f, 1.0f, 1.0f}, // Cyan
        .startColorMax = {0.4f, 0.0f, 1.0f, 1.0f}, // Purple-ish
        .endColorMin = {0.0f, 0.1f, 0.4f, 0.0f},   // Deep Indigo
        .endColorMax = {0.1f, 0.0f, 0.2f, 0.0f},
    };


    EmitterProperties magicFirePreset = {
        .spawnRate = 45.0f,
        .maxParticles = 300,
        .minLifetime = 0.5f,
        .maxLifetime = 0.9f,
        .minSpeed = 120.0f,
        .maxSpeed = 180.0f,
        .minAngle = -1.65f, // -95 degrees
        .maxAngle = -1.48f, // -85 degrees
        .minScale = 0.5f,
        .maxScale = 1.5f,
        .startColorMin = {0.0f, 0.8f, 1.0f, 1.0f}, // Cyan
        .startColorMax = {0.5f, 0.0f, 1.0f, 1.0f}, // Purple
        .endColorMin = {0.1f, 0.0f, 0.3f, 0.0f},   // Deep Indigo
        .endColorMax = {0.2f, 0.0f, 0.5f, 0.0f}    // Faded Violet
    };

    EmitterProperties torchPreset = {
        .spawnRate = 80.0f,       // Constant stream
        .maxParticles = 200,
        .minLifetime = 0.6f,
        .maxLifetime = 1.0f,
        .minSpeed = 150.0f,       // High speed for a "jet" look
        .maxSpeed = 250.0f,
        .minAngle = -1.65f,       // Narrow cone pointing up
        .maxAngle = -1.48f,
        .minScale = 1.5f,
        .maxScale = 3.0f,
        .startColorMin = {1.0f, 0.6f, 0.0f, 1.0f}, // Bright Orange
        .startColorMax = {1.0f, 0.9f, 0.2f, 1.0f}, // Golden Yellow
        .endColorMin = {0.3f, 0.0f, 0.0f, 0.0f},   // Fades to deep red
        .endColorMax = {0.5f, 0.1f, 0.0f, 0.0f}
    };


    EmitterProperties smokePreset = {
        .spawnRate = 15.0f,
        .maxParticles = 40,
        .minLifetime = 2.0f,      // Smoke lingers
        .maxLifetime = 4.0f,
        .minSpeed = 20.0f,        // Drifts slowly upward
        .maxSpeed = 50.0f,
        .minAngle = -2.0f,        // Wider spread than the flame
        .maxAngle = -1.14f,
        .minScale = 2.0f,
        .maxScale = 5.0f,         // Grows as it rises
        .startColorMin = {0.2f, 0.2f, 0.2f, 0.6f}, // Semi-transparent grey
        .startColorMax = {0.3f, 0.3f, 0.3f, 0.8f},
        .endColorMin = {0.0f, 0.0f, 0.0f, 0.0f},   // Fades to nothing
        .endColorMax = {0.1f, 0.1f, 0.1f, 0.0f}
    };
} //nameSpace
