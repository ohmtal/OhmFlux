//-----------------------------------------------------------------------------
// Copyright (c) 2012 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Flux Game Engine
//
// Author: T.Huehn (XXTH)
// Desc  : Globals
//-----------------------------------------------------------------------------
// stuct hint because i always forget this
// Example this is known           : Color4F foo = { 1.f, 1.f, 1.f, 1.f };
//         this i always forget ;) : Color4F bar = { .a = 0.7f }
//-----------------------------------------------------------------------------
#pragma once
#ifndef _FLUXGLOBALS_H_
#define _FLUXGLOBALS_H_

#include <cmath>
#include <string>

//--------------------------- GetScreenObject by global Instance ------------
class FluxScreen; // Forward declaration: No #include needed yet!
FluxScreen* getScreenObject();

class FluxQuadtree;
FluxQuadtree* getQuadTreeObject();
static constexpr auto getContainer = getQuadTreeObject;



//--------- failsave check for sprintf style functions  like Log ------------
#ifndef PRINTF_CHECK
#  if defined(__GNUC__) || defined(__clang__)
// format(printf, format_string_index, first_arg_to_check_index)
// 'this' pointer in C++ counts as index 1, so indices are usually 2 and 3
#    define PRINTF_CHECK(fmt, args) __attribute__((format(printf, fmt, args)))
#  else
#    define PRINTF_CHECK(fmt, args)
#  endif
#endif

//------------------------------------------------------------------------------

enum FontAlign {
    FontAlign_Left,
    FontAlign_Center,
    FontAlign_Right
};


// For standard objects
template<typename T>
inline void SAFE_DELETE(T*& ptr) {
    if (ptr != nullptr) {
        delete ptr;
        ptr = nullptr;
    }
}


// For arrays (delete [])
template<typename T>
inline void SAFE_DELETE_ARRAY(T*& ptr) {
    if (ptr != nullptr) {
        delete[] ptr;
        ptr = nullptr;
    }
}

// For C-style allocations (malloc/free)
template<typename T>
inline void SAFE_FREE(T*& ptr) {
    if (ptr != nullptr) {
        free(ptr); // Or your custom dFree(ptr)
        ptr = nullptr;
    }
}



#define BIT(x) (1 << (x))           ///< Returns value with bit x set (2^x)
#ifndef FLUX_PI
#define FLUX_PI 3.14159265358979323846f
#endif

#if defined(_WIN32) || defined(__CYGWIN__)
#define FN_CDECL __cdecl
#else
#define FN_CDECL
#endif
//-------------------------------------- Basic Types...

typedef signed char        S8;      ///< Compiler independent Signed Char
typedef unsigned char      U8;      ///< Compiler independent Unsigned Char

typedef signed short       S16;     ///< Compiler independent Signed 16-bit short
typedef unsigned short     U16;     ///< Compiler independent Unsigned 16-bit short

typedef signed int         S32;     ///< Compiler independent Signed 32-bit integer
typedef unsigned int       U32;     ///< Compiler independent Unsigned 32-bit integer
typedef unsigned long long  U64;    ///< Compiler independent Unsigned 64-bit integer

typedef float              F32;     ///< Compiler independent 32-bit float
typedef double             F64;     ///< Compiler independent 64-bit float


//------------------------------------------------------------------------------
//------------------------------------- String Types

typedef char           UTF8;        ///< Compiler independent 8  bit Unicode encoded character
typedef unsigned short UTF16;       ///< Compiler independent 16 bit Unicode encoded character
typedef unsigned int   UTF32;       ///< Compiler independent 32 bit Unicode encoded character


//------------------------------------------------------------------------------
//------------------------------------- EnhancedTypes
// replaced by Vertex2D
// struct Vertex {
//     F32 position[3]; // X, Y, Z
//     F32 texCoord[2]; // U, V
// };
//-----

struct Color4F {
    F32 r = 1.f;
    F32 g = 1.f;
    F32 b = 1.f;
    F32 a = 1.f;


