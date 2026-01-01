//-----------------------------------------------------------------------------
// Copyright (c) 2012 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------

#include "core/fluxGlobals.h"
#include <SDL3/SDL.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include <assert.h>
#include <vector>
#include <time.h>
#include <algorithm>

#include "fluxMain.h"
#include "core/fluxMath.h"
#include "utils/errorlog.h"
#include "render/fluxRender2D.h"
#include "particle/fluxParticleManager.h"
#include "utils/fluxScheduler.h"

float gFrameTime = 0.f; // we need that Global for timming
float gGameTime  = 0.f;

extern FluxQuadtree* g_CurrentQuadTree;
//--------------------------------------------------------------------------------------
FluxMain::FluxMain()
{
	 // _instance = this;

	FluxBaseObject();
	mSettings.ScreenWidth  = 1152; //800;
	mSettings.ScreenHeight = 648; //600;
	mSettings.FullScreen   = false;
	mSettings.initialVsync = true;
	mSettings.ScaleScreen  = true;

	mSettings.Caption        = "Flux";
	mSettings.Version        = "0.260101";
	mSettings.IconFilename   = nullptr;
	mSettings.CursorFilename = nullptr;
	mSettings.cursorHotSpotX = 0;
	mSettings.cursorHotSpotY = 0;


}

FluxMain::~FluxMain() {

}

// FluxMain* FluxMain::_instance = nullptr;

// FluxMain* FluxMain::Instance() {
// 	static_assert(std::is_default_constructible<FluxMain>::value,
// 				  "FluxMain must have a default constructor to be a Singleton");
// 	static FluxMain instance;
// 	return & instance;
// }



//--------------------------------------------------------------------------------------
bool FluxMain::Initialize()
{
#if !defined(__EMSCRIPTEN__) && !defined(__ANDROID__)
	if (mSettings.enableLogFile)
	{
		std::string lLogFileString = sanitizeFilenameWithUnderScores(mSettings.Caption) + ".log";
		InitErrorLog(lLogFileString.c_str(),this->mSettings.Caption,this->mSettings.Version);
	}

#endif
	Log("Running on SDL3");

	mAppStatus.Visible		 = true;
	mAppStatus.Paused		 = false;
	mAppStatus.MouseFocus	 = true;
	mAppStatus.KeyboardFocus = true;

	mLastTick = SDL_GetPerformanceCounter(); //SDL_GetTicks64();

	//seed random number generator
    srand((unsigned) time(nullptr));

	g_CurrentScreen = new FluxScreen();
	// mScreen = new FluxScreen();
	if(getScreen()->getVideoFailed())
	{
		Log("Failed to init  Video / Audio  or Shader");
		return false;
	}
	Log("Setup Screen: Resolution:%d x %d, FullScreen: %d"
		,mSettings.ScreenWidth,mSettings.ScreenHeight, mSettings.FullScreen);


	if (!getScreen()->prepareMode(mSettings))
	{
		Log("Failed to prepareMode(%d,%d,%d,%d)",mSettings.ScreenWidth,mSettings.ScreenHeight, mSettings.FullScreen, mSettings.initialVsync);
		return false;
	}
	if (!getScreen()->init())
		return false;


	getScreen()->setVSync(mSettings.initialVsync);

	getScreen()->setScaleScreen(mSettings.ScaleScreen);

	if (mSettings.Caption)
		getScreen()->setCaption(mSettings.Caption);

	if (mSettings.IconFilename )
		getScreen()->setIcon(mSettings.IconFilename);

	if (mSettings.CursorFilename)
		getScreen()->setCursor(mSettings.CursorFilename, mSettings.cursorHotSpotX, mSettings.cursorHotSpotY);


	//some informations
	Log("---------------------------------------");
	Log("CPU Cores: %d with %dbytes cache, %dMiB RAM."
	, SDL_GetNumLogicalCPUCores(), SDL_GetCPUCacheLineSize()
	, SDL_GetSystemRAM());

	// display
	SDL_DisplayID lDisplayID = SDL_GetDisplayForWindow(getScreen()->getWindow());
	const SDL_DisplayMode *  lDM = new SDL_DisplayMode();
	lDM = SDL_GetCurrentDisplayMode(lDisplayID);
	if (!lDM) {
		Log("SDL failed to get Displaymode: %s", SDL_GetError());
		lDM = SDL_GetDesktopDisplayMode(lDisplayID);
	}
	if (lDM)
		Log("Display (%d) resolution: %dx%d, refresh rate: %6.2f)", lDisplayID, lDM->w, lDM->h, lDM->refresh_rate);
	else
		Log("SDL failed to get DesktopMode: %s", SDL_GetError());
// #endif

	Log( "Renderer  : %s",glGetString(GL_RENDERER));
	Log( "Vendor    : %s",glGetString(GL_VENDOR));
	Log( "GLVersion : %s",glGetString(GL_VERSION));
	Log("---------------------------------------");

	if (mSettings.useQuadTree) {
		g_CurrentQuadTree = new FluxQuadtree(mSettings.WorldBounds);
	}


	return true;
}
//--------------------------------------------------------------------------------------
void FluxMain::Deinitialize()
{
	// Shutdown scheduler
	dLog("FluxMain: shutdown FluxSchedule");
	FluxSchedule.shutdown();



	// Cleanup Game Objects
	dLog("FluxMain: Cleaning up queued game objects");
	for (auto* obj : mQueueObjects) {
		auto* baseObj = static_cast<FluxBaseObject*>(obj); // Explicit cast if needed
		SAFE_DELETE(baseObj);
	}
	mQueueObjects.clear();

	// Cleanup Textures
	dLog("FluxMain: Cleaning up Textrue resources");
	for (auto& [key, val] : mTextureCache) {
		SAFE_DELETE(val);
	}
	mTextureCache.clear();


	dLog("FluxMain: Cleaning up screen");
	SAFE_DELETE(g_CurrentScreen);

	if (g_CurrentQuadTree)
	{
		dLog("FluxMain: Cleaning up quad tree");
		g_CurrentQuadTree->clear();
		SAFE_DELETE(g_CurrentQuadTree);
	}

	CloseErrorLog();
}
//--------------------------------------------------------------------------------------
/*
 * add to queue
 *
 */
