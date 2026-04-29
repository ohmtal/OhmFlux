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
local ballVel = Point2F.new(150, 150)
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

    -- Render Objects
    for key, wall in pairs(borders) do
        local tmpWall = FluxRenderObject.new(self.tex_white, wall)
        app:queueObject(tmpWall)
    end

    self.paddle = FluxRenderObject.new(self.tex_white, paddle)
    app:queueObject(self.paddle)

    self.ball = FluxRenderObject.new(self.tex_white, ball)
    app:queueObject(self.ball)

--     -- Font / labels
--     self.MonoFont = FluxTrueTypeFont.new("assets/fonts/JetBrainsMono/JetBrainsMono-Medium.ttf", 20);
--     if self.MonoFont then
--         self.MonoFont:set("Alder Babsack", Point2F.new( 0, screen:getHeight() -20 ), color.crimson, 2);
--     app:queueObject(self.MonoFont);
--     end



    return true
end

function Pong:Deinitialize()
    print("Pong:Deinitialize")
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


--  ball
    if  not gameOver then

        ball.x = ball.x + ballVel.x * ballSpeed * dt
        ball.y = ball.y + ballVel.y * ballSpeed * dt


        for i = 1, #borders do
            if checkCollision(ball, borders[i]) then
                local wall = borders[i]

                if wall == walls.left then
                    ballVel.x = math.abs(ballVel.x)
                    ball.x = wall.x + wall.w
                    doScore()


                elseif wall == walls.top or wall == walls.bottom then
                    ballVel.y = -ballVel.y
                    if wall == walls.top then ball.y = wall.y + wall.h else ball.y = wall.y - ball.h end
                end

                playBounce()
            end
        end
    end


    if ball.x > screenRect.x + screenRect.w then
        gameOver = true;
        ball.x, ball.y = screenRect.x + screenRect.w / 2.0, screenRect.y + screenRect.h / 2.0
    end

--  paddle
    paddle.y = paddle.y + paddleVel * dt

    if checkCollision(walls.top, paddle) then
        paddleVel = 0.0
        paddle.y =  walls.top.y + walls.top.h
    elseif checkCollision(walls.bottom, paddle) then
        paddleVel = 0.0
        paddle.y = walls.bottom.y - paddle.h
    end



    if ballVel.x > 0  and not gameOver then
        if checkCollision(ball, paddle) then
            ballVel.x = -ballVel.x

            local paddleCenter = paddle.y + (paddle.h / 2)
            local ballCenter = ball.y + (ball.h / 2)
            ballVel.y = (ballCenter - paddleCenter) * 5.0

            ball.x = paddle.x - ball.w
            playBounce()
        end
    end


    self.paddle:setRectF(paddle)
    self.ball:setRectF(ball)




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
            ballSpeed = 1.0
            self.gameOver = false;
--             FIXME playSound(startSound)

        end

    elseif (self.keys["Up"]) then
        paddleVel = paddleSpeed * - 1
    elseif(self.keys["Down"]) then
        paddleVel = paddleSpeed
    else
        paddleVel = 0
    end

end
-- ----------------------------------------------------------------------------

-- Assign this logic table to the C++ instance's lua_self
app.self = Pong
print("Pong Loaded")
