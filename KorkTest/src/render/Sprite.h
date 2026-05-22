#pragma once

#include "core/Globals.h"
#include "core/fluxBaseObject.h"
#include "render/fluxRender2D.h"
#include "sim/simBase.h"
#include "console/consoleTypes.h"

namespace KorkFlux {

class Sprite : public SimObject, public FluxBaseObject {

    typedef SimObject Parent;

public:
    DrawParams2D mDrawParams;
    StringTableEntry mTextureName;
    U32 mTexCols;
    U32 mTexRows;


    Sprite()
    {
        mTexCols = 1;
        mTexRows = 1;
        mTextureName = StringTable->insert("");

    }

    virtual void Update(const double& dt) override;
    virtual void Draw() override;;


    static void initPersistFields();

    bool onAdd() override;
    void onRemove() override;


    DECLARE_CONOBJECT(Sprite);
};



} //namespace