// void FluxMain::queueObject(FluxBaseObject* lObject)
// {
// 	mQueueObjects.push_back(lObject);
// }
void FluxMain::queueObject(FluxBaseObject* lObject)
{
	if (lObject == nullptr) return;

	// Check if it already exists in the queue
	auto it = std::find(mQueueObjects.begin(), mQueueObjects.end(), lObject);

	if (it == mQueueObjects.end()) {
		mQueueObjects.push_back(lObject);
	} else {
		Log(">>>>>>>>>>> FluxMain::queueObject :: Object already queued, skipping. <<<<<<<<<<<<<<<<<<<<<<");
	}
}

/**
 * only unqueue
 */
bool FluxMain::unQueueObject(FluxBaseObject* lObject)
{

	auto it = std::find(mQueueObjects.begin(), mQueueObjects.end(), lObject);
	if (it != mQueueObjects.end()) {
		mQueueObjects.erase(it);
		return true;
	}

	return false;
}
/**
 * unqueue from render Queue and add to mDeletedObjects
 */
bool FluxMain::queueDelete(FluxBaseObject* lObject)
{
	// 1. Find the object in the vector
	auto it = std::find(mQueueObjects.begin(), mQueueObjects.end(), lObject);

	// 2. If found, move to deleted list and erase from active queue
	if (it != mQueueObjects.end())
	{
		mDeletedObjects.push_back(*it);
		mQueueObjects.erase(it);
		return true;
	}

	return false;
}
//--------------------------------------------------------------------------------------
// Texture load wrapper
// 2026-01-01 optimized with thread safety and prevent multiple loading
FluxTexture* FluxMain::loadTexture(const char* filename, int cols, int rows, bool setColorKeyAtZeroPixel, bool usePixelPerfect)
{
	// Create a key using string_view to avoid allocation during the search
	// Note: We use a helper auto-key to check existence
	auto lookupKey = std::make_tuple(std::string_view(filename), cols, rows, setColorKeyAtZeroPixel, usePixelPerfect);

	// Transparent Lookup
	auto it = mTextureCache.find(lookupKey);
	if (it != mTextureCache.end()) {
		return it->second;
	}

	// --- Cache Miss: Now we proceed with loading ---
	FluxTexture* result = new FluxTexture();
	if (!usePixelPerfect) result->setUseTrilinearFiltering();

	bool success = (!setColorKeyAtZeroPixel)
	? result->loadTextureDirect(filename)
	: result->loadTexture(filename, setColorKeyAtZeroPixel);

	if (!success) {
		Log("Cannot load graphic: %s", filename);
		SAFE_DELETE(result);
		return nullptr;
	}

	result->setParts(cols, rows);

	// Store in cache: Only here is the string actually copied/allocated
	mTextureCache.emplace(lookupKey, result);

	return result;
}


