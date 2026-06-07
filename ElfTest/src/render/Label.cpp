#include "Label.h"
#include "appMain.h"
#include <platform/platformString.h>
#include "Font.h"
#include "core/Globals.h"

namespace KorkFlux {

    IMPLEMENT_CONOBJECT(Label);
    // ------------------------------------------------------------------------.
    bool Label::setFontBySimID(const char* consoleobject) {

        if (!consoleobject) return false;


        Font* simFont = dynamic_cast<Font*>(Sim::findObject(consoleobject));

        if (simFont && simFont->mFont) {
           if (!mLabel.setFont(simFont->mFont)) return false;
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
        mLabel = nullptr;
        Parent::onRemove();
    }

    // ------------------------------------------------------------------------.
    void Label::initPersistFields()
    {
        Parent::initPersistFields();
        addGroup("Label");
        addField("Font", TypeS32, Offset(mFontSimID, Label));
 //FIXME        addProtectedField("caption", TypeString,&setCaption, &getCaption, &defaultProtectedWriteFn);
        addField("caption", TypeString, Offset(mCaption, Label));
        addField("x", TypeF32, Offset(mLabel.mDrawParams.x,Label));
        addField("y", TypeF32, Offset(mLabel.mDrawParams.y,Label));
        addField("color", TypeColor4F, Offset(mLabel.mColor, Label));
        addField("scale", TypeF32, Offset(mLabel.mScale,Label));
        addField("align", TypeS32, Offset(mLabel.mAlign,Label), "0=left, 1=center, 2=right");

        addField("shadow", TypeBool, Offset(mLabel.mShadow, Label));
        addField("shadowColor", TypeColor4F, Offset(mLabel.mShadowColor, Label));
        addField("shadowOffset", TypeF32, Offset(mLabel.mShadowOffset, Label));
        endGroup("Label");


    }
    // ------------------------------------------------------------------------.
    void Label::Update(const double& dt){

    }
    // ------------------------------------------------------------------------.
    void Label::Draw() {
        if (!getVisible() || !mLabel.isInitialized())
            return;

        // mLabel->setScale(mScale);
        // mLabel->Print(mCaption, mPosition, mAlign, mColor);
        mLabel.setCaption("%s", mCaption); // FIXME really every draw
        mLabel.Draw();
    }
    // ------------------------------------------------------------------------.

} //namespace

