//-----------------------------------------------------------------------------
// Copyright (c) 2012..2025 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Includes
#ifdef WIN32
#include <windows.h>
#else
#include <stdio.h>
#include <stdlib.h>
#endif
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h> //<<< Android! and Windows
#include <assert.h>
#include <vector>
#include <algorithm>




#include "main.h"
#include "globals.h"
#include "myFish.h"
#include "GuiFishLabel.h"

#include <errorlog.h>
#include <fluxGlobals.h>
#include <fluxRender2D.h>
#include <misc.h>
#include <fluxLight.h>
#include <fluxLightManager.h>
#include <fluxParticleEmitter.h>
#include <fluxParticleManager.h>
#include <fluxParticlePresets.h>
#include <fluxScheduler.h>


//--------------------------------------------------------------------------------------
void _MainPeep_(S32 someValue)
{
	LogFMT("_MainPeep_ someValue:{}", someValue);
}

//--------------------------------------------------------------------------------------

DemoGame::DemoGame() {
	mFishCount = 40;

	//init variables 
	mScore = 0;
	mScoreInc = 0;
	mChallengeTime = 0;
	mChallengeRunning = false;
	mFishGoalReached = false;
	for (int i = 0; i < FishTypeCount; i++)
		mFishGoals[i] = 0;

}
//--------------------------------------------------------------------------------------
bool DemoGame::Initialize() {
	if (!Parent::Initialize()) return false;



	//load a texture - flux main cares about destroying them 
	mBackTex     = loadTexture("assets/backgrounds/background.bmp");
	mRockFarTex  = loadTexture("assets/backgrounds/rocksfar.bmp");
	mRockNearTex = loadTexture("assets/backgrounds/rocksnear.bmp");
	mWaveTex 	 = loadTexture("assets/backgrounds/wave.bmp", 1, 1, true);




	/*
	 *  change FishTypeCount if you add remove a texture !!!!!!!!!!!
	 */
	int i = 0;
	mFishTextures[i] =  loadTexture("assets/fish/triggerfish1sheet.bmp",2,2);i++;
	// mFishTextures[i] =  loadTexture("assets/fish/triggerfish2sheet.bmp",2,2);i++;

	// mFishTextures[i] =  loadTexture("assets/fish/seahorse1sheet.bmp",4,1);i++;
	// mFishTextures[i] =  loadTexture("assets/fish/seahorse2sheet.bmp",4,1);i++;

	mFishTextures[i] =  loadTexture("assets/fish/rockfish2sheet.bmp",2,2);i++;
	mFishTextures[i] =  loadTexture("assets/fish/rockfish1sheet.bmp",2,2);i++;

	// mFishTextures[i] =  loadTexture("assets/fish/pufferfishpuffsheet.bmp",2,2);i++;
	mFishTextures[i] =  loadTexture("assets/fish/pufferfishswimsheet.bmp",2,2);i++;

	// mFishTextures[i] =  loadTexture("assets/fish/eelsheet.bmp",1,4);i++;

	mFishTextures[i] =  loadTexture("assets/fish/butterflyfishsheet.bmp",2,2);i++;

	mFishTextures[i] =  loadTexture("assets/fish/angelfish2sheet.bmp",2,2);i++;
	mFishTextures[i] =  loadTexture("assets/fish/angelfish1sheet.bmp",2,2);i++;
	

	/* Font Texture
	 *
	 */
	mHackNerdTex = loadTexture("assets/fonts/hackNerd_13x25.bmp", 10,10, false);
	mMono16Tex   = loadTexture("assets/fonts/monoSpace_13x28.bmp", 10,10, false);




	if (!getScreen())
	{
		Log("Failed to get screen!!!");
		return false;
	}

	
	// test BitMapFonr
	//mLabel1 = new FluxBitmapFont(mHackNerdTex, getScreen());
	mLabel1 = new FluxBitmapFont(mMono16Tex, getScreen());
	mLabel1->set("XX XX XX", 7, getScreen()->getHeight() - 14,  13, 28, { 1.f, 0.f, 1.f, LabelAlpha} );
	mLabel1->setLayer(0.f); //push to front
	mLabel1->setIsGuiElement(true);
	queueObject(mLabel1);

	mLabel2 = new FluxBitmapFont(mMono16Tex, getScreen());
	mLabel2->set("XX XX XX", 250,13, 13, 28, { 1.f, 1.f, 1.f, LabelAlpha });
	mLabel2->setLayer(0.f); //push to front
	mLabel2->setIsGuiElement(true);
	queueObject(mLabel2);

	mLabel1->setVisible(false);
	mLabel2->setVisible(false);

	//mobile webGL debug i want to see whats going on
#if defined(__EMSCRIPTEN__) && defined(FLUX_DEBUG)
	mLabel2->setVisible(true);
#endif

	// ************ SOUNDS ******************
	loadSound(mPling, "pling.wav", 0.2f);
	loadSound(mPlingeling, "plingeling.wav", 0.2f);
	loadSound(mBrrooii, "brrooii.wav", 1.f);
	loadSound(mFailSound, "fail.wav", 1.f);


	// ************* Game Labels *********************
	mScoreLabel = new FluxBitmapFont(mHackNerdTex, getScreen());
	mScoreLabel->set("000000", getScreen()->getWidth() - 6 * 13 ,13, 13, 25, { 0.5f, 0.5f, 1.f, LabelAlpha });
	mScoreLabel->setLayer(0.f); //push to front
	mScoreLabel->setCaption("%06d", 0);
	mScoreLabel->setIsGuiElement(true);
	queueObject(mScoreLabel);


	mChallengeTimeLabel = new FluxBitmapFont(mHackNerdTex, getScreen());
	mChallengeTimeLabel->set("000000",
							getScreen()->getWidth() - 6 * 13 ,
							getScreen()->getHeight() - 13,
							13, 25,
							{ 1.f, 0.f, 1.f, LabelAlpha });
	mChallengeTimeLabel->setLayer(0.f); //push to front
	mChallengeTimeLabel->setCaption("%06d", 0);
	mChallengeTimeLabel->setIsGuiElement(true);
	queueObject(mChallengeTimeLabel);


	int y = 13;
	for (int i = 0; i < FishTypeCount; i++)
	{
		// counter of current fishes
		mFishLabel[i] = new GuiFishLabel(mFishTextures[i], mMono16Tex, getScreen());
		mFishLabel[i]->setIsGuiElement(true);
		mFishLabel[i]->set("XXXX", 15, y, 13, 28, { 1.f, 1.f, 1.f, LabelAlpha });
		queueObject(mFishLabel[i]);
		// goal display
		mFishGoalDisplay[i] = new FluxBitmapFont( mMono16Tex, getScreen());
		mFishGoalDisplay[i]->set("XXX", 15 + (4*13), y, 13, 28,  { 0.f, 0.f, 1.f, LabelAlpha });
		mFishGoalDisplay[i]->setVisible(false);
		queueObject(mFishGoalDisplay[i]);

		//---
		y+=28;

	}
	mScore = 0;
	mChallengeTime = 0;

	// Lights >>>
	const Color4F cl_seaLight = {1.f,1.f,0.8f,3.f};
	const Point2F pt_LightVector = {0.15f, 0.25f};
	const F32 f_LightAngle = 30.f; // 25.f;
	const F32 f_LightRadius = 600.f;

	Render2D.setAmbientColor( { 0.05f,0.051f,0.15f,1.f} );
	for ( S32 i = 880; i > -300; i -= 320)
		LightManager.addLight(
			FluxLight({ (F32)i, -100.0f,0.0f}, cl_seaLight, f_LightRadius)
			.setAsSpotlight(pt_LightVector, f_LightAngle)
		);

	//<<< lights

	// paricles >>>
	FluxTexture* lBubble = loadTexture( "assets/particles2/bubble.png" );
	ParticleManager.addEmitter(
		ParticlePresets::waterBubblePreset
		.setTexture(lBubble)
		.setScaleMinMax( 1.f, 3.f)
		.setLifeTimeMinMax(6.f,10.f)
		.setRotationSpeedMinMax(1.f, 2.f)
	)->setPosition({ 400.f ,550.f, 0.10f })->play();

	ParticleManager.addEmitter(
		ParticlePresets::waterBubblePreset
		.setTexture(lBubble)
		.setScaleMinMax( 1.f, 3.f)
		.setLifeTimeMinMax(6.f,10.f)
	)->setPosition({ 960.f ,550.f, 0.10f })->play();

	//<<< paricles

	// must be on bottom !!
	respawnFishes();

	SDL_ShowCursor();


    return true;
}
//--------------------------------------------------------------------------------------
void DemoGame::respawnFishes(int setNewCount )
{
	if (setNewCount > 0)
		mFishCount = setNewCount;

	static std::vector<myFish *>::iterator curObj;
	for (curObj=mFishes.begin(); curObj!=mFishes.end(); ++curObj) {
		queueDelete((*curObj));
	}
	mFishes.clear();

	for (Uint32 i = 0; i < mFishCount; i++)
	{
		spawnFish();
	}

}

