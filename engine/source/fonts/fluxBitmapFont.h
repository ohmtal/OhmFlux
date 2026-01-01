//-----------------------------------------------------------------------------
// Copyright (c) 2024 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#pragma once
#ifndef _FLUXBITMAPFONT_H_
#define _FLUXBITMAPFONT_H_

#include <SDL3/SDL.h>
#include <format>
#include <string_view>

#include "core/fluxGlobals.h"
#include "core/fluxRenderObject.h"

class FluxBitmapFont : public FluxRenderObject
{
    typedef FluxRenderObject Parent;
protected:
    S32 mStartChar =  32,
        mEndChar   = 127;
    char  mCaption[256]; // do not change directly !!
    S32 mCharWidth, mCharHeight;
    S32 mTextlen;
    Color4F mColor;
    FontAlign mAlign;
    bool mIsGuiElement;
private:
    using Parent::getDrawParams;
public:
    FluxBitmapFont(FluxTexture* lTex,
                    // FluxScreen* lScreen,
                    S32 lStartChar = 32,
                    S32 lEndChar=127)
    : FluxRenderObject(lTex/*, lScreen*/)
    , mStartChar(lStartChar)
    , mEndChar(lEndChar)
    , mCharWidth(16)
    , mCharHeight(32)
    , mColor({1.f,1.f,1.f,1.f})
    , mAlign(FontAlign_Left)
    , mIsGuiElement(true)
    { }

    // one liner
    FluxBitmapFont(FluxTexture* lTex,
                   const char * lCaption,
                   Point2I  lPos,
                   Point2I  lCharSize,
                   FontAlign lAlign = FontAlign_Left,
                   Color4F  lColor = cl_White
    )
    : FluxRenderObject(lTex)
    , mIsGuiElement(true)
    {
        setCaption("%s", lCaption);
        setPos(lPos.x,lPos.y);
        setCharSize(lCharSize.x, lCharSize.y);
        setAlign(lAlign);
        setColor(lColor);
    }


    // THIS MUST BE CALLED EVERY TIME THE CAPTION IS CHANGED !!!!
    void updateSize()
    {
        mTextlen = strlen(mCaption);

        getDrawParams().w = mTextlen * getCharWidth();
        getDrawParams().h = getCharHeight();
    }

    void setCaption(const char *szFormat, ...) PRINTF_CHECK(2, 3);

    // failsave but need a other format: font->setCaption("Score: {}", 100);
    template<typename... Args>
    void setCaptionFMT(std::string_view fmt, Args&&... args) {
        try {
            std::string s = std::vformat(fmt, std::make_format_args(args...));
            SDL_strlcpy(mCaption, s.c_str(), sizeof(mCaption));
        } catch (const std::format_error& e) {
            SDL_strlcpy(mCaption, "Format Error!", sizeof(mCaption));
            Log("Format error in setCaption: %s", e.what());
        }
        updateSize();
    }


    Point3F getPositonAndLayer() { return { getDrawParams().x, getDrawParams().y, getDrawParams().z };}

    void setAlign(FontAlign lAlign) { mAlign = lAlign; };
    FontAlign getAlign() { return mAlign; };

    void setCharSize(int w, int h)
    {
        mCharWidth  = w;
        mCharHeight = h;
        // here too !!
        updateSize();
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
        setCaption("%s", lCaption);
        setPos(x,y);
        setCharSize(charW, charH);
        setColor(lColor);
        setIsGuiElement(true);
    }

    void setIsGuiElement( bool value ) override { mIsGuiElement = value; }
    bool getIsGuiElement() override { return mIsGuiElement; }

    virtual void Draw() override;


    RectI getRectI() const override;

}; //class FluxBitmapFont


#endif //_FLUXBITMAPFONT_H_
