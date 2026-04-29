#include "luaState.h"
#include <fluxMain.h>
#include <sol/sol.hpp>
#include <SDL3/SDL.h>

namespace OhmFlux::Lua {

    //--------------------------------------------------------------------------
    sol::state* LuaState::getLua(){
        return &mLua;
    }
    //--------------------------------------------------------------------------
    void LuaState::setScript(std::string script){
        mCurrentScript = script;
        LoadScript();
    }

    const std::string LuaState::getScript(){
        return mCurrentScript;
    }

    bool LuaState::LoadScript(){
        if (!mInitialied) return false;

        LuaDeInitialize();

        auto result = mLua.safe_script_file( getGamePath() + "assets/" + mCurrentScript, sol::script_pass_on_error);
        if (!result.valid()) {
            sol::error err = result;
            Log("[error] SCRIPT %s LOAD ERROR: %s\n", mCurrentScript.c_str(),  err.what());
            return false;
        } else {
            Log("[info] script %s loaded", mCurrentScript.c_str());
            mLua["app"] = this;
            LuaInitialize();
            return true;
        }
    }
    //--------------------------------------------------------------------------
    bool LuaState::initLua() {
        mLua.open_libraries(
            sol::lib::base
            ,sol::lib::package
            ,sol::lib::coroutine
            ,sol::lib::os
            ,sol::lib::math
            ,sol::lib::table
            ,sol::lib::string
            ,sol::lib::debug
            ,sol::lib::io

        );

        bindFluxBaseObject(mLua);   // Root
        bindFluxScreen(mLua);       // Utility
        bindFluxTexture(mLua);      // Resource
        bindFluxAudioStream(mLua);
        bindDrawParams2D(mLua);
        bindFluxRenderObject(mLua); // Parent of Font
        bindFluxFonts(mLua);   // Child of RenderObject
        bindFluxMain(mLua);         // Engine
        bindSDLEvents(mLua);
        bindConstants(mLua);


        mLua.set_function("quit", [&]() { TerminateApplication(); });
        // --------- CONSOLE REDIRECT -----------
        // took me ages to find this !
        mLua.set_function("print", [](sol::variadic_args va, sol::this_state s) {
            std::string out;
            for (sol::object obj : va) {
                obj.push();
                size_t len;
                const char* str = luaL_tolstring(s, -1, &len);
                if (str) {
                    out += std::string(str, len) + "    ";
                }
                lua_pop(s, 2);
            }
            SDL_Log("[Lua] %s", out.c_str());
        });


        // SELF
        auto type = mLua.new_usertype<LuaState>("LuaState",
                                                sol::base_classes, sol::bases<FluxMain, FluxBaseObject>()
        );
        type["self"] = &LuaState::mLuaSelf;

        mLua["app"] = this;

        mInitialied = true;
        return true;
    }
    //--------------------------------------------------------------------------
    bool LuaState::LuaDeInitialize() {
        CleanQueue();
        if (mLuaSelf.valid() && mLuaSelf != sol::lua_nil) {
            sol::protected_function lua_init = mLuaSelf["Deinitialize"];

            if (!lua_init.valid()) {
                // Log("[error] CRITICAL: Lua function 'Deinitialize' not found in myLogic table!\n");
                return false;
            }

            auto result = lua_init(mLuaSelf);

            if (!result.valid()) {
                sol::error err = result;
                Log("[error]  %s\n", err.what());
                return false;
            }

            return true;
        }
        return true;
    }
    //--------------------------------------------------------------------------
    bool LuaState::LuaInitialize() {
        if (mLuaSelf.valid() && mLuaSelf != sol::lua_nil) {
            sol::protected_function lua_init = mLuaSelf["Initialize"];

            if (!lua_init.valid()) {
                // Log("[error] CRITICAL: Lua function 'Initialize' not found in myLogic table!\n");
                return false;
            }

            auto result = lua_init(mLuaSelf);

            if (!result.valid()) {
                sol::error err = result;
                Log("[error]  %s\n", err.what());
                return false;
            }
            return true;
        }
        return false;
    }

    bool LuaState::Initialize() {
        if (!Parent::Initialize()) return false;

        if (!initLua()) return false;
        LoadScript();


        return true;
    }
    //--------------------------------------------------------------------------
    void LuaState::onMouseButtonEvent(SDL_MouseButtonEvent event){
        if (mLuaSelf.valid()) {
            sol::protected_function fx = mLuaSelf["onMouseButton"];
            if (fx.valid()) {
                fx(mLuaSelf, (int)event.button, event.x, event.y, (bool)event.down);
            }
        }
    }
    //--------------------------------------------------------------------------
    void LuaState::onKeyEvent(SDL_KeyboardEvent event) {
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
    void LuaState::onEvent(SDL_Event event) {
        if (!mInitialied) return;

        if (mLuaSelf != sol::lua_nil) {
            sol::optional<sol::function> fx = mLuaSelf["onEvent"];
            if (fx) {
                fx.value()(mLuaSelf, event);
            }
        }
    }

    void LuaState::onDraw() {
        if (mLuaSelf != sol::lua_nil) {
            sol::optional<sol::function> fx = mLuaSelf["onDraw"];
            if (fx) {
                fx.value()(mLuaSelf);
            }
        }

    }
    //--------------------------------------------------------------------------
    void LuaState::Update(const double& dt) {
        FluxMain::Update(dt);

        // Look for onUpdate inside the table we linked
        if (mLuaSelf != sol::lua_nil) {
            sol::optional<sol::function> fx = mLuaSelf["onUpdate"];
            if (fx) {
                fx.value()(mLuaSelf, dt);
            }
        }
    }

}
