# RadioWana II

##Status: WiP 

A cross platform Internet Radio. 

I created RadioWana back in the early 2000th as a Internet Radio Recorder.
It is written in Object Pascal using Delphi 5. 
It still works fine but i wanted to create a cross platform version with C++ and OhmFlux. 

[RadioWana Prototype](https://github.com/ohmtal/RadioWana/tree/main/prototype)

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

- [ ] Version 1
    - [X] switch to OhmFlux agian with it's build system 
    - [X] setup Project (cmake)
    - [X] copy code from Prototype and make it basicly run 
    - [X] callback for https errors 
    - [X] Gui enhancements 
        - [X] base design Decision: rack style 90th 
        - [X] define and save settings 
        - [ ] only one LCD display .. bottom is for station except there is a "next"
        - [X] info popup: cut url when to long, desc double
        - [X] Favo add/edit manual 
        - [ ] add Tune Button => Favo popup or stepper ? 
            - [ ] mAppSettings.CurrentFavId gets out of sync when list is changed
        - [~] Eq9 and volume as slider ? 
        - [ ] Add Recorder to "Rack"
        - [ ] final touch 
        
    - [ ] Add SDL3 Icon
    - [ ] Add Background Image 
    - [X] Bug in header parser: icy-decription when empty
    
- [ ] Version 1.1
    - [ ] Move Back to RadioWana github ... stop creating new widgets ;)
    - [ ] Cleanup Design (Windows) for Android 
    - [ ] Connect
        - Instead of URL - Favo only ? => save current stationuuid
    - [ ] Recorder
        - [ ] add delay to switch file name, this should be saved for each station 
            - rock antenne sends meta data too early for example 3 sec or so 
            - i can use OhmFlux Scheduler :) 
        - [ ] mixTape like in the good old days :D << auto switch does not work so good anyway 
            - [ ] do not switch the file name it like MixTape_DATE or let the user name it 
            - [ ] append to stream
            
    - [ ] radio-browser.info
        - [ ] SRV DNS lookup - _api._tcp.radio-browser.info
        - [ ] Update Favo on  "Click Responce" / RequestType::CLICK - if stationuuid is set
    
    - [ ] Build
        - [ ] Windows 
            - Add Icon 
            - write docu like https://github.com/ohmtal/OhmFlux/blob/main/README_BUILD_WINDOWS.md but add curl 
        - [ ] Android 
            - Create Android Studio Project 

