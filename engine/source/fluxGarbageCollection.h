//-----------------------------------------------------------------------------
// Copyright (c) 2025 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
/*
 * FluxGarbageCollection
 * using FluxBaseObject
 *
 *
 * Example usage:
 * ==============
 * // 1. Setup
 * FluxGarbageCollection gc;
 *
 * // 2. Create different types of objects
 * auto genericObj = std::make_shared<FluxBaseObject>();
 * auto renderObj = std::make_shared<FluxRenderObject>(); // Inherits Base
 * auto fontBtn = std::make_shared<FluxBitmapFont>();     // Inherits RenderObject (and thus Base)
 *
 * // 3. Add them all to the same list
 * gc.add(genericObj);
 * gc.add(render_obj);
 * gc.add(fontBtn);
 *
 * // 4. Manual deletion scenario
 * renderObj.reset(); // Destroy the render object manually
 *
 * // 5. Clean
 * // gc will see renderObj is null/expired and skip it.
 * // it will see genericObj and fontBtn are still alive and handle them.
 * gc.clean();
 *
 */

#include <vector>
#include <memory>
#include <iostream>
#include "fluxBaseObject.h"
#include "errorlog.h"


class FluxGarbageCollection {
private:
    // Holds weak references to the BASE class
    std::vector<std::weak_ptr<FluxBaseObject>> mRegistry;

public:
    // Accepts a shared_ptr of the base OR any child (like FluxRenderObject)
    void add(std::shared_ptr<FluxBaseObject> lObject) {
        if (lObject) mRegistry.push_back(lObject);
    }

    void clean() {
        auto it = mRegistry.begin();
        while (it != mRegistry.end()) {
            // Check if the object still exists in memory
            if (auto sharedRef = it->lock()) {
                // Object is alive.
                // Because sharedRef is a FluxBaseObject*, you can only call
                // methods defined in FluxBaseObject (like virtual destructors).
                it = mRegistry.erase(it);
            } else {
                // Object was deleted elsewhere; remove the "expired" tracker
                it = mRegistry.erase(it);
            }
        }
    }
};
