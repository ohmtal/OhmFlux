//-----------------------------------------------------------------------------
// Copyright (c) 2012 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#include "core/fluxGlobals.h"

#include "platform/fluxGL.h"
#include "platform/fluxEmscripten.h"

#include <SDL3/SDL.h>

#include "fluxScreen.h"
#include "utils/errorlog.h"
#include "render/fluxRender2D.h"
#include "particle/fluxParticleManager.h"

#include "audio/fluxAudio.h"


//-------------------------------------------------------------------------------
// Howto:
//  1.) create Object with Videomode, default is OPENGL2D
//  2.) check getVideoFailed(), it true you may terminate the application!
//  3.) call prepare Video to see if the machine can handle your settings
//	4.) call setCaption and optional setIcon
//  5.) call init
//  6.) since Version 0.241126 pseudo FullScreen is used.
//-------------------------------------------------------------------------------
//Con/Destructor
FluxScreen::FluxScreen(VideoMode lVM)
{
	if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
		mSDLVideoFailed = true;
		return;
	}

	AudioManager.init();

	mSDLVideoFailed = false;
	mWindow		 = nullptr;
	mScreenMode  = lVM;
	setWidth(1152);
	setHeight(648);
	mColor_depth = 32;
	mFullScreen	 = false;
	mVsync		 = true;
	mScaleScreen = true;

	mCaption 	 = "OhmFlux game engine";
	mScaleX 	 = 1.f;
	mScaleY		 = 1.f;
	mMinWindowSize = { 320, 200 };


	mScreenFlags = SDL_WINDOW_OPENGL;
	mScreenFlags |= SDL_WINDOW_RESIZABLE ;
//2026-01-08
// #ifdef __EMSCRIPTEN__
// 	mScreenFlags |= SDL_WINDOW_HIGH_PIXEL_DENSITY;
// #endif




#if defined(__EMSCRIPTEN__) || defined(__ANDROID__)
	static int lGL_Context = SDL_GL_CONTEXT_PROFILE_ES;
	static int lGL_MajorVersion = 3;
	static int lGL_MinorVersion = 0;
#else
	static int lGL_Context = SDL_GL_CONTEXT_PROFILE_CORE;
	static int lGL_MajorVersion = 3;
	static int lGL_MinorVersion = 3;
#endif

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, lGL_MajorVersion);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, lGL_MinorVersion);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,  lGL_Context);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

}
//-------------------------------------------------------------------------------
FluxScreen::~FluxScreen()
{

	// Shut down your Render2D singleton
	Render2D.shutdown();

	// shutdown ParticleManager
	ParticleManager.clear();

	if (mGL_Context) {
		SDL_GL_DestroyContext(mGL_Context);
		mGL_Context = nullptr;
	}


	if (mWindow) {
		SDL_DestroyWindow(mWindow);
		mWindow = nullptr;
	}
}
//-------------------------------------------------------------------------------
//FIXME should be in an extra file ... cleanup...

//-------------------------------------------------------------------------------

