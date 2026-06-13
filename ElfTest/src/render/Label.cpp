#include "Label.h"
#include "appMain.h"
#include "Font.h"
#include "core/Globals.h"
#include "console/engineAPI.h"
#include "core/GameCtrl.h"

namespace ElfFlux {

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
        // it's allowed to add the GameCtrl as font object
        GameCtrl* gameCtrl =  dynamic_cast<GameCtrl*>(Sim::findObject(consoleobject));
        if (gameCtrl) {
            if (! gameCtrl->getFont() || !mLabel.setFont(gameCtrl->getFont())) return false;
            mFontSimID = gameCtrl->getId();
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
        addField("Font", TypeS32, Offset(mFontSimID, Label), "SimId of the Font - NOTE only set on Create");
        // addProtectedField("caption", TypeString, Offset(mCaption, Label), &setCaption,&getCaption, "Set the caption.");
        // test with offset 0 . i done need a offset here using getter/setter
        addProtectedField("caption", TypeString, 0, &setCaption,&getCaption, "Set the caption.");
        addField("x", TypeF32, Offset(mLabel.mDrawParams.x,Label), "x position of the label");
        addField("y", TypeF32, Offset(mLabel.mDrawParams.y,Label), "x position of the label");
        addField("color", TypeColorF, Offset(mLabel.mColor, Label), "color of the label");
        addField("scale", TypeF32, Offset(mLabel.mScale,Label), "scale  the label");
        addField("align", TypeS32, Offset(mLabel.mAlign,Label), "0=left, 1=center, 2=right");

        addField("shadow", TypeBool, Offset(mLabel.mShadow, Label), "bool - render shadow ");
        addField("shadowColor", TypeColorF, Offset(mLabel.mShadowColor, Label), "Color of the shadow");
        addField("shadowOffset", TypeF32, Offset(mLabel.mShadowOffset, Label), "float offset of the shadow");
        endGroup("Label");


    }
    // ------------------------------------------------------------------------.
    void Label::Update(const double& dt){

    }
    // ------------------------------------------------------------------------.
    void Label::Draw() {
        if (!getVisible() || !mLabel.isInitialized())
            return;

        mLabel.Draw();
    }
    // ------------------------------------------------------------------------
    DefineEngineMethod(Label, getRect, RectI, (),, "get the rect of the label") {
        return object->mLabel.getRectI();
    }

} //namespace

