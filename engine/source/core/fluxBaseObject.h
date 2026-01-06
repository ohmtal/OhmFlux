//-----------------------------------------------------------------------------
// Copyright (c) 2012 Ohmtal Game Studio
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
private:
	bool mVisible;
	bool mScheduleUsed = false;
public:
	FluxBaseObject() {
		mVisible=true;
		mScheduleUsed = false;
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

	void  setVisible(bool value) {mVisible = value;}
	bool  getVisible() {return mVisible; }
};
#endif
