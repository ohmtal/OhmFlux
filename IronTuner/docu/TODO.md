
## Todos

- [X] Carousel View and Redesign:
    - [X] Code Redesign
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
    - [X] Play View 
        - Display
            - Title
            - Station || next Title
            - Recoding * 
        
    - [X] Radio Browser 
        - Search
        - Filter results 
        - Result list 
            - Item Enter open menu
                - Favourite
                - Play
                - Info 
    - [X] Tune: 
        - Filter results 
        - Result list 
            - Item Enter open menu
                - Favourite
                - Play
                - Info 
    - [X] Equalizer + Volume
        - up down should select fader and enter toggle edit mode
    - [X] Background Mode selector
        

    (*) Desktop only 

- [X] Auto Reconnect on timeout
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
    
- [X] Small changes
    - [X] hide top scroller on rack - define hide pages on initialize
        - => if ( std::ranges::find(mTopScrollerIgnorePages, mTargetPageIndex) == mTopScrollerIgnorePages.end() )
    - [X] url click for Stream Info URL  => if (!info->url.empty() && ImGui::TextLink(info->url.c_str())) SDL_OpenURL(info->url.c_str());
    - [X] rename browser in station search or find a station ...
    
- [X] Station Lists 
    - [X] reduce to ONE list again and :
        - [X] remove favId and add bool isLocalFavo
            - [X] FIX EDIT => mStationContextData.editId << favId
        - [X] remove when it's not current and not favo << cleanup 
    - [X] Fix Table to show Cached Stations so when clicking on favo it can be re-done . 
    - [X] Favo add your own clicks 
    - [X] SideBar - tune sort by clicks 


    
- [ ] Display Text from ImGui Font in OpenGL Shader:

    - [ ] Add ImGuiTextGlue Class which : 
        - Set the texture
        - build the text with the UV values (look at the disabled TTF class)
        
    - [ ] Draw Params need a custom vertex/fragment shader property or something 
    
    
```
ImTextureID fontTexId = ImGui::GetIO().Fonts->TexID;
GLuint glFontTexId = (GLuint)(intptr_t)fontTexId;
//......
ImFont* font = ImGui::GetFont(); // Aktueller Font
const ImFontGlyph* glyph = font->FindGlyph('A'); // UVs für den Buchstaben 'A'

float u0 = glyph->V0.x, v0 = glyph->V0.y; // top left 
float u1 = glyph->V1.x, v1 = glyph->V1.y; // bottom right
```
BuildTexture without Render2D:

``` 
void BuildTextMesh(const char* text, float x, float y, std::vector<float>& outVertices) {
    ImFont* font = ImGui::GetFont();
    float scale = 1.0f; 

    for (const char* p = text; *p; p++) {
        const ImFontGlyph* glyph = font->FindGlyph(*p);
        if (!glyph) continue;

        float x0 = x + glyph->X0 * scale;
        float y0 = y + glyph->Y0 * scale;
        float x1 = x + glyph->X1 * scale;
        float y1 = y + glyph->Y1 * scale;

        float u0 = glyph->U0, v0 = glyph->V0;
        float u1 = glyph->U1, v1 = glyph->V1;

        // Format: {x, y, u, v}
        outVertices.insert(outVertices.end(), {
            x0, y0, u0, v0,
            x1, y0, u1, v0,
            x1, y1, u1, v1,

            x0, y0, u0, v0,
            x1, y1, u1, v1,
            x0, y1, u0, v1
        });

        x += glyph->AdvanceX * scale;
    }
}
```

For FluxTexture: 
``` 
ImGuiIO& io = ImGui::GetIO();

// if not initialized:
// unsigned char* pixels;
// int width, height;
// io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

// if initialized: 
int w = io.Fonts->TexWidth;
int h = io.Fonts->TexHeight;

ImTextureID fontTexId = io.Fonts->TexID;
GLuint glFontTexId = (GLuint)(intptr_t)fontTexId;


mTexture.setManual(glFontTexId, w, h);
``` 





- [ ] OpenGL/ES shader:
    - Liquid looks awfull - on restart it's better but why does reload not work ? 

- [ ] on smartphone some times i not able to connect and get no error message - dLog is disabled there also because of release build

- [ ] Build
    - [X] Windows 
        - [X] Add Icon 
        - [X] updated docu https://github.com/ohmtal/OhmFlux/blob/main/README_BUILD_WINDOWS.md optional curl 
    - [~] Emscripten
    - [ ] Android 
        - [X] shaders fail to compile 
        - [X] added separate handling in Makefile which fetch [PROJECTDIR]/res/app
        
        - [ ] Build Gradle and IronTunerActivity / Service 
            - [ ] Background only changeing the AndroidManifest is not enough: FIXME in java source: startForegroundService() and Notification and SDL_APP_WILLENTERBACKGROUND
            - [ ] more ABI's  << firetv and old smartphones 


- [ ] Fresh start welcome window with introduction 
        
---
    
- [ ] Milestone III
    - [ ] Autoconnect on start 

- [ ] Build
    - [ ] Android 
        - [ ] SSL / CURL ..  
            - [X] quick and dirty: rewrite https to http in source ! - it's a radio stream why is it encoded ? 

    - [ ] OGG support but disable recorder ?! or need to 
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


    - [ ] radio-browser.info
        - [ ] search by highest click count and tag 
        - [ ] SRV DNS lookup - _api._tcp.radio-browser.info
        - [ ] Update Favo on  "Click Responce" / RequestType::CLICK - if stationuuid is set
        - [ ] add favicon support 
    
            
    - Finishing
        - [ ] add error message on http 4xx 5xx
        - [ ] different layouts not only factory 
