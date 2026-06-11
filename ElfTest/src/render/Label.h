#pragma once

#include "core/fluxBaseObject.h"
#include "console/simBase.h"
#include "console/consoleTypes.h"
#include <fonts/fluxTTFont.h>
#include <fonts/fluxLabel.h>


namespace ElfFlux {

class Label : public SimObject, public FluxBaseObject {

    typedef SimObject Parent;

public:
    ~Label();
    FluxLabel mLabel;
    U32 mFontSimID = 0;
    // StringTableEntry mCaption;


    Label()
    {
        mFontSimID = 0;
        // mCaption =  StringTable->insert("");
    }

    bool setFontBySimID(const char* consoleobject );

    virtual void Update(const double& dt) override;
    virtual void Draw() override;;


    static void initPersistFields();

    bool onAdd() override;
    void onRemove() override;


    static bool setCaption(void* obj,const char* ,  const char* data) {
        Label* l = static_cast<Label*>(obj);
        if (!l) return false; //better an assert ?
        l->mLabel.setCaption("%s",data);
        return false; //return false => do not update the offset
    }
    static const char *getCaption(void* obj, const char* data) {
       Label* l = static_cast<Label*>(obj);
       if (!l) return "";
       return StringTable->insert(l->mLabel.getCaption());

    }

    DECLARE_CONOBJECT(Label);
};



} //namespace
