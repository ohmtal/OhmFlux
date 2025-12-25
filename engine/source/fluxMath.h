//-----------------------------------------------------------------------------
// Copyright (c) 2012/2025 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#pragma once
#ifndef _FLUX_MATH_H_
#define _FLUX_MATH_H_

#include <math.h>
#include <cassert>

#include "fluxGlobals.h"

/**
 * @brief Checks for intersection between two Axis-Aligned Bounding Boxes (AABB).
 * @param rectA The first rectangle (e.g., the search area).
 * @param rectB The second rectangle (e.g., the node bounds or object bounds).
 * @return True if the rectangles overlap, false otherwise.
 */
inline bool checkAABBIntersectionI(const RectI& rectA, const RectI& rectB) {
    // Check if the rectangles overlap on the X axis
    if (rectA.x < rectB.x + rectB.w && rectA.x + rectA.w > rectB.x) {
        // Check if the rectangles overlap on the Y axis
        if (rectA.y < rectB.y + rectB.h && rectA.y + rectA.h > rectB.y) {
            return true; // Intersection found
        }
    }
    return false; // No intersection
}

inline bool checkAABBIntersectionF(const RectF& rectA, const RectF& rectB) {
    // Check if the rectangles overlap on the X axis
    if (rectA.x < rectB.x + rectB.w && rectA.x + rectA.w > rectB.x) {
        // Check if the rectangles overlap on the Y axis
        if (rectA.y < rectB.y + rectB.h && rectA.y + rectA.h > rectB.y) {
            return true; // Intersection found
        }
    }
    return false; // No intersection
}


//----------------------------------------------------------------------------
//  Math function for createOrthoMatrix
//----------------------------------------------------------------------------


// Creates a matrix that maps 0-800 pixels to OpenGL space
inline void createOrthoMatrix(float left, float right, float bottom, float top, float near, float far, float* m) {
    for(int i = 0; i < 16; i++) m[i] = 0.0f;
    m[0]  = 2.0f / (right - left);
    m[5]  = 2.0f / (top - bottom);
    m[10] = -2.0f / (far - near);
    m[12] = -(right + left) / (right - left);
    m[13] = -(top + bottom) / (top - bottom);
    m[14] = -(far + near) / (far - near);
    m[15] = 1.0f;
}

// Handles your rotation and position
inline void calculateModelMatrix(float x, float y, float z, float angleDeg, float scaleX, float scaleY, float* m) {
    float rad = angleDeg * 3.14159265f / 180.0f;
    float s = sinf(rad);
    float c = cosf(rad);

    for(int i = 0; i < 16; i++) m[i] = 0.0f;
    // Column 0
    m[0] = c * scaleX;
    m[1] = s * scaleX;
    // Column 1
    m[4] = -s * scaleY;
    m[5] = c * scaleY;
    // Column 2 & 3
    m[10] = 1.0f;
    m[12] = x;    // Translation X (in pixels)
    m[13] = y;    // Translation Y (in pixels)
    m[14] = z;   // ADDED: Use the Z-coordinate here
    m[15] = 1.0f;
}

//----------------------------------------------------------------------------
//  some random number functions.
//----------------------------------------------------------------------------

//returns a random integer between x and y
inline int   RandInt(int x,int y)
{
    assert(y>=x && "<RandInt>: y is less than x");
    return rand()%(y-x+1)+x;
}

//returns a random double between zero and 1
inline double RandFloat()      {return ((rand())/(RAND_MAX+1.0));}

inline double RandInRange(double x, double y)
{
    return x + RandFloat()*(y-x);
}

inline F32 RandInRangeF(F32 x, F32 y)
{
    return static_cast<F32>(RandInRange(x,y));
}


//returns a random bool
inline bool   RandBool()
{
    if (RandFloat() > 0.5)
        return true;
    else
        return false;
}

//returns a random double in the range -1 < n < 1
inline double RandomClamped()    {return RandFloat() - RandFloat();}


//returns a random number with a normal distribution. See method at
//http://www.taygeta.com/random/gaussian.html
inline double RandGaussian(double mean = 0.0, double standard_deviation = 1.0)
{
    double x1, x2, w, y1;
    static double y2;
    static int use_last = 0;

    if (use_last)		        /* use value from previous call */
    {
        y1 = y2;
        use_last = 0;
    }
    else
    {
        do
        {
            x1 = 2.0 * RandFloat() - 1.0;
            x2 = 2.0 * RandFloat() - 1.0;
            w = x1 * x1 + x2 * x2;
        }
        while ( w >= 1.0 );

        w = sqrt( (-2.0 * log( w ) ) / w );
        y1 = x1 * w;
        y2 = x2 * w;
        use_last = 1;
    }

    return( mean + y1 * standard_deviation );
}


#endif // _FLUX_MATH_H_
