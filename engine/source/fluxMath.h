//-----------------------------------------------------------------------------
// Copyright (c) 2012/2025 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#pragma once
#ifndef _FLUX_MATH_H_
#define _FLUX_MATH_H_

#include <math.h>
#include <cassert>
#include <random>
#include <algorithm>

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
inline void createOrthoMatrix(float left, float right, float bottom, float top, float zNear, float zFar, float* m) {
    for(int i = 0; i < 16; i++) m[i] = 0.0f;
    m[0]  = 2.0f / (right - left);
    m[5]  = 2.0f / (top - bottom);
    m[10] = -2.0f / (zFar - zNear);
    m[12] = -(right + left) / (right - left);
    m[13] = -(top + bottom) / (top - bottom);
    m[14] = -(zFar + zNear) / (zFar - zNear);
    m[15] = 1.0f;
}

// inline void createOrthoMatrix(float left, float right, float bottom, float top, float near, float far, float* m) {
//     for(int i = 0; i < 16; i++) m[i] = 0.0f;
//     m[0]  = 2.0f / (right - left);
//     m[5]  = 2.0f / (top - bottom);
//     m[10] = -2.0f / (far - near);
//     m[12] = -(right + left) / (right - left);
//     m[13] = -(top + bottom) / (top - bottom);
//     m[14] = -(far + near) / (far - near);
//     m[15] = 1.0f;
// }

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

// Optimized Integer Range
inline int RandInt(int x, int y) {
    static thread_local std::mt19937 gen(std::random_device{}());
    static thread_local std::uniform_int_distribution<int> dist;
    return dist(gen, std::uniform_int_distribution<int>::param_type{x, y});
}

// Optimized Float Range
inline float RandInRange(float min, float max) {
    // Falls min > max, werden sie hier automatisch getauscht
    auto range = std::minmax(min, max);

    static thread_local std::mt19937 gen(std::random_device{}());
    static thread_local std::uniform_real_distribution<float> dist;

    return dist(gen, std::uniform_real_distribution<float>::param_type{range.first, range.second});
}

// Zero to One Float
inline float RandFloat() {
    static thread_local std::mt19937 gen(std::random_device{}());
    static thread_local std::uniform_real_distribution<float> dist(0.f, 1.f);
    return dist(gen);
}

//  RandBool: Uses 1 bit of entropy instead of a full float
inline bool RandBool() {
    static thread_local std::mt19937 gen(std::random_device{}());
    static thread_local std::bernoulli_distribution dist(0.5); // 50/50 chance
    return dist(gen);
}

// Faster & Correct RandomClamped: One call, perfectly uniform distribution
inline double RandomClamped() {
    static thread_local std::mt19937 gen(std::random_device{}());
    static thread_local std::uniform_real_distribution<double> dist(-1.0, 1.0);
    return dist(gen);
}

//returns a random number with a normal distribution.
inline double RandGaussian(double mean = 0.0, double std_dev = 1.0) {
    // Thread-local generator and distribution for maximum speed and safety
    static thread_local std::mt19937 gen(std::random_device{}());
    static thread_local std::normal_distribution<double> dist;

    // Use param_type to update mean/std_dev without re-initialization cost
    return dist(gen, std::normal_distribution<double>::param_type{mean, std_dev});
}

#endif // _FLUX_MATH_H_
