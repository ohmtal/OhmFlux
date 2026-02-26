# OhmFlux Game Engine

A lightweight, high-performance,  modern C++ game engine With ongoing optimizations for current development standards. Built around **SDL3**, it provides a streamlined pipeline for 2D graphics, particle simulations, and cross-platform deployment.

## 💻 Platforms
- **Desktop**: tested on Linux. FreeBSD and Windows 
- **WebAssembly**: using Emscripten

---

## 🚀 Features
- **Strict SDL3 Architecture**: Fully migrated from SDL2 for modern memory management and advanced pixel handling.
- **Rendering**: inline shaders for Primitives and Sprites using batch rendering.
- **Tilemap**: methods for loading and saving tilemaps from filesystem.
- **Header-Only Loaders**: Native PNG, JPG, and TGA support via `stb_image` and TrueType font support via `stb_truetype`. 
- **Advanced Particle System**: High-performance "Swap-and-Pop" particle management with dedicated presets for Fire, Explosions, and Sparks and some more. 
- **Hybrid Build System**: Unified CMake configuration for Native Desktop (OpenGL/GLEW) and WebAssembly (WebGL 2.0).
- **TrueType fonts**:  High-performance text rendering via stb_truetype with dynamic atlas baking, proportional kerning, and batch-optimized UV mapping.
- **Audio**: High-performance SDL3 streaming with OGG/WAV support, 2D spatial attenuation, and distance-based CPU culling.
- **Lights**: on the TODO list.
- **Posteffects**:on the TODO list.

---

## 🕹 Included Demos
- **FishTankDemo**: A stress test for high-entity counts. Press `Space` to start; reach the fish target and press `Space` again to finish.
- **TestBed**: A sandbox environment for testing new engine features and particle presets.

---

## 📎 Libraries used in this project: 
- OpenGL
- Glew
- SDL3
- ImGui
- Box2D
- nlohmann json 
- stb
- ymfm

---

## 🏗 Build Instructions (Native Desktop)

Requires a C++20 compiler, **SDL3**, and **GLEW**.

```shell
# 1. Configure the project
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release

# 2. Build everything
cmake --build build --config Release
```


For Windows-build see also: [README_BUILD_WINDOWS.md](README_BUILD_WINDOWS.md).


---

## 🌐 Build with Emscripten (Web)

Optimized for Arch Linux and systems with the Emscripten SDK.

```shell 
# 1. Setup environment (Standard Arch Linux path)
source /etc/profile.d/emscripten.sh

# 2. Configure with Emscripten Toolchain
cmake -DCMAKE_TOOLCHAIN_FILE=/usr/lib/emscripten/cmake/Modules/Platform/Emscripten.cmake -S . -B build_web

# 3. Build
cmake --build build_web

# 4. Run a demo locally
cd FishTankDemo
emrun index.html .
``` 

---

## 🏗 OhmFlux Build System via Make 

For lazy people like me I added a Makefile :


### Build Commands

| Command              | Description                                                |
|----------------------|------------------------------------------------------------|
| **make debug**      | Build native Desktop (Debug)                               |
| **make release**    | Build native Desktop (Release)                             |
| **make webdebug**   | Build WebGL (Debug)                                       |
| **make webrelease** | Build WebGL (Release)                                     |
| **make webdist**    | Build WebGL Release and deploy to `$(WEBDIST_DIR)`       |
| **make clean**      | Remove build artifacts                                     |
| **make distclean**  | Remove all build artifacts, logs, binaries, and `$(WEBDIST_DIR)` |


---

## ➕ Adding a New Project

The build system uses a unified macro to make adding new games extremely fast.

1. **Create your source**: Create `MyGame/src/main.cpp` and place assets in `MyGame/assets/`.
2. **Define path**: In `CMakeLists.txt`, add under the "Directories" section:
   ```cmake
   set(MYGAME_DIR "${CMAKE_CURRENT_SOURCE_DIR}/MyGame")
   ``` 
3. Copy-Paste Project Block: Append this to the bottom of the file:

    ```cmake
    option(BUILD_MYGAME "Build MyGame demo" ON)
    if(BUILD_MYGAME)
        set(GAME_SOURCES "${MYGAME_DIR}/src/main.cpp") 
        configure_demo_target(MyGame ${MYGAME_DIR} "${MYGAME_DIR}/assets@assets" "${GAME_SOURCES}")
    endif()
    ``` 
