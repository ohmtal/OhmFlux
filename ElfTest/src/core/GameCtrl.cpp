#include "GameCtrl.h"

#include "appMain.h"
#include "core/Globals.h"
#include <platform/platformString.h>
#include "render/Texture.h"

#include "fonts/fluxTTFont.h"
#include "fonts/fluxLabel.h"
#include "gui/fonts/HackNerdFontPropo-Regular.h"

#include <console/console.h>


namespace ElfFlux {

IMPLEMENT_CONOBJECT(GameCtrl);


//--------------------------------------------------------------------------
void GameCtrl::initPersistFields()   {
    Parent::initPersistFields();
}
//--------------------------------------------------------------------------
bool GameCtrl::onAdd(){
    if (!gMain)  return false;

    mDefaultFont = new FluxTTFont();
    if (mDefaultFont->LoadFontFromMemory(HackNerdFontPropo_Regular_ttf, HackNerdFontPropo_Regular_ttf_len, 32))
    {
        mLabel = new FluxLabel(mDefaultFont);
    }


    mEventListener = true;

    gMain->queueObject(this);

    return Parent::onAdd();
}
//--------------------------------------------------------------------------
void GameCtrl::onRemove() {
    mEventListener = false;

    if (mDefaultFont) SAFE_DELETE(mDefaultFont);
    mDefaultFont = nullptr;
    if (mLabel) SAFE_DELETE(mLabel);
    mLabel = nullptr;

    gMain->unQueueObject(this);
    Parent::onRemove();
}
//--------------------------------------------------------------------------
void GameCtrl::onEvent(SDL_Event event) {

    switch (event.type) {
        case SDL_EVENT_KEY_UP:
        case SDL_EVENT_KEY_DOWN:  if ( isMethod( "onInputEvent" ) ) {
            // NOTE: keyname is only without modifierts like shift so a "(" becomes a 9 (qwertz)

            const char* keyName = SDL_GetKeyName(event.key.key);
            // dLog("KEY pressed: %s", keyName.c_str());

            KorkApi::ConsoleValue deviceString =  Con::getReturnBuffer("keyboard");
            KorkApi::ConsoleValue actionString = Con::getReturnBuffer(keyName);
            KorkApi::ConsoleValue mouseX = Con::getFloatArg(gAppStatus.MousePos.x);
            KorkApi::ConsoleValue mouseY =  Con::getFloatArg(gAppStatus.MousePos.y);
            KorkApi::ConsoleValue keyValue = Con::getBoolArg(event.type == SDL_EVENT_KEY_DOWN );

            // function invaderGame::onInputEvent( %this, %deviceString, %actionString, %mouseX, %mouseY, %keyValue ) {
            Con::executef( this, "onInputEvent"
            , deviceString
            , actionString
            , mouseX
            , mouseY
            , keyValue
            );
        }
        break;
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
        case SDL_EVENT_MOUSE_BUTTON_UP:   if ( isMethod( "onInputEvent" ) ) {
            char buttonNameBuf[32];
            dSprintf( buttonNameBuf, sizeof(buttonNameBuf), "button%d", event.button.button );
            KorkApi::ConsoleValue mouseX = Con::getFloatArg(gAppStatus.MousePos.x);
            KorkApi::ConsoleValue mouseY =  Con::getFloatArg(gAppStatus.MousePos.y);
            KorkApi::ConsoleValue keyValue = Con::getBoolArg(event.type == SDL_EVENT_MOUSE_BUTTON_DOWN );

            Con::executef( this, "onInputEvent"
            , Con::getReturnBuffer("mouse0") // mouse0 for TGE compat!
            , Con::getReturnBuffer(buttonNameBuf)
            , mouseX
            , mouseY
            , keyValue
            );
        }
        break;

    } //switch

}
//--------------------------------------------------------------------------
void GameCtrl::Draw() {
    //NOTE old dt was ms! so it's 0.016 instead of 16.6 now!
    if ( isMethod( "onRender" ) )  Con::executef( this, "onRender", Con::getFloatArg(gFrameTime ));
}
//--------------------------------------------------------------------------
void GameCtrl::Update(const double& dt) {
    //NOTE dt = ms! only for compat - bad or medicore performance
    if ( isMethod( "onUpdate" ) ) Con::executef( this, "onUpdate", Con::getFloatArg(dt));
}
//--------------------------------------------------------------------------
// NOTE: only for Compat! - use Sprite!
ConsoleMethod( GameCtrl, draw, ConsoleBool, 6, 6, "(Texture,x,y,layer)"
"draw a image, Layer 1-99 possible.")
{
    Texture* simTex = dynamic_cast<Texture*>(Sim::findObject(argv[2]));
    if (!simTex || !simTex->mTexture) return false;

    DrawParams2D params;
    params.image = simTex->mTexture;

    params.x = dAtof(argv[3]);
    params.y = dAtof(argv[4]);
    params.z = dAtof(argv[5]) / HS2D_MAXLAYERS;

    params.w = simTex->mTexture->getWidth();
    params.h = simTex->mTexture->getHeight();

    Render2D.drawSprite(params);
    return true;
}


// NOTE: only for Compat! - use Sprite!
ConsoleMethod( GameCtrl, drawstretch, ConsoleBool, 9, 14,
               "(Texture,imgId,x,y,layer,w,h,[rotation,flipX,flipY, alpha channel default 0.1], optimizetransparent)"
                "draw a image, Layer 1-99 possible.")
{
    Texture* simTex = dynamic_cast<Texture*>(Sim::findObject(argv[2]));
    if (!simTex || !simTex->mTexture) return false;

    DrawParams2D params;
    params.image = simTex->mTexture;
    params.imgId = dAtoi(argv[3]);

    params.x = dAtof(argv[4]);
    params.y = dAtof(argv[5]);
    params.z = dAtof(argv[6]) / HS2D_MAXLAYERS;

    params.w = dAtof(argv[7]);
    params.h = dAtof(argv[8]);

    if (argc > 9)
        params.rotation = dAtof(argv[9]);
    if (argc > 10)
        params.flipX = dAtob(argv[10]);
    if (argc > 11)
        params.flipY = dAtob(argv[11]);
    if (argc > 12)
        params.alpha = dAtof(argv[12]);
    // if (argc > 13)
    //     ldoBlend = dAtob(argv[13]);

    Render2D.drawSprite(params);
    return true;
}
// NOTE: only for Compat! - use Label!
ConsoleMethod( GameCtrl, writeText, ConsoleBool, 6, 8, "(x,y,string, align (0=left,2=right,3=center, color, bool do shadow"
"write text on screen.")
{

    // const Point2F FluxLabel::Print(const char* text, Point2F pos, FontAlign align )
    Point2F pos = { dAtof(argv[2]), dAtof(argv[3]) };
    const char* text = argv[4];
    FontAlign align = FontAlign_Left;
    Color4F color = cl_White;
    bool doShadow = false;
    if (argc > 5) align = (FontAlign)dAtoi(argv[5]);
    if (argc > 6) {
        dAtoColor4F(color, argv[6]);
    }


    if (!object->mLabel || !text) return false;
    object->mLabel->Print(text, pos, align, color, doShadow);

    // object->writeText(argv[4],lProfile, dAtoi(argv[2]),dAtoi(argv[3]),dAtoi(argv[5]), lFontColorType);
    return true;
}


// //---------
// ConsoleMethod( tom2DCtrl, drawRect, void, 10, 11, "(tom2DTexture,imgId,x,y,layer,w,h,srcRect,(optimizetransparent)"
// "draw a image from srcRect, Layer 1-99 possible.")
// {
//     tom2DTexture* obj = (tom2DTexture*)Sim::findObject(dAtoi(argv[2]));
//     F32 z = dAtof(argv[6]) / HS2D_MAXLAYERS;
//
//     F32  rot   = 0;
//     bool flipX = false;
//     bool flipY = false;
//     F32  lAlpha = 0.1f;
//     bool ldoBlend=false;
//     if (argc > 10)
//         ldoBlend = dAtob(argv[10]);
//
//     RectF lsrcRect;
//     dSscanf(argv[9],"%g %g %g %g",
//             &lsrcRect.point.x,&lsrcRect.point.y,&lsrcRect.extent.x,&lsrcRect.extent.y);
//
//
//     // void tom2DCtrl::drawRect(tom2DTexture* img, U32 imgId, Point3F worldPos,RectF lSrc, Point2I dimension, bool doBlend
//     object->drawRect(obj, dAtoi(argv[3]), Point3F(dAtof(argv[4]),dAtof(argv[5]),z), lsrcRect, Point2I(dAtoi(argv[7]),dAtoi(argv[8])),ldoBlend);
// }
//
//
//
// ConsoleMethod( tom2DCtrl, writeText, void, 6, 8, "(x,y,string, align (0=left,1=middle,2=right,3=center), profile, fontcolortype (0=normal,1=NA,2=HL)"
// "write text on screen.")
// {
//     GuiControlProfile *lProfile =  object->mProfile;
//     if (argc > 6)
//     {
//         SimObject *obj = Sim::findObject(argv[6]);
//         if(obj)
//             lProfile = static_cast<GuiControlProfile*>(obj);
//     }
//     U8 lFontColorType = 0;
//     if (argc > 7)
//         lFontColorType = dAtoi( argv[7] );
//
//     object->writeText(argv[4],lProfile, dAtoi(argv[2]),dAtoi(argv[3]),dAtoi(argv[5]), lFontColorType);
// }
// //------------------------------------------------------------------------------
//
// ConsoleMethod( tom2DCtrl, writeCustomText, void, 9, 9, "(x,y,text, align (0=left,1=middle,2=right,3=center), fontName, fontSize, fontColor ==> like 200 200 200 [255])"
// "write text on screen with font / fontsize and fontcolor.")
// {
//     //void tom2DCtrl::writeCustomText(const char* text, U32 x, U32 y, U32 align, const char* fontFace, U32 fontSize, const char* FontColorString)
//     object->writeCustomText(argv[4],dAtoi(argv[2]),dAtoi(argv[3]),dAtoi(argv[5]),argv[6],dAtoi(argv[7]),argv[8]);
//
// }
//
// //------------------------------------------------------------------------------
// // 2.24  writeColoredText
// ConsoleMethod(tom2DCtrl, writeColoredText, void, 8, 8,
//               "(x,y,text, align (0=left,1=middle,2=right,3=center), profile, fontColor ==> like 200 200 200 [255])"
//               "write text on screen with  fontcolor.")
// {
//     GuiControlProfile* lProfile = object->mProfile;
//     SimObject* obj = Sim::findObject(argv[6]);
//     if (obj)
//         lProfile = static_cast<GuiControlProfile*>(obj);
//
//
//     object->writeColoredText(argv[4], lProfile, dAtoi(argv[2]), dAtoi(argv[3]), dAtoi(argv[5]), object->scanColorText(argv[7]));
// }
// //------------------------------------------------------------------------------
// // 2.24  writeColoredText
// ConsoleMethod(tom2DCtrl, writeColoredShadowText, void, 11, 11,
//               "(x,y,text, align (0=left,1=middle,2=right,3=center), profile, xoffset,yoffset, fontColor ==> like 200 200 200 [255], shadowcolor)"
//               "write text on screen with  fontcolor.")
// {
//     GuiControlProfile* lProfile = object->mProfile;
//     SimObject* obj = Sim::findObject(argv[6]);
//     if (obj)
//         lProfile = static_cast<GuiControlProfile*>(obj);
//
//     int x = dAtoi(argv[2]);
//     int y = dAtoi(argv[3]);
//     int xoffset = dAtoi(argv[7]);
//     int yoffset = dAtoi(argv[8]);
//     ColorI fontColor = object->scanColorText(argv[9]);
//     ColorI shadowColor = object->scanColorText(argv[10]);
//
//     object->writeColoredText(argv[4], lProfile, x+xoffset,y+yoffset, dAtoi(argv[5]), shadowColor);
//     object->writeColoredText(argv[4], lProfile, x, y, dAtoi(argv[5]), fontColor);
// }
//------------------------------------------------------------------------------
} //namespace
