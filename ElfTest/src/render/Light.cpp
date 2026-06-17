#include "Light.h"
#include "core/Globals.h"
#include "lights/fluxLightManager.h"
#include "console/engineAPI.h"
#include "render/fluxRender2D.h"
namespace ElfFlux {

// -----------------------------------------------------------------------------
// Global LightManager functions
// -----------------------------------------------------------------------------
DefineEngineFunction(setAmbientColor,void, (Color4F color), , "Set the scene AmbientColor") {
    Render2D.setAmbientColor(color);
}
DefineEngineFunction(clearLights,void, (), , "clear all lights") {
    LightManager.clearLights();
}
// -----------------------------------------------------------------------------
// Light
// -----------------------------------------------------------------------------
IMPLEMENT_CONOBJECT(Light);

void Light::initPersistFields() {
    // Parent::initPersistFields();
    addGroup("Light");
    addField("position", TypePoint3F, Offset(mLight.position, Light), "");
    addField("color",    TypeColorF,  Offset(mLight.color, Light), "");
    addField("radius", TypeF32, Offset(mLight.radius, Light), "");
    addField("direction", TypePoint2F, Offset(mLight.direction, Light), "Spotlight direction, Default pointing down or forward");
    addField("cutoff", TypeF32, Offset(mLight.cutoff, Light), "Spotlight cutoff, default -1 == omnilight");
    endGroup("Light");
}
bool Light::onAdd() {
    LightManager.addLight(&mLight, false); // no autoDelete!
    return Parent::onAdd();
}
void Light::onRemove() {
    LightManager.removeLight(&mLight);
    Parent::onRemove();
}

DefineEngineMethod(Light, setAsSpotlight, void, (Point2F dir, F32 angleInDegrees), ,
        "Set as Spotlight with direction and angle in degrees") {
    object->mLight.setAsSpotlight(dir, angleInDegrees);
}

} //namespace
