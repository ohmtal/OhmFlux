//-----------------------------------------------------------------------------
// Copyright (c) 2012..2025 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#pragma once
#ifndef _MAIN_H_
#define _MAIN_H_

#include "source/fluxMain.h"
#include "source/misc.h"
#include "source/errorlog.h"
#include "globals.h"
#include "myFish.h"
#include "GuiFishLabel.h"

class DemoGame : public FluxMain
{
	typedef FluxMain Parent;
private:
	
	FluxTexture* mFishTextures[FishTypeCount];

    FluxTexture* mBackTex;
    FluxTexture* mRockFarTex;
    FluxTexture* mRockNearTex;
	FluxTexture* mWaveTex;

	FluxTexture* mHackNerdTex;
	FluxTexture* mMono16Tex;


	std::vector<myFish*> mFishes;

	Uint32 mFishCount;
	int mFishCounter[FishTypeCount];


	int  mScore;
	int  mScoreInc;
	int  mChallengeTime;
	bool mChallengeRunning;
	bool mFishGoalReached;
	int mFishGoals[FishTypeCount];
	FluxBitmapFont* mScoreLabel;
	FluxBitmapFont* mChallengeTimeLabel;
	FluxBitmapFont* mFishGoalDisplay[FishTypeCount];
	void onChallengeKey();


	FluxBitmapFont* mLabel1;
	FluxBitmapFont* mLabel2;


	GuiFishLabel* mFishLabel[FishTypeCount];

	FluxAudioStream* mPling;
	FluxAudioStream* mPlingeling;
	FluxAudioStream* mBrrooii;
	FluxAudioStream* mFailSound;

	void sortFishes();
public:
	virtual bool Initialize();
	virtual void Deinitialize();
	virtual void onEvent(SDL_Event event);
	virtual void Update(const double& dt);
	virtual void onDraw();
	virtual void onKeyEvent(SDL_KeyboardEvent event);
	virtual void onMouseButtonEvent(SDL_MouseButtonEvent event);
	DemoGame();
	virtual ~DemoGame() { };

	void spawnFish();
	void respawnFishes(int setNewCount = 0);
	void updateFishCounter();

	void listFishes(); //<<debug stuff

};
#endif

