#include "Texture.h"
#include "appMain.h"
#include <platform/platformString.h>

namespace KorkFlux {

    IMPLEMENT_CONOBJECT(Texture);
    // ------------------------------------------------------------------------.
    bool Texture::onAdd(){
        if (!gMain)  return false;

        if (*mTextureName) {
            mTexture =  gMain->loadTexture(mTextureName, mTexCols, mTexRows);
            if (!mTexture) return false;

        }

        Log("[info] Texture %d created.", getId());

        return Parent::onAdd();
    }
    // ------------------------------------------------------------------------.
    void Texture::onRemove() {

    }
    // ------------------------------------------------------------------------.
    // FluxTexture* loadTexture(std::string filename, int cols = 1, int rows = 1, bool setColorKeyAtZeroPixel = false, bool usePixelPerfect  = false);
    ConsoleMethod(Texture, setTexture, bool, 3, 5, "(string filename),  int rows = 1 , int cols = 1 "
    "Set / Override Textue ")
    {

        if (argc > 3) object->mTexCols = dAtoi(argv[3]);
        if (argc > 4) object->mTexRows = dAtoi(argv[4]);
        object->mTextureName = argv[2];

        object->mTexture =  gMain->loadTexture(object->mTextureName, object->mTexCols, object->mTexRows);
        if (!object->mTexture) return false;


        return true;
    }


    // ------------------------------------------------------------------------.
    void Texture::initPersistFields()
    {
        Parent::initPersistFields();
        addGroup("Texture");

        addField("Texture", TypeString, Offset(mTextureName, Texture));
        addField("TexCols", TypeS32, Offset(mTexCols, Texture));
        addField("TexRows", TypeS32, Offset(mTexRows, Texture));

        endGroup("Texture");

    }

} //namespace

