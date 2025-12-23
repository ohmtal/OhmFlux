//-----------------------------------------------------------------------------
// Copyright (c) 2012 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#pragma once
#ifndef _FLUX_MISC_H_
#define _FLUX_MISC_H_

#include "fluxMath.h"
#include <iostream>
#include <string>
#include <algorithm>
#include <cctype>
//-----------------------------------------------------------------------------
extern float gFrameTime;
extern float gGameTime;

inline float getFrameTime() {
	return gFrameTime;
}
inline float getGameTime() {
	return gGameTime;
}
//-----------------------------------------------------------------------------

inline std::string sanitizeFilenameWithUnderScores(std::string name)
{
	std::string result;
	for (unsigned char c : name) {
		if (std::isalnum(c)) {
			result += c;
		} else if (std::isspace(c)) {
			result += '_';
		}
		// Special characters (like '.') are ignored/dropped here
	}
	return result;
}

#endif
