//-----------------------------------------------------------------------------
// Copyright (c) 2012 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#pragma once

#include <SDL3/SDL.h>
#include <string>
#include "../fluxBaseObject.h"


class FluxScene : public FluxBaseObject
{
protected:
	bool mInitialized = false; // for child usage !
	std::string mCaption = "FluxScene";
public:
	void setCaption(const std::string& lCaption) { mCaption = lCaption; }
	std::string getCaption() const { return mCaption; }
	virtual void onEnter() {};
	virtual void onExit() {};

	virtual void onEvent(SDL_Event event) {};
	virtual void onKeyEvent(SDL_KeyboardEvent event) {};
	virtual void onMouseButtonEvent(SDL_MouseButtonEvent event) {};

};
