//-----------------------------------------------------------------------------
// Copyright (c) 2024 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#include "fluxTrueTypeFont.h"
#include "fluxRender2D.h"
#include <stdio.h>
#include "errorlog.h"
//-----------------------------------------------------------------------------
FluxTrueTypeFont::FluxTrueTypeFont(const char* filename, U32 fontSize)
: Parent(nullptr)
{
    mAlign = FontAlign_Left;
    mIsGuiElement = true;
    mColor = {1.0f, 1.0f, 1.0f, 1.0f};

    // 1. Load TTF file using SDL3 (Works on Android APK assets and Desktop)
    size_t size;
    void* ttfData = SDL_LoadFile(filename, &size);

    if (!ttfData) {
        Log("Can not load font %s: %s", filename, SDL_GetError());
        return;
    }

    // 2. Bake to Bitmap
    // Note: stbtt_BakeFontBitmap does NOT keep a pointer to ttfData,
    // it only reads it, so we can free ttfData safely after this step.
    std::vector<unsigned char> alphaBitmap(512 * 512);
    stbtt_BakeFontBitmap((unsigned char*)ttfData, 0, (float)fontSize,
                         alphaBitmap.data(), 512, 512,
                         32, 95, mFont.chardata);

    // 3. Free the file buffer immediately after baking
    SDL_free(ttfData);

    // 4. Create Texture (Atlas)
    FluxTexture* lTexture = new FluxTexture();
    lTexture->bindOpenGLAlphaDirect(alphaBitmap.data(), 512, 512);
    setTexture(lTexture);
}


//-----------------------------------------------------------------------------
void FluxTrueTypeFont::setCaption(const char *szFormat, ...)
{
    if (!szFormat) return;
    va_list Arg;
    va_start(Arg, szFormat);
    // Use sizeof(mCaption) to stay within bounds
    vsnprintf(mCaption, sizeof(mCaption), szFormat, Arg);
    mCaption[sizeof(mCaption) - 1] = '\0'; // Manual safety terminator
    va_end(Arg);
}

//-----------------------------------------------------------------------------
void FluxTrueTypeFont::Draw()
{
    if (mCaption[0] == '\0') return;

    // Use float for internal calculations to maintain precision
    float startX = (float)getX();
    float startY = (float)getY();
    float currentX = startX;
    float currentY = startY;

    // 1. PRE-PASS: Calculate scaled total width for alignment
    // eats fps because calculated ecery time
    // but need it for object width
    /*if (mAlign != FontAlign_Left)*/ {
        float tempX = 0, tempY = 0;
        for (int i = 0; mCaption[i]; ++i) {
            unsigned char c = (unsigned char)mCaption[i];
            if (c >= 32 && c < 127)
            {
                stbtt_aligned_quad q;
                stbtt_GetBakedQuad(mFont.chardata, 512, 512, mCaption[i] - 32, &tempX, &tempY, &q, 1);
            }
        }
        float totalWidth = tempX * mScale; // Scale the total accumulated width
        if (mAlign == FontAlign_Center) currentX -= (totalWidth * 0.5f);
        else if (mAlign == FontAlign_Right) currentX -= totalWidth;

        // hack in Object width
        getDrawParams().w = totalWidth; //updateSize
    }

    S32 lMaxHeight = 0;
    // 2. DRAW-PASS
    for (int i = 0; mCaption[i]; ++i)
    {
        unsigned char c = (unsigned char)mCaption[i];
        if (c >= 32 && c < 127)
        {
            stbtt_aligned_quad q;
            float oldX = currentX;

            // Get original quad relative to a 0,0 baseline
            // Note: We pass temporary 0,0 to get local offsets, then apply scale and startX
            float nextX = 0, nextY = 0;
            stbtt_GetBakedQuad(mFont.chardata, 512, 512, mCaption[i] - 32, &nextX, &nextY, &q, 1);

            DrawParams2D dp;
            dp.useUV = true;
            dp.image = getTexture();
            dp.z = getLayer();
            dp.color = mColor;
            dp.isGuiElement = mIsGuiElement;

            // Apply Scale to Dimensions
            dp.w = (S32)((q.x1 - q.x0) * mScale);
            dp.h = (S32)((q.y1 - q.y0) * mScale);

            if ( dp.h > lMaxHeight )
                lMaxHeight = dp.h;

            // Apply Scale to Position
            // q.x0/y0 are offsets from the baseline. We scale the offset and add to current position.
            dp.x = currentX + (q.x0 * mScale) + (dp.w * 0.5f);
            dp.y = currentY + (q.y0 * mScale) + (dp.h * 0.5f);

            // UVs remain the same regardless of scale
            dp.u0 = q.s0; dp.v0 = q.t0;
            dp.u1 = q.s1; dp.v1 = q.t1;

            Render2D.drawSprite(dp);

            // Advance the cursor by the scaled character width
            currentX += (nextX * mScale);
        }
    }
    getDrawParams().h = lMaxHeight; //updateSize
}

//-----------------------------------------------------------------------------
RectI FluxTrueTypeFont::getRectI() const
{
    RectI lResult = getDrawParams().getRectI();
    S32 halfWidth = static_cast<S32>(static_cast<F32>(getDrawParams().w) / 2.f);

    if ( mAlign == FontAlign_Left ) {
        lResult.x += halfWidth;
    }
    else if ( mAlign == FontAlign_Right )
    {
        lResult.x -= halfWidth;
    }

    return lResult;
}