    // --- Arithmetic Operators ---

    // Add a scalar to all color channels
    Color4F operator+(F32 v) const { return { r + v, g + v, b + v, a + v }; }

    // Subtract a scalar from all color channels
    Color4F operator-(F32 v) const { return { r - v, g - v, b - v, a - v }; }

    // Multiply all color channels by a scalar (common for alpha fading)
    Color4F operator*(F32 v) const { return { r * v, g * v, b * v, a * v }; }

    // Divide all color channels by a scalar
    Color4F operator/(F32 v) const { return { r / v, g / v, b / v, a / v }; }

    // --- Compound Assignment Operators ---

    Color4F& operator+=(F32 v) { r += v; g += v; b += v; a += v; return *this; }
    Color4F& operator-=(F32 v) { r -= v; g -= v; b -= v; a -= v; return *this; }
    Color4F& operator*=(F32 v) { r *= v; g *= v; b *= v; a *= v; return *this; }
    Color4F& operator/=(F32 v) { r /= v; g /= v; b /= v; a /= v; return *this; }

    // --- Color-to-Color Operators (Optional but recommended for tinting) ---

    Color4F operator*(const Color4F& v) const { return { r * v.r, g * v.g, b * v.b, a * v.a }; }
    Color4F& operator*=(const Color4F& v) { r *= v.r; g *= v.g; b *= v.b; a *= v.a; return *this; }

    // Binary addition (Color + Color)
    Color4F operator+(const Color4F& v) const { return { r + v.r, g + v.g, b + v.b, a + v.a }; }


    bool operator==(const Color4F& other) const {
        return r == other.r && g == other.g && b == other.b && a == other.a;
    }

    // Helper to create a color from a Hex integer (e.g., 0xFF5733)
    static Color4F FromHex(U32 hex) {
        Color4F c;
        // If the hex is > 0xFFFFFF, we assume it has Alpha (0xRRGGBBAA)
        if (hex > 0xFFFFFF) {
            c.r = ((hex >> 24) & 0xFF) / 255.0f;
            c.g = ((hex >> 16) & 0xFF) / 255.0f;
            c.b = ((hex >> 8)  & 0xFF) / 255.0f;
            c.a = (hex & 0xFF) / 255.0f;
        } else {
            // Otherwise assume standard RGB (0xRRGGBB)
            c.r = ((hex >> 16) & 0xFF) / 255.0f;
            c.g = ((hex >> 8)  & 0xFF) / 255.0f;
            c.b = (hex & 0xFF) / 255.0f;
            c.a = 1.0f;
        }
        return c;
    }

    // Converts the current float color back to a 32-bit Hex integer (0xRRGGBBAA)
    U32 toHex() const {
        U32 rr = static_cast<U32>(r * 255.0f + 0.5f);
        U32 gg = static_cast<U32>(g * 255.0f + 0.5f);
        U32 bb = static_cast<U32>(b * 255.0f + 0.5f);
        U32 aa = static_cast<U32>(a * 255.0f + 0.5f);

        // Bit-shift into the 0xRRGGBBAA pattern
        return (rr << 24) | (gg << 16) | (bb << 8) | aa;
    }

};

