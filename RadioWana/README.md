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
- Noticed interrupted sound on pipewite-pulse. need to test more my ringbuffer/threaded decoder did not fix it 
--- 

## Todos

- [ ] Version 1.x ==> target release: Mid April '26
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
        - [X] Sidebar: 
            - [X] Initial window implemented 
            - [X] Radio
                - info 
                - connect / disconnect 
            - [X] Favorites
            - [X] Radio Browser 
            - [X] Windows
            - [X] Background effects - selector None, ...

        
        - [ ] Player Window
            - [X] add favo 
            - [ ] Display playerwindow with Equalizer or Recorder is enabled  => keep top scroll 
            - [ ] center output :D
            
        - [X] deny station when no meta-int is set !!  => Invalid station detected...
        
        - [X] add a BIG Knob list for tune 
            - [X] seamless/endless toggle favo station  / Tune-Modes
            - [X] click connects or disconnects
            - [X] Enhance Hint: Disconnect or station name only 
            
        - [ ] Options
            - [X] Window Save States << look at console command "window"
                - [X] add a struct for this with json stuff
                    - [X] Maximized
                    - [X] Size and Position 
                    
                - [X] set better min size  => 720 x 320
            - [ ] Autoconnect on start 
            - [ ] Auto Reconnect on timeout
            
        - [X] TuneButton: station list cache not updated when station added to favo 
        
    - [ ] Recorder a bit nicer - or maybe finish it see Version 2
    
    - [X] Player: need a function to add remove from Favo in "info?"
    
    - [X] Kubuntu test: Music stutter 
        - [X] added thread safe ring buffer
        - [X] added threaded Decoder Worker 
        - [~] still does it - is it a curl buffer problem ? check raw buffer in console with "dd" (decodeDebug): It stucks at 30-80k while i have about 300k on my development machine
        
    - [X] sucks: Played station from RadioBrowser is added to Favorites automaticly 
        - find a other solution to handle the currentStation and tuning 
        - => added stationcache ... 
        
    - [X] Connect
        - Instead of URL - Favo only ? => save current stationuuid
    
    - [X] Overwrite stream station name with database station name if empty 
    - [X] When not in favo list current station from AppSettings is not used after restart
        - add current station to favo automaticly .. 
        - FIXME ?! 
    - [~] Radio as internal fullscreen window 
        - Tested was bad :P 
    - [X] fix wrong mouse over hint on tune button - should be show station name and not disconnect when it's not the current station
    - [X] Keyboard and GamePad: 
        - [X] Equalizer Fader
        - [X] Tune Button 
    - [X] reset fullheader if redirect 3xx else content-type is not detected correctly. 
    - [X] add lowspeed timeout << 
    
    - [X] Add SDL3 Icon

    - [X] Bug in header parser: icy-decription when empty
    
    - [ ] Release
        - [ ] Windows Installer
            - [X] Try windows build
            - [X] cmake add a windows icon 
            - [ ] Create installer 
        - [ ] Linux/BSD Installer shell script 
        - [ ] Try a flatpak - flathub ? 
    
---
    
- [ ] Version 2.x
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
            - FIX Curl build - need also openssl if you add it via fetchcontent 
            - Create Android Studio Project 
            - firetv 
    - [ ] Cleanup Design  for Android
    - Finishing
        - [ ] add error message on http 4xx 5xx
        - [ ] About << make nice - markdown renderer ? 
        - [ ] Help - markdown renderer ? 
        - [ ] different layouts not only factory 

---
            
- TBD
    - [~] RSS Podcast/Feeds
        - [ ] Handle mp3 stream without header data ! Take a url from a rss feed - handle empty meta-int and play the stream - visual feedback 
            - [X] fixed meta-int not set so stream with audio continue
            - [ ] Problem: when stream complete it calles disconnect, which close the audiostream and kick the buffer away . 
            - THIS NEEDS an extra object - it's an download only !
            
