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
    FluxLabel mLabel;
    U32 mFontSimID = 0;
    StringTableEntry mCaption;


    Label()
    {
        mFontSimID = 0;
        mCaption =  StringTable->insert("");
    }

    bool setFontBySimID(const char* consoleobject );

    virtual void Update(const double& dt) override;
    virtual void Draw() override;;


    static void initPersistFields();

    bool onAdd() override;
    void onRemove() override;

    static bool setCaption(void* obj, const char* data) {
        static_cast<Label*>(obj)->mLabel.setCaption("%s",data);
        return false;
    }
    static StringTableEntry getCaption(void* obj) {
       Label* l=  static_cast<Label*>(obj);
       return StringTable->insert(l->mLabel.getCaption());

    }

    DECLARE_CONOBJECT(Label);
};



} //namespace
