# RadioWana II

A cross platform Internet Radio. 

I created RadioWana back in the early 2000th as a Internet Radio Recorder.
It is written in Object Pascal using Delphi 5. 
It still works fine but i wanted to create a cross platform version with C++ and OhmFlux. 

[RadioWana Prototype](https://github.com/ohmtal/RadioWana/tree/main/eval)

--- 

In this Project I use: 

- Framework: OhmFlux
- Backend: SDL3
- Gui: ImGui 
- Https handling: libCurl 
- MP3 Decoder: miniaudio 
- Development
    - IDE: KDevelop 
    - OS: Arch Linux


Limitation: 
- Only MP3 streams. 

--- 

## Todos

- [ ] Final Version 
    - [ ] switch to OhmFlux agian with it's build system 
    - [ ] add delay to switch file name, this should be saved for each station 
        - rock antenne sends meta data too early for example 3 sec or so 
        - i can use OhmFlux Scheduler :) 
    - [ ] Gui enhancements 
        - [ ] design like an old 70th/80th radio recorder 
        - [ ] keep android GUI in mind (no context menus)
        - [ ] define and save settings 
        - [ ] ...fixme write todos ;) ...
        
    - [ ] Test on windows 
        - write docu like https://github.com/ohmtal/OhmFlux/blob/main/README_BUILD_WINDOWS.md but add curl 
        - i may add curl to OhmFlux so simple add this there 
    