//------------------------------------- Default Colors
const Color4F cl_White        = { 1.0f, 1.0f, 1.0f, 1.0f };
const Color4F cl_Black        = { 0.0f, 0.0f, 0.0f, 1.0f };
const Color4F cl_Red          = { 1.0f, 0.0f, 0.0f, 1.0f };
const Color4F cl_Green        = { 0.0f, 1.0f, 0.0f, 1.0f };
const Color4F cl_Blue         = { 0.0f, 0.0f, 1.0f, 1.0f };
const Color4F cl_Yellow       = { 1.0f, 1.0f, 0.0f, 1.0f };
const Color4F cl_Cyan         = { 0.0f, 1.0f, 1.0f, 1.0f };
const Color4F cl_Magenta      = { 1.0f, 0.0f, 1.0f, 1.0f };
const Color4F cl_Gray         = { 0.5f, 0.5f, 0.5f, 1.0f };
const Color4F cl_LightGray    = { 0.75f, 0.75f, 0.75f, 1.0f };
const Color4F cl_DarkGray     = { 0.25f, 0.25f, 0.25f, 1.0f };
const Color4F cl_Orange       = { 1.0f, 0.5f, 0.0f, 1.0f };
const Color4F cl_Purple       = { 0.5f, 0.0f, 0.5f, 1.0f };
const Color4F cl_Brown        = { 0.6f, 0.3f, 0.0f, 1.0f };
const Color4F cl_Lime         = { 0.75f, 1.0f, 0.0f, 1.0f };
const Color4F cl_Pink         = { 1.0f, 0.4f, 0.7f, 1.0f };

//------------------------------------- Some more Colors
const Color4F cl_Crimson      = { 0.86f, 0.08f, 0.24f, 1.0f }; // Modern Alert/Red
const Color4F cl_Emerald      = { 0.16f, 0.71f, 0.44f, 1.0f }; // Pleasant Success/Green
const Color4F cl_SkyBlue      = { 0.53f, 0.81f, 0.98f, 1.0f }; // UI Accent
const Color4F cl_Slate        = { 0.18f, 0.24f, 0.31f, 1.0f }; // Modern Dark Background
const Color4F cl_Gold         = { 1.00f, 0.84f, 0.00f, 1.0f }; // Pickups/Rare items
const Color4F cl_Transparent   = { 0.00f, 0.00f, 0.00f, 0.00f }; // Invisible/Default

const Color4F cl_Aquamarine   = { 0.50f, 1.00f, 0.83f, 1.0f }; // Tropical Water
const Color4F cl_Coral        = { 1.00f, 0.50f, 0.31f, 1.0f }; // Warm Reef color
const Color4F cl_DeepSea      = { 0.00f, 0.08f, 0.20f, 1.0f }; // Background Depths
const Color4F cl_Seafoam      = { 0.60f, 0.85f, 0.75f, 1.0f }; // Bubbles/Foam
const Color4F cl_Sand         = { 0.76f, 0.70f, 0.50f, 1.0f }; // Floor/Dirt
const Color4F cl_Kelp         = { 0.13f, 0.29f, 0.13f, 1.0f }; // Dark Underwater Plant

const Color4F cl_NeonPink     = { 1.00f, 0.00f, 0.50f, 1.0f }; // Physics Hitboxes
const Color4F cl_ElectricBlue = { 0.00f, 1.00f, 1.00f, 1.0f }; // Pathfinding nodes
const Color4F cl_AcidGreen    = { 0.50f, 1.00f, 0.00f, 1.0f }; // Collision triggers

const Color4F cl_Glass        = { 1.00f, 1.00f, 1.00f, 0.25f }; // Semi-transparent White
const Color4F cl_Shadow       = { 0.00f, 0.00f, 0.00f, 0.40f }; // Shadow Overlay
const Color4F cl_Ghost        = { 0.70f, 0.70f, 1.00f, 0.50f }; // Semi-transparent Blue




//------------------------------------- Points
struct  Point2I{
    S32 x, y;
};

//-----------------------------------------------------------------------------
// Copyright (c) 2025 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#include <cmath> // For std::sqrt

struct Point2F {
    F32 x, y;

    // --- Scalar Operators ---
    Point2F operator-(F32 v) const { return {x - v, y - v}; }
    Point2F operator+(F32 v) const { return {x + v, y + v}; }
    Point2F operator*(F32 v) const { return {x * v, y * v}; }
    Point2F operator/(F32 v) const { return {x / v, y / v}; }

