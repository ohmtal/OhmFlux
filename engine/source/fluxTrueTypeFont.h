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

#include <SDL3/SDL.h>
#include <format>
#include <string_view>


#include "fluxRenderObject.h"
#include "stb_truetype.h"
#include "fluxGlobals.h"


struct FontData {
    stbtt_bakedchar chardata[96]; // ASCII 32..126
    // GLuint textureID;
};


class FluxTrueTypeFont : public FluxRenderObject
{
    typedef FluxRenderObject Parent;
protected:
    char  mCaption[256];
    FontData mFont;
    Color4F mColor;
    FontAlign mAlign;
    bool mIsGuiElement;
    F32 mScale = 1.f;
private:
    using Parent::getDrawParams;

public:
    FluxTrueTypeFont(const char* filename, U32 fontSize = 32);


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
    }

    Point3F getPositonAndLayer() { return { getDrawParams().x, getDrawParams().y, getDrawParams().z };}

    void setAlign(FontAlign lAlign) { mAlign = lAlign; };
    FontAlign getAlign() { return mAlign; };

    void setColor( Color4F lColor) { mColor = lColor; }

    void setIsGuiElement( bool value ) override { mIsGuiElement = value; }
    bool getIsGuiElement() override { return mIsGuiElement; }

    void setScale( F32 value ) { mScale = value; }
    F32 getScale () { return mScale; }

    void set(
        const char* lCaption,
        Point2F lPos,
        Color4F lColor = cl_White,
        F32     lScale = 1.f
    )
    {
        setCaption("%s", lCaption);
        setPos(lPos);
        setColor(lColor);
        setScale(lScale);
    }


    virtual void Draw() override;

    RectI getRectI() const override;

}; //class FluxBitmapFont

