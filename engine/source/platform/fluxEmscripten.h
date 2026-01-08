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
#include "SDL3/SDL.h"


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
// 2026-01-08 >>>>

//-----------------------------
void sync_opengl_high_dpi(SDL_Window* window) {
    // 1. Query the browser directly for the viewport size
    double browser_w, browser_h;
    emscripten_get_element_css_size("#canvas", &browser_w, &browser_h);

    // 2. If browser reports 0, we are too early; retry in 50ms
    if (browser_w <= 0 || browser_h <= 0) {
        emscripten_async_call([](void* arg) { sync_opengl_high_dpi((SDL_Window*)arg); }, window, 50);
        return;
    }

    // 3. Get physical pixel scale (DPR)
    double dpr = emscripten_get_device_pixel_ratio();
    int phys_w = (int)(browser_w * dpr);
    int phys_h = (int)(browser_h * dpr);

    // 4. FORCE the HTML Canvas element to have the correct physical pixels
    // Without this, OpenGL renders into a tiny backbuffer and stretches it (blur/bottom-left)
    emscripten_set_canvas_element_size("#canvas", phys_w, phys_h);

    getScreenObject()->updateWindowSize(phys_w, phys_h);

    // Log to confirm (it should now show ~1080x2400)
    Log("New Resolution: %dx%d (DPR: %f)", phys_w, phys_h, dpr);
}
//-----------------------------

EM_BOOL on_fullscreen_change_attempt2(int eventType, const EmscriptenFullscreenChangeEvent *e, void *userData) {
    SDL_Window* window = (SDL_Window*)userData;

    if (e->isFullscreen) {
        // 1. Enter fullscreen via SDL3
        SDL_SetWindowFullscreen(window, true);

        emscripten_async_call([](void* arg) {
            sync_opengl_high_dpi((SDL_Window*)arg);

        }, window, 250);

    } else {
        SDL_SetWindowFullscreen(window, false);
    }
    return EM_TRUE;
}


// EM_BOOL on_fullscreen_change_attempt2(int eventType, const EmscriptenFullscreenChangeEvent *e, void *userData) {
//     if (e->isFullscreen) {
//         SDL_SetWindowFullscreen(getScreenObject()->getWindow(), true);
//
//         // Mobile browsers need a tiny moment to finish the rotation animation
//         // 100ms is the standard 'stabilization' delay for 2026 mobile web apps
//         emscripten_async_run_in_main_runtime_thread(EM_FUNC_SIG_V, (void*)window, [](void* arg) {
//             SDL_Window* win = (SDL_Window*)arg;
//             int w, h;
//
//             // Get the REAL physical pixel count (DPR included)
//             SDL_GL_GetDrawableSize(win, &w, &h);
//
//             // Update the raw OpenGL Viewport
//             glViewport(0, 0, w, h);
//
//             // If you have a custom projection matrix (e.g., ortho or perspective),
//             // update it here to match the new aspect ratio (w/h).
//             getScreenObject()->updateWindowSize(w,h);
//
//         }, 100);
//     }
//     return EM_TRUE;_WI
//
// }
//

//-----------------------------
void sync_canvas_size() {
    double width, height;
    emscripten_get_element_css_size("#canvas", &width, &height);
    double dpr = emscripten_get_device_pixel_ratio();
    emscripten_set_canvas_element_size("#canvas", (int)(width * dpr), (int)(height * dpr));
}

//-----------------------------

EM_BOOL on_orientation_change(int eventType, const EmscriptenOrientationChangeEvent *orientationEvent, void *userData) {
    sync_canvas_size();
    return EM_TRUE;
}
//-----------------------------

EM_BOOL on_resize(int eventType, const EmscriptenUiEvent *uiEvent, void *userData) {
    sync_canvas_size();
    return EM_TRUE;
}
//<<<<<
//-------------------------------------------------------------------------------
// JavaScript helper to trigger browser download
EM_JS(void, js_impl_download, (const char* name), {
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
//-------------------------------------------------------------------------------
// This defines a C function: void loadFileToWasm(int fileHandle, const char* virtualPath)
// Note: In 2026, passing complex JS objects like 'File' directly to EM_JS
// is usually handled by passing a reference or using a global JS object.
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

    //2026-01-08
    // sucks!
    // emscripten_set_fullscreenchange_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, nullptr, EM_TRUE, on_fullscreen_change_attempt2);
    // emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, EM_FALSE, on_resize);
    // emscripten_set_orientationchange_callback(nullptr, EM_FALSE, on_orientation_change);


}

extern "C" {
    void emscripten_trigger_download(const char* name) {
        js_impl_download(name);
    }
}





#endif // __EMSCRIPTEN__
#endif // _FLUX_ENSCRIPTEN_
