#pragma once

#include "core/fluxBaseObject.h"
#include "sim/simBase.h"
#include "console/consoleTypes.h"
#include <fonts/fluxTTFont.h>
#include <fonts/fluxLabel.h>

namespace KorkFlux {

class Label : public SimObject, public FluxBaseObject {

    typedef SimObject Parent;

public:
    FluxLabel* mLabel;
    U32 mFontSimID = 0;
    StringTableEntry mCaption;
    Point2F mPosition;
    F32 mScale;
    FontAlign mAlign;

    Label()
    {
        mFontSimID = 0;
        mLabel = nullptr;

        mCaption =  StringTable->insert("");
        mPosition = {0.f, 0.f};
        mScale = 1.f;
        mAlign = FontAlign_Left;
    }

    bool setFontBySimID(const char* consoleobject );

    virtual void Update(const double& dt) override;
    virtual void Draw() override;;


    static void initPersistFields();

    bool onAdd() override;
    void onRemove() override;


    DECLARE_CONOBJECT(Label);
};



} //namespace
