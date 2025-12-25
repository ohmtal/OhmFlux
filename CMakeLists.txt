cmake_minimum_required(VERSION 3.16)
project(ohmFlux LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Speed Optimization (LTO) ---
include(CheckIPOSupported)
check_ipo_supported(RESULT ipo_supported)
if(ipo_supported)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)
endif()

add_library(flux_speed_profile INTERFACE)
target_compile_options(flux_speed_profile INTERFACE
    $<$<CONFIG:Release>:
        $<$<CXX_COMPILER_ID:MSVC>:/O2 /Oi /Ot>
        $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>:-O3 $<$<NOT:$<BOOL:${EMSCRIPTEN}>>:-march=native>>
    >
)

# --- Path Definitions ---
set(ENGINE_DIR  "${CMAKE_CURRENT_SOURCE_DIR}/engine/source")
set(STB_DIR     "${CMAKE_CURRENT_SOURCE_DIR}/engine/stb")
# =============================================================================
# PROJECTS SECTION >>>>
# =============================================================================
set(DEMO_DIR    "${CMAKE_CURRENT_SOURCE_DIR}/FishTankDemo")
set(TTT_DIR     "${CMAKE_CURRENT_SOURCE_DIR}/TickTacToe")
set(TESTBED_DIR "${CMAKE_CURRENT_SOURCE_DIR}/TestBed")
set(LUATEST_DIR "${CMAKE_CURRENT_SOURCE_DIR}/LuaTest")
# =============================================================================
# <<<< PROJECTS SECTION
# =============================================================================

# --- Engine Library Sources ---
set(ENGINE_SOURCES
    ${ENGINE_DIR}/fluxMain.cpp
    ${ENGINE_DIR}/fluxScreen.cpp
    ${ENGINE_DIR}/fluxTexture.cpp
    ${ENGINE_DIR}/errorlog.cpp
    ${ENGINE_DIR}/fluxRenderObject.cpp
    ${ENGINE_DIR}/fluxBitmapFont.cpp
    ${ENGINE_DIR}/fluxAudioStream.cpp
    ${ENGINE_DIR}/fluxQuadtree.cpp
    ${ENGINE_DIR}/fluxRender2D.cpp
    ${ENGINE_DIR}/fluxShader.cpp
    ${ENGINE_DIR}/fluxMesh.cpp
    ${ENGINE_DIR}/fluxCamera.cpp
    ${ENGINE_DIR}/fluxParticleEmitter.cpp
    ${ENGINE_DIR}/fluxParticleManager.cpp
    ${ENGINE_DIR}/fluxTrueTypeFont.cpp
    ${ENGINE_DIR}/fluxGlobals.cpp
    ${ENGINE_DIR}/grid/basicgrid.cpp
    ${STB_DIR}/flux_stb_impl.cpp
)

# --- Dependencies (SDL3 Only) ---
set(FLUX_LIBS "")
if(EMSCRIPTEN)
    add_compile_options("-sUSE_SDL=3")
    add_link_options("-sUSE_SDL=3" "-sFULL_ES3=1" "-sUSE_WEBGL2=1" "-sALLOW_MEMORY_GROWTH=1")
else()
    find_package(SDL3 REQUIRED CONFIG)
    list(APPEND FLUX_LIBS SDL3::SDL3)
    find_package(OpenGL REQUIRED)
    list(APPEND FLUX_LIBS OpenGL::GL)
    find_package(GLEW REQUIRED)
    list(APPEND FLUX_LIBS GLEW::GLEW)
endif()

