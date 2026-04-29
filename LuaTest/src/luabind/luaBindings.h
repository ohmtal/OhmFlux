/*
 * sol2: https://github.com/ThePhD/sol2
 *
 */
#pragma once
#include <sol/sol.hpp>

namespace OhmFlux::Lua {

    void bindFluxBaseObject(sol::state& lua);
    void bindFluxMain(sol::state& lua);
    void bindFluxTexture(sol::state& lua);
    void bindDrawParams2D(sol::state& lua);
    void bindFluxRenderObject(sol::state& lua);
    void bindFluxScreen(sol::state& lua);
    void bindFluxBitmapFont(sol::state& lua);
    void bindFluxTrueTypeFont(sol::state& lua);
    void bindFluxAudioStream(sol::state& lua);
    void bindSDLEvents(sol::state& lua);
    void bindConstants(sol::state& lua);

}; //namespace

