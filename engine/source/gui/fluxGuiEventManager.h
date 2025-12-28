//-----------------------------------------------------------------------------
// Copyright (c) 2012 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// usage with Lambdas:
//
//   *** W a r n i n g ***
// You need to manually unbind when FluxGuiEvent.target is deleted
// i first tried to change this to shared_ptr but i did not all the changes
// and how to use it.
//
//   *** W a r n i n g  II ***
// Fonts depends on rendered center, because they are shifted to left or right on render !!
// and this function only check against middle = x,y
// FIXME should be rewritten by overwrite getRectI with the parmeter
//
// Standard Click:
//      guiManager.bind(myButton, SDL_EVENT_MOUSE_BUTTON_DOWN, [](const SDL_Event& e){ ... });
// Keyboard Shortcut (when object is active):
//      guiManager.bind(myWindow, SDL_EVENT_KEY_DOWN, [](const SDL_Event& e){ if(e.key.key == SDLK_ESCAPE) close(); });
// Mouse Over (Motion):
//      guiManager.bind(myButton, SDL_EVENT_MOUSE_MOTION, [](const SDL_Event& e){ highlight(); });

//-----------------------------------------------------------------------------
#pragma once

#include <SDL3/SDL.h>
#include <functional>
#include <vector>
#include <algorithm>
#include <map>

#include "../fluxRenderObject.h"

struct FluxGuiEvent {
     FluxRenderObject* target;
    std::map<Uint32, std::function<void(const SDL_Event&)>> callbacks;
};

class FluxGuiEventManager {
    std::vector<FluxGuiEvent> mListeners;

public:
    ~FluxGuiEventManager() { clear();}

    void clear() {
        mListeners.clear();
    }
    //--------------------------------------------------------------------------
    // Bind: Overwrites existing or adds new
    void bind(FluxRenderObject* obj, Uint32 eventType, std::function<void(const SDL_Event&)> callback) {
        if (!obj) return;

        auto it = std::find_if(mListeners.begin(), mListeners.end(),
                               [obj](const FluxGuiEvent& e) { return e.target == obj; });

        if (it != mListeners.end()) {
            it->callbacks[eventType] = callback;
        } else {
            mListeners.push_back({obj, {{eventType, callback}}});
        }
    }
    //--------------------------------------------------------------------------
    // Unbind: Manual cleanup called by object destructor
    void unbind(FluxRenderObject* obj) {
        mListeners.erase(
            std::remove_if(mListeners.begin(), mListeners.end(),
                           [obj](const FluxGuiEvent& e) { return e.target == obj; }),
                         mListeners.end()
        );
    }
    //--------------------------------------------------------------------------
    void onEvent(const SDL_Event& e)
    {
        if (mListeners.empty()) return; // Early exit for safety

        // Use a standard index-based loop if iterators are failing
        for (int i = (int)mListeners.size() - 1; i >= 0; --i)
        {
            auto& listener = mListeners[i];
            if (listener.callbacks.count(e.type))
            {

                bool hit = true;

                // Handle SDL3 float coordinates
                if (e.type >= SDL_EVENT_MOUSE_MOTION && e.type <= SDL_EVENT_MOUSE_WHEEL) {
                    float mx, my;
                    SDL_GetMouseState(&mx, &my);
                    hit = listener.target->getRectI().pointInRect({(int)mx, (int)my});
                }

                if (hit)
                {


                    listener.callbacks[e.type](e);
                    return; // Event consumed
                }
            }
        }
    }
    //--------------------------------------------------------------------------

};



// class FluxGuiEventManager
// {
//     std::vector<FluxGuiEvent> mListeners;
//
// public:
//     ~FluxGuiEventManager() { clear();}
//
//     void clear() {
//         mListeners.clear();
//     }
//
//     // Bind any SDL event type to a callback for a specific object
//     void bind(std::shared_ptr<FluxRenderObject> obj, Uint32 eventType, std::function<void(const SDL_Event&)> callback) {
//         auto it = std::find_if(mListeners.begin(), mListeners.end(),
//                                [&obj](const FluxGuiEvent& e) { return e.target.lock() == obj; });
//
//         if (it != mListeners.end()) {
//             it->callbacks[eventType] = callback;
//         } else {
//             FluxGuiEvent newEvent;
//             newEvent.target = obj;
//             newEvent.callbacks[eventType] = callback;
//             mListeners.push_back(newEvent);
//         }
//     }
//
//     void onEvent(const SDL_Event& e) {
//         // Top-to-bottom iteration for GUI
//         for (auto it = mListeners.rbegin(); it != mListeners.rend(); ) {
//             if (auto obj = it->target.lock()) {
//
//                 // 1. Check if this listener cares about this specific event type
//                 if (it->callbacks.count(e.type)) {
//
//                     // 2. Spatial check: If it's a mouse event, is it inside the object?
//                     bool isMouseEvent = (e.type >= SDL_EVENT_MOUSE_MOTION && e.type <= SDL_EVENT_MOUSE_WHEEL);
//                     bool hit = true;
//
//                     if (isMouseEvent) {
//                         float mx, my;
//                         SDL_GetMouseState(&mx, &my); // SDL3 uses floats for coordinates
//                         hit = obj->getRectI().pointInRect({(int)mx, (int)my});
//                     }
//
//                     if (hit) {
//                         it->callbacks[e.type](e); // Execute the bound callback
//                         return; // Consume the event
//                     }
//                 }
//                 ++it;
//             } else {
//                 // Automatic cleanup of deleted objects
//                 auto baseIt = std::next(it).base();
//                 mListeners.erase(baseIt);
//                 it = std::vector<FluxGuiEvent>::reverse_iterator(baseIt);
//             }
//         }
//     }
//
// };
