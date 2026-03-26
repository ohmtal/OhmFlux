# RadioWana II

##Status: WiP 

A cross platform Internet Radio. 

I created RadioWana back in the early 2000th as a Internet Radio Recorder.
It is written in Object Pascal using Delphi 5. 
It still works fine but i wanted to create a cross platform version with C++ and OhmFlux. 

[RadioWana Prototype](https://github.com/ohmtal/RadioWana/tree/main/prototype)

--- 

In this Project I use: 

- Framework: [OhmFlux](https://github.com/ohmtal/OhmFlux)
- Backend: [SDL3](https://www.libsdl.org/)
- Gui: [Dear ImGui](https://github.com/ocornut/imgui)
- Https handling: [libCurl](https://curl.se/libcurl/)
- MP3 Decoder: [miniaudio](https://github.com/mackron/miniaudio)
- Development
    - IDE/Text: [KDevelop](https://kdevelop.org/), [Kate](https://apps.kde.org/kate/)
    - Devel/Testing OS: [Arch Linux](https://archlinux.org/), [FreeBSD](https://freebsd.org/)

- Database [RadioBrowser](https://www.radio-browser.info/)
    

Limitation: 
- Only MP3 streams.

--- 

## Todos

- [ ] Version 1.0
    - [X] switch to OhmFlux agian with it's build system 
    - [X] setup Project (cmake)
    - [X] copy code from Prototype and make it basicly run 
    - [X] callback for https errors 
    - [X] Gui enhancements 
        - [X] base design Decision: rack style 90th 
        - [X] define and save settings 
        - [X] only one LCD display .. bottom is for station except there is a "next"
        - [X] info popup: cut url when to long, desc double
        - [X] Favo add/edit manual 
        - [X] edit popup auto size again
        - [X] replace lcd with a new scrolling text widget 
        - [X] remove stepper again 
        - [ ] left menubar for window toggle - windows in fullscreen (not console)
        - [ ] add a BIG menu list for tune 
        - [ ] final touch 
            
            
    - [ ] Connect
        - Instead of URL - Favo only ? => save current stationuuid
    - [ ] Recorder
        - [X] Add recorder to Rack 
        - [ ] add Modes: 
            - split by Meta Data - save a song with the name of the meta data information 
                - Delay in ms to switch 
                - start recording on new song ( meta data )
            - Mix-Tape - append to tape bei manually start stop recoring
                - Tape name
            - Manual - save a new stream be station name (or record if no name) - DATETIME.mp3
            
    - [X] reset fullheader if redirect 3xx else content-type is not detected correctly. 
    - [X] add lowspeed timeout << 
    - [ ] add error message on http 4xx 5xx
    - [ ] Add SDL3 Icon
    - [ ] Add Background Image 
    - [X] Bug in header parser: icy-decription when empty
    
- [ ] Version 1.x
    - [ ] lowspeed time out optional reconnect 
    - [ ] Move Back to RadioWana github ... stop creating new widgets ;)
    - [ ] Cleanup Design (Windows) for Android 
    - [ ] radio-browser.info
        - [ ] SRV DNS lookup - _api._tcp.radio-browser.info
        - [ ] Update Favo on  "Click Responce" / RequestType::CLICK - if stationuuid is set
        - [ ] add favicon support 
    
    - [ ] Build
        - [ ] Windows 
            - Add Icon 
            - write docu like https://github.com/ohmtal/OhmFlux/blob/main/README_BUILD_WINDOWS.md but add curl 
        - [ ] Emscripten
            - FIX Curl build - need also openssl if you add it via fetchcontent 
        - [ ] Android 
            - FIX Curl build - need also openssl if you add it via fetchcontent 
            - Create Android Studio Project 
            - firetv 
