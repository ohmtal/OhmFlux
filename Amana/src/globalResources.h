#pragma once

#include <iostream>
#include <assert.h>
#include <fluxMain.h>
#include <gui/fluxGuiEventManager.h>




//FIXME
// // wrapper for sounds
// bool loadSound(FluxAudioStream* &object, const char * fileName, F32 gain=1.f)
// {
//     char fileNameWithPath[255];
//     sprintf(fileNameWithPath, "assets/sound/%s",fileName);
//     object = new FluxAudioStream(fileNameWithPath);
//     if (!object->getInitDone())
//         return false;
//     object->setGain(gain);
//     queueObject(object);
//     return true;
// }

//------------------------------------------------------------------------------
// GlobalResources alias gRes
//------------------------------------------------------------------------------
class GlobalResources
{
private:
    bool initialized = false;

    GlobalResources()  = default;
    ~GlobalResources() = default;

public:
    static GlobalResources& getInstance() {
        static GlobalResources instance;
        return instance;
    }

    GlobalResources(const GlobalResources&) = delete;
    GlobalResources& operator=(const GlobalResources&) = delete;

public:
    // ----------------- load the global resources --------------------------
    FluxTexture*     FontSourceCodeTexture = nullptr;
    FluxBitmapFont*  StatusLabel = nullptr;

    FluxGuiEventManager* GuiEvents = nullptr;

    bool init();


}; //class GlobalResources

#define gRes GlobalResources::getInstance()