//--------------------------------------------------------------------------------------
void DemoGame::spawnFish()
{
	int rTex = RandInt(0,FishTypeCount - 1 );
	myFish* f = new myFish(mFishTextures[rTex], mMono16Tex, getScreen(), rTex);
	queueObject(f);
	mFishes.push_back(f);
	updateFishCounter();

}
//--------------------------------------------------------------------------------------
void DemoGame::updateFishCounter(){
	//count the different fishes

	for (int i = 0 ; i < FishTypeCount; i++) {
		mFishCounter[i] = 0;
	}

	static std::vector<myFish *>::iterator curObj;
	for (curObj=mFishes.begin(); curObj!=mFishes.end(); ++curObj) {
		mFishCounter[(*curObj)->mFishType]++;
	}

	// compare goals
	mScoreInc = 0;
	int goalOKCounter = 0;
	int diff = 0;
	for (int i = 0 ; i < FishTypeCount; i++) {
		if (mFishCounter[i] == mFishGoals[i]) {
			mFishGoalDisplay[i]->setColor({0.f,1.f,0.f,LabelAlpha});
			goalOKCounter++;
			mScoreInc+=2;
			mFishGoalDisplay[i]->setCaption(" OK");
		} else {
			diff = abs(mFishCounter[i] - mFishGoals[i]);
			if (mFishCounter[i] > mFishGoals[i]) {
				mFishGoalDisplay[i]->setColor({1.f,0.2f,0.2f,LabelAlpha});
				mFishGoalDisplay[i]->setCaption("-%2d",diff);
				mScoreInc--;
			} else {
				mFishGoalDisplay[i]->setColor({0.7f,0.1f,0.7f,LabelAlpha});
				mFishGoalDisplay[i]->setCaption("+%2d",diff);
			}

		}

	}
	mFishGoalReached = goalOKCounter == FishTypeCount;
	if (mFishGoalReached)
		mScoreInc = FishTypeCount * 3; //tripple score
}
//--------------------------------------------------------------------------------------
void DemoGame::listFishes()
{
	Log("~~~~~~~~~~ FISHES: (screen size is: %d,%d)", getScreen()->getWidth(), getScreen()->getHeight());


	static std::vector<myFish*>::iterator curObj;
	for (curObj = mFishes.begin(); curObj != mFishes.end(); ++curObj) {
		Log("Fish (%d) pos:%7.2f, %7.2f, flipX:%d, layer:%5.2f, speed: %5.2f",
			(*curObj)->mFishType
			, (*curObj)->getX(), (*curObj)->getY()
			, (*curObj)->getFlipX()
			, (*curObj)->getLayer()
			, (*curObj)->getSpeed()
		);
	}
	Log("--------------------------------------------");

}
//--------------------------------------------------------------------------------------
void DemoGame::Deinitialize() {
	Parent::Deinitialize();

}
//--------------------------------------------------------------------------------------
void DemoGame::Update(const double& dt)
{

	static float lSpawnTimer = 0;
	lSpawnTimer += dt / 3000;
	if (lSpawnTimer > 1) {
		lSpawnTimer -= 1;
		spawnFish();
	}

	mLabel1->setCaption("%zu fishes, FPS:%d, FrameLimiter:%5.2f FrameTime:%8.6f GameTime:%8.6f", mFishes.size(),  getFPS(), mSettings.frameLimiter, getFrameTime(), getGameTime());

	mLabel2->setCaption( "MOUSE x:%d (%d), y:%d (%d)"
	  , getStatus().getMousePosI().x, getStatus().getWorldMousePosI().x
	  , getStatus().getMousePosI().y, getStatus().getWorldMousePosI().y
	);

	// update display fishcount
	for (int i = 0 ; i < FishTypeCount; i++) {
		mFishLabel[i]->setCaption("%4d", mFishCounter[i]);
	}



	static float lScoreTimer = 0.f;
	static float lChallengeTimer = 0.f;

	if (mChallengeRunning)
	{
		lScoreTimer += dt / 200.f;
		if (lScoreTimer > 1) {
			lScoreTimer -= 1;
			mScore += mScoreInc;
			mScoreLabel->setCaption("%06d", mScore);
		}

		lChallengeTimer += dt / 1000.f;
		if (lChallengeTimer > 1) {
			lChallengeTimer -= 1;
			mChallengeTime++;
			mChallengeTimeLabel->setCaption("%06d", mChallengeTime);
		}

	}

	Parent::Update(dt);
}
//--------------------------------------------------------------------------------------
// An optimized version of Bubble Sort
// void DemoGame::sortFishes()
// {
//     int n = mFishes.size();
// 	if (n < 2)
// 		return;
//
//     bool swapped;
//
//     for (int i = 0; i < n - 1; i++) {
//         swapped = false;
//         for (int j = 0; j < n - i - 1; j++) {
//             if (mFishes[j]->getLayer() > mFishes[j + 1]->getLayer()) {
//                 std::swap(mFishes[j], mFishes[j+1] );
//                 swapped = true;
//             }
//         }
//
//         // If no two elements were swapped, then break
//         if (!swapped)
//             break;
//     }
// }
void DemoGame::sortFishes()
{
	// Check size is good practice, but std::sort handles empty/small lists efficiently anyway
	if (mFishes.size() < 2)
		return;

	// Use std::sort to efficiently sort the vector of pointers
	// We use a lambda function to define the custom comparison logic based on getLayer()
	std::sort(mFishes.begin(), mFishes.end(),
			  [](const myFish* a, const myFish* b) {
				  // This returns true if 'a' has a smaller layer than 'b', sorting them in ASCENDING order.
				  return a->getLayer() < b->getLayer();
			  }
	);

	// The vector mFishes is now sorted by layer.
}
//--------------------------------------------------------------------------------------
void DemoGame::onChallengeKey()
{
	if (mChallengeRunning)
	{
		if (mFishGoalReached)
		{
			mChallengeRunning = false;
			for (int i = 0; i < FishTypeCount; i++)
			{
				mFishGoalDisplay[i]->setVisible(false);
			}
			mBrrooii->play();
		} else {
			mFailSound->play();
		}

		// FIXME display fail or win
	} else {
		// new challenge
		mScore = 0;
		mChallengeTime = 0;

		mPlingeling->play();
		respawnFishes(100);
		mFishGoalReached = false;
		for (int i = 0; i < FishTypeCount; i++)
		{
			mFishGoals[i] = RandInt(5,25);
			mFishGoalDisplay[i]->setCaption("%3d", mFishGoals[i]);
			mFishGoalDisplay[i]->setVisible(true);
		}
		updateFishCounter();
		mChallengeRunning = true;
	}
}


