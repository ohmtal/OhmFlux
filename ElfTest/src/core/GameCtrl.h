// -----------------------------------------------------------------------------
// This is a wrapper for compat with my 2D scripts from TGE/OGE
//
// Methods (Note: SHOULD BE CALLED inside onRender loop):
// ConsoleMethod( tom2DCtrl, draw, void, 6, 6, "(tom2DTexture,x,y,layer)"
// "draw a image Note. Layer 1-9999 possible.")
//
// ConsoleMethod( tom2DCtrl, drawstretch, void, 9, 9, "(tom2DTexture,imgId,x,y,layer,w,h,rotation,flipX,flipY, alpha channel default 0.1)"
// "draw a image Note. Layer 1-9999 possible.")
//
// ConsoleMethod( tom2DCtrl, writeText, void, 5, 6, "(x,y,string, align (0=left,1=middle,2=right,3=center)"
// "write text on screen.")
//
// Events:
// Con::executef( this, 3, "onRender", getScreen()->getIdString(), Con::getIntArg(dt)); NOTE: INT???
// Con::executef( this, 2, "onUpdate", Con::getFloatArg(fDt));
// Con::executef( this, 6, "onInputEvent", deviceString, actionString, Con::getIntArg(mMousePos.x), Con::getIntArg(mMousePos.y), [bool up or down || axis value] );



// -----------------------------------------------------------------------------
#pragma once

#include "core/fluxBaseObject.h"
#include "render/fluxRender2D.h"
#include "console/simBase.h"
#include "console/consoleTypes.h"

class FluxTTFont;
class FluxLabel;
namespace ElfFlux {

    class GameCtrl : public SimSet, public FluxBaseObject {

        typedef SimSet Parent;

         FluxTTFont* mDefaultFont = nullptr;

    public:
        DrawParams2D mDrawParams;
        FluxLabel* mLabel = nullptr;

        FluxTTFont* getFont() { return mDefaultFont; }

        void onEvent(SDL_Event event) override;
        void Update(const double& dt) override;
        void Draw() override;

        static void initPersistFields();

        bool onAdd() override;
        void onRemove() override;


        DECLARE_CONOBJECT(GameCtrl);
        DECLARE_CALLBACK(void, onRender, (F32 DT));
        DECLARE_CALLBACK(void, onUpdate, (F32 DT));
        //function Game::onInputEvent( %this, %deviceString, %actionString, %mouseX, %mouseY, %keyValue ) {

        DECLARE_CALLBACK(void, onInputEvent, (String device, String action, F32 mouseX, F32 mouseY, String keyValue ));




    };


}
