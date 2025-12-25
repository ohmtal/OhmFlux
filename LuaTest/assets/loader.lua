-- ### Testing Lua #####

local game = LuaTestGame.new()
local myLogic = {}

function myLogic:Initialize()
    print("Initializing from Lua!")
return true
end

function myLogic:onUpdate(dt)
    print("spam....")
end

game.lua_self = myLogic

game.mSettings.Caption = "Lua Test Game"
game:Execute()

