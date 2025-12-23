//-----------------------------------------------------------------------------
// Copyright (c) 2012..2024 Ohmtal Game Studio
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//-----------------------------------------------------------------------------
#ifndef _GUIFISHLABEL_H_
#define _GUIFISHLABEL_H_
#include "source/fluxMain.h"
#include "globals.h"

class GuiFishLabel : public FluxBitmapFont
{
	typedef FluxBitmapFont Parent;
private:
	FluxTexture* mFishTexture;

public:

	GuiFishLabel(FluxTexture* lFishTex, FluxTexture* lFontTex, FluxScreen* lScreen)
		:FluxBitmapFont(lFontTex, lScreen)
	{
        mFishTexture = lFishTex;
	}


	virtual void Draw() override
	{
		Parent::Draw();

		DrawParams2D  lFishDrawParams;
		lFishDrawParams.image = mFishTexture;
		Point3F lPosAndLayer = getPositonAndLayer();
		lPosAndLayer.z -= 0.01f; //put fished above font
		lFishDrawParams.setPositionAndLayer(lPosAndLayer);
		lFishDrawParams.w = getCharHeight();
		lFishDrawParams.h = getCharHeight();
		lFishDrawParams.isGuiElement = true;

		Render2D.drawSprite(lFishDrawParams);


		//FIXME uglyDraw2DStretch
		// Batch rendering layer must be lower than the text!!!
        // Render2D.uglyDraw2DStretch(mFishTexture ,0, getX(),getY(),getLayer()-0.01f,getCharHeight(), getCharHeight(), 0,false,false);


	}

};
#endif //_GUIFISHLABEL_H_
