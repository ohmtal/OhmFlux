//-----------------------------------------------------------------------------
// Copyright (c) 2025 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Spatial Grid would be better i guess
//-----------------------------------------------------------------------------

#include "fluxQuadtree.h"
#include "fluxMath.h"
#include "utils/errorlog.h"
#include <iostream>
//------------------------------------------------------------------------------
FluxQuadtree::FluxQuadtree(RectI worldBounds)
{
    dLog("Init FluxQuadtree with rect: %d %d %d %d", worldBounds.x, worldBounds.y, worldBounds.w, worldBounds.h);
    root = new Node(0, worldBounds);
}
//------------------------------------------------------------------------------
FluxQuadtree::~FluxQuadtree() {
    clear();
    delete root; // Root is cleaned by the Node destructor's recursive call
}
//------------------------------------------------------------------------------
// Clears all objects from the tree (used at the start of a frame update)
void FluxQuadtree::clear() {
    clear(root);
}
//------------------------------------------------------------------------------
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
//------------------------------------------------------------------------------
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
//------------------------------------------------------------------------------
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
//------------------------------------------------------------------------------
// Public insert function
void FluxQuadtree::insert(FluxRenderObject* obj) {
    insert(root, obj);
}
//------------------------------------------------------------------------------
// Internal recursive insert function
void FluxQuadtree::insert(Node* node, FluxRenderObject* obj) {
    if (node->children[0]) {
        int index = getIndex(node, obj);
        if (index != -1) {
            insert(node->children[index], obj);
            return;
        }
    }

    node->objects.push_back(obj);
    obj->setQuadNode(node); // Object now knows exactly which node it lives in


    if (node->objects.size() > MAX_OBJECTS_PER_NODE && node->level < MAX_QUADTREE_LEVELS) {
        if (!node->children[0]) {
            split(node);
        }

        int i = 0;
        while (i < node->objects.size()) {
            int index = getIndex(node, node->objects[i]);
            if (index != -1) {
                FluxRenderObject* movingObj = node->objects[i];

                // Remove from current node
                node->objects.erase(node->objects.begin() + i);

                // Re-insert into child (insert will call setQuadNode for the child)
                insert(node->children[index], movingObj);
            } else {
                i++;
            }
        }
    }
}
//------------------------------------------------------------------------------
// Public retrieve function
std::vector<FluxRenderObject*> FluxQuadtree::retrieve(RectI area) {
    std::vector<FluxRenderObject*> foundObjects;
    retrieve(root, foundObjects, area);
    return foundObjects;
}

//------------------------------------------------------------------------------
void FluxQuadtree::retrieve(Node* node, std::vector<FluxRenderObject*>& returnObjects, RectI area) {
    if (node->bounds.intersects(area)) { // Use the new method
        returnObjects.insert(returnObjects.end(), node->objects.begin(), node->objects.end());
        if (node->children[0]) {
            for (int i = 0; i < 4; ++i) retrieve(node->children[i], returnObjects, area);
        }
    }
}

