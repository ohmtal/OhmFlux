#include <stdio.h>
#include <fluxMain.h>
#include "luabind/luaFluxMain.h"
#include <sol/sol.hpp>
#include "SDL3/SDL.h"
/*
 Binding Lua is much more headache than i thought when i read:
    It is the industry standard for "I just want to add scripts to my C++ project without a headache".

 I guess i will stay with c++ for the moment :P
*/


class LuaTestGame : public FluxMain
{
    typedef FluxMain Parent;
private:

public:
    // This stores the Lua table/object
    sol::table lua_self;

    std::function<bool()> onInitialize;

    // virtual bool Initialize() override {
    //     if (!Parent::Initialize()) {
    //         return false;
    //     }
    //     // Look for "Initialize" in the Lua object
    //     sol::optional<sol::function> lua_init = lua_self["Initialize"];
    //     if (lua_init) {
    //         return (*lua_init)(lua_self); // Call Lua version
    //     }
    //     return true; //??
    // }

    virtual bool Initialize() override {
        if (!Parent::Initialize()) return false;

        if (lua_self.valid() && lua_self != sol::lua_nil) {
            // 1. Get the function
            sol::protected_function lua_init = lua_self["Initialize"];

            if (!lua_init.valid()) {
                printf("CRITICAL: Lua function 'Initialize' not found in myLogic table!\n");
                return false;
            }

            // 2. Call it WITHOUT a handler first to see if it even runs
            auto result = lua_init(lua_self);

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
        if (lua_self != sol::lua_nil) {
            sol::optional<sol::function> fx = lua_self["onUpdate"];
            if (fx) {
                fx.value()(lua_self, dt); // Call as myLogic:onUpdate(dt)
            }
        }
    }
    //--------------------------------------------------------------------------
    virtual void onKeyEvent(SDL_KeyboardEvent event) override {
        if (lua_self.valid()) {
            sol::protected_function fx = lua_self["onKeyEvent"];
            if (fx.valid()) {
                const char* keyName = SDL_GetKeyName(event.key);
                bool isDown = event.down;

                // SDL3: Check modifiers using the 'mod' field and bitwise AND
                bool isAlt   = (event.mod & SDL_KMOD_ALT) != 0;
                bool isCtrl  = (event.mod & SDL_KMOD_CTRL) != 0;
                bool isShift = (event.mod & SDL_KMOD_SHIFT) != 0;

                // Call Lua with modifiers: myLogic:onKeyEvent(key, isDown, alt, ctrl, shift)
                sol::state_view lua(lua_self.lua_state());
                fx.set_error_handler(lua["debug"]["traceback"]);

                fx(lua_self, keyName, isDown, isAlt, isCtrl, isShift);
            }
        }
    }
    //--------------------------------------------------------------------------
    virtual void onMouseButtonEvent(SDL_MouseButtonEvent event) override {
        if (lua_self.valid()) {
            sol::protected_function fx = lua_self["onMouseButton"];
            if (fx.valid()) {
                fx(lua_self, (int)event.button, event.x, event.y, (bool)event.down);
            }
        }
    }
    //--------------------------------------------------------------------------
    void reloadScript() {
        sol::state_view lua(lua_self.lua_state());
        auto result = lua.script_file("assets/loader.lua");

        if (result.valid()) {
            printf("Lua Script Reloaded Successfully!\n");
            // Re-assign the new logic table to ensure C++ has the latest references
            this->lua_self = lua["Game"];
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
    bindFluxBaseObject(lua);   // Root
    bindFluxScreen(lua);       // Utility
    bindFluxTexture(lua);      // Resource
    bindFluxAudioStream(lua);
    bindDrawParams2D(lua);
    bindFluxRenderObject(lua); // Parent of Font
    bindFluxBitmapFont(lua);   // Child of RenderObject
    bindFluxMain(lua);         // Engine


    auto type = lua.new_usertype<LuaTestGame>("LuaTestGame",
        sol::base_classes, sol::bases<FluxMain, FluxBaseObject>()
    );
    type["lua_self"] = &LuaTestGame::lua_self;
    type["reloadScript"] = &LuaTestGame::reloadScript;

}
//------------------------------------------------------------------------------
int main(int argc, char* argv[]) {
    sol::state lua;
    lua.open_libraries(sol::lib::base, sol::lib::package, sol::lib::debug);

    BindLuaTestGame(lua);

    // 2. Create the C++ game instance
    LuaTestGame game;

    // 3. CRITICAL: Inject the 'game' pointer into Lua's global scope FIRST
    lua["app"] = &game;

    // 4. Load the script NOW. The script can now safely access the global 'app'.
    auto result = lua.script_file("assets/loader.lua");
    if (!result.valid()) {
        sol::error err = result;
        printf("SCRIPT LOAD ERROR: %s\n", err.what());
        return 1;
    }

    // 5. Run the engine (which calls your Lua Initialize function)
    game.Execute();

    return 0;
}
