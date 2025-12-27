//-----------------------------------------------------------------------------
// Copyright (c) 2025 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#include "fluxCamera.h"
#include <cstring>
#include "fluxMain.h"
#include "fluxRenderObject.h"

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

    static bool lOverWriteDirty = false;
    if ( mObjectToFollow != nullptr )
    {
        mPosition = mObjectToFollow->getPosition();
        lOverWriteDirty = true;
    } else if (mMoveSpeed != 0.f && !mMoveVector.isZero())
    {
        mPosition += mMoveVector * mMoveSpeed * getFrameTime();
        // dLog("new position is %4.2f, %4.2f", mPosition.x, mPosition.y);
        lOverWriteDirty = true;
    }

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

    mDirty = lOverWriteDirty;
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
//-----------------------------------------------------------------------------
void FluxCamera::setAutoMove(Point2F lMoveVector, F32 lMoveSpeed)
{
    mMoveVector = lMoveVector;
    mMoveSpeed  = lMoveSpeed;
    mObjectToFollow = nullptr;
    setDirty();
}

void FluxCamera::setMoveVector(Point2F lMoveVector)
{
    mMoveVector = lMoveVector;
    mObjectToFollow = nullptr; //here also  ?
    setDirty();
}


void FluxCamera::clearAutoMove()
{
    mMoveVector = { 0.f, 0.f };
    mMoveSpeed  = 0.f;
    mObjectToFollow = nullptr; // this also ?
    setDirty();
}

//-----------------------------------------------------------------------------
void FluxCamera::setObjectToFollow( FluxRenderObject* lObject )
{
    mMoveVector = { 0.f, 0.f };
    mMoveSpeed  = 0.f;
    mObjectToFollow = lObject;
    setDirty();
}
//-----------------------------------------------------------------------------
