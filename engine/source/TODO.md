# TODO


- FluxMain::loadTexture mTextures should use a std::unordered_map<std::string, FluxTexture> and 
  lookup the filename to prevent double loading ... this may also be usefull to get an FluxTexture
  without createing an object 
  
## Open:

- Amana 
- Integrate imggui
- Integrate OPLController with ymfw
- Integrate SFXGenerator 
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