    // --- Point Operators ---
    Point2F operator-(const Point2F& v) const { return {x - v.x, y - v.y}; }
    Point2F operator+(const Point2F& v) const { return {x + v.x, y + v.y}; }
    Point2F operator*(const Point2F& v) const { return {x * v.x, y * v.y}; }
    Point2F operator/(const Point2F& v) const { return {x / v.x, y / v.y}; }

    // --- Compound Assignment Operators ---
    Point2F& operator+=(F32 v) { x += v; y += v; return *this; }
    Point2F& operator-=(F32 v) { x -= v; y -= v; return *this; }
    Point2F& operator*=(F32 v) { x *= v; y *= v; return *this; }
    Point2F& operator/=(F32 v) { x /= v; y /= v; return *this; }

    Point2F& operator+=(const Point2F& v) { x += v.x; y += v.y; return *this; }
    Point2F& operator-=(const Point2F& v) { x -= v.x; y -= v.y; return *this; }
    Point2F& operator*=(const Point2F& v) { x *= v.x; y *= v.y; return *this; }
    Point2F& operator/=(const Point2F& v) { x /= v.x; y /= v.y; return *this; }

    // --- Conversion ---
    explicit operator Point2I() const {
        return { static_cast<S32>(x), static_cast<S32>(y) };
    }

    // --- Distance and Length ---
    F32 distSq(const Point2F& v) const {
        F32 dx = x - v.x;
        F32 dy = y - v.y;
        return (dx * dx) + (dy * dy);
    }

    F32 dist(const Point2F& v) const { return std::sqrt(distSq(v)); }
    F32 distance(const Point2F& v) const { return dist(v); }

    F32 lenSquared() const { return (x * x + y * y); }

    F32 len() const { return std::sqrt(lenSquared()); }
    F32 isZero() const { return (x == 0.f && y == 0.f); }

    // --- Vector Operations ---
    void normalize() {
        F32 l = len();
        if (l > 0.0f) { x /= l; y /= l; }
    }

    Point2F normalized() const {
        F32 l = len();
        if (l > 0.0f) return { x / l, y / l };
        return { 0.0f, 0.0f };
    }

    F32 dot(const Point2F& v) const { return (x * v.x + y * v.y); }

    // 2D "Cross Product" (Scalar Z-component)
    F32 cross(const Point2F& v) const { return (x * v.y - y * v.x); }

    Point2I toPoint2I() const { return { static_cast<S32>(x), static_cast<S32>(y)  };}

}; //Point2F


struct Point3F {
    F32 x, y, z;

    F32 distSq(const Point3F& other) const {
        F32 dx = x - other.x;
        F32 dy = y - other.y;
        F32 dz = z - other.z;
        return dx*dx + dy*dy + dz*dz;
    }

    F32 lenSquared() const {
        return (x * x + y * y + z * z);
    }
    F32 len() const {
        return sqrt(x*x + y*y + z*z);
    }

    Point3F operator+(const Point3F& v) const { return {x + v.x, y + v.y, z + v.z}; }
    Point3F operator-(const Point3F& v) const { return {x - v.x, y - v.y, z - v.z}; }

    Point3F& operator+=(const Point2F& v) { x += v.x; y += v.y; return *this; }

};
//------------------------------------- Rects

struct RectI{
    S32 x, y, w, h;
    Point2I getPoint() const { return {x,y}; }
    Point2I getExtent() const { return { w, h };}

    bool pointInRect(const Point2I& pt) const {
        return (pt.x >= x && pt.x < x + w && pt.y >= y && pt.y < y + h);
    }
    S32 len_x() const { return w; }
    S32 len_y() const { return h; }
    bool isValidRect() const { return (w > 0 && h > 0); }

    // Checks if THIS rect fully contains OTHER rect
    // Used by update() to see if an object can stay in its current node
    bool contains(const RectI& other) const {
        return (other.x >= x &&
        other.y >= y &&
        other.x + other.w <= x + w &&
        other.y + other.h <= y + h);
    }

