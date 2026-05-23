//-----------------------------------------------------------------------------
// Copyright (c) 2024 Thomas Hühn (XXTH) 
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#include "fluxTTFont.h"
#include "render/fluxRender2D.h"
#include <stdio.h>
#include "utils/errorlog.h"

// Need this because ImGui also implemen t TrueType:
namespace STB_Internal {
    #define STB_TRUETYPE_IMPLEMENTATION
    #include "stb_truetype.h"
}

// bool FluxTTFont::LoadFont(const char* filename, U32 fontSize, U32 textureSize)
// {
//     size_t size;
//     void* ttfData = SDL_LoadFile(filename, &size);
//
//     if (!ttfData) {
//         Log("Can not load font %s: %s", filename, SDL_GetError());
//         return false;
//     }
//     STB_Internal::stbtt_fontinfo info;
//     if (!STB_Internal::stbtt_InitFont(&info, (unsigned char*)ttfData, 0)) {
//         Log("Can not load font: Invalid Font Data!");
//         SDL_free(ttfData);
//         return false;
//     }
//
//
//     std::vector<unsigned char> alphaBitmap(512 * 512);
//     STB_Internal::stbtt_BakeFontBitmap((unsigned char*)ttfData, 0, (float)fontSize,
//                                        alphaBitmap.data(), 512, 512,
//                                        32, 95,
//                                        reinterpret_cast<STB_Internal::stbtt_bakedchar*>(mFont.chardata)
//     );
//
//     SDL_free(ttfData);
//
//     // Create Texture (Atlas)
//     mTexture = new FluxTexture();
//     mTexture->bindOpenGLAlphaDirect(alphaBitmap.data(), 512, 512);
//     return true;
// }
bool FluxTTFont::LoadFont(const char* filename, F32 fontSize, U32 textureSize )
{
    size_t size;
    void* ttfData = SDL_LoadFile(filename, &size);

    if (!ttfData) {
        Log("Can not load font %s: %s", filename, SDL_GetError());
        return false;
    }

    bool result = LoadFontFromMemory((const unsigned char*)ttfData, size, fontSize, textureSize);

    SDL_free(ttfData);
    return result;
}


bool FluxTTFont::LoadFontFromMemory(const unsigned char* fontData, unsigned int dataLen, F32 fontSize, U32 textureSize )
{
    if (!fontData || dataLen == 0) {
        Log("Can not load font: Invalid memory pointer or zero length");
        return false;
    }
    STB_Internal::stbtt_fontinfo info;
    if (!STB_Internal::stbtt_InitFont(&info, fontData, 0)) {
        Log("Can not load font: Invalid Font Data!");
        return false;
    }

    mFont.textureSize = textureSize;
    std::vector<unsigned char> alphaBitmap(textureSize  * textureSize );

    S32 result = STB_Internal::stbtt_BakeFontBitmap(fontData, 0, fontSize,
                                       alphaBitmap.data(), textureSize , textureSize ,
                                       32, 95,
                                       reinterpret_cast<STB_Internal::stbtt_bakedchar*>(mFont.chardata)
    );

    //     std::vector<unsigned char> alphaBitmap(512 * 512);
    //     STB_Internal::stbtt_BakeFontBitmap((unsigned char*)ttfData, 0, (float)fontSize,
    //                                        alphaBitmap.data(), 512, 512,
    //                                        32, 95,
    //                                        reinterpret_cast<STB_Internal::stbtt_bakedchar*>(mFont.chardata)
    //     );


    if (result <= 0) {
        Log("Warning: Font texture atlas (%d, %d) was too small for size %f!", textureSize, textureSize, fontSize);
        return false;
    }

    // Create Texture (Atlas)
    mTexture = new FluxTexture();
    mTexture->bindOpenGLAlphaDirect(alphaBitmap.data(), textureSize , textureSize );
    return true;
}



FluxTTFont::FluxTTFont(const char* filename, U32 fontSize)
{
    if (filename) LoadFont(filename, (F32)fontSize);

}

FluxTTFont::~FluxTTFont(void)
{
    SAFE_DELETE(mTexture);
}