bool FluxScreen::toggleFullScreen()
{
	S32 windowFlags = SDL_GetWindowFlags(mWindow);
	bool isFullscreen = (windowFlags & SDL_WINDOW_FULLSCREEN) != 0;
	bool targetFullscreen = !isFullscreen;

	bool lResultBool = SDL_SetWindowFullscreen(mWindow, targetFullscreen);

	if (!lResultBool)
	{
		Log("Failed to toggle FullScreen !");
		return false;
	}

	mFullScreen = targetFullscreen;


	return true;
}
//-------------------------------------------------------------------------------
void FluxScreen::setVSync(const bool& lEnabled)
{
	mVsync = lEnabled;
	#ifdef __EMSCRIPTEN__
	// On Web, ALWAYS use 1.
	// This uses requestAnimationFrame, matches monitor Hz, and kills the warning.
	SDL_GL_SetSwapInterval(1);
	Log("Emscripten: Forced VSync (requestAnimationFrame) enabled for performance.");

	#else
	// Desktop behavior remains unchanged
	if (!mVsync) {
		SDL_GL_SetSwapInterval(0);
		Log("Vertical sync is disabled.");
	} else {
		SDL_GL_SetSwapInterval(1);
		Log("Vertical sync is enabled.");
	}
	#endif

}
//-------------------------------------------------------------------------------
bool  FluxScreen::prepareMode(const FluxSettings& lSettings )
{

	// if(!mScreen->prepareMode(mSettings.ScreenWidth,mSettings.ScreenHeight, mSettings.FullScreen, mSettings.Vsync, mSettings.Fsaa))
	setWidth(lSettings.ScreenWidth);
	setHeight(lSettings.ScreenHeight);
	mColor_depth = 32;
	mFullScreen	 = lSettings.FullScreen;
	mVsync = lSettings.initialVsync;
	mMinWindowSize = lSettings.minWindowSize;
	mWindowMaximized = lSettings.WindowMaximized;

	mMaxSprites = lSettings.maxSprites;



	return true;
}
//-------------------------------------------------------------------------------
bool FluxScreen::init()
{
	S32 sdlflags = mScreenFlags;
	if (mFullScreen) {
		sdlflags |= SDL_WINDOW_FULLSCREEN;
	}


	mWindow = SDL_CreateWindow(mCaption,  getWidth(), getHeight(), sdlflags );

	if (!mWindow)
	{
		Log("CRITICAL: Failed to create the SDL Window: %s", SDL_GetError());
		return false;
	}

	SDL_SetWindowSize(mWindow,getWidth(),getHeight());
	SDL_SetWindowMinimumSize(mWindow, mMinWindowSize.x, mMinWindowSize.y);

	if (mWindowMaximized)
		SDL_MaximizeWindow(mWindow);

	mGL_Context = SDL_GL_CreateContext(mWindow);

	if (!mGL_Context) {
		Log("CRITICAL:  Failed to create the OpenGL Context: %s", SDL_GetError());
		return false;
	}

#if !defined(__EMSCRIPTEN__) && !defined(__ANDROID__)
	glewExperimental = GL_TRUE;
	glewInit();

	#ifdef FLUX_DEBUG
		// catch OpenGL Errors - Enable it after glewInit()
		#define GL_CHECK_ERROR() checkGLError(__FILE__, __LINE__)
		if (glDebugMessageCallback) {
			glEnable(GL_DEBUG_OUTPUT);
			glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); // Ensures error is reported exactly when it happens
			glDebugMessageCallback(glDebugOutput, nullptr);
			glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
			Log("Debug Mode: OpenGL Error Logging Enabled");
		}
	#else
		#define GL_CHECK_ERROR()
		Log("Release Mode: Performance Optimized");
	#endif
#endif //#if !defined(__EMSCRIPTEN__) && !defined(__ANDROID__)

	//************ DEFAULT VM_OPENGL2D ********
	if (mScreenMode == VM_OPENGL2D) {

		glViewport(0, 0, getWidth(), getHeight());
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Smoothing (Optional)
		// glEnable(GL_LINE_SMOOTH);
		// glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);


	} else if (mScreenMode == VM_OPENGL) {

		Log("Error ScreenMode VM_OPENGL (%d) not implemented !", mScreenMode);

	} else {
		Log("Error ScreenMode %d not implemented !", mScreenMode);
	}


	// Set the projection once (or when window resizes)
	// createOrthoMatrix(0.0f, getWidth(), getHeight(), 0.0f, -100.0f, 100.0f, mOrtho);
	Render2D.updateOrtho(getWidth(), getHeight());


#ifdef __EMSCRIPTEN__
	initEmScripten();
