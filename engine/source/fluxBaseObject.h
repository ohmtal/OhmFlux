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

#include "fluxGlobals.h"


class FluxBaseObject
{
private:
	bool mVisible;
public:
	FluxBaseObject() { mVisible=true;};
	virtual ~FluxBaseObject() {};
	virtual void Execute() {};
	virtual bool Initialize() { return true;};
	virtual void Deinitialize() {};
	virtual void Update(const double& dt) {};
	virtual void Draw() {};

	void  setVisible(bool value) {mVisible = value;}
	bool  getVisible() {return mVisible; }
};
#endif
