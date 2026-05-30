//-----------------------------------------------------------------------------
// Copyright (c) 2024 Thomas Hühn (XXTH) 
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// TODO: only support ASCII !!
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
    U32 textureSize;
};


class FluxTTFont
{
protected:
    FontData mFont;
    FluxTexture* mTexture = nullptr;

public:

    const FontData* getFont() { return &mFont; }
    FluxTexture* getTexture() { return mTexture; }

    FluxTTFont(const char* filename = nullptr, U32 fontSize = 32);
    ~FluxTTFont();


    bool LoadFont(const char* filename, F32 fontSize = 32.0f, U32 textureSize = 512);
    bool LoadFontFromMemory(const unsigned char* fontData, unsigned int dataLen, F32 fontSize, U32 textureSize = 512);

};
