#pragma once

#include "console/simBase.h"
#include "console/consoleTypes.h"
#include <fonts/fluxTTFont.h>

namespace ElfFlux {

class Font : public SimObject {

    typedef SimObject Parent;

public:
    FluxTTFont* mFont;
    StringTableEntry mFileName;
    F32 mFontSize;
    U32 mTextureSize;

    Font()
    {
        mFont = nullptr;
        mFileName = StringTable->insert("");
        mFontSize = 32.f;
        mTextureSize = 512;
    }

    static void initPersistFields();

    bool onAdd() override;
    void onRemove() override;


    DECLARE_CONOBJECT(Font);
};



} //namespace
