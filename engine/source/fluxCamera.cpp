//-----------------------------------------------------------------------------
// Copyright (c) 2025 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#include "fluxCamera.h"
#include <cstring>
#include "fluxMain.h"

FluxCamera::FluxCamera(F32 width, F32 height)
: mWidth(width), mHeight(height), mZoom(1.0f), mDirty(true)
{
    mPosition = { width * 0.5f, height * 0.5f };
    update();
}
//-----------------------------------------------------------------------------
void FluxCamera::update()
{
    if (!mDirty) return;

    // Reset to Identity Matrix
    std::memset(mViewMatrix, 0, sizeof(mViewMatrix));

    mViewMatrix[0] = mZoom;  // Scale X
    mViewMatrix[5] = mZoom;  // Scale Y
    mViewMatrix[10] = 1.0f;  // Scale Z
    mViewMatrix[15] = 1.0f;  // W

    // Translation (Column-major order for OpenGL)
    // Formula: -pos * zoom + (screen_size / 2)
    mViewMatrix[12] = -mPosition.x * mZoom + (mWidth * 0.5f);
    mViewMatrix[13] = -mPosition.y * mZoom + (mHeight * 0.5f);

    mDirty = false;
}
//-----------------------------------------------------------------------------
RectF FluxCamera::getVisibleWorldRect(bool lDoSnap) // Add bool parameter
{
    float zoom = getZoom();
    Point2F screenSize = getScreenObject()->getScreenSizeF();
    Point2F rawPos = getPosition();
    Point2F finalPos = rawPos;

    if (lDoSnap)
    {
        // Snap logic: converts world units to screen pixels, floors, then back
        finalPos.x = std::floor(rawPos.x * zoom) / zoom;
        finalPos.y = std::floor(rawPos.y * zoom) / zoom;
    }

    Point2F worldSize = screenSize / zoom;
    Point2F topLeft = finalPos - (worldSize / 2.0f);

    return { topLeft.x, topLeft.y, worldSize.x, worldSize.y };
}

// RectF FluxCamera::getVisibleWorldRect()
// {
//     float zoom = getZoom();
//     Point2F screenSize = getScreenObject()->getScreenSizeF();
//
//     // 1. Precise position
//     Point2F rawPos = getPosition();
//
//     // 2. The Snap: Calculate the position as it will appear on screen
//     // We floor the pixel-space coordinate, then convert back to world units
//     Point2F snappedPos;
//     snappedPos.x = std::floor(rawPos.x * zoom) / zoom;
//     snappedPos.y = std::floor(rawPos.y * zoom) / zoom;
//
//     // 3. Use snappedPos for the rectangle
//     Point2F worldSize = screenSize / zoom;
//     Point2F topLeft = snappedPos - (worldSize / 2.0f);
//
//     return { topLeft.x, topLeft.y, worldSize.x, worldSize.y };
// }
//