// FluxTexture* FluxMain::loadTexture(const char* filename, int cols, int rows, bool setColorKeyAtZeroPixel, bool usePixelPerfect)
// {
// 	FluxTexture* result = new FluxTexture();
// 	if (!usePixelPerfect) result->setUseTrilinearFiltering();
//
// 	bool success = false;
//
// 	// If no software-colorkeying is requested, use the Fast Path
// 	if (!setColorKeyAtZeroPixel) {
// 		success = result->loadTextureDirect(filename);
// 	} else {
// 		// Use the Surface-based path (Double-copy) because it's required for ColorKeying
// 		success = result->loadTexture(filename, setColorKeyAtZeroPixel);
// 	}
//
// 	if (!success) {
// 		Log("Cannot load graphic: %s", filename);
// 		SAFE_DELETE(result);
// 		return nullptr;
// 	}
//
// 	result->setParts(cols, rows);
// 	mTextures.push_back(result);
// 	return result;
// }
//--------------------------------------------------------------------------------------
bool FluxMain::toggleFullScreen()
{
  mAppStatus.Visible = false;
  bool result = getScreen()->toggleFullScreen();
  mAppStatus.Visible = true; 
  return result;
}
//--------------------------------------------------------------------------------------
void FluxMain::Update(const double& dt)
{
	FluxSchedule.update(dt);

	for (U32 i = 0; i < mQueueObjects.size(); )
	{
		FluxBaseObject* obj = mQueueObjects[i];
		if (obj)
			obj->Update(dt);


		if (i < mQueueObjects.size() && mQueueObjects[i] == obj)
			++i;
	}
	ParticleManager.update(dt / 1000.f );
}
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
void FluxMain::Draw() {
	assert(getScreen());
    /* layers ... init */
	// disabled for batch rendering
	// glEnable(GL_DEPTH_TEST);
	// glDepthFunc(GL_LEQUAL);

	// Inside your main render loop, before drawing anything:
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	// ADD GL_DEPTH_BUFFER_BIT TO THE CLEAR COMMAND:
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//Camera
	Render2D.beginFrame();

	for (auto* obj : mQueueObjects)
	{
		if (obj && obj->getVisible())
		{
			obj->Draw();
		}
	}

	onDraw();
	ParticleManager.render();
	/* layers ... end */ 
	Render2D.renderBatch();

	// disabled for batch rendering
    // glClear(GL_DEPTH_BUFFER_BIT);
    // glDisable(GL_DEPTH_TEST);
}
//--------------------------------------------------------------------------------------
#ifdef __EMSCRIPTEN__
//-------------------------------------------------------------------------------
// Audio webGL :
//-------------------------------------------------------------------------------
void HandleAudioResume() {
	// This JavaScript snippet checks if the context is suspended and resumes it.
	// In SDL, the AudioContext is usually stored on the 'Module' or 'SDL2' object.
	MAIN_THREAD_EM_ASM({
		if (typeof SDL2 !== 'undefined' && SDL2.audioContext && SDL2.audioContext.state === 'suspended') {
			SDL2.audioContext.resume().then(() => {
				console.log('AudioContext resumed successfully!');
			});
		}
		// If using SDL3 or a different wrapper:
		if (window.audioContext && window.audioContext.state === 'suspended') {
			window.audioContext.resume();
		}
	});
}
//-------------------------------------------------------------------------------
// webGL main loop:
//-------------------------------------------------------------------------------
// This wrapper bridges the C-style callback to your C++ class instance
void emscripten_loop_wrapper(void* arg)
{
	// On the first mouse click or key press, try to resume
	static bool audioResumed = false;
	if (!audioResumed) {
		// Check for any SDL input event
		SDL_Event event;
		if (SDL_PeepEvents(&event, 1, SDL_PEEKEVENT, SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_EVENT_KEY_DOWN) > 0) {
			HandleAudioResume();
			audioResumed = true;
		}
	}
	FluxMain* app = static_cast<FluxMain*>(arg);
	app->IterateFrame();
}
#endif
//--------------------------------------------------------------------------------------
void FluxMain::Execute() {
	if (!Initialize()) {
		Log("App init failed: %s", SDL_GetError());
		exit(1);
	}

	mPerformanceFrequency = SDL_GetPerformanceFrequency();
	// INITIALIZE time once here before the loop starts
	mLastTick = SDL_GetPerformanceCounter();
	mRunning = true;

	#ifdef __EMSCRIPTEN__
	// 0 = use requestAnimationFrame, 1 = simulate infinite loop
	emscripten_set_main_loop_arg(emscripten_loop_wrapper, this, 0, 1);
	emscripten_set_main_loop_timing(EM_TIMING_RAF, 1); //force RAF
	#else
	while (mRunning) {
		IterateFrame();
	}
	Deinitialize(); // Cleanup once on Desktop
	#endif
}

