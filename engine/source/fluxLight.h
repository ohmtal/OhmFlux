#pragma once
#include "fluxGlobals.h" // For Point3F, Color4F

struct FluxLight
{
    Point3F position = {0.f, 0.f, 0.f};
    Color4F color    = {1.f, 1.f, 1.f, 1.f}; // White at full intensity
    F32 radius     = 100.f;

    // Spotlight specific members
    Point2F direction = {0.f, -1.f}; // Default pointing "down" or "forward"
    F32 cutoff      = -1.0f;       // -1.0 means 360 degrees (Omni-light)

    // Standard constructor
    FluxLight(Point3F p = {0.f, 0.f, 0.f},
              Color4F c = {1.f, 1.f, 1.f, 1.f},
              F32 r = 100.f)
    : position(p), color(c), radius(r) {}

    // Helper to turn this light into a spotlight
    FluxLight& setAsSpotlight(Point2F dir, F32 angleInDegrees) {
        direction = dir;
        F32 radians = angleInDegrees * 0.5f * (3.14159265f / 180.0f);
        cutoff = cosf(radians);

        return *this; // Return the current object
    }

    // Generates a bounding rectangle where position is the center
    RectF getRectF() const {
        // Calculate the top-left corner by subtracting radius from center (x, y)
        // Calculate size as diameter (radius * 2)
        return RectF {
            position.x - radius,
            position.y - radius,
            radius * 2.0f,
            radius * 2.0f
        };
    }
};
