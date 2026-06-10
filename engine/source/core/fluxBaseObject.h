//-----------------------------------------------------------------------------
// Copyright (c) 2012 Thomas Hühn (XXTH) 
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Flux Game Engine
//
// Author: T.Huehn (XXTH)
// Desc  : Base class for inheritance
//-----------------------------------------------------------------------------
#pragma once
#ifndef _FLUXBASE_H_
#define _FLUXBASE_H_

#include "core/fluxGlobals.h"
#include "utils/fluxScheduler.h"



class FluxBaseObject
{
protected:
	bool mVisible = true;
	bool mScheduleUsed = false;
	bool mEventListener = false;

public:
	FluxBaseObject() {
		mVisible=true;
		mScheduleUsed = false;
		mEventListener = false;
	};

	void setScheduleUsed( bool value = true )  { mScheduleUsed = value; }

	virtual ~FluxBaseObject() {
		if ( mScheduleUsed )
		{
			mScheduleUsed = false;
			FluxSchedule.cleanByOwner(this);
		}
	}

	virtual void Execute() {};
	virtual bool Initialize() { return true;};
	virtual void Deinitialize() {};
	virtual void Update(const double& dt) {};
	virtual void Draw() {};
	virtual void onEvent(SDL_Event event) {};

	void  setVisible(bool value) {mVisible = value;}
	bool  getVisible() {return mVisible; }


	bool  isEventListener() {return mEventListener; }

	bool mAutoDelete = true;
};
#endif