# --- Engine Target ---
add_library(ohmFlux_engine STATIC ${ENGINE_SOURCES})
target_include_directories(ohmFlux_engine PUBLIC
    $<BUILD_INTERFACE:${ENGINE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/engine>
    $<BUILD_INTERFACE:${STB_DIR}>
)
target_link_libraries(ohmFlux_engine PUBLIC ${FLUX_LIBS} flux_speed_profile)
target_compile_definitions(ohmFlux_engine PUBLIC
    $<$<OR:$<CONFIG:Debug>,$<CONFIG:DEBUG>>:FLUX_DEBUG>
)



# --- Unified Demo Macro ---
macro(configure_demo_target TARGET_NAME SOURCE_DIR PRELOAD_PATH SOURCES_LIST)
    add_executable(${TARGET_NAME} ${SOURCES_LIST})
    target_include_directories(${TARGET_NAME} PRIVATE ${SOURCE_DIR})
    target_link_libraries(${TARGET_NAME} PRIVATE ohmFlux_engine)

    if(EMSCRIPTEN)
        set_target_properties(${TARGET_NAME} PROPERTIES
            SUFFIX ".html" OUTPUT_NAME "index"
            RUNTIME_OUTPUT_DIRECTORY "${SOURCE_DIR}"
        )
        target_link_options(${TARGET_NAME} PRIVATE
            "-sUSE_SDL=3" "-sMAX_WEBGL_VERSION=2" "-sMIN_WEBGL_VERSION=2"
            "--preload-file=${PRELOAD_PATH}" "-sALLOW_MEMORY_GROWTH=1" "-sNO_EXIT_RUNTIME=1" "-sMINIFY_HTML=1"
        )
    else()
        set_target_properties(${TARGET_NAME} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${SOURCE_DIR}"
            OUTPUT_NAME "$<$<CONFIG:Release>:${TARGET_NAME}.x86_64>$<$<NOT:$<CONFIG:Release>>:${TARGET_NAME}_$<CONFIG>.x86_64>"
        )

    endif()
endmacro()

# =============================================================================
# PROJECTS SECTION >>>>>
# =============================================================================
# --- 1. FishTank ---
option(BUILD_FISHTANK "Build FishTank demo" ON)
if(BUILD_FISHTANK)
    set(DEMO_SOURCES "${DEMO_DIR}/src/main.cpp")
    configure_demo_target(FishTankDemo ${DEMO_DIR} "${DEMO_DIR}/assets@assets" "${DEMO_SOURCES}")
endif()

# --- 2. TicTacToe ---
option(BUILD_TICTACTOE "Build TicTacToe demo" ON)
if(BUILD_TICTACTOE)
    set(DEMO_SOURCES "${TTT_DIR}/src/main.cpp")
    configure_demo_target(TicTacToeDemo ${TTT_DIR} "${TTT_DIR}/art@art" "${DEMO_SOURCES}")
endif()

# --- 3. TestBed ---
option(BUILD_TESTBED "Build TestBed application" ON)
if(BUILD_TESTBED)
    set(DEMO_SOURCES "${TESTBED_DIR}/src/main.cpp")
    configure_demo_target(TestBed ${TESTBED_DIR} "${TESTBED_DIR}/assets@assets" "${DEMO_SOURCES}")
endif()

# --- 4. LuaTest ---
# Work in Progess
option(BUILD_LUATEST "Build LuaTest application" OFF)

if(BUILD_LUATEST)
    # Fetch dependencies
    include(FetchContent)

    FetchContent_Declare(
        lua
        GIT_REPOSITORY https://github.com/lua/lua.git
        GIT_TAG v5.4.7
    )

    FetchContent_Declare(
        sol2
        GIT_REPOSITORY https://github.com/ThePhD/sol2.git
        GIT_TAG v3.5.0
    )

    # This command downloads and adds the subprojects to the build tree
    FetchContent_MakeAvailable(lua sol2)

    # Define your sources
    set(DEMO_SOURCES
        ${LUATEST_DIR}/src/main.cpp
        # ${LUATEST_DIR}/src/luabind/luaFluxMain.cpp
    )

    # Configure the target
    configure_demo_target(LuaTest ${LUATEST_DIR} "${LUATEST_DIR}/assets@assets" "${DEMO_SOURCES}")

    # Link Lua and sol2 specifically to this target
    # target_link_libraries ensures these are only linked to LuaTest
    target_link_libraries(LuaTest PRIVATE lua sol2::sol2)

    # sol2 requires C++17 or higher
    target_compile_features(LuaTest PRIVATE cxx_std_17)
    target_compile_definitions(LuaTest PRIVATE SOL_ALL_SAFETIES_ON=1)
endif()
# =============================================================================
# <<<<< PROJECTS SECTION
# =============================================================================
