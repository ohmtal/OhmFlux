# 🌶️ Ohmflux ElfScript implementation


<img src="./res/icon.png" align="left" width="128" style="margin-right: 10px;">
Using: <a href="https://github.com/ohmtal/TorqueScript">https://github.com/ohmtal/TorqueScript</a>
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

```
//--------------------------------------------------------------------------------------
namespace engineAPI
{
    bool gUseConsoleInterop = true;
    bool gIsInitialized = false;

    // -----------------------------------------------------------------------------
    void init()
    {
        // Asserts should be created FIRST
        PlatformAssert::create();
        // ManagedSingleton< ThreadManager >::createSingleton();
        FrameAllocator::init(TORQUE_FRAME_SIZE);      // See comments in torqueConfig.h
        _StringTable::create();
        Con::init();
        // Platform::initConsole();
        NetStringTable::create();

        Platform::FS::InstallFileSystems(); // install all drives for now until we have everything using the volume stuff
        Platform::FS::MountDefaults();
        Torque::FS::SetCwd( "assets:/" );
        Platform::setCurrentDirectory( Platform::getMainDotCsDir() );

        Platform::init();    // platform specific initialization
        // Set engineAPI initialized to true
        engineAPI::gIsInitialized = true;
        Sim::init();
    }

    void shutDown() {
        Sim::shutdown();

        Platform::shutdown();

        NetStringTable::destroy();
        Con::shutdown();

        _StringTable::destroy();
        FrameAllocator::destroy();
        // asserts should be destroyed LAST
        PlatformAssert::destroy();

        engineAPI::gIsInitialized = false;
    }
}

```

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
        engineAPI::init();
        Con::addConsumer(MyLogger); // add the LogConsumer

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
        engineAPI::shutDown(); //Before Deinitialize else crash!
        Con::removeConsumer(MyLogger); // remove the LogConsumer
```

In your Mainloop:
```
//Time if you use SDL it's somethink like that:
    static U32 lastTick = 0;
    Sim::advanceTime(SDL_GetTicks() - lastTick);
    lastTick = SDL_GetTicks();
// This is very important, else your programm eats all memory
    ConsoleValue::resetConversionBuffer();
...
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