    // Checks if THIS rect overlaps at all with OTHER rect
    // Used by retrieve() to find candidates for mouse clicks
    bool intersects(const RectI& other) const {
        return (x < other.x + other.w &&
        x + w > other.x &&
        y < other.y + other.h &&
        y + h > other.y);
    }

};

struct RectF
{
    F32 x, y, w, h;

    Point2F getPoint() const { return {x,y}; }
    Point2F getExtent() const { return { w, h };}
    bool pointInRect(const Point2F& pt) const {
        return (pt.x >= x && pt.x < x + w && pt.y >= y && pt.y < y + h);
    }
    F32 len_x() const { return w; }
    F32 len_y() const { return h; }
    bool isValidRect() const { return (w > 0 && h > 0); }
    RectI asRectI() const { return { static_cast<S32>(x), static_cast<S32>(y),static_cast<S32>(w), static_cast<S32>(h)};}

    void inflate(F32 amountX, F32 amountY) {
        x -= amountX;      // Move left
        y -= amountY;      // Move up
        w += amountX * 2;  // Expand width both ways
        h += amountY * 2;  // Expand height both ways
    }
};

#define MAX_LIGHTS 16 // Maximum number of 2D lights supported by the shader

//------------------------------------- Vertex2D for batch rendering
struct Vertex2D {
    Point3F pos;      // x, y, z
    Point2F uv;       // Final calculated UV (after offset/size/flip)
    Color4F color;    // uTint and uAlphaThreshold combined
    // Optional: float textureIndex; (If using Multi-texture batching)
};


//-------------------------------------- game parameter structs

// Data Types
struct FluxAppStatus
{
    bool Visible;
    bool Paused;
    bool MouseFocus;
    bool KeyboardFocus;

    // this is setup in FluxMain:
    Point2F MousePos;       // MousePositon relative to screenSize
    Point2F RealMousePos;   // MousePositon reported by SDL
    Point2F WorldMousePos;  // MousePositon based on MousePos and relative to Camera Postion and Zoom

    Point2F getMousePos() const { return MousePos; }
    Point2F getRealMousePos() const { return RealMousePos; }
    Point2F getWorldMousePos() const { return WorldMousePos; }

    // as integer:
    Point2I getMousePosI() const  { return static_cast<Point2I>(MousePos); }
    Point2I getRealMousePosI() const  { return static_cast<Point2I>(RealMousePos); }
    Point2I getWorldMousePosI() const  { return static_cast<Point2I>(WorldMousePos); }

};

const U32 DEFAULT_MAX_SPRITES = 4000;

struct FluxSettings
{
    U32 ScreenWidth  = 800;
    U32	ScreenHeight = 600;
    bool FullScreen  = false;
    bool initialVsync = true; //only used when screen starts
    bool ScaleScreen = true;
    double updateDt        = 16.666f; //fixed update Dt
    double frameLimiter = 0.f; //sleep milliseconds

    Point2I minWindowSize = { 320, 200 };
    const char* Caption;
    const char* Version;
    const char* IconFilename;
    const char* CursorFilename;
    S32 cursorHotSpotX = 0;
    S32 cursorHotSpotY = 0;
    bool useQuadTree    = false;
    RectI WorldBounds = { -2000, -2000, 4000, 4000 };

    U32 maxSprites = DEFAULT_MAX_SPRITES; // this need to be set since we use Batchrendering

    bool enableLogFile = true;

} ;

//-----------------------------------------------------------------------------
extern float gFrameTime;
extern float gGameTime;

inline float getFrameTime() {
    return gFrameTime;
}
inline float getGameTime() {
    return gGameTime;
}
//-----------------------------------------------------------------------------

inline std::string sanitizeFilenameWithUnderScores(std::string name)
{
    std::string result;
    for (unsigned char c : name) {
        if (std::isalnum(c)) {
            result += c;
        } else if (std::isspace(c)) {
            result += '_';
        }
        // Special characters (like '.') are ignored/dropped here
    }
    return result;
}




#endif //_FLUXGLOBALS_H_
