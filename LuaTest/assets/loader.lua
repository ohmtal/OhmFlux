-- ### Testing Lua #####

-- Create a logic table
local Game = {}


-- @tom property are: foo.bar = , methods are: foo:peep(1)

function Game:Initialize()
    print("Lua: Initializing assets...")
    -- 'app' is the global variable we set in C++ (lua["app"])
    self.FontMonoTex = app:loadTexture("assets/fonts/monoSpace_13x28.bmp", 10, 10)
    self.bgTex = app:loadTexture("assets/background.bmp")

    local screen = app:getScreen()

    -- DEBUG CHECK
    if not self.FontMonoTex then print("ERROR: FontMonoTex is nil!") end
    if not screen then print("ERROR: Screen is nil!") end
    if not self.bgTex  then print("ERROR: Background Texture is nil!") end


    print("Lua: Setup font....")
    self.font = FluxBitmapFont.new(self.FontMonoTex, screen)
    print("Lua: Setup font....2")
    self.font:set("Hello World", 20 , 20 , 26, 32, { 0.9, 0.9, 1., 1.} );
    print("Lua: Setup font....3")
    app:queueObject(self.font)

    print("Lua: Setup background:")
    self.background = FluxRenderObject.new(self.bgTex, screen)

    local params = self.background:getDrawParams()

    -- Member functions MUST use :
    params.x = screen:getCenterX()
    params.y = screen:getCenterY()
    params.z = 1.0
    params.w = screen:getWidth()
    params.h = screen:getHeight()

    -- Member variables of a struct use .
    params.alpha = 0.5
    params.isGuiElement = false

    app:queueObject(self.background)


    -- Music
    self.bgMusic = FluxAudioStream.new("assets/music/sample1_loop.ogg")

    if self.bgMusic:getInitDone() then
        self.bgMusic:setLooping(true)
        self.bgMusic:setGain(0.5) -- 50% volume
        self.bgMusic:play()
    end

    -- 2. Queue for update
    -- Since it inherits from BaseObject, you can queue it for the engine
    app:queueObject(self.bgMusic)


    return true
end

-- Define the update function
function Game:onUpdate(dt)
--     self.font:setCaption(string.format("Score: %d", score))

end

    function Game:onKeyEvent(key, isDown, alt, ctrl, shift)
        if isDown then
            if key == "Escape" then
                app:terminateApplication()
            elseif key == "P" then
                app:togglePause()
            elseif key == "Return" and ctrl then
                app:toggleFullScreen()
            elseif key == "F5" then
                print("Reloading...")
                app:reloadScript()
            end
            print("Lua: key down:" .. key)
        end
        self.keys = self.keys or {}
        self.keys[key] = isDown

    end

    function Game:onUpdate(dt)
        if self.keys and self.keys["W"] then
            -- Handle continuous movement
        end
    end


-- Assign this logic table to the C++ instance's lua_self
app.lua_self = Game
