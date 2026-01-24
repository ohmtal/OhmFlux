//-----------------------------------------------------------------------------
// Copyright (c) 2012 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Flux Game Engine
//
// Author: T.Huehn (XXTH)
// Desc  : Main Class handles input and init screen stuff.
//-----------------------------------------------------------------------------
#pragma once
#ifndef _FLUXMAIN_H_
#define _FLUXMAIN_H_
#include "core/fluxGlobals.h"
#include <SDL3/SDL.h>

#include <vector>
#include <cassert>
#include <tuple>
#include <map>
#include <string>
#include <mutex>

#include "core/fluxBaseObject.h"
#include "core/fluxTexture.h"
#include "core/fluxScreen.h"
#include "core/fluxRenderObject.h"
#include "fonts/fluxBitmapFont.h"
#include "audio/fluxAudioStream.h"
#include "core/fluxQuadtree.h"
#include "core/fluxCamera.h"
#include "core/fluxGlobals.h"


extern FluxScreen* g_CurrentScreen;

class FluxMain : public FluxBaseObject
{
protected:
	// static FluxMain* _instance;

private:
	// FluxScreen* mScreen;
	// std::vector<FluxTexture*> mTextures;

	// added prevent of multiple Texture loadings.
	// this open a more lazy way of creating objects like
	// FluxBitmapFont's and FluxRenderObject's '
	// the mutex is added to for thread safety
	std::mutex mTextureMutex;
	typedef std::tuple<std::string, int, int, bool, bool> TextureKey;
	std::map<TextureKey, FluxTexture*, std::less<>> mTextureCache;

	std::vector<FluxBaseObject*> mQueueObjects;
	std::vector<FluxBaseObject*> mDeletedObjects;


	FluxAppStatus mAppStatus;

	bool mRunning;

	Uint64 mLastTick;
	Uint64 mTickCount;
	Uint64 mPerformanceFrequency;

	S32 mFPS = 60;
public:
	FluxMain();
	~FluxMain();

	// fake singelton
	// 2025-12-28 removed AGAIN i did only need this for getScreen so i added this as global variable
	// static FluxMain* Instance() {
	// 	// Optional: add your assert here to ensure a game was created
	// 	assert(_instance != nullptr && "No game instance created! Did you call new [YourGameClass]()?");
	// 	return _instance;
	// }


	// FluxScreen* getScreen() { return mScreen; }
	FluxScreen* getScreen() { return getScreenObject(); }

	// FluxTexture* loadTransparentTexture(const char* filename, int cols = 1, int rows = 1);
	// usePixelPerfect is set so a pixel color looks like a pixel
	FluxTexture* loadTexture(std::string filename, int cols = 1, int rows = 1, bool setColorKeyAtZeroPixel = false, bool usePixelPerfect  = false);

	bool toggleFullScreen();
	FluxSettings mSettings;

	void TerminateApplication();

	FluxAppStatus getStatus() { return mAppStatus; }

	// SDL_AudioDeviceID getAudioDevice() { return mAudioDevice; }

	void queueObject(FluxBaseObject* lObject);
	bool unQueueObject(FluxBaseObject* lObject);

	bool queueDelete(FluxBaseObject* lObject); //<< this rock :D

	FluxQuadtree* GetQuadtree() { return getQuadTreeObject(); }

	// moved to global FluxRenderObject* rayCast( const Point2I& lPos );

	void Draw();

	virtual void Execute();
	virtual void IterateFrame();
	virtual bool Initialize();
	virtual void Deinitialize();

	virtual void onEvent(SDL_Event event) {};
	virtual void onKeyEvent(SDL_KeyboardEvent event) {};
	virtual void onMouseButtonEvent(SDL_MouseButtonEvent event) {};
	virtual void onWindowSizeChanged() {};

	virtual void Update(const double& dt);
	virtual void onDraw() {};
	virtual void onDrawTopMost() {}; // rendered After RenderBatch

	std::vector<FluxBaseObject*> getQueueObjects() { return mQueueObjects; }

	S32 getFPS() const {return mFPS;}

	void  setPause(bool value) {mAppStatus.Paused=value;}
	bool  togglePause()  { mAppStatus.Paused = !mAppStatus.Paused; return mAppStatus.Paused;}
	bool getPause() const { return mAppStatus.Paused;}

	void setupMousePositions( F32 lX, F32 lY ); //SDL2 compat SDL_MouseMotionEvent lMouseMotionEvent );
	void setupWorldMousePositions( );

};// class



#endif //#ifndef _FLUXMAIN_H_
