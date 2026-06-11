#pragma once

#include "console/simBase.h"
#include "console/consoleTypes.h"

#include "core/fluxBaseObject.h"
#include "render/fluxRender2D.h"
#include "core/fluxRenderObject.h"

namespace ElfFlux {

class Sprite : public SimObject, public FluxBaseObject {

    typedef SimObject Parent;

public:
    ~Sprite();
    FluxRenderObject mRenderObject;
    U32 mTextureSimID = 0;

    Sprite()
    {
        mTextureSimID = 0;
    }

    bool setTextureBySimID(const char* consoleobject );

    virtual void Update(const double& dt) override;
    virtual void Draw() override;;


    static void initPersistFields();

    bool onAdd() override;
    void onRemove() override;


    DECLARE_CONOBJECT(Sprite);
};



} //namespace
