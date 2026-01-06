//-----------------------------------------------------------------------------
// Copyright (c) 2012 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Flux Game Engine
//
// Author: T.Huehn (XXTH)
// Desc  : Basic render object
//-----------------------------------------------------------------------------
#pragma once
#ifndef _FLUXRENDEROBJECT_H_
#define _FLUXRENDEROBJECT_H_

#include <SDL3/SDL.h>
#include "fluxBaseObject.h"
#include "core/fluxTexture.h"
#include "fluxScreen.h"
#include "render/fluxRender2D.h"


// Forward declarations
class FluxQuadtree;
struct FluxNode; // If you made Node a standalone struct or public


class FluxRenderObject : public FluxBaseObject
{

protected:
	// F32 mX,mY,mLayer, mRotation;
	// S32 mWidth, mHeight;
	// FluxTexture* mTexture;
	// bool mFlipX, mFlipY;
	// S32 mFrame; //Textureframe
	// bool mIsGuiElement;
	DrawParams2D mDrawParams;

	// FluxScreen* mScreen;
	F32 mSpeed, mDirX, mDirY;
	S32 mFramesStart, mFramesEnd;
	S32 mAnimationDelay;
	S32 mAnimationTime;

private:
	//quadtree support
	// Use a pointer to an incomplete type.
	// The compiler doesn't need to know the size of the struct to store a poin
	void* myQuadNode = nullptr;

public:

	FluxRenderObject(FluxTexture* lTexture, S32 framesStart = 0, S32 framesEnd = 0)
	// FluxRenderObject(FluxTexture* lTexture, FluxScreen* lScreen, S32 framesStart = 0, S32 framesEnd = 0)
	: // mScreen(lScreen),
	 mSpeed(0.0f)
	, mDirX(0.0f)
	, mDirY(0.0f)
	, mFramesStart(framesStart)
	, mFramesEnd(framesEnd)
	, mAnimationDelay(500)
	, mAnimationTime(0)
	{
		// Set DrawParams defaults
		mDrawParams.image = lTexture;
		mDrawParams.x = 0.f;
		mDrawParams.y = 0.f;
		mDrawParams.z = 0.f;
		mDrawParams.w = 0;
		mDrawParams.h = 0;
		mDrawParams.rotation = 0.f;
		mDrawParams.flipX = false;
		mDrawParams.flipY = false;
		mDrawParams.imgId = 0;
		mDrawParams.isGuiElement = false;
	}

	virtual ~FluxRenderObject();
	FluxScreen* getScreen() { return getScreenObject(); } // only for compat.
	FluxTexture* getTexture() { return mDrawParams.image; }

	DrawParams2D& getDrawParams() { return mDrawParams; }
	const DrawParams2D& getDrawParams() const { return mDrawParams; }

	F32 getX() const {return mDrawParams.x; }
	F32 getY() const {return mDrawParams.y; }
	S32 getWidth() const { return mDrawParams.w; }
	F32 getWidthF() const { return mDrawParams.getWidthF(); }
	S32 getHeight() const { return mDrawParams.h; }
	F32 getHeightF() const { return mDrawParams.getHeightF(); }
	virtual RectI getRectI() const { return mDrawParams.getRectI(); }
	F32 getRotation() const {return mDrawParams.rotation; }
	F32 getLayer() const {return mDrawParams.getLayer(); }
	F32 getSpeed() const { return mSpeed; }
	F32 getDirX() const {return mDirX; }
	F32 getDirY() const {return mDirY; }
	bool  getFlipX() const { return mDrawParams.flipX; }
	bool  getFlipY() const { return mDrawParams.flipY; }
	S32   getFramesStart() const { return mFramesStart; }
	S32   getFramesEnd() const { return mFramesEnd; }
	S32   getAnimationDelay() const { return mAnimationDelay; }
 
	void setX(const F32& x) {
		mDrawParams.x = x;
		//FIXME see below
	}
	void setY(const F32& y) {
		mDrawParams.y = y;
		//FIXME update quadtree here
		// 		<< but then i need to call it twice
	}
	void setSize(const Point2I value) { setWidth(value.x), setHeight(value.y);}
	void setWidth(const S32& w) { mDrawParams.w = w; }
	void setHeight(const S32& h) { mDrawParams.h = h; }
	void setPos(const F32& x, const F32& y) { setX(x); setY(y); }
	void setPos( const Point2F lPos ) { setX(lPos.x); setY(lPos.y); }
	Point2F getPosition() const { return { mDrawParams.x, mDrawParams.y }; }
	void setRotation(const F32& rotation) { mDrawParams.rotation = rotation; }
	void setLayer(const F32& layer) { mDrawParams.z = layer; }
	void setSpeed(const F32& speed) { this->mSpeed = speed; }
	void setDirX(const F32& dirX) { this->mDirX = dirX; }
	void setDirY(const F32& dirY) { this->mDirY = dirY; }
	void setFlipX(const bool& flipX) { mDrawParams.flipX = flipX; }
	void setFlipY(const bool& flipY) { mDrawParams.flipY = flipY; }
	void setFramesStart(const S32& framesStart)  { mFramesStart = framesStart; }
	void setFramesEnd( const S32& framesEnd)  { mFramesEnd = framesEnd; }
	void setAnimationDelay(const S32& animationDelay)  { mAnimationDelay = animationDelay; }
	void setTexture( FluxTexture* texture)  { mDrawParams.image = texture; }

	virtual void setIsGuiElement( bool value ) { mDrawParams.isGuiElement = value; }
	virtual bool getIsGuiElement() { return mDrawParams.isGuiElement; }

	bool pointCollide(const Point2F& lPoint);
	bool pointCollide(const F32& lX, const F32& lY);

	virtual void Update(const double& dt) override;
	virtual void Draw() override;

	//quadtree container
	void setQuadNode(void* node) { myQuadNode = node; }
	void* getQuadNode() const { return myQuadNode; }

};//class renderObject
#endif // #ifndef _FLUXRENDEROBJECT_H_
