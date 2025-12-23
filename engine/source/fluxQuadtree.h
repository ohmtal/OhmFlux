//FIXME !!!!!!!!!!!! need more testing does not work correctly !!!!!!!!!!!

//-----------------------------------------------------------------------------
// Copyright (c) 2025 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#pragma once
#ifndef _FLUX_QUADTREE_H
#define _FLUX_QUADTREE_H

#include <vector>
#include "fluxGlobals.h"
#include "fluxRenderObject.h" // Assuming this defines your render objects

// Constants
const int MAX_OBJECTS_PER_NODE = 10;
const int MAX_QUADTREE_LEVELS = 5;

class FluxQuadtree {
private:
    // Inner structure representing one node/quadrant in the tree
    struct Node {
        int level;
        RectI bounds;
        std::vector<FluxRenderObject*> objects;
        Node* children[4]; // 0: Top-Right, 1: Top-Left, 2: Bottom-Left, 3: Bottom-Right

        Node(int pLevel, RectI pBounds) : level(pLevel), bounds(pBounds) {
            for (int i = 0; i < 4; ++i) children[i] = nullptr;
        }

        ~Node() {
            for (int i = 0; i < 4; ++i) {
                if (children[i]) delete children[i];
            }
        }
    };

    Node* root;

    // Helper functions for internal logic
    void clear(Node* node);
    void split(Node* node);
    int getIndex(Node* node, FluxRenderObject* obj);
    void insert(Node* node, FluxRenderObject* obj);
    void retrieve(Node* node, std::vector<FluxRenderObject*>& returnObjects, RectI area);

public:
    FluxQuadtree(RectI worldBounds);
    ~FluxQuadtree();

    // Public API
    void clear();
    void insert(FluxRenderObject* obj);
    std::vector<FluxRenderObject*> retrieve(RectI area);
};

#endif // _FLUX_QUADTREE_H
