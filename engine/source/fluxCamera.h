//-----------------------------------------------------------------------------
// Copyright (c) 2025 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#pragma once
#include "fluxGlobals.h"

class FluxCamera {
public:
    FluxCamera(F32 width, F32 height);

    // Transformation methods
    void setPosition(const Point2F& pos) { mPosition = pos; mDirty = true; }
    void move(const Point2F& delta)      { mPosition.x += delta.x; mPosition.y += delta.y; mDirty = true; }
    void setZoom(F32 zoom)               { mZoom = (zoom > 0.01f) ? zoom : 0.01f; mDirty = true; }
    void moveZoom( F32 delta )           { setZoom(mZoom + delta);}
    void onResize(F32 newWidth, F32 newHeight) { mWidth = newWidth; mHeight = newHeight; mDirty = true; }
    // Update and Access
    void update();
    const float* getViewMatrix() const { return mViewMatrix; }

    // Utility: Center camera on a target
    void lookAt(F32 x, F32 y) { setPosition({x, y}); }

    Point2F getPosition() const { return mPosition; }
    F32     getZoom() const { return mZoom; }

    Point2F getSize() const { return { mWidth, mHeight }; }

    RectF getVisibleWorldRect(bool lDoSnap = true);
private:
    F32 mWidth;
    F32 mHeight;
    F32 mZoom;
    bool mDirty;
    Point2F mPosition;
    alignas(16) float mViewMatrix[16];

};
