#pragma once

#include <sol/sol.hpp>


#include <fluxMain.h>
#include "utils/errorlog.h"
#include "luaBindings.h"


namespace OhmFlux::Lua {

    class LuaState : public FluxMain {
        typedef FluxMain Parent;

        bool mInitialied = false;

        sol::state mLua;
        sol::table mLuaSelf;

        std::string mCurrentScript = "main.lua";

        bool initLua();

    public:
        sol::state* getLua();

        const std::string getScript();
        void setScript(std::string script);
        bool LoadScript();

        virtual bool Initialize() override;
        bool LuaInitialize();
        bool LuaDeInitialize();
        virtual void Update(const double& dt) override;
        virtual void onDraw() override;
        virtual void onEvent(SDL_Event event) override;
        virtual void onKeyEvent(SDL_KeyboardEvent event) override;
        virtual void onMouseButtonEvent(SDL_MouseButtonEvent event) override;


    };


}