#endif

	if (!Render2D.init(mMaxSprites))
	{
		// Show a message box or log the error
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Renderer Error",
								 "Could not initialize 2D Shader system.", nullptr);
		return false;
	}

	return true;
}
//-------------------------------------------------------------------------------
bool FluxScreen::updateWindowSize(S32 lWidth, S32 lHeight)
{


	mRealScreenSize.x = lWidth;
	mRealScreenSize.y = lHeight;
	//************ DEFAULT VM_OPENGL2D ********
	if (mScreenMode == VM_OPENGL2D) {

		// NOTE: with mScreenFlags |= SDL_WINDOW_HIGH_PIXEL_DENSITY
		// uses HighDPI but this make it very laggy
		// and does _NOT_ fix my FluxEditor is rendered to small
		#ifdef __EMSCRIPTEN__ //2026-01-08
		S32 lPixelW, lPixelH;
		SDL_GetWindowSizeInPixels(mWindow, &lPixelW, &lPixelH);
		glViewport(0,0, lPixelW, lPixelH);
		dLog("EMSCRIPTEN: Viewport size: %dx%d", lPixelW, lPixelH);
		#else
		if (mScaleScreen) {
			glViewport(0,0, lWidth, lHeight);
		} else {
			glViewport(0,0, getWidth(), getHeight());
		}
		#endif


		mScaleX = (float)lWidth / (float)getWidth();
		mScaleY = (float)lHeight / (float)getHeight();

		Render2D.updateOrtho(getWidth(), getHeight());


		// dLog("------------- FluxScreen::updateWindowSize --------------- ");
		// dLog(" New size is %d,%d", lWidth, lHeight);
		// dLog(" ScreenSize  %d,%d", getWidth(), getHeight());
		// dLog(" Mouse scale is  %4.2f,%4.2f", mScaleX, mScaleY);
		// dLog("----------------------------");


	} else /*if (mScreenMode == VM_OPENGL)*/ {
		// not implemented
	}

	//fire FLUX_EVENT_SCALE_CHANGED >>>
	SDL_Event event;
	SDL_zero(event); // Initialize all fields to 0
	event.type = FLUX_EVENT_SCALE_CHANGED;
	// Example Data ...
	// event.user.code = 42;           // A custom integer code
	// event.user.data1 = somePointer; // Pointer to a struct/object (optional)
	SDL_PushEvent(&event);
	//<< FLUX_EVENT_SCALE_CHANGED

	return true;
}
//-------------------------------------------------------------------------------
bool FluxScreen::setCaption(const char* lCaption)
{
	if (!mWindow)
		return false;
	mCaption = lCaption;


	if (mWindow && !SDL_SetWindowTitle(mWindow, mCaption)) {
		Log("Failed to set Window Title!!!");
		return false;
	}
	return true;
}
//-------------------------------------------------------------------------------
bool FluxScreen::setIcon(const char* lFilename)
{
	if (!mWindow){
		Log("Cant set Icon, init Window first!");
		return false;
	}

	SDL_Surface *Icon;

	if( (Icon = SDL_LoadBMP(lFilename)) == nullptr)
	{
		Log("Unable to load icon: %s", SDL_GetError() );
		return false;
	}

	if( (Icon->w%8) != 0)
	{
		Log("Icon width must be a multiple of 8" );
		SDL_DestroySurface(Icon);

		return false;
	}

	SDL_SetSurfaceColorKey(Icon, true, *((Uint8 *)Icon->pixels));

	SDL_SetWindowIcon(mWindow, Icon);

	return true;
}

//-------------------------------------------------------------------------------
bool FluxScreen::setCursor(const char* lFilename,  int hot_x, int hot_y)
{
	if (!mWindow){
		Log("Cant set cursor, init Window first!");
		return false;
	}

	SDL_Surface *myCursor;
	if( (myCursor = SDL_LoadBMP(lFilename)) == nullptr)
	{
		Log("Unable to load cursor: %s", SDL_GetError() );
		return false;
	}

	SDL_Cursor *cursor = SDL_CreateColorCursor(myCursor, hot_x, hot_y);
	if (cursor == nullptr)
	{
		Log("failed to create cursor: %s", SDL_GetError() );
		return false;
	}
	return SDL_SetCursor(cursor);
}
//-------------------------------------------------------------------------------
