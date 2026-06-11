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
GameCtrl::~GameCtrl() {
    if (gMain) gMain->unQueueObject(this);
}
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

            Con::executef( this, "onInputEvent"
            , "keyboard"
            , keyName
            , gAppStatus.MousePos.x
            , gAppStatus.MousePos.y
            , (event.type == SDL_EVENT_KEY_DOWN )
            );
        }
        break;
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
        case SDL_EVENT_MOUSE_BUTTON_UP:   if ( isMethod( "onInputEvent" ) ) {
            char buttonNameBuf[32];
            dSprintf( buttonNameBuf, sizeof(buttonNameBuf), "button%d", event.button.button );

            // Con::errorf("DEBUG: mouse action %s (%d)", buttonNameBuf, (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN ));
            Con::executef( this, "onInputEvent"
            , "mouse0"
            , Con::getReturnBuffer( buttonNameBuf )
            , gAppStatus.MousePos.x
            , gAppStatus.MousePos.y
            , (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN )
            );
        }
        break;

    } //switch

}
//--------------------------------------------------------------------------
void GameCtrl::Draw() {
    //NOTE old dt was ms! so it's 0.016 instead of 16.6 now!

    //FIXME
    // Direct leak of 45 byte(s) in 5 object(s) allocated from:
    // #0 0x7f042b12c161 in malloc (/usr/lib/libasan.so.8+0x12c161) (BuildId: ee5fbab73143ab257a66a33afe0f038a4af7a74e)
    // #1 0x56212582dc93 in dMalloc_r(unsigned long, char const*, unsigned long) /opt/TorqueScript/TorqueScript/platform/platformMemory.cpp:69
    // #2 0x5621257ab3db in ConsoleValue::setString(char const*, int) /opt/TorqueScript/TorqueScript/console/console.h:342
    // #3 0x5621257ab3db in ConsoleValue::copyFrom(ConsoleValue const&) /opt/TorqueScript/TorqueScript/console/console.h:466
    // #4 0x5621257aa4f6 in ConsoleValue::operator=(ConsoleValue const&) /opt/TorqueScript/TorqueScript/console/console.h:203
    // #5 0x5621257a5a61 in ConsoleValueStack<4096>::push(ConsoleValue) /opt/TorqueScript/TorqueScript/console/consoleValueStack.h:95
    // #6 0x5621257a5a61 in CodeBlock::exec(unsigned int, char const*, Namespace*, unsigned int, ConsoleValue*, bool, char const*, int) /opt/TorqueScript/TorqueScript/console/torquescript/compiledEval.cpp:2176
    // #7 0x5621256594b6 in Namespace::Entry::execute(int, ConsoleValue*, SimObject*) /opt/TorqueScript/TorqueScript/console/consoleInternal.cpp:1177
    // #8 0x5621255f248e in _internalExecute /opt/TorqueScript/TorqueScript/console/console.cpp:1276
    // #9 0x5621255fa4a4 in _BaseEngineConsoleCallbackHelper::_exec() /opt/TorqueScript/TorqueScript/console/console.cpp:2328
    // #10 0x562125389010 in ConsoleValue _EngineConsoleExecCallbackHelper<ElfFlux::GameCtrl*>::call<ConsoleValue, char const*, char*>(char const*, char*) /opt/TorqueScript/TorqueScript/console/engineAPI.h:1241
    // #11 0x562125385881 in ConsoleValue Con::executef<ElfFlux::GameCtrl*, char const*, char*>(ElfFlux::GameCtrl*, char const*, char*) /opt/TorqueScript/TorqueScript/console/console.h:1128
    // #12 0x56212537fea6 in ElfFlux::GameCtrl::Draw() /opt/OhmFlux/ElfTest/src/core/GameCtrl.cpp:96


    if ( isMethod( "onRender" ) )  Con::executef( this, "onRender", Con::getFloatArg(gFrameTime ));
}
//--------------------------------------------------------------------------
void GameCtrl::Update(const double& dt) {
    //NOTE dt = ms! only for compat - bad or medicore performance
    if ( isMethod( "onUpdate" ) ) Con::executef( this, "onUpdate", Con::getFloatArg(dt));
}
//--------------------------------------------------------------------------
DefineEngineMethod(GameCtrl, draw, bool, (Texture* simTex, F32 x, F32 y, F32 layer),
                   , "(Texture,x,y,layer)" "draw a image, Layer 1-99 possible.")
{
    if (!simTex || !simTex->mTexture) return false;

    DrawParams2D params;
    params.image = simTex->mTexture;

    params.x = x;
    params.y = y;
    params.z = layer / HS2D_MAXLAYERS;

    params.w = simTex->mTexture->getWidth();
    params.h = simTex->mTexture->getHeight();

    Render2D.drawSprite(params);
    return true;
}


// NOTE: only for Compat! - use Sprite!
DefineEngineMethod(GameCtrl, drawstretch, bool,
    (Texture* simTex, S32 imgId, F32 x, F32 y, F32 layer, F32 w,  F32 h
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
    params.z = layer / HS2D_MAXLAYERS;

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

} //namespace
