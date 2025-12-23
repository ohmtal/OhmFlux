//-----------------------------------------------------------------------------
// Copyright (c) 2012 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#pragma once
#ifndef _FLUX_MISC_H_
#define _FLUX_MISC_H_

#include "fluxMath.h"

extern float gFrameTime;
extern float gGameTime;

inline float getFrameTime() {
	return gFrameTime;
}
inline float getGameTime() {
	return gGameTime;
}


#endif