//--------------------------------------------------------------------------------------
void DemoGame::onEvent(SDL_Event event)
{
	//TESTING camera
	#if defined(FLUX_DEBUG)

	if (event.type == SDL_EVENT_MOUSE_WHEEL) {
		// Zoom speed is usually much higher for the wheel
		float scrollZoom = event.wheel.y *  0.01f ; // * getFrameTime();
		Render2D.getCamera()->moveZoom(scrollZoom);
	}

	// Testing automove
	// FIXME This is not perfect .. it should test agains which key is down
	// instead of key up
	Point2F lFinaleMoveVector = { 0.f , 0.f };
	if ( event.type ==  SDL_EVENT_KEY_DOWN )
	{
		switch ( event.key.key ) {
			case SDLK_RIGHT: 	lFinaleMoveVector.x = 1.f;; break;
			case SDLK_LEFT: 	lFinaleMoveVector.x = -1.f;; break;
			case SDLK_UP:   	lFinaleMoveVector.y = 1.f;; break;
			case SDLK_DOWN: 	lFinaleMoveVector.y = -1.f;; break;
		}
	}
	if ( event.type ==  SDL_EVENT_KEY_UP )
	{
		switch ( event.key.key ) {
			case SDLK_RIGHT: 	lFinaleMoveVector.x = 0.f;; break;
			case SDLK_LEFT: 	lFinaleMoveVector.x = 0.f;; break;
			case SDLK_UP:   	lFinaleMoveVector.y = 0.f;; break;
			case SDLK_DOWN: 	lFinaleMoveVector.y = 0.f;; break;
		}
	}
	Render2D.getCamera()->setAutoMove(lFinaleMoveVector, 0.1f);

	#endif

}