//------------------------------------------------------------------------------
void FluxQuadtree::updateObject(FluxRenderObject* obj)
{
    Node* currentNode = static_cast<Node*>(obj->getQuadNode());

    if (!currentNode) {
        insert(obj);
        return;
    }

    // 1. Check if the object still fits in its current node
    if (currentNode->bounds.contains(obj->getRectI())) {
        // Optimization: It's still inside, but check if it can move DOWN
        // to a child for better precision (optional).
        return;
    }

    // 2. It moved out. Remove it from current node.
    auto& objs = currentNode->objects;
    objs.erase(std::remove(objs.begin(), objs.end(), obj), objs.end());

    // 3. Move UP the tree until we find a parent that contains the new position
    Node* ancestor = currentNode->parent;
    while (ancestor != nullptr && !ancestor->bounds.contains(obj->getRectI())) {
        ancestor = ancestor->parent;
    }

    // 4. Re-insert from the common ancestor (or root if it left the world)
    if (ancestor) {
        insert(ancestor, obj);
    } else {
        insert(root, obj);
    }
}
//------------------------------------------------------------------------------
void FluxQuadtree::removeObject(FluxRenderObject* obj)
{
    Node* currentNode = static_cast<Node*>(obj->getQuadNode());
    if (!currentNode) return;

    // 1. Remove the object from this node's vector
    auto& objs = currentNode->objects;
    objs.erase(std::remove(objs.begin(), objs.end(), obj), objs.end());

    // 2. Clear the object's back-reference to the tree
    obj->setQuadNode(nullptr);

    // 3. Optional: Try to collapse the tree upwards
    checkAndCollapse(currentNode);
}
//------------------------------------------------------------------------------
void FluxQuadtree::checkAndCollapse(Node* node) {
    if (!node || !node->parent || !node->children[0]) return;

    // Count objects in all siblings
    int totalObjects = 0;
    Node* p = node->parent;
    for (int i = 0; i < 4; ++i) {
        if (p->children[i]->children[0]) return; // Can't collapse if children have children
        totalObjects += p->children[i]->objects.size();
    }

    // If total objects fit in one node, pull them up and delete children
    if (totalObjects < MAX_OBJECTS_PER_NODE) {
        for (int i = 0; i < 4; ++i) {
            for (auto* obj : p->children[i]->objects) {
                p->objects.push_back(obj);
                obj->setQuadNode(p);
            }
            delete p->children[i];
            p->children[i] = nullptr;
        }
    }
}
//------------------------------------------------------------------------------
/**
 * return the object which is at lPos sorted by layer (z)
 */


bool FluxQuadtree::rayCast(FluxRenderObject* &foundObject, const Point2I& lPos, bool onlyGui) {
    foundObject = nullptr;
    RectI clickArea = {lPos.x, lPos.y, 1, 1};
    rayCastRecursive(root, foundObject, clickArea, onlyGui);
    return (foundObject != nullptr);
}

void FluxQuadtree::rayCastRecursive(Node* node, FluxRenderObject* &bestMatch, const RectI lRect, bool onlyGui)
{
    // Use intersects to ensure we don't miss clicks on node boundaries
    if (!node || !node->bounds.intersects(lRect)) return;

    Point2I clickPt = lRect.getPoint();

    // 1. Check objects in this node
    for (FluxRenderObject* obj : node->objects) {
        if (onlyGui && !obj->getIsGuiElement()) continue;

        if (obj->getRectI().pointInRect(clickPt)) {
            // FIXED: Use < if Layer 0 is the top-most (front)
            if (!bestMatch || obj->getLayer() < bestMatch->getLayer()) {
                bestMatch = obj;
            }
        }
    }

    // 2. Recurse into children
    if (node->children[0]) { // Only recurse if the node has been split
        for (int i = 0; i < 4; ++i) {
            rayCastRecursive(node->children[i], bestMatch, lRect, onlyGui);
        }
    }
}
//------------------------------------------------------------------------------
/**
 * return a list (vector) of objects found at lPos position
 */
std::vector<FluxRenderObject*> FluxQuadtree::rayCastList(const Point2I& lPos, bool onlyGuiObjects)
{
    std::vector<FluxRenderObject*> hitList;
    RectI clickArea = {lPos.x, lPos.y, 1, 1};

    // Retrieve candidates from Quadtree
    std::vector<FluxRenderObject*> candidates = this->retrieve(clickArea);

    // Iterate and filter
    for (FluxRenderObject* hit : candidates) {
        // Filter by GUI flag if requested
        if (onlyGuiObjects && !hit->getIsGuiElement()) {
            continue;
        }

        // Pixel-perfect point detection
        if (hit->getRectI().pointInRect(lPos)) {
            hitList.push_back(hit);
        }
    }

    return hitList;
}
//------------------------------------------------------------------------------

