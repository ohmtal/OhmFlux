//-----------------------------------------------------------------------------
// Copyright (c) 2024 Thomas Hühn (XXTH) 
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#include "fluxTTFont.h"
#include "render/fluxRender2D.h"
#include <stdio.h>
#include "utils/errorlog.h"


namespace STB_Internal {
    #define STB_TRUETYPE_IMPLEMENTATION
    #include "stb_truetype.h"
}




FluxTTFont::FluxTTFont(const char* filename, U32 fontSize)
{

    size_t size;
    void* ttfData = SDL_LoadFile(filename, &size);

    if (!ttfData) {
        Log("Can not load font %s: %s", filename, SDL_GetError());
        return;
    }

    std::vector<unsigned char> alphaBitmap(512 * 512);
    STB_Internal::stbtt_BakeFontBitmap((unsigned char*)ttfData, 0, (float)fontSize,
                         alphaBitmap.data(), 512, 512,
                         32, 95,
                          reinterpret_cast<STB_Internal::stbtt_bakedchar*>(mFont.chardata)
    );

    SDL_free(ttfData);

    // Create Texture (Atlas)
    mTexture = new FluxTexture();
    mTexture->bindOpenGLAlphaDirect(alphaBitmap.data(), 512, 512);

}

FluxTTFont::~FluxTTFont(void)
{
    SAFE_DELETE(mTexture);
}

