#pragma once
#include "fluxGlobals.h" // For Point3F, Color4F

struct FluxLight {
    Point3F position = {0.f, 0.f, 0.f};
    Color4F color    = {1.f, 1.f, 1.f, 1.f}; // White at full intensity
    float radius     = 100.f;

    // Spotlight specific members
    Point2F direction = {0.f, -1.f}; // Default pointing "down" or "forward"
    float cutoff      = -1.0f;       // -1.0 means 360 degrees (Omni-light)

    // Standard constructor
    FluxLight(Point3F p = {0.f, 0.f, 0.f},
              Color4F c = {1.f, 1.f, 1.f, 1.f},
              float r = 100.f)
    : position(p), color(c), radius(r) {}

    // Helper to turn this light into a spotlight
    FluxLight& setAsSpotlight(Point2F dir, float angleInDegrees) {
        direction = dir;
        float radians = angleInDegrees * 0.5f * (3.14159265f / 180.0f);
        cutoff = cosf(radians);

        return *this; // Return the current object
    }
};
