-- Create a logic table
local Game = {}

-- ----------------------------------------------------------------------------
function Game:Initialize()
    print("Lua: Initializing assets...")

    return true
end
-- ----------------------------------------------------------------------------
function Game:onUpdate(dt)

end
-- ----------------------------------------------------------------------------
function Game:onKeyEvent(key, isDown, alt, ctrl, shift)
    self.keys = self.keys or {}
    self.keys[key] = isDown
end
-- ----------------------------------------------------------------------------
function Game:onUpdate(dt)
end
-- ----------------------------------------------------------------------------

-- Assign this logic table to the C++ instance's lua_self
app.self = Game
print("Lua Script Loaded")
