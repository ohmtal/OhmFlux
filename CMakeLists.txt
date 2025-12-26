cmake_minimum_required(VERSION 3.16)
project(ohmFlux LANGUAGES CXX)

# set(CMAKE_CXX_STANDARD 17)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# --- Cross-Platform Compatibility Fixes ---
if(ANDROID)
    add_compile_definitions(ANDROID)
    list(APPEND FLUX_LIBS GLESv3 EGL log android)
elseif(WIN32 AND NOT MSVC)
    # Ensure we link necessary windows libs for MinGW cross-compile
    list(APPEND FLUX_LIBS opengl32 glu32)
endif()

# --- Speed Optimization (LTO) ---
include(CheckIPOSupported)
check_ipo_supported(RESULT ipo_supported)
if(ipo_supported AND NOT ANDROID) # Android NDK handles LTO differently via flags
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)
endif()

add_library(flux_speed_profile INTERFACE)
target_compile_options(flux_speed_profile INTERFACE
    $<$<CONFIG:Release>:
        $<$<CXX_COMPILER_ID:MSVC>:/O2 /Oi /Ot>
        $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>:-O3
            $<$<AND:$<NOT:$<BOOL:${EMSCRIPTEN}>>,$<NOT:$<BOOL:${ANDROID}>>>:-march=native>>
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


# ------------ Dependencies (SDL3 & Graphics) --------------
set(FLUX_LIBS "")

if(EMSCRIPTEN)
    add_compile_options("-sUSE_SDL=3")
    add_link_options("-sUSE_SDL=3" "-sFULL_ES3=1" "-sUSE_WEBGL2=1" "-sALLOW_MEMORY_GROWTH=1")
elseif(ANDROID)
    include(FetchContent)

    FetchContent_Declare(
        SDL3
        GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
        # fixme set a stable tag like release-3.2.28
        GIT_TAG main
    )

    # --- Android Specific Configuration ---
    # SDL3 MUST be SHARED for Android to work with the Java/Kotlin wrapper
    set(SDL_STATIC OFF CACHE BOOL "" FORCE)
    set(SDL_SHARED ON CACHE BOOL "" FORCE)
    set(SDL_TESTS OFF CACHE BOOL "" FORCE)
    set(SDL_INSTALL OFF CACHE BOOL "" FORCE)

    # Enable the SDL3 target for APK generation
    set(SDL_ANDROID_APK ON CACHE BOOL "" FORCE)


    FetchContent_MakeAvailable(SDL3)

    # Link against the SHARED target and required NDK system libs
    list(APPEND FLUX_LIBS SDL3::SDL3-shared GLESv3 EGL log android)


elseif(WIN32 AND NOT MSVC)
    # Force Static C++ Runtime (Fixes previous libstdc++ errors)
    add_link_options("-static-libgcc" "-static-libstdc++" "-static")

    # This is the "Magic" for suppressing the console window in Release
    # but keeping it in Debug so you can see your logs.
    link_libraries("-Wl,--subsystem,windows") # Global for MinGW

    # 1. Force CMake to find .a files only
    set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")

    # 2. SDL3 Static
    set(SDL3_SHARED FALSE CACHE BOOL "" FORCE)
    find_package(SDL3 REQUIRED CONFIG COMPONENTS SDL3-static)

    # 3. GLEW Static - Manual Search to avoid DLL import libs
    # On Arch MinGW, the static lib is usually named 'glew32' or 'glew32s'
    find_library(GLEW_STATIC_LIB NAMES glew32s glew32 glew32.a)

    # 4. Collection of Windows libs
    list(APPEND FLUX_LIBS
        SDL3::SDL3-static
        ${GLEW_STATIC_LIB}
        opengl32 glu32 m dinput8 dxguid dxerr8
        user32 gdi32 winmm imm32 ole32 oleaut32
        shell32 setupapi version uuid
    )
else()
    # Default Desktop (Linux)
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

# Apply Static Interface Properties for Windows/MinGW
if(WIN32 AND NOT MSVC AND NOT EMSCRIPTEN AND NOT ANDROID)
    set_property(TARGET ohmFlux_engine PROPERTY INTERFACE_SDL3_SHARED FALSE)
    target_compile_definitions(ohmFlux_engine PUBLIC GLEW_STATIC)
endif()

target_link_libraries(ohmFlux_engine PUBLIC ${FLUX_LIBS} flux_speed_profile)

# ------------------- Compiler Flags -------------------
# Add debug flag
target_compile_definitions(ohmFlux_engine PUBLIC
    $<$<OR:$<CONFIG:Debug>,$<CONFIG:DEBUG>>:FLUX_DEBUG>
)

# ---------------- Macro for Projects ------------------
# Android and Windows build added:
macro(configure_demo_target TARGET_NAME SOURCE_DIR PRELOAD_PATH SOURCES_LIST)

    # Determine if we need the WIN32 GUI flag (only for Windows Release)
    set(GUI_TYPE "")
    if(WIN32 AND NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(GUI_TYPE "WIN32")
    endif()

    # Android uses shared libraries loaded by a Java/Kotlin wrapper
    if(ANDROID)
        add_library(${TARGET_NAME} SHARED ${SOURCES_LIST})
    else()
        # add_executable(${TARGET_NAME} ${SOURCES_LIST})
         add_executable(${TARGET_NAME} ${GUI_TYPE} ${SOURCES_LIST})
    endif()

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
    elseif(WIN32)
        # Windows standard: target.exe or target_Debug.exe
        # 1. Define the base path
        set(BASE_OUT_DIR "${SOURCE_DIR}")

        # 2. Force MSVC (Visual Studio) to stop adding /Debug or /Release subfolders
        # This has no negative effect on MinGW
        set_target_properties(${TARGET_NAME} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${BASE_OUT_DIR}"
            RUNTIME_OUTPUT_DIRECTORY_DEBUG "${BASE_OUT_DIR}"
            RUNTIME_OUTPUT_DIRECTORY_RELEASE "${BASE_OUT_DIR}"
            RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${BASE_OUT_DIR}"
            RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${BASE_OUT_DIR}"
            
            # 3. Apply your naming logic
            OUTPUT_NAME "$<$<CONFIG:Release>:${TARGET_NAME}>$<$<NOT:$<CONFIG:Release>>:${TARGET_NAME}_$<CONFIG>>"
        )

        # 4. For Visual Studio F5 "Run" support, set the working directory to find assets
        if(MSVC)
            set_target_properties(${TARGET_NAME} PROPERTIES
                VS_DEBUGGER_WORKING_DIRECTORY "${SOURCE_DIR}"
            )
        endif()
	elseif(ANDROID)
        # Android standard: libTARGET_NAME.so
        # No special suffix needed, CMake handles .so for libraries
        set_target_properties(${TARGET_NAME} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${SOURCE_DIR}"
        )
    else()
        # Linux standard (your original preference)
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
    configure_demo_target(TicTacToeDemo ${TTT_DIR} "${TTT_DIR}/assets@assets" "${DEMO_SOURCES}")
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

    target_compile_features(LuaTest PRIVATE cxx_std_17)
    target_compile_definitions(LuaTest PRIVATE SOL_ALL_SAFETIES_ON=1)
endif()
# =============================================================================
# <<<<< PROJECTS SECTION
# =============================================================================
