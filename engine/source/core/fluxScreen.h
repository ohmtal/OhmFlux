//-----------------------------------------------------------------------------
// Copyright (c) 2012 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
//
// Author: T.Huehn (XXTH)
// Desc  : Screen class handles SDL surface
//-----------------------------------------------------------------------------
#pragma once
#ifndef _FLUXSCREEN_H_
#define _FLUXSCREEN_H_

#include "core/fluxGlobals.h"
#include "core/fluxTexture.h"


class FluxScreen
{
public:
	enum VideoMode {
		VM_SDL,
		VM_OPENGL,
		VM_OPENGL2D
	};

private:
	SDL_Window* mWindow;
	SDL_GLContext mGL_Context;
	VideoMode mScreenMode;
	S32  _mWidth;
	S32  _mHeight;
	S32  _mCenterX;
	S32  _mCenterY;
	S32  mColor_depth;
	bool mFullScreen;
	bool mVsync;
	bool mScaleScreen;
	float mScaleX, mScaleY;

	const char* mCaption;
	Point2I mMinWindowSize;
	bool mWindowMaximized = false;

	U32 mMaxSprites = DEFAULT_MAX_SPRITES;


	bool mSDLVideoFailed;
	S32 mScreenFlags;


	Point2I mRealScreenSize = { 1152, 648 };



public:
	FluxScreen(VideoMode lVM = VM_OPENGL2D);
	~FluxScreen();

	SDL_Window * getWindow() { return mWindow; }
	SDL_GLContext getGLContext() { return mGL_Context; }


	bool getVideoFailed() { return mSDLVideoFailed; }
	bool prepareMode(const FluxSettings& lSettings );
	bool init();
	bool updateWindowSize(S32 lWidth, S32 lHeight);
	bool setCaption(const char* lCaption);
	bool setIcon(const char* lFilename);
	bool setCursor(const char* lFilename,  int hot_x = 0, int hot_y = 0);



	void setWidth(S32 value)  { _mWidth =  value; _mCenterX = (S32)(_mWidth/2.f); }
	void setHeight(S32 value) { _mHeight = value; _mCenterY = (S32)(_mHeight/2.f);}
	S32  getWidth()		 const { return _mWidth; }
	S32  getHeight()	 const { return _mHeight; }
	S32  getCenterX()	 const { return _mCenterX; }
	S32  getCenterY()	 const { return _mCenterY; }

	Point2F getCenterF() const { return { static_cast<F32>(_mCenterX ), static_cast<F32>(_mCenterY ) }; }
	Point2I getScreenSize() const { return { _mWidth , _mHeight  }; }
	Point2F getScreenSizeF() const { return { static_cast<F32>(_mWidth ), static_cast<F32>(_mHeight ) }; }
	Point2F getScreenCenterF() const { return { static_cast<F32>(_mWidth ) / 2.f , static_cast<F32>(_mHeight ) / 2.f }; }
	Point2I getRealScreenSize() const { return mRealScreenSize; }

	float getScaleX() const {return mScaleX;}
	float getScaleY() const {return mScaleY;}
	void  setScaleX(const float& value ) {mScaleX = value;}
	void  setScaleY(const float& value ) {mScaleY = value;}


	bool getFullScreen() const { return mFullScreen; }
	bool getVsync()		 const { return mVsync; }
	void setVSync(const bool& lEnabled);

    bool toggleFullScreen();

	void setScaleScreen(bool value) 	{mScaleScreen = value;}
	bool setScaleScreen() 	const {return mScaleScreen;}

	bool getWindowMaximized() {
		mWindowMaximized = (SDL_GetWindowFlags(mWindow) & SDL_WINDOW_MAXIMIZED) != 0;
		return mWindowMaximized;
	}
	void setWindowMaximized(bool value) {
		if (value) {
			SDL_MaximizeWindow(mWindow);
		} else {
			SDL_RestoreWindow(mWindow);
		}
		mWindowMaximized = (SDL_GetWindowFlags(mWindow) & SDL_WINDOW_MAXIMIZED) != 0;
	}




};

#endif //_FLUXSCREEN_H_
