#include "Texture.h"
#include "appMain.h"
#include "core/Globals.h"

#include "console/engineAPI.h"

namespace ElfFlux {

    IMPLEMENT_CONOBJECT(Texture);
    // ------------------------------------------------------------------------.
    bool Texture::onAdd(){
        if (!gMain)  return false;

        if (*mFileName) {
            mTexture =  gMain->loadTexture(mFileName, mTexCols, mTexRows);
        }
        // fallback to one pixel white texture
        // 1 pixel have no cols or rows ;)
        if (!mTexture) mTexture = Render2D.getWhiteTexture();

        Log("[info] Texture %d created.", getId());

        return Parent::onAdd();
    }
    // ------------------------------------------------------------------------.
    void Texture::onRemove() {
        Parent::onRemove();
    }
    // ------------------------------------------------------------------------.
    // FluxTexture* loadTexture(std::string filename, int cols = 1, int rows = 1, bool setColorKeyAtZeroPixel = false, bool usePixelPerfect  = false);

    DefineEngineMethod(Texture, setTexture, bool
        , (String filename, S32 rows, S32 cols) //parameters
        , (1,1) // default values from behind
        , "(string filename),  int rows = 1 , int cols = 1 ")
    {
        object->mTexCols = cols;
        object->mTexRows = rows;
        object->mFileName = filename;
        object->mTexture =  gMain->loadTexture(object->mFileName, object->mTexCols, object->mTexRows);
        if (!object->mTexture) return false;
        return true;
    };

    // ConsoleMethod(Texture, setTextureX, bool, 3, 5, "(string filename),  int rows = 1 , int cols = 1 "
    // "Set / Override Textue ")
    // {
    //
    //     if (argc > 3) object->mTexCols = dAtoi(argv[3]);
    //     if (argc > 4) object->mTexRows = dAtoi(argv[4]);
    //     object->mFileName = argv[2];
    //
    //     object->mTexture =  gMain->loadTexture(object->mFileName, object->mTexCols, object->mTexRows);
    //     if (!object->mTexture) return false;
    //
    //
    //     return true;
    // }

    DefineEngineMethod(Texture, getSize, Point2I,(),,"get the size of the texture") {
        return { object->mTexture->getWidth(),object->mTexture->getHeight() };
    }


    // ------------------------------------------------------------------------.
    void Texture::initPersistFields()
    {
        Parent::initPersistFields();
        addGroup("Texture");

        addField("filename", TypeString, Offset(mFileName, Texture), "If not set White Texture (one pixel is used");
        addField("TexCols", TypeS32, Offset(mTexCols, Texture));
        addField("TexRows", TypeS32, Offset(mTexRows, Texture));

        endGroup("Texture");

    }

} //namespace

