#pragma once

#include "sim/simBase.h"
#include "console/consoleTypes.h"
#include <core/fluxTexture.h>

namespace KorkFlux {

class Texture : public SimObject {

    typedef SimObject Parent;

public:

    StringTableEntry mFileName;
    U32 mTexCols;
    U32 mTexRows;

    FluxTexture* mTexture;

    Texture()
    {
        mTexCols = 1;
        mTexRows = 1;
        mFileName = StringTable->insert("");
        mTexture = nullptr;
    }



    static void initPersistFields();

    bool onAdd() override;
    void onRemove() override;


    DECLARE_CONOBJECT(Texture);
};



} //namespace
