//-----------------------------------------------------------------------------
// Copyright (c) 2025 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#include "fluxGlobals.h"
#include "fluxMain.h" // Now we can safely include the heavy header

// #include <iostream>
// #include <string>
// #include <algorithm>
// #include <cctype>


FluxScreen* getScreenObject() {
    return FluxMain::Instance()->getScreen();
}

