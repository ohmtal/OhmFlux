-- Create a logic table
local Pong = {}


--  Wall
local wallThickness = 10
local screenRect = RectF.new(20, 40, 600, 350)

local walls = {
    top    = RectF.new(screenRect.x, screenRect.y,  screenRect.w,   wallThickness),
    bottom = RectF.new(screenRect.x, screenRect.y + screenRect.h - wallThickness,  screenRect.w, wallThickness),
    left   = RectF.new(screenRect.x, screenRect.y,  wallThickness, screenRect.h),
    right  = RectF.new( screenRect.x + screenRect.w - wallThickness, screenRect.y , wallThickness, screenRect.h)
}

local borders = { walls.left, walls.top, walls.bottom }
local goals = { left = walls.left, right = walls.right }

-- Ball
local ballSpawnPoint = Point2F.new(screenRect.x + screenRect.w / 2.0, screenRect.y + screenRect.h / 2.0)
local ball = RectF.new(ballSpawnPoint.x, ballSpawnPoint.y, 10, 10)
local ballVel = { x = 150, y = 150 }
local ballSpeed = 1.0

-- paddle
local paddleSize = { w = 5.0 , h = 60.0 }
local paddleVel = 0.0;
local paddleSpeed = 250.0;
local paddle = RectF.new(screenRect.x + screenRect.w - 20.0, (screenRect.y + screenRect.h - paddleSize.w) / 2.0, 5.0, 60.0)



-- ----------------------------------------------------------------------------
-- Custom init
function Pong:Initialize()
    print("Pong:Initialize")
    -- Textures
    self.tex_white = app:loadTexture("assets/texture/white.png")
    self.tex_black = app:loadTexture("assets/texture/black.png")

    self.gameOver = true
    self.score = 0
    self.hiscore = 0

    for key, wall in pairs(borders) do
        local tmpWall = FluxRenderObject.new(self.tex_white, wall)
        app:queueObject(tmpWall)
    end

    self.paddle = FluxRenderObject.new(self.tex_white, paddle)
    self.paddle:setSpeed( 150)
    app:queueObject(self.paddle)

    self.ball = FluxRenderObject.new(self.tex_white, ball)
    self.ball:setSpeed( 0 )
    self.ball:setDirX(150)
    self.ball:setDirY(150)
    app:queueObject(self.ball)


--     print("Hello ?!")

    return true
end

function Pong:Deinitialize()
    print("Pong:Deinitialize")
end

-- ----------------------------------------------------------------------------
function Pong:onDraw(dt)
-- print("DRAW!")

end

-- --------- Collision check -----------
local function checkCollision(a, b)
return a.x < b.x + b.w and
a.x + a.w > b.x and
a.y < b.y + b.h and
a.y + a.h > b.y
end

-- ----------------------------------------------------------------------------
local function playBounce()
-- FIXME
end
-- ----------------------------------------------------------------------------
function Pong:onUpdate(dt)

    if self.gameOver then return end

    for i = 1, #borders do
        if checkCollision(self.ball:getRectF(), borders[i]) then


            local wall = borders[i]

            if wall == walls.left then
                self.ball:setDirX( math.abs(self.ball:getDirX()) )
--FIXME                    ball.x = wall.x + wall.w
                doScore()

            elseif wall == walls.top or wall == walls.bottom then
                self.ball:setDirY(-self.ball:getDirY())
--FIXME                      if wall == walls.top then ball.y = wall.y + wall.h else ball.y = wall.y - ball.h end
            end

            playBounce()
        end
    end


    if self.ball:getX() > walls.right.x then
        print("GAME OVER")
        self.gameOver = true
        self.ball:setSpeed(0)
        self.ball:setPosition(ballSpawnPoint)
    end


    --  paddle border collide
    if checkCollision(walls.top, self.paddle:getRectF()) then
        self.paddle:setDirY( 0 )
        self.paddle:setY(walls.top.y + walls.top.h + self.paddle:getHeight() / 2)
    elseif checkCollision(walls.bottom, self.paddle:getRectF()) then
        self.paddle:setDirY( 0 )
        self.paddle:setY(walls.bottom.y - self.paddle:getHeight() / 2)
    end

    -- paddle ball collide
    if self.ball:getDirX() > 0  and not self.gameOver then
        if checkCollision(self.ball:getRectF(), self.paddle:getRectF()) then
            self.ball:setDirX(-self.ball:getDirX())

            self.ball:setDirY ( (self.ball:getPosition() - self.paddle:getPosition()) * 5.0 )

            local x = self.paddle:getX() - self.paddle:getWidth() - self.ball.getWidth()
            self.ball.setX(x)

            playBounce()
        end
    end



end
-- ----------------------------------------------------------------------------
function Pong:onKeyEvent(key, isDown, alt, ctrl, shift)
if (isDown) then print(key, "pressed") end
    self.keys = self.keys or {}
    self.keys[key] = isDown

    if (self.keys["Space"]) then
        if self.gameOver then
            print("new round ...")
            self.score = 0
            self.ball:setSpeed(1.0)
            self.gameOver = false;
--             FIXME playSound(startSound)

        end

    elseif (self.keys["Up"]) then
        self.paddle:setDirY( -1 )
    elseif(self.keys["Down"]) then
        self.paddle:setDirY( 1 )
    else
        self.paddle:setDirY( 0 )
    end

end
-- ----------------------------------------------------------------------------

-- Assign this logic table to the C++ instance's lua_self
app.self = Pong
print("Pong Loaded")
