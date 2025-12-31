//-----------------------------------------------------------------------------
// Copyright (c) 2025 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
/**
 * fluxEmscripten
 * special calls for mobile landscape mode
 *
 * SDL3 only ... after i got it work i will not backport to SDL2 for
 * WebGL!!!
 *
 * -------------------------------------------------------------------
 * Compile and test with:
 * bash
 * source /etc/profile.d/emscripten.sh
 * cmake --build ../build/build_web/
 * emrun --no_browser index.html
 * -------------------------------------------------------------------
 *
 * FishTankDemo Testing
 *  BaseScreen is: 1152x648
 *  LandScape given resolution is: 800x360
 *  MouseScale is: 0.69 0.56
 *  SDL_GetMouseState seams to be x/y swapped
 *  NOT x and y is swapped the resolution is swapped :
 *      bottom right is: 360x800
 *  ==> SDL_SetWindowSize(g_FluxScreen->getWindow(), (int)cssW, (int)cssH);
 *          does NOT HELP !!!
 *          ==> because it was not updated, longer delay did help but how long to set .. this sucks
 * i added SDL_SetWindowSize(g_FluxScreen->getWindow(), w, h); to bridge_set_landscape
 * realMouse looks ok and scale too
 *   fixed SDL_EVENT_MOUSE_MOTION, i changed to multiply yesterday which is wrong ..!!
 *
 *
 */
#pragma once

#ifndef _FLUX_ENSCRIPTEN_
#define _FLUX_ENSCRIPTEN_

#ifdef __EMSCRIPTEN__

#include <GLES3/gl3.h>
#include <emscripten.h>
#include <emscripten/html5.h>
#include <emscripten/em_js.h>

#include "utils/errorlog.h"
#include "core/fluxScreen.h"
#include "fluxMain.h"



//-------------------------------------------------------------------------------
// 1. Create a C-bridge so JavaScript can talk to your C++ class
extern "C" {
    EMSCRIPTEN_KEEPALIVE
    void bridge_set_landscape(int w, int h) {
        if (getScreenObject())
        {
            // call window size here to fix "mouse/touch" problems
            SDL_SetWindowSize(getScreenObject()->getWindow(), w, h);
            getScreenObject()->updateWindowSize(w,h);
            dLog("Engine Viewport Updated to Landscape: %d, %d", w,h);
        }
    }
}
//-------------------------------------------------------------------------------
EM_JS(void, unlock_orientation, (), {
    if (screen.orientation && screen.orientation.unlock) {
        screen.orientation.unlock();
    }
});
//-------------------------------------------------------------------------------
EM_JS(void, lock_orientation_landscape, (int targetWidth, int targetHeight), {
    if (screen.orientation && screen.orientation.lock) {
        screen.orientation.lock('landscape-primary').then(function() {
            var canvas = document.getElementById('canvas');
            if (canvas) {
                canvas.width = targetWidth;
                canvas.height = targetHeight;
                canvas.style.width = "100vw";
                canvas.style.height = "100vh";
            }
            // 2. Call the C-bridge using the underscore prefix
            _bridge_set_landscape(targetWidth, targetHeight);

        }).catch(function(error) {
            dLog("Lock failed:%s", error);
        });
    }
});
//-------------------------------------------------------------------------------
void restore_canvas_size()
{
    int w = getScreenObject()->getWidth();
    int h = getScreenObject()->getHeight();

    getScreenObject()->updateWindowSize(w,h);
    dLog("Engine Viewport restored to: %d, %d", w,h);
    emscripten_set_canvas_element_size("#canvas", w, h);

    // Reset CSS so it's not 100vw anymore
    EM_ASM({
        var canvas = document.getElementById('canvas');
        if (canvas) {
            canvas.style.width = $0 + 'px';
    canvas.style.height = $1 + 'px';
        }
    }, w, h);
}
//-------------------------------------------------------------------------------
EM_BOOL on_fullscreen_change(int eventType, const EmscriptenFullscreenChangeEvent *e, void *userData) {
    if (e->isFullscreen)
    {

        // This makes the render normal but Click/Touch does not work
        int sw = e->screenWidth;
        int sh = e->screenHeight;
        if (sw < sh) { // Ensure landscape dimensions
            int temp = sw; sw = sh; sh = temp;
        }
        lock_orientation_landscape(sw, sh);


    } else {
        unlock_orientation();
        restore_canvas_size();
    }
    return EM_TRUE;
}

//-------------------------------------------------------------------------------
// Init:
//-------------------------------------------------------------------------------
inline void initEmScripten()
{
    // g_FluxScreen = lScreen;
    emscripten_set_fullscreenchange_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, nullptr, EM_TRUE, on_fullscreen_change);
}





#endif // __EMSCRIPTEN__
#endif // _FLUX_ENSCRIPTEN_
