//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#include "fluxLabel.h"
#include "render/fluxRender2D.h"
#include <stdio.h>
#include "utils/errorlog.h"
#include "fluxTTFont.h"


namespace STB_Internal {
    // #define STB_TRUETYPE_IMPLEMENTATION
    #include "stb_truetype.h"
}


//-----------------------------------------------------------------------------
// FIXME need a resource manager to prevent multiple copies of ttf font in
//       memory. For loaded Textures FluxMain loadTextures does this but
//       here we push the data directly to FluxTexture
//
//-----------------------------------------------------------------------------


FluxLabel::FluxLabel(FluxTTFont* ttfont)
: Parent(nullptr)
{
    if (!ttfont || !ttfont->getTexture() || !ttfont->getFont()) {
        Log("[error] FluxLabel invalid TrueType Font Object!");
        return;
    }
    mTTFont = ttfont;

    mAlign = FontAlign_Left;
    mIsGuiElement = true;
    mColor = {1.0f, 1.0f, 1.0f, 1.0f};


    setTexture(mTTFont->getTexture());
}

FluxLabel::~FluxLabel(void)
{
}

//-----------------------------------------------------------------------------
void FluxLabel::setCaption(const char *szFormat, ...)
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
//FIXME align!
const Point2F FluxLabel::Print(const char* text, Point2F pos, FontAlign align )
{
    Point2F result = Point2F(0.f,0.f);
    if (text[0] == '\0') return result;
    if (!mTTFont || !mTTFont->getFont()) return result;
    FontData fontData = *mTTFont->getFont();


    float startX = pos.x;
    float startY = pos.y;
    float currentX = startX;
    float currentY = startY;

    S32 lMaxHeight = 0;
    for (int i = 0; text[i]; ++i)
    {
        unsigned char c = (unsigned char)text[i];
        if (c >= 32 && c < 127)
        {
            STB_Internal::stbtt_aligned_quad q;
            float nextX = 0, nextY = 0;
            STB_Internal::stbtt_GetBakedQuad(
                reinterpret_cast<STB_Internal::stbtt_bakedchar*>(fontData.chardata),
                                             fontData.textureSize, fontData.textureSize, text[i] - 32, &nextX, &nextY, &q, 1);

            DrawParams2D dp;
            dp.useUV = true;
            dp.image = getTexture();
            dp.z = getLayer();
            dp.color = mColor;
            dp.isGuiElement = mIsGuiElement;

            // Apply Scale to Dimensions
            dp.w = ((q.x1 - q.x0) * mScale);
            dp.h = ((q.y1 - q.y0) * mScale);

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

            currentX += (nextX * mScale);
        }
    }
    //updateSize
    result.x = currentX;
    result.y = lMaxHeight;
    return result;
}

void FluxLabel::Draw()
{
    if (mCaption[0] == '\0') return;
    if (!mTTFont || !mTTFont->getFont()) return;

    Point2F size = Print(mCaption, getPosition(), mAlign);

    getDrawParams().w = size.x;
    getDrawParams().h = size.y;
}

//-----------------------------------------------------------------------------
RectI FluxLabel::getRectI() const
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

