/*
 * sol2: https://github.com/ThePhD/sol2
 *
 */
#pragma once
#include <fluxMain.h>
#include <sol/sol.hpp>
//==============================================================================
void bindFluxBaseObject(sol::state& lua) {
    // Register the root class
    auto type = lua.new_usertype<FluxBaseObject>("FluxBaseObject",
                                                 sol::constructors<FluxBaseObject()>()
    );

    // Visibility
    type["setVisible"] = &FluxBaseObject::setVisible;
    type["getVisible"] = &FluxBaseObject::getVisible;

    // Execution Flow (Virtuals)
    type["execute"] = &FluxBaseObject::Execute;
    type["initialize"] = &FluxBaseObject::Initialize;
    type["deinitialize"] = &FluxBaseObject::Deinitialize;
    type["update"] = &FluxBaseObject::Update;
    type["draw"] = &FluxBaseObject::Draw;

    // Internal Management
    type["setScheduleUsed"] = &FluxBaseObject::setScheduleUsed;
}
//==============================================================================
void bindFluxMain(sol::state& lua) {
    // 1. Create the usertype with ONLY base classes.
    // This defines the inheritance without triggering the complex argument-pairing check.
    auto type = lua.new_usertype<FluxMain>("FluxMain",
                                           sol::base_classes, sol::bases<FluxBaseObject>()
    );

    // 2. Manually set the constructor.
    // This bypasses the variadic constructor logic entirely.
    type.set(sol::call_constructor, sol::constructors<FluxMain()>());

    // 3. Optional: Bind your logic as you did before.
    type["initialize"] = &FluxMain::Initialize;
    type["update"] = &FluxMain::Update;
    type["loadTexture"] = sol::overload(
        [](FluxMain& self, const char* f) { return self.loadTexture(f); },
                                        [](FluxMain& self, const char* f, int c) { return self.loadTexture(f, c); },
                                        [](FluxMain& self, const char* f, int c, int r) { return self.loadTexture(f, c, r); },
                                        &FluxMain::loadTexture
    );
    type["getScreen"] = &FluxMain::getScreen;
    type["terminateApplication"] = &FluxMain::TerminateApplication;
    type["toggleFullScreen"] = &FluxMain::toggleFullScreen;
    type["getStatus"] = &FluxMain::getStatus;
    type["getFPS"] = &FluxMain::getFPS;
    type["setPause"] = &FluxMain::setPause;
    type["togglePause"] = &FluxMain::togglePause;
    type["getPause"] = &FluxMain::getPause;
    type["queueObject"] = &FluxMain::queueObject;
    type["unQueueObject"] = &FluxMain::unQueueObject;
    type["queueDelete"] = &FluxMain::queueDelete;
    type["getQueueObjects"] = &FluxMain::getQueueObjects;
    type["getQuadtree"] = &FluxMain::GetQuadtree;
    type["setupMousePositions"] = &FluxMain::setupMousePositions;
    type["setupWorldMousePositions"] = &FluxMain::setupWorldMousePositions;
    type["execute"] = &FluxMain::Execute;
    type["iterateFrame"] = &FluxMain::IterateFrame;
    type["initialize"] = &FluxMain::Initialize;
    type["deinitialize"] = &FluxMain::Deinitialize;
    type["update"] = &FluxMain::Update;
    type["draw"] = &FluxMain::Draw;
    type["settings"] = &FluxMain::mSettings;
}
//==============================================================================

void bindFluxTexture(sol::state& lua) {
    auto type = lua.new_usertype<FluxTexture>("FluxTexture",
                                              sol::no_constructor
    );

    type["getWidth"] = &FluxTexture::getWidth;
    type["getHeight"] = &FluxTexture::getHeight;
    type["getCols"] = &FluxTexture::getCols;
    type["getRows"] = &FluxTexture::getRows;
    type["savePNGToFile"] = &FluxTexture::savePNGToFile;
    type["getHandle"] = &FluxTexture::getHandle;
}
//==============================================================================
void bindDrawParams2D(sol::state& lua) {
    lua.new_usertype<DrawParams2D>("DrawParams2D",
                                   // Position and Layer
                                   "x", &DrawParams2D::x,
                                   "y", &DrawParams2D::y,
                                   "z", &DrawParams2D::z,
                                   "w", &DrawParams2D::w,
                                   "h", &DrawParams2D::h,

                                   // Visuals
                                   "imgId", &DrawParams2D::imgId,
                                   "rotation", &DrawParams2D::rotation,
                                   "alpha", &DrawParams2D::alpha,
                                   "color", &DrawParams2D::color,
                                   "flipX", &DrawParams2D::flipX,
                                   "flipY", &DrawParams2D::flipY,

                                   // Flags
                                   "isGuiElement", &DrawParams2D::isGuiElement,
                                   "useUV", &DrawParams2D::useUV,

                                   // Helpers
                                   "getLayer", &DrawParams2D::getLayer,
                                   "getTexture", &DrawParams2D::getTexture
    );
}
//==============================================================================
void bindFluxRenderObject(sol::state& lua) {
    // Register the usertype with its base class
    auto type = lua.new_usertype<FluxRenderObject>("FluxRenderObject",
                                                   sol::base_classes, sol::bases<FluxBaseObject>()
    );

    // Constructor Factory
    type["new"] = sol::factories(
        [](FluxTexture* tex, FluxScreen* screen) {
            return new FluxRenderObject(tex, screen);
        },
        [](FluxTexture* tex, FluxScreen* screen, int fs, int fe) {
            return new FluxRenderObject(tex, screen, fs, fe);
        }
    );

    // Getters
    // Explicitly pick the non-const version: DrawParams2D& ()
    type["getDrawParams"] = sol::resolve<DrawParams2D&()>(&FluxRenderObject::getDrawParams);
    //<< most important!

    type["getX"] = &FluxRenderObject::getX;
    type["getY"] = &FluxRenderObject::getY;
    type["getWidth"] = &FluxRenderObject::getWidth;
    type["getHeight"] = &FluxRenderObject::getHeight;
    type["getRotation"] = &FluxRenderObject::getRotation;
    type["getLayer"] = &FluxRenderObject::getLayer;
    type["getTexture"] = &FluxRenderObject::getTexture;
    type["getIsGuiElement"] = &FluxRenderObject::getIsGuiElement;
    type["getPosition"] = &FluxRenderObject::getPosition;

    // Setters
    type["setX"] = &FluxRenderObject::setX;
    type["setY"] = &FluxRenderObject::setY;
    type["setPos"] = sol::overload(
        sol::resolve<void(const F32&, const F32&)>(&FluxRenderObject::setPos),
                                   sol::resolve<void(Point2F)>(&FluxRenderObject::setPos)
    );
    type["setLayer"] = &FluxRenderObject::setLayer;
    type["setRotation"] = &FluxRenderObject::setRotation;
    type["setIsGuiElement"] = &FluxRenderObject::setIsGuiElement;
    type["setTexture"] = &FluxRenderObject::setTexture;
    type["setAnimationDelay"] = &FluxRenderObject::setAnimationDelay;

    // Logic and Rendering
    type["update"] = &FluxRenderObject::Update;
    type["draw"] = &FluxRenderObject::Draw;
    type["pointCollide"] = sol::overload(
        sol::resolve<bool(const Point2F&)>(&FluxRenderObject::pointCollide),
                                         sol::resolve<bool(const F32&, const F32&)>(&FluxRenderObject::pointCollide)
    );
}

