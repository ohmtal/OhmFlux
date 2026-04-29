//-----------------------------------------------------------------------------
// Copyright (c) 2024 Thomas Hühn (XXTH) 
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// FIXME garbage collection like in FluxTexture >>
// - maybe a new baseClass fluxResource
//   this is added to garbagecollection and can be used for sounds too!
//-----------------------------------------------------------------------------
#pragma once

#include <SDL3/SDL.h>
#include <format>
#include <string_view>


#include "core/fluxRenderObject.h"
#include "core/fluxGlobals.h"

struct FluxBakedChar {
    unsigned short x0, y0, x1, y1;
    float xoff, yoff, xadvance;
};

struct FontData {
    FluxBakedChar chardata[96]; // ASCII 32..126
    // GLuint textureID;
};


class FluxTTFont
{
protected:
    FontData mFont;
    FluxTexture* mTexture;

public:

    const FontData* getFont() { return &mFont; }
    FluxTexture* getTexture() { return mTexture; }

    FluxTTFont(const char* filename, U32 fontSize = 32);
    ~FluxTTFont();


}; //class FluxBitmapFont

