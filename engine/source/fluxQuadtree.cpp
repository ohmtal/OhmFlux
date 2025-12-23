//FIXME !!!!!!!!!!!! need more testing does not work correctly !!!!!!!!!!!

//-----------------------------------------------------------------------------
// Copyright (c) 2025 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------

#include "fluxQuadtree.h"
#include "fluxMath.h"
#include "errorlog.h"
#include <iostream>

FluxQuadtree::FluxQuadtree(RectI worldBounds)
{
    dLog("Init FluxQuadtree with rect: %d %d %d %d", worldBounds.x, worldBounds.y, worldBounds.w, worldBounds.h);
    root = new Node(0, worldBounds);
}

FluxQuadtree::~FluxQuadtree() {
    clear();
    delete root; // Root is cleaned by the Node destructor's recursive call
}

// Clears all objects from the tree (used at the start of a frame update)
void FluxQuadtree::clear() {
    clear(root);
}

void FluxQuadtree::clear(Node* node) {
    node->objects.clear();
    for (int i = 0; i < 4; ++i) {
        if (node->children[i]) {
            clear(node->children[i]);
            delete node->children[i];
            node->children[i] = nullptr;
        }
    }
}

// Splits a node into four children nodes
void FluxQuadtree::split(Node* node) {
    int subWidth = node->bounds.w / 2;
    int subHeight = node->bounds.h / 2;
    int x = node->bounds.x;
    int y = node->bounds.y;
    int level = node->level + 1;
    int subWidth2 = node->bounds.w - subWidth;
    int subHeight2 = node->bounds.h - subHeight;

    node->children[0] = new Node(level, {x + subWidth, y, subWidth2, subHeight});        // Top-Right
    node->children[1] = new Node(level, {x, y, subWidth, subHeight});                    // Top-Left
    node->children[2] = new Node(level, {x, y + subHeight, subWidth, subHeight2});       // Bottom-Left
    node->children[3] = new Node(level, {x + subWidth, y + subHeight, subWidth2, subHeight2}); // Bottom-Right

}

// Determines which child node an object belongs to (-1 if it spans multiple nodes or stays in the parent)
int FluxQuadtree::getIndex(Node* node, FluxRenderObject* obj) {
    int index = -1;
    double verticalMidpoint = node->bounds.x + (node->bounds.w / 2);
    double horizontalMidpoint = node->bounds.y + (node->bounds.h / 2);

    bool topQuadrant = (obj->getY() < horizontalMidpoint && obj->getY() + obj->getHeight() < horizontalMidpoint);
    bool bottomQuadrant = (obj->getY() > horizontalMidpoint);

    if (obj->getX() < verticalMidpoint && obj->getX() + obj->getWidth() < verticalMidpoint) { // Left side
        if (topQuadrant) index = 1;
        else if (bottomQuadrant) index = 2;
    } else if (obj->getX() > verticalMidpoint) { // Right side
        if (topQuadrant) index = 0;
        else if (bottomQuadrant) index = 3;
    }
    // If index is -1, the object spans boundary lines and stays in the current node.

    return index;
}

// Public insert function
void FluxQuadtree::insert(FluxRenderObject* obj) {
    insert(root, obj);
}

// Internal recursive insert function
void FluxQuadtree::insert(Node* node, FluxRenderObject* obj) {
    if (node->children[0]) { // If the node has been split
        int index = getIndex(node, obj);

        if (index != -1) {
            insert(node->children[index], obj); // Insert into the child
            return;
        }
    }

    node->objects.push_back(obj); // Object stays in this node

    // If we exceed capacity and haven't reached max depth, split the node and redistribute
    if (node->objects.size() > MAX_OBJECTS_PER_NODE && node->level < MAX_QUADTREE_LEVELS) {
        if (!node->children[0]) {
            split(node);
        }

        // Re-distribute existing objects to new children
        int i = 0;
        while (i < node->objects.size()) {
            int index = getIndex(node, node->objects[i]);
            if (index != -1) {
                insert(node->children[index], node->objects[i]);
                node->objects.erase(node->objects.begin() + i); // Remove from parent
            } else {
                i++; // Keep it in the parent node if it spans boundaries
            }
        }
    }
}

// Public retrieve function
std::vector<FluxRenderObject*> FluxQuadtree::retrieve(RectI area) {
    std::vector<FluxRenderObject*> foundObjects;
    retrieve(root, foundObjects, area);
    return foundObjects;
}

// Internal recursive retrieve function
void FluxQuadtree::retrieve(Node* node, std::vector<FluxRenderObject*>& returnObjects, RectI area)
{

    // Simple AABB intersection check for retrieve
    if (checkAABBIntersectionI(node->bounds, area)) {
        // Add objects stored directly in this node
        returnObjects.insert(returnObjects.end(), node->objects.begin(), node->objects.end());

        // If split, check children recursively
        if (node->children[0]) {
            for (int i = 0; i < 4; ++i) {
                retrieve(node->children[i], returnObjects, area);
            }
        }
    }
}
