#pragma once
// NOTE: named it AudioProfile is use to make it compatible with old scripts ...

// NOTE: too bad i did not finish my AudioResourceManager and AudioInstance so far
// FIXME: so this will use fluxAudioStream ... wav and ogg only

#pragma once

#include "sim/simBase.h"
#include "console/consoleTypes.h"
#include <audio/fluxAudioStream.h>

namespace KorkFlux {
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
