//-----------------------------------------------------------------------------
// Copyright (c) 2012 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------

#include "fluxRenderObject.h"
#include "fluxRender2D.h"

//--------------------------------------------------------------------------------------
void FluxRenderObject::Update(const double& dt)
{

	if (mFramesEnd - mFramesStart > 0){
		mAnimationTime += (int) dt;
		if ( mAnimationTime  > mAnimationDelay){
			if (mDrawParams.imgId + 1 > mFramesEnd)
				mDrawParams.imgId = mFramesStart;
			else
				mDrawParams.imgId++;
			mAnimationTime = 0;
		}
	}
}
//--------------------------------------------------------------------------------------
void FluxRenderObject::Draw()
{
	if (!getVisible())
		return;

	Render2D.drawSprite(mDrawParams);
}

bool FluxRenderObject::pointCollide(const Point2F& lPoint)
{
	return pointCollide(lPoint.x, lPoint.y);
}

bool FluxRenderObject::pointCollide(const F32& lX, const F32& lY)
{
	F32 halfWidth = getWidthF() / 2.f;
	F32 halfHeight =  getHeightF() / 2.f;
 	if (lX >= getX() - halfWidth && lX <= getX()+halfWidth
		&& lY >= getY() - halfHeight && lY <= getY() + halfHeight)
		return true;
	return false;
}
