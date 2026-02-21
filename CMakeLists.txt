cmake_minimum_required(VERSION 3.16)
project(ohmFlux LANGUAGES C CXX)

# set(CMAKE_CXX_STANDARD 17)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "No build type selected, defaulting to Debug")
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of build." FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()


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
set(ENGINE_DIR "${CMAKE_CURRENT_LIST_DIR}/engine/source")
set(OPL3_DIR   "${CMAKE_CURRENT_LIST_DIR}/engine/opl3")
set(DSP_DIR    "${CMAKE_CURRENT_LIST_DIR}/engine/dsp")
set(SFX_DIR    "${CMAKE_CURRENT_LIST_DIR}/engine/sfx")

set(STB_DIR    "${CMAKE_CURRENT_LIST_DIR}/lib/stb")
set(BOX2D_DIR  "${CMAKE_CURRENT_LIST_DIR}/lib/box2d")
set(YMFM_DIR   "${CMAKE_CURRENT_LIST_DIR}/lib/ymfm/src")
set(IMGGUI_DIR "${CMAKE_CURRENT_LIST_DIR}/lib/imgui")
set(JSON_DIR   "${CMAKE_CURRENT_LIST_DIR}/lib/json/single_include")
# =============================================================================
# PROJECTS SECTION >>>>
# =============================================================================
set(DEMO_DIR    "${CMAKE_CURRENT_LIST_DIR}/FishTankDemo")
set(TESTBED_DIR "${CMAKE_CURRENT_LIST_DIR}/TestBed")
set(LUATEST_DIR "${CMAKE_CURRENT_LIST_DIR}/LuaTest")
set(AMANA_DIR "${CMAKE_CURRENT_LIST_DIR}/Amana")
set(SOUNDSTUDIO_DIR "${CMAKE_CURRENT_LIST_DIR}/SoundStudio")
set(TRACKER_DIR "${CMAKE_CURRENT_LIST_DIR}/OhmtalTracker")
set(AXE_DIR "${CMAKE_CURRENT_LIST_DIR}/OhmtalAxe")

# =============================================================================
# <<<< PROJECTS SECTION
# =============================================================================


# --- Engine Library Sources ---
set(ENGINE_SOURCES
    ${ENGINE_DIR}/fluxMain.cpp

    ${ENGINE_DIR}/audio/fluxAudioStream.cpp

    ${ENGINE_DIR}/core/fluxGlue.cpp
    ${ENGINE_DIR}/core/fluxScreen.cpp
    ${ENGINE_DIR}/core/fluxTexture.cpp
    ${ENGINE_DIR}/core/fluxRenderObject.cpp
    ${ENGINE_DIR}/core/fluxQuadtree.cpp
    ${ENGINE_DIR}/core/fluxCamera.cpp

    ${ENGINE_DIR}/fonts/fluxBitmapFont.cpp
    ${ENGINE_DIR}/fonts/fluxTrueTypeFont.cpp

    ${ENGINE_DIR}/grid/basicgrid.cpp

    ${ENGINE_DIR}/particle/fluxParticleEmitter.cpp
    ${ENGINE_DIR}/particle/fluxParticleManager.cpp

    ${ENGINE_DIR}/render/fluxMesh.cpp
    ${ENGINE_DIR}/render/fluxRender2D.cpp
    ${ENGINE_DIR}/render/fluxShader.cpp

    ${ENGINE_DIR}/utils/fluxScheduler.cpp
    ${ENGINE_DIR}/utils/errorlog.cpp

    #  --- stb ---
    ${STB_DIR}/flux_stb_impl.cpp

    #  --- ymfm ---
    ${YMFM_DIR}/ymfm_opl.cpp
    ${YMFM_DIR}/ymfm_misc.cpp
    ${YMFM_DIR}/ymfm_adpcm.cpp
    ${YMFM_DIR}/ymfm_pcm.cpp
    ${YMFM_DIR}/ymfm_ssg.cpp

    #  --- opl3 ---
    ${OPL3_DIR}/OPL3Controller.cpp


    #  --- sfx ---
    ${SFX_DIR}/SFXGenerator.cpp
    ${SFX_DIR}/SFXGeneratorStereo.cpp

    #  --- imggui ---
    ${IMGGUI_DIR}/imgui.cpp
    ${IMGGUI_DIR}/imgui_draw.cpp
    ${IMGGUI_DIR}/imgui_widgets.cpp
    ${IMGGUI_DIR}/imgui_tables.cpp
    # also demo ?
    ${IMGGUI_DIR}/imgui_demo.cpp
    ${IMGGUI_DIR}/backends/imgui_impl_sdl3.cpp
    ${IMGGUI_DIR}/backends/imgui_impl_opengl3.cpp

)


