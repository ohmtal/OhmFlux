//-----------------------------------------------------------------------------
// Copyright (c) 2024 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#include "fluxBitmapFont.h"
#include "fluxRender2D.h"

#include <SDL3/SDL.h>
#include <stdio.h>
#include "errorlog.h"
//-----------------------------------------------------------------------------
void FluxBitmapFont::setCaption(const char *szFormat, ...)
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
void FluxBitmapFont::Draw()
{
    int x = getX();
    int y = getY();

    int lTextlen = strlen(mCaption);
    int lTextWidth =  lTextlen * mCharWidth;
    if ( mAlign == FontAlign_Center ) {
        x -= int(lTextWidth / 2);
    }
    else
    if ( mAlign == FontAlign_Right ) {
        x -= lTextWidth;
    }

    DrawParams2D dp; // Creates a new drawparams object
    dp.image = getTexture();
    dp.imgId = 0;
    dp.x = x;
    dp.y = y;
    dp.z = getLayer();
    dp.w = mCharWidth;
    dp.h = mCharHeight;
    dp.color = mColor;
    dp.isGuiElement = getIsGuiElement();

    for (size_t i = 0; i < lTextlen; i++)
    {
        dp.x = x;
        dp.imgId =  char(mCaption[i])-mStartChar;
        Render2D.drawSprite(dp);
        x += mCharWidth;
    }
}
