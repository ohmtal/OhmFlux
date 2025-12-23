//-----------------------------------------------------------------------------
// Copyright (c) 2024 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Flux Game Engine
//
// @Author  T.Huehn (XXTH)
// @Desc    simple bitmap font class
// @created 2024-12-05
//-----------------------------------------------------------------------------
#pragma once
#ifndef _FLUXBITMAPFONT_H_
#define _FLUXBITMAPFONT_H_

#include "fluxRenderObject.h"

class FluxBitmapFont : public FluxRenderObject
{
    typedef FluxRenderObject Parent;
protected:
    int mStartChar, mEndChar;
    char  mCaption[256];
    int mCharWidth, mCharHeight;
    Color4F mColor;
    FontAlign mAlign;
    bool mIsGuiElement;
private:
    using Parent::getDrawParams;
public:
    FluxBitmapFont(FluxTexture* lTex,
                    FluxScreen* lScreen,
                    int lStartChar = 32,
                    int lEndChar=127)
    : FluxRenderObject(lTex, lScreen)
    , mStartChar(lStartChar)
    , mEndChar(lEndChar)
    , mCharWidth(16)
    , mCharHeight(32)
    , mColor({1.f,1.f,1.f,1.f})
    , mAlign(FontAlign_Left)
    , mIsGuiElement(true)
    { }

    void setCaption(const char *szFormat, ...);

    Point3F getPositonAndLayer() { return { getDrawParams().x, getDrawParams().y, getDrawParams().z };}

    void setAlign(FontAlign lAlign) { mAlign = lAlign; };
    FontAlign getAlign() { return mAlign; };

    void setCharSize(int w, int h)
    {
        mCharWidth  = w;
        mCharHeight = h;
    }
    int getCharHeight() { return mCharHeight; }
    int getCharWidth() { return mCharWidth;}


    void setColor( Color4F lColor)
    {
        mColor = lColor;
    }

    void set(
        const char* lCaption,
        int x, int y,
        int charW, int charH,
        Color4F lColor = {1.f,1.f,1.f,1.f}
    )
    {
        setCaption(lCaption);
        setPos(x,y);
        setCharSize(charW, charH);
        setColor(lColor);
        setIsGuiElement(true);
    }

    void setIsGuiElement( bool value ) override { mIsGuiElement = value; }
    bool getIsGuiElement() override { return mIsGuiElement; }

    virtual void Draw() override;


}; //class FluxBitmapFont


#endif //_FLUXBITMAPFONT_H_
