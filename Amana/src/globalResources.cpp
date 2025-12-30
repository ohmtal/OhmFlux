#include "globalResources.h"
#include "amanaGame.h"

bool GlobalResources::init()
{
    FontSourceCodeTexture = getGame()->loadTexture("assets/fonts/source_code_32x60.bmp",10,10,true, false);
    assert(FontSourceCodeTexture);

    StatusLabel = new FluxBitmapFont(FontSourceCodeTexture);
    StatusLabel->set("...STATUS....", getGame()->getScreen()->getCenterX(), 10, 20, 24, { 0.9f, 0.9f, 1.f, 1.f} );
    StatusLabel->setAlign(FontAlign_Center);
    StatusLabel->setColor( cl_Crimson );
    getGame()->queueObject(StatusLabel);

    GuiEvents = new FluxGuiEventManager();

    return true;
}
