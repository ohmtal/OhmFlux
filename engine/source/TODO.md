# TODO

- [ ] write a docu about setup android project / grade / icons / and so on 
  - Tools: make_android_icons.py, make_windows_icon.py
  - App Name : [PROJECT]/res/app/src/main/res/values/strings.xml
  - App ID: [PROJECT]/res/app/build.gradle => applicationId

- [X] FireTV:
  - [X]  Rendered left bottom only 

- [X] 2026-04-12 DEFAULT SHADER ARE NOT COMPATIBLE WITH GLES2 
  - NOTE: it works but this was a nobody need this waste of time ;)
  - I may use the shader changes as a peformance mode (fragment "monster" shader) or keep it like it is 

---

- 2026-03-02 
  - replaced in headers: Ohmtal Game Studio with my name (testing utf8)
  - removed/disabled  fluxTrueTypeFont
  - ~~Added compile flag FLUX_ENGINE_OPENGL but unused at the moment ;)~~


- evaluation to make opengl and glew optionaly in flavour of SDL Renderer only:
  - Using OpenGL
    - FluxRender2D
      - heavy usage in combination with the shaders
    - FluxMesh 
      - heavy usage using the buffers (VBO/EBO...)
    - FluxShaders 
      - heavy usage handling the shaders 
    - FluxTexture
      - heavy usage of OpenGL for Texture bindings
    - FluxScreen 
      - medicore to heavy usage of OpenGL for Screen 
    - fluxTrueTypeFont  << maybe removed again ?! 
      - low usage because of Texture binding
    - fluxGuiGlue 
      - bind ImGui to OpenGL Context
    - fluxEmscripten 
      - void sync_opengl_high_dpi(SDL_Window* window) << not really depend on OpenGL
      
  - Rendering completly depends on OpenGL
  - Screen can be separated 
  - Textures can be separarted 
  - First: Is it worth to do it ? 
    - (+) with only SDL3 we have less depencies 
    - (-) I think it will be a lot of work to get the Lights working
    - (-) Redundant code for Screen/Texture and Rendering
    - (+) For ImGui Applications - which i did recently i do not need OpenGL / lights
    - (+) Separating opens a future port to vulcan
    - (+/i) not sure whats about Emscripten it works fine with OpenGL 
  - Maybe i reboot the project and refactor the classes or start a new Project 
  
- Starting with Textures: 

|Feature     |OpenGL       |SDL3 Renderer     |Vulkan                  |
|------------|-------------|------------------|------------------------|
|Handle Type |GLuint       |SDL_Texture*      |VkImage / VkImageView   |
|Creation    |glGenTextures|SDL_CreateTexture |vkCreateImage           |
|Binding     |glBindTexture|SDL_RenderTexture |vkCmdBindDescriptorSets |
  
## I think it's not worth to do it OpenGL is fine.## 
  
      
---      

- FluxMain::loadTexture mTextures should use a std::unordered_map<std::string, FluxTexture> and 
  lookup the filename to prevent double loading ... this may also be usefull to get an FluxTexture
  without createing an object 
  
## Open:

- Cancled: Amana 
- Done: Integrate imggui
- Done: Integrate OPLController with ymfw
- Done: Integrate SFXGenerator 
- Integrate box2d

---

## Features 

### Move from OpenGL fixed function Pipeline to Shader/Modern Rendering
Done

### Separate Draw from Screen
Done 

###  Camera 
Done.

### Batch Rendering 
Done

### Tilemap 
Done

### Particles
Done

### Adding .png via stb
Done

### truetype fonts via stb
Done

### Sound 

Done! 

- **with position**: off screen and volume ? 
- looping Music 
- ogg support via stb

### Scheduler
Done. 

### Update/Draw
Done. 
- Update should be not bind to Draw frequency 

### Lights 
- pointLight Done
- spotLight Done

### Cross compile for Windows on Linux
Done.

### Android
experimental 

### Lua scripting 
experimental see also LuaTest Project 

### Shadows
maybe but optional 

### PostEffects
maybe but optional 

### more ... ;)