//--------------------------------------------------------------------------------------
void DemoGame::onKeyEvent(SDL_KeyboardEvent event)
{
	const bool isKeyUp = (event.type == SDL_EVENT_KEY_UP);
	const SDL_Keymod mods = event.mod;
	const SDL_Keycode key = event.key;
	const SDL_Keymod modAlt = SDL_KMOD_LALT;
	const SDL_Keymod modShift = SDL_KMOD_LSHIFT;
	const SDL_Keycode pauseKey = SDLK_P;

	if (isKeyUp){
		switch (key) {
			case SDLK_ESCAPE:
				TerminateApplication();
				break;
			case SDLK_RETURN:
				if (mods & modAlt)
					toggleFullScreen();
				break;
			case pauseKey:
				if (togglePause()) {
					Log("Now paused....");
				}

				else
					Log("pause off");
				break;

			//Launch an finish a challenge
			case SDLK_SPACE:
				if (!getPause())
					onChallengeKey();
				break;
		} //switch

		// debug inputs with left shift key
		if (mods & modShift)
		{
			switch (key) {
				case SDLK_PLUS:
					mSettings.frameLimiter += 1.f;
					break;
				case SDLK_MINUS:
					mSettings.frameLimiter -= 1.f;
					if (mSettings.frameLimiter < 0.f)
						mSettings.frameLimiter = 0.f;
					break;
				case SDLK_RETURN:
					for (Uint32 i = 0; i < 1000; i++)
					{
						spawnFish();
					}
					//disable culling ! :P
					Render2D.setCulling(false);

					break;
				case SDLK_F1:
					listFishes();
					break;
				case SDLK_F2:
					getScreen()->setVSync(!getScreen()->getVsync());
					break;
				case SDLK_F3:
					mLabel1->setVisible(!mLabel1->getVisible());
					mLabel2->setVisible(!mLabel2->getVisible());
					break;

			} //switch
		} //left ctrl
	} //event keyup
}

