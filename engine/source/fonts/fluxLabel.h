//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
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

    bool mIsGuiElement = true;

    FluxTTFont* mTTFont = nullptr;
private:
    using Parent::getDrawParams;

public:
    FluxLabel(FluxTTFont* ttfont = nullptr);
    bool setFont(FluxTTFont* ttfont);
    bool isInitialized() { return mTTFont != nullptr; }
    ~FluxLabel();

    Color4F mColor = cl_White;
    FontAlign mAlign = FontAlign_Left;
    F32 mScale = 1.f;

    bool mShadow = false;
    Color4F mShadowColor = cl_Black;
    F32 mShadowOffset = 1.2f;

    const char* getCaption() { return mCaption; }

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

    void setScale( F32 value ) { if (value > 0.f) mScale = value; }
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
    Point2F getStringSize(const char* text);
    const Point2F Print(const char* text, Point2F pos, FontAlign align = FontAlign_Left
        , Color4F color = cl_White
        , bool shadow = false);

    RectI getRectI() const override;

}; //class FluxBitmapFont

