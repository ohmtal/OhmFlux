#include <stdio.h>
#include <fluxMain.h>
#include "luabind/luaFluxMain.h"
#include <sol/sol.hpp>

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

    virtual bool Initialize() override {
        if (!Parent::Initialize()) {
            return false;
        }
        // Look for "Initialize" in the Lua object
        sol::optional<sol::function> lua_init = lua_self["Initialize"];
        if (lua_init) {
            return (*lua_init)(lua_self); // Call Lua version
        }
        return true; //??
    }

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
};

void BindLuaTestGame(sol::state& lua)
{
    BindFluxMain(lua);
    // lua.new_usertype<LuaTestGame>("LuaTestGame",
    //                               sol::constructors<LuaTestGame()>(), // Allow "new" in Lua
    //                               "mSettings", &LuaTestGame::mSettings,
    //                               "Execute", &LuaTestGame::Execute
    // );

    lua.new_usertype<LuaTestGame>("LuaTestGame",
                                  // 1. Constructor (1 arg)
                                  sol::constructors<LuaTestGame()>(),

                                  // 2. Inheritance (2 args: tag + value)
                                  sol::base_classes, sol::bases<FluxMain>(),

                                  // 3. Property (2 args: name + value)
                                  "lua_self", sol::property(
                                      [](LuaTestGame& self) -> sol::table { return self.lua_self; },
                                                            [](LuaTestGame& self, sol::table t) { self.lua_self = t; }
                                  )
                                  // TOTAL: All arguments are now properly tagged and paired
    );


}


int main(int argc, char* argv[]) {
    sol::state lua;
    lua.open_libraries(sol::lib::base, sol::lib::package);

    BindLuaTestGame(lua);

    // 1. Create the game instance
    LuaTestGame game;

    // 2. Load the script (this defines the global 'onUpdate' function)
    auto result = lua.script_file("assets/loader.lua");
    if (!result.valid()) { return 1; }

    // 3. Connect C++ to Lua: Grab the global function by name
    // This is much safer than binding a member variable!
    // game.mLuaUpdate = lua["onUpdate"];

    // 4. Start the engine
    game.Execute();

    return 0;
}
