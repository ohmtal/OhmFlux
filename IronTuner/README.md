# IronTuner - Solid Steel. Vivid Beats.

- Forget sterile, flat designs. 

    IronTuner brings back the weight and raw aesthetic of high-end 80s and 90s Hi-Fi racks. 
    Experience a digital radio that feels like brushed aluminum and heavy-duty steel, housing a soul of pure, explosive color.

- The Gear

    A dark, metallic interface designed for those who miss the tactile precision of legendary audio gear. 
    No clutter, just power.

- The Vision

    Watch your favorite stations come to life. Behind the iron-clad exterior lies a 
    stunning background visualization that reacts to every frequency, turning your screen into a vivid canvas of light and sound.
    IronTuner. Built for the sound you hear. Crafted for the beats you see.

---    
    
- **Industrial High-Fi Aesthetic:** A rugged, dark metallic interface inspired by the legendary brushed-aluminum stereo racks of the 80s and 90s. No flat, sterile design – just pure, weighted gear.
- **Vivid Audio Visualization:** Don't just listen, watch. Every station features a stunning, reactive background visualizer that transforms sound into a kaleidoscope of light and movement.
- **Global Radio Access:** Stream thousands of internet radio stations from around the world in high fidelity. From synthwave classics to modern beats, your favorite frequency is always tuned in.
- **Cross-Platform Performance:** Seamlessly sync your experience. Whether you’re on mobile, desktop, or tablet, IronTuner delivers a consistent, high-performance look and feel.
- **Tactile Control: Designed for the feel of real hardware.** Large, intuitive controls and a focused layout that puts the music and the visuals front and center.
- **Pure Sound, No Clutter:** A distraction-free environment dedicated to the art of the radio. Just the gear, the visual, and the beat.

---


## Current Status: Carousel View and Redesign WiP 

A cross platform Internet Radio. 


[RadioWana Desktop](https://github.com/ohmtal/RadioWana/tree/main/desktop)

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
- Android Build does not support SSL - only http can be used so far 
- Noticed interrupted sound on pipewite-pulse. need to test more my ringbuffer/threaded decoder did not fix it 
--- 

## Todos

- [ ] Carousel View and Redesign:
    - [ ] Code Redesign
        - RadioWana Class replacement:
            - add a core class which hold the save/load/lists/handline
                - what about the main class ? 
            - create base view and derive the other views 
        - Current Tree:
            - main
                - AppMain (getMain())
                    - * holding Fonts 
                    - Background Effects 
                    - RadioWana: handling gui and app logic
                        - *loading Fonts
                        - AudioHandler
                        - AudioRecoder
                        - RadioBrowser
                        - StreamHandler
                        - StreamInfo
        - New Tree
        
    - Usage Rules:
        - left right change window 
        - up down navigate in window
        - enter change option or popup other options
    - [ ] Play View 
        - Display
            - Title
            - Station || next Title
            - Recoding * 
        
    - [ ] Radio Browser 
        - Search
        - Filter results 
        - Result list 
            - Item Enter open menu
                - Favourite
                - Play
                - Info 
    - [ ] Tune: Favorites or better Station cache ?! 
        - Filter results 
        - Result list 
            - Item Enter open menu
                - Favourite
                - Play
                - Info 
    - [ ] Equalizer + Volume
        - up down should select fader and enter toggle edit mode
    - [ ] Background Mode selector
        - [ ] list with modes 

    (*) Desktop only 
---
    
- [ ] Version 2.x
    - [ ] Autoconnect on start 
    - [X] Auto Reconnect on timeout
    - [ ] Recorder
        - [X] Add recorder to Rack 
        - Enable recording controls without connected 
        - [ ] add Modes: 
            - split by Meta Data - save a song with the name of the meta data information 
                - Delay in ms to switch 
                - start recording on new song ( meta data )
            - Mix-Tape - append to tape bei manually start stop recoring
                - Tape name
            - Manual - save a new stream be station name (or record if no name) - DATETIME.mp3


    
    
    - [ ] Move Back to RadioWana github ... stop creating new widgets ;) **New Name?** 
            
    
    - [ ] radio-browser.info
        - [ ] search by highest click count and tag 
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
            - [ ] shaders fail to compile 
            - [ ] SSL / CURL .. 
            - Create Android Studio Project  << allow internet for testing look at: android/app/src/main/AndroidManifest.xml
            - firetv 
    - [ ] Cleanup Design  for Android
    - Finishing
        - [ ] add error message on http 4xx 5xx
        - [ ] About << make nice - markdown renderer ? 
        - [ ] Help - markdown renderer ? 
        - [ ] different layouts not only factory 

