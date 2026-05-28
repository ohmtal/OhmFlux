#include "luaBindings.h"
#include <fluxMain.h>
#include <sol/sol.hpp>
#include <SDL3/SDL.h>
#include <fonts/fluxTTFont.h>
#include <fonts/fluxLabel.h>

namespace OhmFlux::Lua {

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
        auto type = lua.new_usertype<FluxMain>("FluxMain",
                                               sol::base_classes, sol::bases<FluxBaseObject>()
        );
        type.set(sol::call_constructor, sol::constructors<FluxMain()>());
        type["initialize"] = &FluxMain::Initialize;
        type["update"] = &FluxMain::Update;
        type["loadTexture"] = sol::overload(
            [](FluxMain& self, const char* f) { return self.loadTexture(f); },
                [](FluxMain& self, const char* f, int c) { return self.loadTexture(f, c); },
                [](FluxMain& self, const char* f, int c, int r) { return self.loadTexture(f, c, r); },
                [](FluxMain& self, const char* f, int c, int r, bool transZeroPixel, bool pixelPerfect ) { return self.loadTexture(f, c, r, transZeroPixel, pixelPerfect); },
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
        auto type = lua.new_usertype<DrawParams2D>("DrawParams2D",
                                       "image", &DrawParams2D::image,
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

                                       ,"getRectF",  &DrawParams2D::getRectF
                                       ,"setRectF",  &DrawParams2D::setRectF



        );


        type["Draw"] = [](DrawParams2D& self) {
            return Render2D.drawSprite(self);
        };
        type["setRectF"] = &DrawParams2D::setRectF;
        type["getRectF"] = &DrawParams2D::getRectF;



    }
    //==============================================================================
    void bindFluxRenderObject(sol::state& lua) {
        lua.new_usertype<Point2I>("Point2I",
                                  sol::constructors<Point2I(), Point2I(float, float)>(),
                                  "x", &Point2I::x,
                                  "y", &Point2I::y
        );
        lua.new_usertype<Point2F>("Point2F",
                                  sol::constructors<Point2F(), Point2F(float, float)>(),
                                  "x", &Point2F::x,
                                  "y", &Point2F::y,

                                  sol::meta_function::addition, sol::overload(
                                      [](const Point2F& a, const Point2F& b) { return a + b; },
                                                                              [](const Point2F& a, float b) { return a + b; }
                                  ),
                                  sol::meta_function::subtraction, sol::overload(
                                      [](const Point2F& a, const Point2F& b) { return a - b; },
                                                                                 [](const Point2F& a, float b) { return a - b; }
                                  ),
                                  sol::meta_function::multiplication, sol::overload(
                                      [](const Point2F& a, const Point2F& b) { return a * b; },
                                                                                    [](const Point2F& a, float b) { return a * b; }
                                  ),
                                  sol::meta_function::division, sol::overload(
                                      [](const Point2F& a, const Point2F& b) { return a / b; },
                                                                              [](const Point2F& a, float b) { return a / b; }
                                  ),

                                  sol::meta_function::equal_to, &Point2F::operator==,
                                  "dist", &Point2F::dist,
                                  "len", &Point2F::len,
                                  "normalize", &Point2F::normalize,
                                  "normalized", &Point2F::normalized,
                                  "dot", &Point2F::dot,
                                  "cross", &Point2F::cross,
                                  "toPoint2I", &Point2F::toPoint2I,

                                  // nicer print ;)
                                  sol::meta_function::to_string, [](const Point2F& p) {
                                      return "Point2F(" + std::to_string(p.x) + ", " + std::to_string(p.y) + ")";
                                  }
        );

        lua.new_usertype<Point3F>("Point3F",
                                  sol::constructors<Point3F(), Point3F(float, float)>(),
                                  "x", &Point3F::x,
                                  "y", &Point3F::y,
                                  "z", &Point3F::z,
                                  sol::meta_function::addition,       &Point3F::operator+,
                                  sol::meta_function::subtraction,    &Point3F::operator-,
                                  sol::meta_function::multiplication, &Point3F::operator*,
                                  sol::meta_function::division,       &Point3F::operator/,
                                  sol::meta_function::equal_to,       &Point3F::operator==
        );

        lua.new_usertype<RectI>("RectI",
                                sol::constructors<RectI(), RectI(int, int, int, int )>(),
                                "x", &RectI::x,
                                "y", &RectI::y,
                                "w", &RectI::w,
                                "h", &RectI::h,
                                "getPoint", &RectI::getPoint,
                                "getExtent", &RectI::getExtent,
                                "pointInRect", &RectI::pointInRect,
                                "len_x", &RectI::len_x,
                                "len_y", &RectI::len_y,
                                "isValidRect", &RectI::isValidRect,

                                "contains", &RectI::contains,
                                "intersects", &RectI::intersects


        );


        lua.new_usertype<RectF>("RectF",
                                sol::constructors<RectF(), RectF(float, float, float, float )>(),
                                "x", &RectF::x,
                                "y", &RectF::y,
                                "w", &RectF::w,
                                "h", &RectF::h,

                                "getPoint", &RectF::getPoint,
                                "getExtent", &RectF::getExtent,
                                "pointInRect", &RectF::pointInRect,

                                "len_x", &RectF::len_x,
                                "len_y", &RectF::len_y,
                                "isValidRect", &RectF::isValidRect,
                                "asRectI", &RectF::asRectI,
                                "inflate", &RectF::inflate,

                                "contains", &RectF::contains,
                                "intersects", &RectF::intersects


        );

        lua.new_usertype<Color4F>("Color4F",
                                  sol::constructors<Color4F(), Color4F(float, float, float, float)>(),
                                  "r", &Color4F::r,
                                  "g", &Color4F::g,
                                  "b", &Color4F::b,
                                  "a", &Color4F::a
        );




        // Register the usertype with its base class
        auto type = lua.new_usertype<FluxRenderObject>("FluxRenderObject",
                                                       sol::base_classes, sol::bases<FluxBaseObject>()
        );

        // Constructor Factory
        type["new"] = sol::factories(
            [](FluxTexture* tex) {
                return new FluxRenderObject(tex);
            },
            [](FluxTexture* tex,  int fs, int fe) {
                return new FluxRenderObject(tex, fs, fe);
            },
            [](FluxTexture* tex,  const RectF rect) -> FluxRenderObject* {
                FluxRenderObject* result =  new FluxRenderObject(tex);
                if (!result) return nullptr;
                result->setRectF(rect);
                return result;
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
        type["setPosition"] = &FluxRenderObject::setPosition;
        type["setPos"] = sol::overload(
            sol::resolve<void(const F32&, const F32&)>(&FluxRenderObject::setPos),
                                       sol::resolve<void(Point2F)>(&FluxRenderObject::setPos)
        );

        type["getRectF"] = &FluxRenderObject::getRectF;
        type["setRectF"] = &FluxRenderObject::setRectF;

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

        type["getSpeed"] = &FluxRenderObject::getSpeed;
        type["getVelocity"] = &FluxRenderObject::getVelocity;

        type["setSpeed"] = &FluxRenderObject::setSpeed;
        type["setVelocity"] = &FluxRenderObject::setVelocity;
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
    void bindFluxFonts(sol::state& lua) {

        // BitMapLabel
        {
            auto type = lua.new_usertype<FluxBitmapLabel>("FluxBitmapLabel",
                                                          sol::base_classes, sol::bases<FluxRenderObject, FluxBaseObject>()
            );

            // Use sol::factories with stack_object for maximum debuggability
            type["new"] = sol::factories(
                [](sol::stack_object tex_obj ) -> FluxBitmapLabel* {
                    // Check if these are actually the types we expect
                    if (!tex_obj.is<FluxTexture*>()) {
                        throw sol::error("Font Constructor Error: Argument 1 is not a FluxTexture pointer!");
                    }

                    return new FluxBitmapLabel( tex_obj.as<FluxTexture*>() );
                }
            );

            type["set"] = sol::overload(
                // Original 5-arg version (uses C++ default color)
                [](FluxBitmapLabel& self, const char* c, int x, int y, int w, int h) {
                    self.set(c, x, y, w, h);
                },
                // NEW 6-arg version: Accepts a Lua table for the color
                [](FluxBitmapLabel& self, const char* c, int x, int y, int w, int h, sol::table colTable) {
                    Color4F color;
                    // Access table by index (Lua indices start at 1)
                    color.r = colTable.get_or(1, 1.0f);
                    color.g = colTable.get_or(2, 1.0f);
                    color.b = colTable.get_or(3, 1.0f);
                    color.a = colTable.get_or(4, 1.0f);

                    self.set(c, x, y, w, h, color);
                },
                // Original C++ signature (keeps it available if you pass a bound Color4F object)
                &FluxBitmapLabel::set
            );
            type["setCaption"] = [](FluxBitmapLabel& self, std::string text) {
                self.setCaption("%s", text.c_str());
            };

        }

        // FluxTTFont
        {
            auto type = lua.new_usertype<FluxTTFont>("FluxTTFont");
            type["new"] = sol::factories(
                [](const char* filename, sol::optional<uint32_t> size ) {
                    if (size) return new FluxTTFont(filename, size.value_or(32.f));
                    else return new FluxTTFont(filename);
                }
            );

        }
        // FluxLabel
        {
            auto type = lua.new_usertype<FluxLabel>("FluxLabel",
                                                          sol::base_classes, sol::bases<FluxRenderObject, FluxBaseObject>()
            );

            // Use sol::factories with stack_object for maximum debuggability
            type["new"] = sol::factories(
                [](sol::stack_object tex_obj ) -> FluxLabel* {
                    // Check if these are actually the types we expect
                    if (!tex_obj.is<FluxTTFont*>()) {
                        throw sol::error("Font Constructor Error: Argument 1 is not a FluxTexture pointer!");
                    }

                    return new FluxLabel( tex_obj.as<FluxTTFont*>() );
                }
            );

            type["print"] =  sol::overload(
                [](FluxLabel& self, const char* caption, Point2F pos) { self.Print(caption, pos);}
               ,[](FluxLabel& self, float x, float y, const char* caption) { self.Print(caption, Point2F(x,y));}
               ,&FluxLabel::Print
            );


            type["set"] = sol::overload(
                [](FluxLabel& self, const char* caption, float x, float y) {
                    self.set(caption, Point2F(x,y));
                },
                [](FluxLabel& self, const char* caption, float x, float y, sol::table colTable, sol::optional<float> scale) {
                    Color4F color;
                    // Access table by index (Lua indices start at 1)
                    color.r = colTable.get_or(1, 1.0f);
                    color.g = colTable.get_or(2, 1.0f);
                    color.b = colTable.get_or(3, 1.0f);
                    color.a = colTable.get_or(4, 1.0f);

                    self.set(caption, Point2F(x,y), color, scale.value_or(1.f));
                },
                &FluxLabel::set
            );
            type["setCaption"] = [](FluxLabel& self, std::string text) {
                self.setCaption("%s", text.c_str());
            };

            type["setColor"] = &FluxLabel::setColor;
            type["setScale"] = &FluxLabel::setScale;
        }
    }

    //==============================================================================
    void bindFluxAudioStream(sol::state& lua) {
        auto type = lua.new_usertype<FluxAudioStream>("FluxAudioStream",
                                                      sol::base_classes, sol::bases<FluxBaseObject>()
        );
        type["new"] = sol::factories(
            [](const char* filename) {
                return new FluxAudioStream(filename);
            }
        );
        type["play"] = &FluxAudioStream::play;
        type["stop"] = &FluxAudioStream::stop;
        type["resume"] = &FluxAudioStream::resume;
        type["isPlaying"] = &FluxAudioStream::isPlaying;

        type["setLooping"] = &FluxAudioStream::setLooping;
        type["setGain"] = &FluxAudioStream::setGain;
        type["getGain"] = &FluxAudioStream::getGain;
        type["getInitDone"] = &FluxAudioStream::getInitDone;

        type["setPosition"] = &FluxAudioStream::setPositon;

        type["update"] = &FluxAudioStream::Update;
    }
    //==============================================================================
    void bindSDLEvents(sol::state& lua) {
        lua.new_usertype<SDL_KeyboardEvent>("SDL_KeyboardEvent",
                                            "key", &SDL_KeyboardEvent::key,
                                            "scancode", &SDL_KeyboardEvent::scancode,
                                            "down", &SDL_KeyboardEvent::down
        );

        lua.new_usertype<SDL_Event>("SDL_Event",
                                    "type", &SDL_Event::type,
                                    "key", sol::property([](SDL_Event& ev) { return ev.key; }),
                                    // "key", [](SDL_Event& ev) { return ev.key; },
                                    "motion", [](SDL_Event& ev) { return ev.motion; }
        );

    }
    void bindConstants(sol::state& lua) {

        lua["SDL_EVENT_QUIT"] = SDL_EVENT_QUIT;
        lua["SDL_EVENT_KEY_DOWN"] = SDL_EVENT_KEY_DOWN;
        lua["SDL_EVENT_KEY_UP"] = SDL_EVENT_KEY_UP;
        lua["SDL_EVENT_MOUSE_MOTION"] = SDL_EVENT_MOUSE_MOTION;
        lua["SDL_EVENT_MOUSE_BUTTON_DOWN"] = SDL_EVENT_MOUSE_BUTTON_DOWN;

        lua["SDL_EVENT_WINDOW_RESIZED"]     = SDL_EVENT_WINDOW_RESIZED;
        lua["SDL_EVENT_WINDOW_MINIMIZED"]   =  SDL_EVENT_WINDOW_MINIMIZED;
        lua["SDL_EVENT_WINDOW_RESTORED"]    = SDL_EVENT_WINDOW_RESTORED;
        lua["SDL_EVENT_WINDOW_FOCUS_GAINED"] = SDL_EVENT_WINDOW_FOCUS_GAINED;
        lua["SDL_EVENT_WINDOW_FOCUS_LOST"] = SDL_EVENT_WINDOW_FOCUS_LOST;



        // Keycodes (SDLK_...)
        // Lua: sdl.K_SPACE
        sol::table k = lua.create_named_table("sdl");

        k["K_SPACE"] = SDLK_SPACE;
        k["K_RETURN"] = SDLK_RETURN;
        k["K_ESCAPE"] = SDLK_ESCAPE;
        k["K_BACKSPACE"] = SDLK_BACKSPACE;


        k["K_UP"] = SDLK_UP;
        k["K_DOWN"] = SDLK_DOWN;
        k["K_LEFT"] = SDLK_LEFT;
        k["K_RIGHT"] = SDLK_RIGHT;

        // A-Z
        for (char c = 'a'; c <= 'z'; ++c) {
            std::string name = "K_";
            name += (char)std::toupper(c);
            k[name] = (SDL_Keycode)c;
        }

        //  0-9
        for (char c = '0'; c <= '9'; ++c) {
            std::string name = "K_";
            name += c;
            k[name] = (SDL_Keycode)c;
        }


        sol::table c = lua.create_named_table("color");

        c["white"]       = cl_White;
        c["black"]       = cl_Black;
        c["red"]         = cl_Red;
        c["green"]       = cl_Green;
        c["blue"]        = cl_Blue;
        c["yellow"]      = cl_Yellow;
        c["cyan"]        = cl_Cyan;
        c["magenta"]     = cl_Magenta;
        c["gray"]        = cl_Gray;
        c["lightgray"]   = cl_LightGray;
        c["darkgray"]    = cl_DarkGray;
        c["orange"]      = cl_Orange;
        c["purple"]      = cl_Purple;
        c["brown"]       = cl_Brown;
        c["lime"]        = cl_Lime;
        c["pink"]        = cl_Pink;

        // Modern / UI Colors
        c["crimson"]     = cl_Crimson;
        c["emerald"]     = cl_Emerald;
        c["skyblue"]     = cl_SkyBlue;
        c["slate"]       = cl_Slate;
        c["gold"]        = cl_Gold;
        c["transparent"] = cl_Transparent;

        // Ocean Theme
        c["aquamarine"]  = cl_Aquamarine;
        c["coral"]       = cl_Coral;
        c["deepsea"]     = cl_DeepSea;
        c["seafoam"]     = cl_Seafoam;
        c["sand"]        = cl_Sand;
        c["kelp"]        = cl_Kelp;

        // FX
        c["neonpink"]     = cl_NeonPink;
        c["electricblue"] = cl_ElectricBlue;
        c["acidgreen"]    = cl_AcidGreen;
        c["glass"]        = cl_Glass;
        c["shadow"]       = cl_Shadow;
        c["ghost"]        = cl_Ghost;


    }
}; //namespace
