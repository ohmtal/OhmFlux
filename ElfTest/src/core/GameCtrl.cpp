#include "GameCtrl.h"

#include "appMain.h"
#include "core/Globals.h"
#include "console/engineAPI.h"
#include "render/Texture.h"

#include "fonts/fluxTTFont.h"
#include "fonts/fluxLabel.h"
#include "gui/fonts/HackNerdFontPropo-Regular.h"

#include <console/console.h>


namespace ElfFlux {

IMPLEMENT_CONOBJECT(GameCtrl);
IMPLEMENT_CALLBACK( GameCtrl, onRender, void, (  F32 DT), (  DT ),
                    "Called on Render Loop, dt is only for compat with old scripts - better use onUpdate!\n"
                    "@param DT FrameTime [deprectated] usually 0.016.. (60fps)\n"
);
IMPLEMENT_CALLBACK( GameCtrl, onUpdate, void, ( F32 DT), (  DT ),
                    "Called on Update Loop - not as often calles as onRender.\n"
                    "@param DT usually 0.016.. d \n"
);
IMPLEMENT_CALLBACK( GameCtrl, onInputEvent, void,
                    (String device, String action, F32 mouseX, F32 mouseY, String keyValue ),
                    (  device, action, mouseX, mouseY, keyValue ),
                    "Called on Input Event.\n"
                    "@param device keyboard,mouse... \n"
                    "@param action action string \n"
                    "@param mouseX mouse X position \n"
                    "@param mouseY mouse Y position \n"
                    "@param keyValue value of the action \n"
);

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

            onInputEvent_callback(
                "keyboard"
                , keyName
                , gAppStatus.MousePos.x
                , gAppStatus.MousePos.y
                , (event.type == SDL_EVENT_KEY_DOWN ) ? "1" : "0"
            );

            // Con::executef( this, "onInputEvent"
            // , "keyboard"
            // , keyName
            // , gAppStatus.MousePos.x
            // , gAppStatus.MousePos.y
            // , (event.type == SDL_EVENT_KEY_DOWN )
            // );
        }
        break;
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
        case SDL_EVENT_MOUSE_BUTTON_UP:   if ( isMethod( "onInputEvent" ) ) {
            char buttonNameBuf[32];
            dSprintf( buttonNameBuf, sizeof(buttonNameBuf), "button%d", event.button.button );
            onInputEvent_callback(
                "mouse0"
                , buttonNameBuf
                , gAppStatus.MousePos.x
                , gAppStatus.MousePos.y
                , (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN ) ? "1" : "0"
            );

            // Con::errorf("DEBUG: mouse action %s (%d)", buttonNameBuf, (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN ));
            // Con::executef( this, "onInputEvent"
            // , "mouse0"
            // , buttonNameBuf
            // , gAppStatus.MousePos.x
            // , gAppStatus.MousePos.y
            // , (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN )
            // );
        }
        break;

    } //switch

}

// FIXME mouseMove for mouse over ....
// void GameCtrl::sendMouseEvent(const char* name, const GuiEvent& event)
// {
//     char buf[3][32];
//     dSprintf(buf[0], 32, "%d", event.modifier);
//     dSprintf(buf[1], 32, "%d %d", mMousePos.x, mMousePos.y);
//     dSprintf(buf[2], 32, "%d", event.mouseClickCount);
//     //Con::executef(this, 4, name, buf[0], buf[1], buf[2]);
//     Con::executef(this, name, buf[0], buf[1], buf[2]);
// }
//--------------------------------------------------------------------------
void GameCtrl::Draw() {
    //NOTE old dt was ms! so it's 0.016 instead of 16.6 now!

    // if ( isMethod( "onRender" ) )  Con::executef( this, "onRender", Con::getFloatArg(gFrameTime ));
    // -----------

    onRender_callback((F32)gFrameTime );
}
//--------------------------------------------------------------------------
void GameCtrl::Update(const double& dt) {
    onUpdate_callback((F32)dt);
    // if ( isMethod( "onUpdate" ) ) Con::executef( this, "onUpdate", Con::getFloatArg(dt));
}
//--------------------------------------------------------------------------
DefineEngineMethod(GameCtrl, draw, bool, (Texture* simTex, F32 x, F32 y, U32  layer),
                   , "(Texture,x,y,layer)" "draw a image, Layer 1-999 possible.")
{
    if (!simTex || !simTex->mTexture) return false;

    DrawParams2D params;
    params.image = simTex->mTexture;

    params.x = x;
    params.y = y;
    params.z = LAYER_F(layer);

    params.w = simTex->mTexture->getWidth();
    params.h = simTex->mTexture->getHeight();

    Render2D.drawSprite(params);
    return true;
}


