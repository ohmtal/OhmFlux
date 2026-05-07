# ---------------------------------------------------------
# LuaTest
#
# LUA Compile and Linking:
# ========================
# I finally added lua and sol2 to include, compile Lua as
# c++ to fix the linker errors. I also generate a lua.hpp
# which include the files directly without "extern C"
# There are some deprecated messages ... I will go over
# this bridge when it's fail.
#
# Why did I change it:
# ====================
# In my previous CMakeList i thought my custom compiled
# lua is used but it was not. After the system lua was
# updated to 5.5x the problems began. Took me some hours
# to get sol2 using the correct headers - i guess the
# problem was that in the lua sources not lua.hpp exists.
# After getting over this the Linker failed. So i changed
# it so CXX and added a lua.hpp.
# ---------------------------------------------------------
set (CUR_PROJECT LuaTest)
set(LUATEST_DIR "${CMAKE_CURRENT_LIST_DIR}")

set(PROJECT_SRC
    ${LUATEST_DIR}/src/main.cpp
    ${LUATEST_DIR}/src/luabind/luaState.cpp
    ${LUATEST_DIR}/src/luabind/luaBindings.cpp
)


# =============== Lua >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
set(lua_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include/lua)


set(LUA_SOURCES
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
    ${lua_SOURCE_DIR}/lundump.c
    ${lua_SOURCE_DIR}/lutf8lib.c
    ${lua_SOURCE_DIR}/lvm.c
    ${lua_SOURCE_DIR}/lzio.c
)

# COMPILE AS CXX to prevent linker errors !!!
set_source_files_properties(${LUA_SOURCES} PROPERTIES LANGUAGE CXX)
add_library(lua STATIC ${LUA_SOURCES})
set_target_properties(lua PROPERTIES LINKER_LANGUAGE CXX)

# --- generate lua.hpp -----
set(LUA_HPP_PATH "${lua_SOURCE_DIR}/lua.hpp")

if(NOT EXISTS "${LUA_HPP_PATH}")
    file(WRITE "${LUA_HPP_PATH}"
        "// XXTH lua.hpp \n"
        "#pragma once \n"
        "#include \"lua.h\"\n"
        "#include \"lualib.h\"\n"
        "#include \"lauxlib.h\"\n"
    )
    message(STATUS "generated: ${LUA_HPP_PATH}")
endif()


# ------- lua compile parameter:
if (WIN32)
  target_compile_definitions(lua PRIVATE LUA_USE_WINDOWS)
endif()

if (UNIX)
  target_compile_definitions(lua PRIVATE LUA_USE_LINUX)
  target_link_libraries(lua PRIVATE dl)
endif()


# <<<<<<<<<<<<<<<<<<<<<<<<<<<< Lua ========================

configure_project_target(${CUR_PROJECT} ${LUATEST_DIR} "${LUATEST_DIR}/assets@assets" "${PROJECT_SRC}")

target_compile_definitions(${CUR_PROJECT} PRIVATE
    SOL_ALL_SAFETIES_ON=1
)

target_include_directories(${CUR_PROJECT} BEFORE PRIVATE
  ${lua_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(${CUR_PROJECT} PRIVATE
    lua
)

