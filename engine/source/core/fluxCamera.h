//-----------------------------------------------------------------------------
// Copyright (c) 2025 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#pragma once
#include "core/fluxGlobals.h"

class FluxRenderObject;

class FluxCamera {
private:
    F32 mWidth;
    F32 mHeight;
    F32 mZoom;
    bool mDirty;
    Point2F mPosition;
    alignas(16) float mViewMatrix[16];

    // movement
    FluxRenderObject* mObjectToFollow = nullptr;
    Point2F mMoveVector = { 0.f, 0.f };
    F32 mMoveSpeed = 0.f;

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

    void setDirty() { mDirty = true; }
    void setAutoMove( Point2F lMoveVector, F32 lMoveSpeed );
    void setMoveVector(Point2F lMoveVector);
    Point2F getMoveVector( ) { return mMoveVector; };
    void clearAutoMove(  );
    void setObjectToFollow( FluxRenderObject* lObject );

};
