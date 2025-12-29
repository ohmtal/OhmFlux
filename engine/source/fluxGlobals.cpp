//-----------------------------------------------------------------------------
// Copyright (c) 2025 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#include "fluxGlobals.h"
#include "fluxMain.h" // Now we can safely include the heavy header



FluxScreen* g_CurrentScreen = nullptr;

FluxScreen* getScreenObject() {
    return g_CurrentScreen;
}

FluxQuadtree* g_CurrentQuadTree = nullptr;

FluxQuadtree* getQuadTreeObject()  {
    return g_CurrentQuadTree;
}


