//-----------------------------------------------------------------------------
// Copyright (c) 2024 Thomas Hühn (XXTH) 
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#pragma once

#include <SDL3/SDL.h>
#include <format>
#include <string_view>


#include "core/fluxRenderObject.h"
#include "core/fluxGlobals.h"

class FluxTTFont; //fwd
class FluxLabel : public FluxRenderObject
{
    typedef FluxRenderObject Parent;
protected:
    char  mCaption[256];
    Color4F mColor;
    FontAlign mAlign;
    bool mIsGuiElement;
    F32 mScale = 1.f;
    FluxTTFont* mTTFont;
private:
    using Parent::getDrawParams;

public:
    FluxLabel(FluxTTFont* ttfont);
    ~FluxLabel();


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

