#pragma once

#include "console/simBase.h"
#include "console/consoleTypes.h"
#include <audio/fluxAudioStream.h>

namespace ElfFlux {

class AudioProfile: public SimObject, public FluxBaseObject {
    typedef SimObject Parent;
public:

    FluxAudioStream mAudioStream;
    StringTableEntry mFileName;

    static void initPersistFields();

    bool onAdd() override;
    void onRemove() override;


    DECLARE_CONOBJECT(AudioProfile);

};
}
