#include "Sprite.h"
#include "appMain.h"
#include "Texture.h"
#include "core/Globals.h"

#include "console/engineAPI.h"


namespace ElfFlux {

    IMPLEMENT_CONOBJECT(Sprite);
    // ------------------------------------------------------------------------.
    bool Sprite::setTextureBySimID(const char* consoleobject) {

        if (!consoleobject) return false;

        Texture* simTex = dynamic_cast<Texture*>(Sim::findObject(consoleobject));

        if (simTex && simTex->mTexture) {
            mRenderObject.mDrawParams.image = simTex->mTexture;
            mTextureSimID = simTex->getId();
            return true;
        }
        return false;
    }
    // ------------------------------------------------------------------------.
    bool Sprite::onAdd(){
        if (!gMain)  return false;

        bool useWhiteTexure = mTextureSimID == 0; //fallback or lazy add ...
        if (!useWhiteTexure) useWhiteTexure = !setTextureBySimID(std::to_string(mTextureSimID).c_str());
        if (useWhiteTexure) mRenderObject.mDrawParams.image = Render2D.getWhiteTexture();

        gMain->queueObject(this);
        dLog("[info] Sprite %d queued.", getId());

        return Parent::onAdd();
    }
    // ------------------------------------------------------------------------.
    void Sprite::onRemove() {
        dLog("[info] Sprite %d unQueued.", getId());
        gMain->unQueueObject(this);
        Parent::onRemove();
    }
    // ------------------------------------------------------------------------.
    DefineEngineMethod(Sprite, setTexture, bool, (String objectStr),,
                       "@param Texture id "
                        "Set the Textue"
    )
    {
        return object->setTextureBySimID(objectStr);
    }
    // ------------------------------------------------------------------------.
    DefineEngineMethod(Sprite, getRectF, RectF, (),,"Get the Sprite Rect" ) {
        return object->mRenderObject.getRectF();
    }

    DefineEngineMethod(Sprite, setRectF, void, (RectF rect),,"Set the Sprite Rect" ) {
        object->mRenderObject.setRectF(rect);
    }

    // ------------------------------------------------------------------------.
    void Sprite::initPersistFields()
    {
        Parent::initPersistFields();
        addGroup("Texture");
        addField("Texture", TypeS32, Offset(mTextureSimID, Sprite));
        endGroup("Texture");


        addGroup("Params");
        addField("imgId", TypeS32 , Offset(mRenderObject.mDrawParams.imgId, Sprite) );
        addField("x", TypeF32, Offset(mRenderObject.mDrawParams.x, Sprite));
        addField("y", TypeF32, Offset(mRenderObject.mDrawParams.y, Sprite));
        addField("z", TypeF32, Offset(mRenderObject.mDrawParams.z, Sprite));
        addField("w", TypeF32, Offset(mRenderObject.mDrawParams.w, Sprite));
        addField("h", TypeF32, Offset(mRenderObject.mDrawParams.h, Sprite));
        addField("color", TypeColorF, Offset(mRenderObject.mDrawParams.color, Sprite));
        addField("rotation", TypeF32, Offset(mRenderObject.mDrawParams.rotation, Sprite));
        addField("flipX", TypeBool, Offset(mRenderObject.mDrawParams.flipX, Sprite));
        addField("flipY", TypeBool, Offset(mRenderObject.mDrawParams.flipY, Sprite));
        endGroup("Params");

        addGroup("Action");
        addField("Velocity", TypePoint2F, Offset(mRenderObject.mVelocity, Sprite));
        addField("Speed", TypeF32, Offset(mRenderObject.mSpeed, Sprite));
        endGroup("Action");

        addGroup("Animation");
        addField("FramesStart", TypeS32, Offset(mRenderObject.mFramesStart, Sprite));
        addField("FramesEnd", TypeS32, Offset(mRenderObject.mFramesEnd, Sprite));
        addField("AnimationDelay", TypeS32, Offset(mRenderObject.mAnimationDelay, Sprite));
        addField("AnimationTime", TypeS32, Offset(mRenderObject.mAnimationTime, Sprite));
        endGroup("Animation");
    }
    // ------------------------------------------------------------------------.
    void Sprite::Update(const double& dt){
        mRenderObject.Update(dt);
    }
    // ------------------------------------------------------------------------.
    void Sprite::Draw() {
        mRenderObject.Draw();
    }
    // ------------------------------------------------------------------------.

} //namespace

