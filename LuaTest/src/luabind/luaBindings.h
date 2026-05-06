/*
 * sol2: https://github.com/ThePhD/sol2
 *
 */
#pragma once

#define SOL_ALL_SAFETIES_ON 1
#define SOL_LUA_VERSION 504
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <sol/sol.hpp>



namespace OhmFlux::Lua {

    void bindFluxBaseObject(sol::state& lua);
    void bindFluxMain(sol::state& lua);
    void bindFluxTexture(sol::state& lua);
    void bindDrawParams2D(sol::state& lua);
    void bindFluxRenderObject(sol::state& lua);
    void bindFluxScreen(sol::state& lua);
    void bindFluxFonts(sol::state& lua);
    void bindFluxAudioStream(sol::state& lua);
    void bindSDLEvents(sol::state& lua);
    void bindConstants(sol::state& lua);

}; //namespace

