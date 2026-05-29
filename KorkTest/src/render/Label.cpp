#include "Label.h"
#include "appMain.h"
#include <platform/platformString.h>
#include "Font.h"

namespace KorkFlux {

    IMPLEMENT_CONOBJECT(Label);
    // ------------------------------------------------------------------------.
    bool Label::setFontBySimID(const char* consoleobject) {

        if (!consoleobject) return false;


        Font* simFont = dynamic_cast<Font*>(Sim::findObject(consoleobject));

        if (simFont && simFont->mFont) {

            mLabel = new FluxLabel(simFont->mFont);
            mFontSimID = simFont->getId();
            return true;
        }
        return false;
    }
    // ------------------------------------------------------------------------.
    bool Label::onAdd(){
        if (!gMain)  return false;

        if (!setFontBySimID(std::to_string(mFontSimID).c_str())) return false;
        gMain->queueObject(this);
        Log("[info] Label %d queued.", getId());

        return Parent::onAdd();
    }
    // ------------------------------------------------------------------------.
    void Label::onRemove() {
        gMain->unQueueObject(this);
        SAFE_DELETE(mLabel);
        mLabel = nullptr;
        Parent::onRemove();
    }

    // ------------------------------------------------------------------------.
    void Label::initPersistFields()
    {
        Parent::initPersistFields();
        addGroup("Label");
        addField("Font", TypeS32, Offset(mFontSimID, Label));

        addField("caption", TypeString, Offset(mCaption, Label));
        addField("x", TypeF32, Offset(mPosition.x,Label));
        addField("y", TypeF32, Offset(mPosition.y,Label));
        addField("scale", TypeF32, Offset(mScale,Label));
        addField("align", TypeS32, Offset(mAlign,Label), "0=left, 1=center, 2=right");
        endGroup("Label");


    }
    // ------------------------------------------------------------------------.
    void Label::Update(const double& dt){

    }
    // ------------------------------------------------------------------------.
    void Label::Draw() {
        if (!getVisible() || !mLabel)
            return;

        mLabel->setScale(mScale);
        mLabel->Print(mCaption, mPosition, mAlign);
    }
    // ------------------------------------------------------------------------.

} //namespace