void DemoGame::onMouseButtonEvent(SDL_MouseButtonEvent event)
{
	const bool isDown = event.down;
	const Uint8 button = event.button;

	if (isDown && !getPause())
	{
		switch (button)
		{
			case SDL_BUTTON_LEFT: {

				// we enabled the container
				if (getQuadTreeObject())
				{
					// 1. Must use FluxRenderObject* because rayCast takes FluxRenderObject*&
					FluxRenderObject* hitObj = nullptr;

					if (getQuadTreeObject()->rayCast(hitObj, getStatus().WorldMousePos.toPoint2I()))
					{
						#ifdef FLUX_DEBUG
							// ****** Scheduler test on deleted object ***** >>>
							myFish* lFish = static_cast<myFish*>(hitObj);
							// it call peep after the fish is deleted or better not ;)
							FluxSchedule.add(2.f, lFish, [lFish]() {
								lFish->peep("2");
							});
							// queueDelete is faster .......
							FluxSchedule.add(0.f, lFish, [lFish]() {
								lFish->peep("0");
							});
							// test "global schedule still works" sending the current fps to it
							FluxSchedule.add(2.0, nullptr, [savedFPS = getFPS()]() {
								_MainPeep_(savedFPS);
							});
							// <<<
						#endif

						// 2. queueDelete likely takes FluxBaseObject*, which works via implicit upcast
						if (!queueDelete(hitObj)) {
							Log("FAILED TO DELETE FROM QUEUE!!!!!!!!!!!!!!!!!");
						} else {
							// 3. Find the object in the vector to get a valid iterator for erase()
							// Cast hitObj to myFish* to match the vector's element type
							auto it = std::find(mFishes.begin(), mFishes.end(), static_cast<myFish*>(hitObj));

							if (it != mFishes.end()) {
								mFishes.erase(it); // erase requires an iterator
								mPling->play();
								updateFishCounter();
							}
						}
					}

				} else {
					sortFishes(); //sort by layer
					static std::vector<myFish *>::iterator curObj;
					for (curObj=mFishes.begin(); curObj!=mFishes.end(); ++curObj) {

						if ((*curObj) && (*curObj)->pointCollide(getStatus().WorldMousePos)) {

							//remove from render/updatequeue and add it to delete
							if (!queueDelete((*curObj)))
								Log("FAILED TO DELETE FROM QUEUE!!!!!!!!!!!!!!!!!");
							else {
								mFishes.erase(curObj); //remove from fishes
								mPling->play();
								updateFishCounter();
							}
							break;
						}
					}
				}
				break;
			} //CASE LEFT
		}
	}
}
//--------------------------------------------------------------------------------------
void DemoGame::onDraw()
{
	// init shared dp
	DrawParams2D dp;
	dp.image = nullptr;
	dp.imgId = 0;
	dp.x = getScreen()->getCenterX();
	dp.y = getScreen()->getCenterY();
	dp.z = 0.f;
	dp.w = getScreen()->getWidth();
	dp.h = getScreen()->getHeight();

	dp.image = mBackTex;		dp.z 	 = 0.99f; dp.alpha=0.1f;	Render2D.drawSprite(dp);
	dp.image = mRockFarTex;		dp.z 	 = 0.50f; dp.alpha=0.1f;	Render2D.drawSprite(dp);
	dp.image = mRockNearTex;	dp.z 	 = 0.05f; dp.alpha=0.1f;	Render2D.drawSprite(dp);

	dp.image = mWaveTex;
	dp.z = 0.75f;
	dp.color = { 1.f,1.f,1.f, 0.75f };
	dp.horizontalScrollSpeed = 20.f;

	Render2D.drawSprite(dp);


}


