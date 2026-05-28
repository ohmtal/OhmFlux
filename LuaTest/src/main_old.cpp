#include <stdio.h>
#include <fluxMain.h>
#include "luabind/luaBindings.h"
#include <sol/sol.hpp>
#include "SDL3/SDL.h"


class LuaTestGame : public FluxMain
{
    typedef FluxMain Parent;
private:

public:
    // This stores the Lua table/object
    sol::table mLuaSelf;

    std::function<bool()> onInitialize;


    virtual bool Initialize() override {
        if (!Parent::Initialize()) return false;

        if (mLuaSelf.valid() && mLuaSelf != sol::lua_nil) {
            // 1. Get the function
            sol::protected_function lua_init = mLuaSelf["Initialize"];

            if (!lua_init.valid()) {
                printf("CRITICAL: Lua function 'Initialize' not found in myLogic table!\n");
                return false;
            }

            // 2. Call it WITHOUT a handler first to see if it even runs
            auto result = lua_init(mLuaSelf);

            if (!result.valid()) {
                sol::error err = result;
                // THIS WILL PRINT THE REAL ERROR
                printf("LUA ERROR: %s\n", err.what());
                return false;
            }
            return true;
        }
        return true;
    }
    //--------------------------------------------------------------------------
    virtual void Update(const double& dt) override {
        FluxMain::Update(dt);

        // Look for onUpdate inside the table we linked
        if (mLuaSelf != sol::lua_nil) {
            sol::optional<sol::function> fx = mLuaSelf["onUpdate"];
            if (fx) {
                fx.value()(mLuaSelf, dt); // Call as myLogic:onUpdate(dt)
            }
        }
    }
    //--------------------------------------------------------------------------
    virtual void onKeyEvent(SDL_KeyboardEvent event) override {
        if (mLuaSelf.valid()) {
            sol::protected_function fx = mLuaSelf["onKeyEvent"];
            if (fx.valid()) {
                const char* keyName = SDL_GetKeyName(event.key);
                bool isDown = event.down;

                bool isAlt   = (event.mod & SDL_KMOD_ALT) != 0;
                bool isCtrl  = (event.mod & SDL_KMOD_CTRL) != 0;
                bool isShift = (event.mod & SDL_KMOD_SHIFT) != 0;

                sol::state_view lua(mLuaSelf.lua_state());
                fx.set_error_handler(lua["debug"]["traceback"]);

                fx(mLuaSelf, keyName, isDown, isAlt, isCtrl, isShift);
            }
        }
    }
    //--------------------------------------------------------------------------
    virtual void onMouseButtonEvent(SDL_MouseButtonEvent event) override {
        if (mLuaSelf.valid()) {
            sol::protected_function fx = mLuaSelf["onMouseButton"];
            if (fx.valid()) {
                fx(mLuaSelf, (int)event.button, event.x, event.y, (bool)event.down);
            }
        }
    }
    //--------------------------------------------------------------------------
    void reloadScript() {
        sol::state_view lua(mLuaSelf.lua_state());
        auto result = lua.script_file("assets/main.lua");

        if (result.valid()) {
            printf("Lua Script Reloaded Successfully!\n");
            this->mLuaSelf = lua["Game"];
        } else {
            sol::error err = result;
            printf("RELOAD ERROR: %s\n", err.what());
        }
    }
    //--------------------------------------------------------------------------

};


//------------------------------------------------------------------------------
void BindLuaTestGame(sol::state& lua) {
    // Bind base classes first
    using namespace OhmFlux::Lua;
    bindFluxBaseObject(lua);   // Root
    bindFluxScreen(lua);       // Utility
    bindFluxTexture(lua);      // Resource
    bindFluxAudioStream(lua);
    bindDrawParams2D(lua);
    bindFluxRenderObject(lua); // Parent of Font
    bindFluxFonts(lua);   // Child of RenderObject
    bindFluxTrueTypeFont(lua);
    bindFluxMain(lua);         // Engine


    auto type = lua.new_usertype<LuaTestGame>("LuaTestGame",
        sol::base_classes, sol::bases<FluxMain, FluxBaseObject>()
    );
    type["lua_self"] = &LuaTestGame::mLuaSelf;
    type["reloadScript"] = &LuaTestGame::reloadScript;

}
//------------------------------------------------------------------------------
int main(int argc, char* argv[]) {
    sol::state lua;
    lua.open_libraries(sol::lib::base, sol::lib::package, sol::lib::debug);

    BindLuaTestGame(lua);

    LuaTestGame game;
    lua["app"] = &game;
    auto result = lua.script_file("assets/main.lua");
    if (!result.valid()) {
        sol::error err = result;
        printf("SCRIPT LOAD ERROR: %s\n", err.what());
        return 1;
    }
    game.Execute();

    return 0;
}
