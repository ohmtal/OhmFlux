# !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
# After my Linux updated to Lua 5.5 i noticed sol2 does not use the self
# compiled lua and with a hackfest i was able to compile but not to link
# - guess less is more! I'll consider bind directly agaist lua'
# !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!


set(LUATEST_DIR "${CMAKE_CURRENT_LIST_DIR}")

set(GAME_SOURCES
    ${LUATEST_DIR}/src/main.cpp
    ${LUATEST_DIR}/src/luabind/luaState.cpp
    ${LUATEST_DIR}/src/luabind/luaBindings.cpp
)


# ------------- Lua + Sol2
include(FetchContent)


FetchContent_Declare(
    lua
    GIT_REPOSITORY https://github.com/lua/lua.git
    GIT_TAG v5.4.7
)

FetchContent_MakeAvailable(lua)


FetchContent_Declare(
    sol2
    GIT_REPOSITORY https://github.com/ThePhD/sol2.git
    GIT_TAG v3.5.0
)


set(LUA_INCLUDE_DIR "${lua_SOURCE_DIR}" CACHE INTERNAL "")
set(LUA_LIBRARIES lua CACHE INTERNAL "")
set(LUA_FOUND TRUE CACHE INTERNAL "")

set(SOL2_LUA_VERSION "5.4.7" CACHE INTERNAL "")
set(SOL2_BUILD_LUA OFF CACHE INTERNAL "")

FetchContent_MakeAvailable(sol2)

# set(SOL2_BUILD_LUA OFF CACHE BOOL "" FORCE)
# set(SOL2_LUA_VERSION "5.4.7" CACHE STRING "" FORCE)
#
# set(Lua_FOUND TRUE CACHE BOOL "" FORCE)
# set(LUA_LIBRARIES lua CACHE STRING "" FORCE)
# set(LUA_INCLUDE_DIR "${lua_SOURCE_DIR}" CACHE PATH "" FORCE)
#
# FetchContent_MakeAvailable(sol2)
#


### lua
add_library(lua STATIC
  ${lua_SOURCE_DIR}/lapi.c
  ${lua_SOURCE_DIR}/lauxlib.c
  ${lua_SOURCE_DIR}/lbaselib.c
  ${lua_SOURCE_DIR}/lcode.c
  ${lua_SOURCE_DIR}/lcorolib.c
  ${lua_SOURCE_DIR}/lctype.c
  ${lua_SOURCE_DIR}/ldblib.c
  ${lua_SOURCE_DIR}/ldebug.c
  ${lua_SOURCE_DIR}/ldo.c
  ${lua_SOURCE_DIR}/ldump.c
  ${lua_SOURCE_DIR}/lfunc.c
  ${lua_SOURCE_DIR}/lgc.c
  ${lua_SOURCE_DIR}/linit.c
  ${lua_SOURCE_DIR}/liolib.c
  ${lua_SOURCE_DIR}/llex.c
  ${lua_SOURCE_DIR}/lmathlib.c
  ${lua_SOURCE_DIR}/lmem.c
  ${lua_SOURCE_DIR}/loadlib.c
  ${lua_SOURCE_DIR}/lobject.c
  ${lua_SOURCE_DIR}/lopcodes.c
  ${lua_SOURCE_DIR}/loslib.c
  ${lua_SOURCE_DIR}/lparser.c
  ${lua_SOURCE_DIR}/lstate.c
  ${lua_SOURCE_DIR}/lstring.c
  ${lua_SOURCE_DIR}/lstrlib.c
  ${lua_SOURCE_DIR}/ltable.c
  ${lua_SOURCE_DIR}/ltablib.c
  ${lua_SOURCE_DIR}/ltests.c
  ${lua_SOURCE_DIR}/ltm.c
  # ${lua_SOURCE_DIR}/lua.c
  ${lua_SOURCE_DIR}/lundump.c
  ${lua_SOURCE_DIR}/lutf8lib.c
  ${lua_SOURCE_DIR}/lvm.c
  ${lua_SOURCE_DIR}/lzio.c
)

if (WIN32)
  target_compile_definitions(lua PRIVATE LUA_USE_WINDOWS)
endif()

if (UNIX)
  target_compile_definitions(lua PRIVATE LUA_USE_LINUX)
  target_link_libraries(lua PRIVATE dl)
endif()

target_include_directories(lua PUBLIC ${lua_SOURCE_DIR})
# add_library(lua::lua ALIAS lua)
target_compile_definitions(lua INTERFACE
    SOL_USING_CXX_LUA=0
    LUA_VERSION_NUM=504
)

# -------------------------------

configure_project_target(LuaTest ${LUATEST_DIR} "${LUATEST_DIR}/assets@assets" "${GAME_SOURCES}")

target_compile_definitions(LuaTest PRIVATE
    SOL_USING_CXX_LUA=0
    SOL_ALL_SAFETIES_ON=1
    SOL_CHECK_ARGUMENTS=1
    SOL_LUA_VERSION=504
)

target_include_directories(LuaTest BEFORE PRIVATE
    ${lua_SOURCE_DIR}
    ${sol2_SOURCE_DIR}/include
)

target_link_libraries(LuaTest PRIVATE
    lua
    sol2::sol2
)