//--------------------------------------------------------------------------------------
// int main(int argc, char **argv)
int main(int argc, char *argv[]) 
{
#if defined (WIN32) && ! defined (_DEBUG)  
	ShowWindow(GetConsoleWindow(), SW_HIDE);
#endif // def WIN32 and not _DEBUG

	atexit(SDL_Quit);

	DemoGame* lDemoGame = new DemoGame;
	lDemoGame->mSettings.Caption="Fish Tank";
	lDemoGame->mSettings.Version="Demo 4.241221";
	lDemoGame->mSettings.ScreenWidth=1152;
	lDemoGame->mSettings.ScreenHeight=648;
	lDemoGame->mSettings.ScaleScreen = true; //default true
	lDemoGame->mSettings.IconFilename = "assets/icon.bmp";
	lDemoGame->mSettings.CursorFilename = "assets/fishnet.bmp";
	lDemoGame->mSettings.cursorHotSpotX = 11;
	lDemoGame->mSettings.cursorHotSpotY = 3;
	lDemoGame->mSettings.initialVsync = false;

	lDemoGame->mSettings.maxSprites = 100000;

	// testing quadTreee with click!
	// while the quadTree should speed up
	// it's slower because of the updates
	// moving from one container to a other
	lDemoGame->mSettings.useQuadTree = true;


	lDemoGame->Execute();
	SAFE_DELETE(lDemoGame);

	exit(0);
	return 0;
}