# ------------ Dependencies (SDL3 & Graphics) --------------
set(FLUX_LIBS "")

# -- box2D
option(WITH_BOX2D "with BOX2D Library" ON)
if(WITH_BOX2D)
    set(BOX2D_BUILD_UNIT_TESTS OFF CACHE BOOL "" FORCE)
    set(BOX2D_BUILD_TESTBED OFF CACHE BOOL "" FORCE)
    set(BOX2D_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
    # add_subdirectory(engine/box2d)
    add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/lib/box2d" "${CMAKE_CURRENT_BINARY_DIR}/lib/box2d")
    list(APPEND FLUX_LIBS box2d)
endif()
# <<<


if(EMSCRIPTEN)
    add_compile_options("-sUSE_SDL=3")
    # add_link_options("-sUSE_SDL=3" "-sFULL_ES3=1" "-sUSE_WEBGL2=1" "-sALLOW_MEMORY_GROWTH=1")
    add_link_options(
        "-sUSE_SDL=3"
        "-sFULL_ES3=1"
        "-sUSE_WEBGL2=1"
        "-sALLOW_MEMORY_GROWTH=1"
        # for virtual filesystem
        "-sFORCE_FILESYSTEM=1"
        "-sEXPORTED_RUNTIME_METHODS=['FS', 'ccall']"
        "-sEXIT_RUNTIME=1"
    )
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

    #  Force CMake to find .a files only
    set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")

    # SDL3 Static
    set(SDL3_SHARED FALSE CACHE BOOL "" FORCE)
    find_package(SDL3 REQUIRED CONFIG COMPONENTS SDL3-static)

    # GLEW Static - Manual Search to avoid DLL import libs
    # On Arch MinGW, the static lib is usually named 'glew32' or 'glew32s'
    find_library(GLEW_STATIC_LIB NAMES glew32s glew32 glew32.a)

    # Collection of Windows libs
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
    $<BUILD_INTERFACE:${ENGINE_DIR}/utils>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/engine>
    $<BUILD_INTERFACE:${STB_DIR}>
    $<BUILD_INTERFACE:${BOX2D_DIR}/include>
    $<BUILD_INTERFACE:${YMFM_DIR}>
    $<BUILD_INTERFACE:${OPL3_DIR}>
    $<BUILD_INTERFACE:${DSP_DIR}>
    $<BUILD_INTERFACE:${SFX_DIR}>
    $<BUILD_INTERFACE:${IMGGUI_DIR}>
    $<BUILD_INTERFACE:${JSON_DIR}>
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

# compile flags 
target_compile_definitions(ohmFlux_engine PUBLIC FLUX_ENGINE)
target_compile_definitions(ohmFlux_engine PUBLIC IMGUI_DEFINE_MATH_OPERATORS)
target_compile_definitions(ohmFlux_engine PUBLIC _USE_MATH_DEFINES)



if(WIN32 AND MSVC)
    target_compile_definitions(ohmFlux_engine PUBLIC NOMINMAX)
    set_target_properties(ohmFlux_engine PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
	 add_compile_options(/MP)
	# add_compile_options(/permissive- /Zc:__cplusplus)
    
endif()

# ---------------- Macro for Projects ------------------
macro(configure_project_target TARGET_NAME SOURCE_DIR PRELOAD_PATH)
    get_filename_component(ABS_OUT_DIR "${SOURCE_DIR}" ABSOLUTE)

    set(GUI_TYPE "")
    if(WIN32 AND NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(GUI_TYPE "WIN32")
    endif()

    if(ANDROID)
        add_library(${TARGET_NAME} SHARED ${ARGN})
    else()
        add_executable(${TARGET_NAME} ${GUI_TYPE} ${ARGN})
    endif()

    target_include_directories(${TARGET_NAME} PRIVATE ${ABS_OUT_DIR})
    target_link_libraries(${TARGET_NAME} PRIVATE ohmFlux_engine)

    if(EMSCRIPTEN)
        set_target_properties(${TARGET_NAME} PROPERTIES
            SUFFIX ".html"
            OUTPUT_NAME "index"
            RUNTIME_OUTPUT_DIRECTORY "${ABS_OUT_DIR}"
        )

        target_link_options(${TARGET_NAME} PRIVATE
            "-sUSE_SDL=3"
            "-sMAX_WEBGL_VERSION=2"
            "-sMIN_WEBGL_VERSION=2"
            "--preload-file=${PRELOAD_PATH}"
            "-sALLOW_MEMORY_GROWTH=1"
            "-sNO_EXIT_RUNTIME=1"
            "-sMINIFY_HTML=1"
        )

        if(CMAKE_BUILD_TYPE STREQUAL "Release")
            # This is correct: refers to the location of this .cmake file
            set(CUSTOM_SHELL_FILE "${CMAKE_CURRENT_LIST_DIR}/deploy/emscripten/shell_minimal.html")

            if(EXISTS "${CUSTOM_SHELL_FILE}")
                target_link_options(${TARGET_NAME} PRIVATE "--shell-file" "${CUSTOM_SHELL_FILE}")
                set_target_properties(${TARGET_NAME} PROPERTIES LINK_DEPENDS "${CUSTOM_SHELL_FILE}")
            endif()
        endif()

    elseif(WIN32)
        set_target_properties(${TARGET_NAME} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${ABS_OUT_DIR}"
            RUNTIME_OUTPUT_DIRECTORY_DEBUG "${ABS_OUT_DIR}"
            RUNTIME_OUTPUT_DIRECTORY_RELEASE "${ABS_OUT_DIR}"
            # Generator expression for naming
            OUTPUT_NAME "$<$<CONFIG:Release>:${TARGET_NAME}>$<$<NOT:$<CONFIG:Release>>:${TARGET_NAME}_$<CONFIG>>"
        )
        if(MSVC)
            set_target_properties(${TARGET_NAME} PROPERTIES
                MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
                VS_DEBUGGER_WORKING_DIRECTORY "${ABS_OUT_DIR}"
            )
        endif()

    elseif(ANDROID)
        set_target_properties(${TARGET_NAME} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${ABS_OUT_DIR}"
        )
    else()
        # Linux / Others
        set_target_properties(${TARGET_NAME} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${ABS_OUT_DIR}"
            OUTPUT_NAME "$<$<CONFIG:Release>:${TARGET_NAME}.x86_64>$<$<NOT:$<CONFIG:Release>>:${TARGET_NAME}_$<CONFIG>.x86_64>"
        )

        # # build for speed
        # if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        #     set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")
        # endif()
    endif()
endmacro()

macro(configure_demo_target TARGET_NAME SOURCE_DIR PRELOAD_PATH )
    if(NOT DISABLE_DEMOS)  
       configure_project_target(${TARGET_NAME} "${SOURCE_DIR}" "${PRELOAD_PATH}" ${ARGN})
    endif() 
endmacro()


# =============================================================================
# PROJECTS SECTION >>>>>
# =============================================================================

option(DISABLE_DEMOS "Disable all Demos" OFF)


# --- FishTank ---
option(BUILD_FISHTANK "Build FishTank demo" ON)
if(BUILD_FISHTANK)
    set(GAME_SOURCES "${DEMO_DIR}/src/main.cpp")
    configure_demo_target(FishTankDemo ${DEMO_DIR} "${DEMO_DIR}/assets@assets" "${GAME_SOURCES}")
endif()


# --- TestBed ---
option(BUILD_TESTBED "Build TestBed application" ON)
if(BUILD_TESTBED)
    set(GAME_SOURCES "${TESTBED_DIR}/src/main.cpp")
    configure_demo_target(TestBed ${TESTBED_DIR} "${TESTBED_DIR}/assets@assets" "${GAME_SOURCES}")
endif()

# --- LuaTest ---
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
    set(GAME_SOURCES
        ${LUATEST_DIR}/src/main.cpp
        # ${LUATEST_DIR}/src/luabind/luaFluxMain.cpp
    )

    # Configure the target
    configure_demo_target(LuaTest ${LUATEST_DIR} "${LUATEST_DIR}/assets@assets" "${GAME_SOURCES}")

    # Link Lua and sol2 specifically to this target
    # target_link_libraries ensures these are only linked to LuaTest
    target_link_libraries(LuaTest PRIVATE lua sol2::sol2)

    target_compile_features(LuaTest PRIVATE cxx_std_17)
    target_compile_definitions(LuaTest PRIVATE SOL_ALL_SAFETIES_ON=1)
endif()

# --- Amana ---
option(BUILD_AMANA "Build Amana Game" ON)
if(BUILD_AMANA)
    set(GAME_SOURCES
        "${AMANA_DIR}/src/main.cpp"
        "${AMANA_DIR}/src/amanaGame.cpp"
        "${AMANA_DIR}/src/globalResources.cpp"
    )
    configure_demo_target(AmanaGame ${AMANA_DIR} "${AMANA_DIR}/assets@assets" "${GAME_SOURCES}")
endif()



# --- SOUNDSTUDIO ---
option(BUILD_SOUNDSTUDIO "Build SoundStudio application" OFF)
if(BUILD_SOUNDSTUDIO)
    set(APP_SOURCES
        "${SOUNDSTUDIO_DIR}/src/main.cpp"
        "${SOUNDSTUDIO_DIR}/src/appGui.cpp"
        "${SOUNDSTUDIO_DIR}/src/modules/soundMixModule.cpp"
        "${SOUNDSTUDIO_DIR}/src/modules/inputModule.cpp"
    )
    configure_demo_target(SoundStudio ${SOUNDSTUDIO_DIR} "${SOUNDSTUDIO_DIR}/assets@assets" "${APP_SOURCES}")
endif()

# --- Ohmtal Axe ---
option(BUILD_AXE "Build Ohmtal Axe application" ON)
if(BUILD_AXE)
    set(APP_SOURCES
        "${AXE_DIR}/src/main.cpp"
        "${AXE_DIR}/src/appGui.cpp"
        "${AXE_DIR}/src/modules/soundMixModule.cpp"
        "${AXE_DIR}/src/modules/inputModule.cpp"
        "${AXE_DIR}/src/modules/rackModule.cpp"
        "${AXE_DIR}/src/modules/keyboardModule.cpp"
        "${AXE_DIR}/src/modules/drumKitLooper.cpp"
    )
    configure_demo_target(Axe ${AXE_DIR} "${AXE_DIR}/assets@assets" "${APP_SOURCES}")
endif()


# --- Ohmtal Tracker ---
option(BUILD_FLUXTRACKER "Build Ohmtal Tracker" ON)
if(BUILD_FLUXTRACKER)
    set(TRACKER_SOURCES
        "${TRACKER_DIR}/src/main.cpp"
        "${TRACKER_DIR}/src/otGui.cpp"
        "${TRACKER_DIR}/src/dspGui.cpp"
        "${TRACKER_DIR}/src/bankGui.cpp"
        "${TRACKER_DIR}/src/songGui.cpp"
        "${TRACKER_DIR}/src/songFunc.cpp"
        "${TRACKER_DIR}/src/keyboardGui.cpp"
        "${TRACKER_DIR}/src/fancyOverlays.cpp"
        "${TRACKER_DIR}/src/ordersGui.cpp"
    )
    configure_demo_target(Tracker ${TRACKER_DIR} "${TRACKER_DIR}/assets@assets" "${TRACKER_SOURCES}")
endif()


# =============================================================================
# <<<<< PROJECTS SECTION
# =============================================================================
