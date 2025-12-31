//-----------------------------------------------------------------------------
// Copyright (c) 2025 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#pragma once
#ifndef _FLUX_QUADTREE_H
#define _FLUX_QUADTREE_H

#include <vector>
#include "core/fluxGlobals.h"
#include "fluxRenderObject.h" // Assuming this defines your render objects

// Constants
const int MAX_OBJECTS_PER_NODE = 10;
const int MAX_QUADTREE_LEVELS = 5;

class FluxQuadtree {
public:
    // Inner structure representing one node/quadrant in the tree
    struct Node {
        int level;
        RectI bounds;
        Node* parent; // Added for upward traversal
        std::vector<FluxRenderObject*> objects;
        Node* children[4];

        Node(int pLevel, RectI pBounds, Node* pParent = nullptr)
        : level(pLevel), bounds(pBounds), parent(pParent) {
            for (int i = 0; i < 4; ++i) children[i] = nullptr;
        }

        ~Node() {
            for (int i = 0; i < 4; ++i) {
                if (children[i]) delete children[i];
            }
        }
    };

private:
    Node* root;

    // Helper functions for internal logic
    void clear(Node* node);
    void split(Node* node);
    int getIndex(Node* node, FluxRenderObject* obj);
    void insert(Node* node, FluxRenderObject* obj);
    void retrieve(Node* node, std::vector<FluxRenderObject*>& returnObjects, RectI area);

    void rayCastRecursive(Node* node, FluxRenderObject* &bestMatch, const RectI lRect, bool onlyGui);

public:
    FluxQuadtree(RectI worldBounds);
    ~FluxQuadtree();

    // Public API
    void clear();
    void insert(FluxRenderObject* obj);
    std::vector<FluxRenderObject*> retrieve(RectI area);

    // API for the Container Manager
    void updateObject(FluxRenderObject* obj);
    void removeObject(FluxRenderObject* obj);
    void checkAndCollapse(Node* node);

    // cast Ray
    // return the object which is clicked sorted by layer (z)
    bool rayCast(FluxRenderObject* &foundObject, const Point2I& lPos, bool onlyGuiObjects = false);
    // return a list of objects found at this position
    std::vector<FluxRenderObject*> rayCastList(const Point2I& lPos, bool onlyGuiObjects);

};

#endif // _FLUX_QUADTREE_H
