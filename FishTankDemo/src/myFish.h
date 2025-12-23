//-----------------------------------------------------------------------------
// Copyright (c) 2012..2025 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#ifndef _MYFISH_H_
#define _MYFISH_H_
#include "source/fluxMain.h"
#include "globals.h"

class myFish : public FluxRenderObject
{
	typedef FluxRenderObject Parent;
private:
	// FluxBitmapFont* mLabel;
public:
	int mFishType;


	myFish(FluxTexture* lTex, FluxTexture* lFontTex, FluxScreen* lScreen, int lFishType)
		:FluxRenderObject(lTex, lScreen, 0,3)
	{

		mFishType = lFishType;

		setX((float)RandInt(0,lScreen->getWidth() ));
		setY((float)RandInt(30,lScreen->getHeight()-80));


		setWidth(lTex->getWidth()/8);
		setHeight(lTex->getHeight()/8);
		// to vary for batch rendering: setLayer((float)RandInRange(0.f, 0.11f));
		float randomLayer = (float)RandInRange(0, 10) / 10.0f;
		setLayer(randomLayer);

		if (RandInt(0,1) == 0)
			setFlipX(false);
		else
			setFlipX(true);

		setSpeed ((float)RandInRange(0.01f, 0.06f));

		// Log("Fish (%d) x:%f, y:%f speed:%f",mFishType,  getX(),getY(),getSpeed());

		setAnimationDelay(250);

		// mLabel = new FluxBitmapFont(lFontTex, lScreen);
		// mLabel->setCharSize(13,28);
		// mLabel->setColor(1.f,1.f,1.f,1.f);
		// mLabel->setCaption("%d", mFishType);


	}

	virtual void Update(const double& dt) override
	{
		Parent::Update(dt);

		// 1. Use local variables to avoid multiple getter calls
		float currentX = getX();
		bool isFlipped = getFlipX();
		float speed = getSpeed() * static_cast<float>(dt);
		float screenWidth = static_cast<float>(getScreen()->getWidth());

		// 2. Consolidate movement logic
		currentX += (isFlipped ? -speed : speed);
		setX(currentX);

		// 3. Unified boundary check
		bool outLeft = (isFlipped && currentX < -100.0f);
		bool outRight = (!isFlipped && currentX > screenWidth + 100.0f);

		if (outLeft || outRight)
		{
			setFlipX(!isFlipped);
			setY(static_cast<float>(RandInt(30, getScreen()->getHeight() - 80)));
		}
	}
	// virtual void Update(const double& dt)  override
	// {
	// 	Parent::Update(dt);
	// 	myFish* f = this;
	// 	if (f->getFlipX())
	// 	{
	// 		f->setX(f->getX() - f->getSpeed()*dt);
	// 		if (f->getX() < -100.f) {
	// 			f->setFlipX (!f->getFlipX());
	// 			f->setY((float)RandInt(30,getScreen()->getHeight()-80 ));
	// 	   }
	// 	} else {
	// 		f->setX(f->getX() + f->getSpeed()*dt);
	// 		if (f->getX() > (float)getScreen()->getWidth() + 100.f)
	// 		{
	// 			f->setFlipX(!f->getFlipX());
	// 			f->setY((float)RandInt(30,getScreen()->getHeight()-80 ));
	// 		}
	// 	}
 //
	// } //update)
	virtual void Draw() override
	{
		Parent::Draw();
		// mLabel->setPos(getX(), getY());
		// mLabel->Draw();
	}
	~myFish() {
		// SAFE_DELETE(mLabel);
	}
};
#endif //_MYFISH_H_
