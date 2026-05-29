#include "Font.h"
#include <platform/platformString.h>

namespace KorkFlux {

    IMPLEMENT_CONOBJECT(Font);
    // ------------------------------------------------------------------------.
    bool Font::onAdd(){

        if (*mFileName) {
            if (mFontSize < 8.f) {
                Con::errorf("Fontsize too small! %f", mFontSize);
                return false;
            }
            if (mTextureSize < 128 ) {
                Con::errorf("Font Texture too small! %f", mTextureSize);
                return false;
            }
            if (mTextureSize > 4096 ) {
                Con::errorf("Font Texture too big! %f", mTextureSize);
                return false;
            }

            mFont = new FluxTTFont;
            // bool LoadFont(const char* filename, F32 fontSize = 32.0f, U32 textureSize = 512);
            if (!mFont->LoadFont(mFileName, mFontSize, mTextureSize)) return false;

        } else {
            return false;
        }

        Log("[info] Font %d created.", getId());

        return Parent::onAdd();
    }
    // ------------------------------------------------------------------------.
    void Font::onRemove() {
        SAFE_DELETE(mFont);
        mFont = nullptr;
        Parent::onRemove();
    }

    // ------------------------------------------------------------------------.
    void Font::initPersistFields()
    {
        Parent::initPersistFields();
        addGroup("Font");

        addField("fileName", TypeString, Offset(mFileName, Font));
        addField("fontSize", TypeS32, Offset(mFontSize, Font));
        addField("textureSize", TypeS32, Offset(mTextureSize, Font));

        endGroup("Font");
    }

} //namespace

