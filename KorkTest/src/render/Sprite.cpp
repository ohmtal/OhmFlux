#include "Sprite.h"
#include "appMain.h"
#include <platform/platformString.h>

namespace KorkFlux {

    IMPLEMENT_CONOBJECT(Sprite);
    // ------------------------------------------------------------------------.
    bool Sprite::onAdd(){
        if (!gMain)  return false;

        if (*mTextureName) {
            FluxTexture*  tex = gMain->loadTexture(mTextureName, mTexCols, mTexRows);
            if (!tex) return false;
            mDrawParams.image = tex;
        }

        gMain->queueObject(this);
        Log("[info] Sprite %d queued.", getId());

        return Parent::onAdd();
    }
    // ------------------------------------------------------------------------.
    void Sprite::onRemove() {
        gMain->unQueueObject(this);
    }
    // ------------------------------------------------------------------------.
    // FluxTexture* loadTexture(std::string filename, int cols = 1, int rows = 1, bool setColorKeyAtZeroPixel = false, bool usePixelPerfect  = false);
    ConsoleMethod(Sprite, setTexture, bool, 3, 5, "(string filename),  int rows = 1 , int cols = 1 "
    "Set sprite setBitmap")
    {

        if (argc > 3) object->mTexCols = dAtoi(argv[3]);
        if (argc > 4) object->mTexRows = dAtoi(argv[4]);
        object->mTextureName = argv[2];

        FluxTexture*  tex = gMain->loadTexture(object->mTextureName, object->mTexCols, object->mTexRows);
        if (!tex) return false;
        object->mDrawParams.image = tex;

        return true;
    }


    // ------------------------------------------------------------------------.
    void Sprite::initPersistFields()
    {
        Parent::initPersistFields();
        addGroup("Texture");

        addField("Texture", TypeString, Offset(mTextureName, Sprite));
        addField("TexCols", TypeS32, Offset(mTexCols, Sprite));
        addField("TexRows", TypeS32, Offset(mTexRows, Sprite));


        endGroup("Texture");


        addGroup("Params");
        // TypeReqFloat << cause crash becuause storageRegister is nullptr
        // TypeS32 works
        // TypeF32 not because outputStorage->data.storageRegister it nullptr

        addField("imgId", TypeS32 , Offset(mDrawParams.imgId, Sprite) );
        addField("x", TypeF32, Offset(mDrawParams.x, Sprite));
        addField("y", TypeF32, Offset(mDrawParams.y, Sprite));
        addField("z", TypeF32, Offset(mDrawParams.z, Sprite));
        addField("w", TypeF32, Offset(mDrawParams.w, Sprite));
        addField("h", TypeF32, Offset(mDrawParams.h, Sprite));
        addField("rotation", TypeF32, Offset(mDrawParams.rotation, Sprite));
        addField("flipX", TypeBool, Offset(mDrawParams.flipX, Sprite));
        addField("flipY", TypeBool, Offset(mDrawParams.flipY, Sprite));
        endGroup("Params");

    }
    // ------------------------------------------------------------------------.
    void Sprite::Update(const double& dt){

    }
    // ------------------------------------------------------------------------.
    void Sprite::Draw() {
        if (!getVisible())
            return;

        Render2D.drawSprite(mDrawParams);
    }
    // ------------------------------------------------------------------------.

} //namespace

