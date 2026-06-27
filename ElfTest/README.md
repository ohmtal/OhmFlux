# 🌶️ Ohmflux ElfScript implementation


<img src="./res/icon.png" align="left" width="128" style="margin-right: 10px;">
Using: <a href="https://github.com/ohmtal/TorqueScript">https://github.com/ohmtal/ElfScript</a>
<br />
<br />
<br />
<br />
<br />
<br />
<div style="clear: both;"></div>
---

## how to use 

Add this to your code (where assets the folder where my scripts live):

MyLogger example, where ***Log*** is my Log function:
```
    void MyLogger(U32 level, const char *consoleLine)
    {
        switch (level) {
            case 1: Log("[warn] %s",  consoleLine); break;
            case 2: Log("[error] %s",  consoleLine); break;
            default: Log("%s",  consoleLine); break;
        }
    }
```

When you startup (Init) add:
```
    #include "main/engineGlue.h"
    .....
    String workingDir = getGamePath().c_str();
    engineGlue::init(MyLogger, workingDir);

```

Load a Script example:
```
        std::string fileName = "assets/main.cs"; //fixme command line parameter for file

        if (!Con::executeFile(fileName.c_str(), false, false)) {
            return false;
        }
```

Execute inline code example:
```
    Con::evaluate( R"(
            function onDebugTest() {
                %res = getRandom(1);
                echo("should be" SPC %res);
                return %res ? "true" : "false";
            }
        )"
    );
    ConsoleValue result = Con::executef("onDebugTest");
    Con::printf("Test result is %d, as string: %s", result.getBool(), result.getString());
```



on the end of your program (Shutdown):
```
        engineGlue::shutDown(); //Before Deinitialize else crash!
        FluxMain::Deinitialize();
```

In your Mainloop:
```
        // advance Torque Time for schedule
        static U32 lastTick = 0;
        engineGlue::process(SDL_GetTicks() - lastTick);
        lastTick = SDL_GetTicks();

```

## development Gui 
- ImGui Console: testing code snippets, modify objects and see or copy  the log entries
- Scripts are looked up in assets for (re)loading via Menu. 
- a small ScriptEditor 

## template.cs 
Objects Demo:
- GameCtrl
- Texture
- Font
- Label
- Sprite
- AudioProfile

Show how to:
- Input: Right click to add a Sprite. 
- scrolling FPS via looping schedule 
- FPS Label color "rainbow" animated using GameCtrl's onUpdate.



<!-- ![Template Screen](./res/Screenshot_2026-06-02_02-24-48.png) -->