//--------------------------------------------------------------------------------------
void FluxMain::setupMousePositions( F32 lX, F32 lY ) //SDL2 compat  SDL_MouseMotionEvent lMouseMotionEvent )
{

	mAppStatus.RealMousePos = { lX, lY };
	mAppStatus.MousePos 	= { lX / getScreen()->getScaleX() , lY / getScreen()->getScaleY() };
	setupWorldMousePositions( );

}
//--------------------------------------------------------------------------------------
// i could also use but to make it more flexible i added the lMousePositon parameter
void FluxMain::setupWorldMousePositions(  )
{
	float zoom = Render2D.getCamera()->getZoom();
	Point2F cameraPos = Render2D.getCamera()->getPosition(); // Camera's world center
	Point2F screenSize = getScreen()->getScreenSizeF();
	Point2F screenCenter = screenSize / 2.0f;

	// 2. Shift click to be relative to screen center (0, 0)
	Point2F relativeClick = mAppStatus.MousePos - screenCenter;

	// 3. Apply inverse zoom
	// If zoom > 1 means "zoomed in", you divide. If zoom > 1 means "larger area", you multiply.
	// Standard logic: worldPos = (screenCoord / zoom) + cameraPos
	Point2F worldOffset = relativeClick / zoom;

	// 4. Final World Position
	mAppStatus.WorldMousePos = cameraPos + worldOffset;

}
//--------------------------------------------------------------------------------------
void FluxMain::IterateFrame()
{
	SDL_Event E;

	// 1. Process ALL pending events in a loop to avoid input lag
	while (SDL_PollEvent(&E)) {
		switch (E.type) {
			case SDL_EVENT_QUIT:
				mRunning = false;
				break;
			case SDL_EVENT_WINDOW_SHOWN:
				mAppStatus.Visible = true;
				getScreen()->updateWindowSize(getScreen()->getWidth(), getScreen()->getHeight());
				break;
			case SDL_EVENT_WINDOW_HIDDEN:
				mAppStatus.Visible = false;
				break;
			case SDL_EVENT_WINDOW_FOCUS_GAINED:
				mAppStatus.KeyboardFocus = true;
				break;
			case SDL_EVENT_WINDOW_FOCUS_LOST:
				mAppStatus.KeyboardFocus = false;
				break;
			case SDL_EVENT_WINDOW_RESIZED:
				getScreen()->updateWindowSize((int)E.window.data1, (int)E.window.data2);
				onWindowSizeChanged();
				break;

			// case SDL_EVENT_MOUSE_WHEEL:  ==> overwrite  onEvent in you game
			// 	onMouseWheelEvent(E);
			// 	break;
			case SDL_EVENT_MOUSE_MOTION:
				//  setupMousePositions( E.motion );
				setupMousePositions( E.motion.x, E.motion.y  );

				break;
			case SDL_EVENT_KEY_UP:
			case SDL_EVENT_KEY_DOWN:
				onKeyEvent(E.key);
				break;
			case SDL_EVENT_MOUSE_BUTTON_DOWN:
			case SDL_EVENT_MOUSE_BUTTON_UP:
				onMouseButtonEvent(E.button);
				break;
		}
		onEvent(E);
	}

	//  Quit check for Emscripten
	if (!mRunning) {
		#ifdef __EMSCRIPTEN__
		Deinitialize();
		emscripten_cancel_main_loop();
		#endif
		return;
	}

	// Handle Visibility/Pause
	if (!mAppStatus.Visible || mAppStatus.Paused) {
		#ifndef __EMSCRIPTEN__
		// Only block on native desktop; Web must never block
		SDL_WaitEventTimeout(nullptr, 100);
		#endif
		mLastTick = SDL_GetPerformanceCounter();
		return;
	}

	//  Time Calculation
	mTickCount = SDL_GetPerformanceCounter();
	gFrameTime = (double)(mTickCount - mLastTick) / (double)mPerformanceFrequency * 1000.0;
	//  Delta Time Cap (Prevent logic jumps if browser tab was suspended)
	if (gFrameTime > 200.0f) gFrameTime = mSettings.updateDt;

	//  Frame Limiter (Native only)
	if (mSettings.frameLimiter > 0.f && gFrameTime < mSettings.frameLimiter) {
		#ifndef __EMSCRIPTEN__
		SDL_Delay((Uint32)(mSettings.frameLimiter - gFrameTime));
		#endif
		mTickCount = SDL_GetPerformanceCounter();
		gFrameTime = (double)(mTickCount - mLastTick) / (double)mPerformanceFrequency * 1000.0;
	}

	gGameTime += (float)gFrameTime / 1000.f;

	// fps update every 1 second:
	static double lFpsTimer = 0;
	static S32 lFrameCounter = 0;

	lFpsTimer += gFrameTime;
	lFrameCounter++;

	if (lFpsTimer >= 1000.0) { // Every 1 second
		mFPS = lFrameCounter;
		lFrameCounter = 0;
		lFpsTimer -= 1000.0;
	}

	// fixed update: >>>>>>>>>>>>>>>>>>>>>>>>>
	static double accumulator = 0.0;
	accumulator += gFrameTime;

	// 4. Fixed Timestep Loop
	// Consume time in chunks of exactly 16.66ms.
	while (accumulator >= mSettings.updateDt) {
		Update(mSettings.updateDt);
		accumulator -= mSettings.updateDt;
	}

	//  Render
	Draw();
	SDL_GL_SwapWindow(getScreen()->getWindow());

	//  Update LastTick AFTER the frame is finished
	mLastTick = mTickCount;

	//  Memory Cleanup
	if (!mDeletedObjects.empty()) {
		for (auto* obj : mDeletedObjects) {
			delete obj;
		}
		mDeletedObjects.clear();
	}
}


//--------------------------------------------------------------------------------------
void FluxMain::TerminateApplication(void)
{
	static SDL_Event Q;					

	Q.type = SDL_EVENT_QUIT;

	if(!SDL_PushEvent(&Q))
	{
		exit(1);
	}
	return;
}
//--------------------------------------------------------------------------------------
