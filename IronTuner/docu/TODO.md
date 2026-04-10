
## Todos

- [X] auto reconnect cause crash !

- [X] Invalid Station: https://us2.maindigitalstream.com/ssl/7739
    - maybe icy infos case sensitive => icy-metaint
    
- [X] Swipe: Ignore when other popup is selected 
    - added in Page.h
    
- [X] remove "final protocol is" error message or change it to normal message 

- [X] Infopage with current station info, raw/ring buffer << "about" replacement
    
- [X] SideBar: scale width by "mScale"

- [X] TuneKnob
    - [X] sync station on Tune
    - [X] too fast (android) changed step from 5.f to 20.f
    - [X] test on android 
    
- [X] check why multiple stations show favo in radio browser (test: radio bob)
    ==> isFavo checks UUID AND url and the **url does match** on multiple Stars in radio browser 

- [X] still => double insert into cache << favID ?! 
    

- [X] usable (android) lists for radio browser / Favo 
    - [X] Height
    - [X] Double click
    - [X] Select favo 
    - [X] Context Menu : Tune, Edit, Info  (see also: FIXME CONTEXT MENU)
    - [X] scrollbar 
    

    
- [ ] Fix Table to show Cached Stations so when clicking on favo it can be re-done . 
    
- [ ] Favo add your own clicks and sort by 
    
- [ ] Display Text from ImGui Font in OpenGL Shader:
```
ImTextureID fontTexId = ImGui::GetIO().Fonts->TexID;
GLuint glFontTexId = (GLuint)(intptr_t)fontTexId;
//......
ImFont* font = ImGui::GetFont(); // Aktueller Font
const ImFontGlyph* glyph = font->FindGlyph('A'); // UVs für den Buchstaben 'A'

float u0 = glyph->V0.x, v0 = glyph->V0.y; // top left 
float u1 = glyph->V1.x, v1 = glyph->V1.y; // bottom right
```

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
    
- [ ] Milestone III
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
            - [X] shaders fail to compile 
            - [ ] SSL / CURL ..  
                - [X] quick and dirty: rewrite https to http in source ! - it's a radio stream why is it encoded ? 
            - [X] added separate handling in Makefile which fetch [PROJECTDIR]/res/app
            - [ ] Background only changeing the AndroidManifest is not enough: FIXME in java source: startForegroundService() and Notification and SDL_APP_WILLENTERBACKGROUND
            - Create Android Studio Project  << allow internet for testing look at: android/app/src/main/AndroidManifest.xml
            - firetv 
    - [ ] Cleanup Design  for Android
    - Finishing
        - [ ] add error message on http 4xx 5xx
        - [ ] About << make nice - markdown renderer ? 
        - [ ] Help - markdown renderer ? 
        - [ ] different layouts not only factory 

