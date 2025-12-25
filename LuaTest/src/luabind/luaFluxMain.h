/*
 * sol2: https://github.com/ThePhD/sol2
 *
 */
#pragma once
#include <fluxMain.h>
#include <sol/sol.hpp>

inline void BindFluxMain(sol::state& lua) {
    // Define the FluxMain class
    lua.new_usertype<FluxMain>("FluxMain",
                               // No constructor needed if it's a singleton managed by C++
                               sol::no_constructor,

                               // Singleton access
                               "Instance", &FluxMain::Instance,

                               // Texture management
                               "loadTexture", &FluxMain::loadTexture,

                               // Application State
                               "terminate", &FluxMain::TerminateApplication,
                               "toggleFullScreen", &FluxMain::toggleFullScreen,
                               "getFPS", &FluxMain::getFPS,

                               // Pause Control
                               "setPause", &FluxMain::setPause,
                               "getPause", &FluxMain::getPause,
                               "togglePause", &FluxMain::togglePause,

                               // Screen and Objects
                               "getScreen", &FluxMain::getScreen,
                               "getQueueObjects", &FluxMain::getQueueObjects, // sol2 handles std::vector automatically
                               "queueDelete", &FluxMain::queueDelete,

                               // Quadtree / Collision
                               "rayCast", &FluxMain::rayCast,
                               "getQuadtree", &FluxMain::GetQuadtree
    );

    lua.new_usertype<FluxSettings>("FluxSettings",
                                   "Caption", &FluxSettings::Caption,
                                   "ScreenWidth", &FluxSettings::ScreenWidth,
                                   "ScreenHeight", &FluxSettings::ScreenHeight,
                                   "FullScreen", &FluxSettings::FullScreen,
                                   "useQuadTree", &FluxSettings::useQuadTree,
                                   "updateDt", &FluxSettings::updateDt
    );

    // 2. Bind the FluxMain class
    lua.new_usertype<FluxMain>("FluxMain",
                               sol::no_constructor,
                               "mSettings", &FluxMain::mSettings, // Links to the struct bound above
                               "Execute", &FluxMain::Execute,
                               "getFPS", &FluxMain::getFPS,
                               "terminate", &FluxMain::TerminateApplication
    );



}
