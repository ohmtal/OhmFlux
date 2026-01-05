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
// JavaScript helper to trigger browser download
extern "C" {
EM_JS(void, emscripten_trigger_download, (const char* name), {
    const filename = UTF8ToString(name);
    const data = FS.readFile(filename); // Read from virtual memory
    const blob = new Blob([data], { type: 'application/octet-stream' });
    const url = URL.createObjectURL(blob);

    const link = document.createElement('a');
    link.href = url;
    link.download = filename;
    link.click();

    // Clean up to prevent memory leaks
    setTimeout(() => URL.revokeObjectURL(url), 10000);
});
}
//-------------------------------------------------------------------------------
// This defines a C function: void loadFileToWasm(int fileHandle, const char* virtualPath)
// Note: In 2026, passing complex JS objects like 'File' directly to EM_JS
// is usually handled by passing a reference or using a global JS object.
extern "C" {
EM_ASYNC_JS(void, loadFileToWasm, (const char* virtualPath), {
    // 1. Get the file from a global or a file input (JS side)
    const fileInput = document.getElementById('myFileInput');
    if (!fileInput.files[0]) return;
    const file = fileInput.files[0];

    // 2. Read the file into an ArrayBuffer
    const buffer = await file.arrayBuffer();
    const data = new Uint8Array(buffer);

    // 3. Write to the virtual filesystem
    // We convert the C string 'virtualPath' to a JS string
    FS.writeFile(UTF8ToString(virtualPath), data);

    console.log("File loaded to:", UTF8ToString(virtualPath));
});
}
// THIS NEED SOMETHING LIKE THAT JavaScript:
// // Example: Processing a user upload
// const fileInput = document.getElementById('myInput');
//
// fileInput.addEventListener('change', async (e) => {
//     const file = e.target.files[0];
//
//     try {
//         // 1. Load the file modularly
//         const path = await loadFileToWasm(file, "temp_input.wav");
//
//         // 2. Call your C++ function via ccall
//         // This C++ function can now safely use: fopen("temp_input.wav", "rb")
//         const result = Module.ccall('process_audio', 'number', ['string'], [path]);
//
//         console.log("Processing finished with code:", result);
//
//         // 3. Optional: Clean up MEMFS to save memory
//         Module.FS.unlink(path);
//
//     } catch (err) {
//         console.error("Failed to load file:", err);
//     }
// });

//-------------------------------------------------------------------------------
// Init:
//-------------------------------------------------------------------------------
inline void initEmScripten()
{
    emscripten_set_fullscreenchange_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, nullptr, EM_TRUE, on_fullscreen_change);
}





#endif // __EMSCRIPTEN__
#endif // _FLUX_ENSCRIPTEN_