// NOTE: only for Compat! - use Sprite!
DefineEngineMethod(GameCtrl, drawstretch, bool,
    (Texture* simTex, S32 imgId, F32 x, F32 y, U32 layer, F32 w,  F32 h
     , F32 rot, bool flipX, bool flipY, F32 alpha,Color4F color,bool dummy3)
    , (32.f, 32.f , 0.f,false,false, 1.f, cl_White, false),
    "(Texture,imgId,x,y,layer,w,h,[rot ,flipX,flipY, alpha, color, dummy])"
    "draw a image, Layer 1-99 possible.")
{
    if (!simTex || !simTex->mTexture) return false;

    DrawParams2D params;
    params.image = simTex->mTexture;
    params.imgId = imgId;

    params.x = x;
    params.y = y;
    params.z = LAYER_F(layer);

    params.w = w;
    params.h = h;

    params.rotation = rot;
    params.flipX = flipY;
    params.flipY = flipX;
    params.alpha = alpha;
    params.color = color;

    Render2D.drawSprite(params);
    return true;
}
// NOTE: only for Compat! - use Label!
DefineEngineMethod(GameCtrl, writeText, bool,
  (F32 x, F32 y, const char* text, S32 align, Color4F color, bool doShadow)
, (0, cl_White, false)
,"(x,y,string, align (0=left,1=center,2=right), profile, color, bool shadowColoer"
"write text on screen.")
{

    if (!object->mLabel || !text) return false;
    if (align < 0 || align > FontAlign_Right) align = 0;
    object->mLabel->Print(text, {x,y}, (FontAlign)align, color, doShadow);
    return true;
}

//------------------------------------------------------------------------------
//primitives:
//------------------------------------------------------------------------------

DefineEngineMethod(GameCtrl, Line, void
    , (Point2F startPt, Point2F endPt, Color4F color, U32 layer)
    , (cl_White, 0) , "draw a line")
{
    Render2D.drawLine(startPt, endPt, color,LAYER_F(layer));
}

DefineEngineMethod(GameCtrl, Rect, void
, (RectF rect, Color4F color, bool filled , U32 layer)
, (cl_White, true, 0) , "draw a Rect")
{
    Render2D.drawRect(rect, color, filled, LAYER_F(layer));
}

DefineEngineMethod(GameCtrl, Circle, void
, (Point2F center, F32 radius,  Color4F color, U32 segments, U32 layer )
, (cl_White, 32, 0) , "draw a Circle")
{
    Render2D.drawCircle(center, radius, color, segments, LAYER_F(layer));
}

DefineEngineMethod(GameCtrl, Elipse, void
, (Point2F center, Point2F axes,  Color4F color, U32 segments, U32 layer )
, (cl_White, 32, 0) , "draw a Circle")
{
    Render2D.drawEllipse(center, axes, color, segments, LAYER_F(layer));
}

DefineEngineMethod(GameCtrl, Tringle, void
, (Point2F p1, Point2F p2, Point2F p3, Color4F color, bool filled, U32 layer )
, (cl_White, true, 0) , "draw a Tringle")
{
    Render2D.drawTriangle( p1,  p2,  p3,  color ,  filled, LAYER_F(layer) );
}


} //namespace