//==============================================================================
void bindFluxScreen(sol::state& lua) {
    // DO NOT include sol::bases<FluxBaseObject> here!
    auto type = lua.new_usertype<FluxScreen>("FluxScreen",
                                             sol::constructors<FluxScreen(), FluxScreen(FluxScreen::VideoMode)>()
    );

    type["getWidth"] = &FluxScreen::getWidth;
    type["getHeight"] = &FluxScreen::getHeight;
    type["getCenterX"] = &FluxScreen::getCenterX;
    type["getCenterY"] = &FluxScreen::getCenterY;
    type["setCaption"] = &FluxScreen::setCaption;
    type["toggleFullScreen"] = &FluxScreen::toggleFullScreen;
    // Add other methods as needed
}
//==============================================================================
void bindFluxBitmapFont(sol::state& lua) {
    auto type = lua.new_usertype<FluxBitmapFont>("FluxBitmapFont",
                                                 sol::base_classes, sol::bases<FluxRenderObject, FluxBaseObject>()
    );

    // Use sol::factories with stack_object for maximum debuggability
    type["new"] = sol::factories(
        [](sol::stack_object tex_obj, sol::stack_object screen_obj) -> FluxBitmapFont* {
            // Check if these are actually the types we expect
            if (!tex_obj.is<FluxTexture*>()) {
                throw sol::error("Font Constructor Error: Argument 1 is not a FluxTexture pointer!");
            }
            if (!screen_obj.is<FluxScreen*>()) {
                throw sol::error("Font Constructor Error: Argument 2 is not a FluxScreen pointer!");
            }

            return new FluxBitmapFont(
                tex_obj.as<FluxTexture*>(),
                                      screen_obj.as<FluxScreen*>()
            );
        }
    );

    type["set"] = sol::overload(
        // Original 5-arg version (uses C++ default color)
        [](FluxBitmapFont& self, const char* c, int x, int y, int w, int h) {
            self.set(c, x, y, w, h);
        },
        // NEW 6-arg version: Accepts a Lua table for the color
        [](FluxBitmapFont& self, const char* c, int x, int y, int w, int h, sol::table colTable) {
            Color4F color;
            // Access table by index (Lua indices start at 1)
            color.r = colTable.get_or(1, 1.0f);
            color.g = colTable.get_or(2, 1.0f);
            color.b = colTable.get_or(3, 1.0f);
            color.a = colTable.get_or(4, 1.0f);

            self.set(c, x, y, w, h, color);
        },
        // Original C++ signature (keeps it available if you pass a bound Color4F object)
        &FluxBitmapFont::set
    );
    type["setCaption"] = [](FluxBitmapFont& self, std::string text) {
        self.setCaption("%s", text.c_str());
    };
}
//==============================================================================
void bindFluxAudioStream(sol::state& lua) {
    // 1. Create usertype with inheritance
    auto type = lua.new_usertype<FluxAudioStream>("FluxAudioStream",
                                                  sol::base_classes, sol::bases<FluxBaseObject>()
    );

    // 2. Define .new() explicitly - This fixes the nil error
    type["new"] = sol::factories(
        [](const char* filename) {
            return new FluxAudioStream(filename);
        }
    );

    // 3. Playback Controls
    type["play"] = &FluxAudioStream::play;
    type["stop"] = &FluxAudioStream::stop;
    type["resume"] = &FluxAudioStream::resume;
    type["isPlaying"] = &FluxAudioStream::isPlaying;

    // 4. Settings
    type["setLooping"] = &FluxAudioStream::setLooping;
    type["setGain"] = &FluxAudioStream::setGain;
    type["getGain"] = &FluxAudioStream::getGain;
    type["getInitDone"] = &FluxAudioStream::getInitDone;

    // 5. Position
    type["setPosition"] = &FluxAudioStream::setPositon;

    // 6. Logic
    type["update"] = &FluxAudioStream::Update;
}
//==============================================================================
