//-----------------------------------------------------------------------------
// Copyright (c) 2012 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#pragma once
//--------------------------- GetScreenObject by global Instance ------------
class FluxScreen; // Forward declaration: No #include needed yet!
FluxScreen* getScreenObject();


class FluxQuadtree;
FluxQuadtree* getQuadTreeObject();
static constexpr auto getContainer = getQuadTreeObject;
