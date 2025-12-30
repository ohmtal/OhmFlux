//-----------------------------------------------------------------------------
// Copyright (c) 2012 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#pragma once

#include <SDL3/SDL.h>
#include <string>
#include "../fluxBaseObject.h"
#include "../gui/fluxGuiEventManager.h"


class FluxScene : public FluxBaseObject
{
	typedef FluxBaseObject Parent;
private:
	bool mInitialized = false;
protected:
	std::string mCaption = "FluxScene";
public:
	void setCaption(const std::string& lCaption) { mCaption = lCaption; }
	std::string getCaption() const { return mCaption; }
	virtual void onEnter() {};
	virtual void onExit() {};

	virtual void onEvent(SDL_Event event) {};
	virtual void onKeyEvent(SDL_KeyboardEvent event) {};
	virtual void onMouseButtonEvent(SDL_MouseButtonEvent event) {};

	// new:
	virtual bool OnInitialize() { return true; }
	bool getInitialized() { return mInitialized; }
	//-----------------------------------------------------------------------------
	bool Initialize() override
	{
		if (!Parent::Initialize())
			return false;
		if (mInitialized)
			return true;

		//... load content here ..
		if (!OnInitialize())
			return false;


		mInitialized = true;
		return true;
	}
	//-----------------------------------------------------------------------------
	bool setupClickEvent(FluxGuiEventManager* lEventManager,  FluxRenderObject* lObject, std::function<void()> lOnClick)
	{
		if (!lEventManager || !lObject || !lOnClick)
		{
			Log("setupClickEvent with invalid parameters!!");
			return false;
		}


		lEventManager->bind(lObject, SDL_EVENT_MOUSE_BUTTON_UP, [action = std::move(lOnClick)](const SDL_Event& e) {
			if (e.button.button == SDL_BUTTON_LEFT) {
				action();
			}
		});
		return true;
	}



};
