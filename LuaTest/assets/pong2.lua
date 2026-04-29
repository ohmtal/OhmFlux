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


    print("Pong:Initialize")
    -- Textures
    local tex_white = app:loadTexture("assets/texture/white.png")
    local tex_black = app:loadTexture("assets/texture/black.png")

    local gameOver = true
    local score = 0
    local hiscore = 0

    -- Render Objects
    for key, wall in pairs(borders) do
        local tmpWall = FluxRenderObject.new(tex_white, wall)
        app:queueObject(tmpWall)
    end

    local paddleObj = FluxRenderObject.new(tex_white, paddle)
    app:queueObject(paddleObj)

    local ballObj = FluxRenderObject.new(tex_white, ball)
    app:queueObject(ballObj)

    local monoFont = FluxTTFont.new("assets/fonts/JetBrainsMono/JetBrainsMono-Medium.ttf", 32);
    local label = nil

    if monoFont then
        label = FluxLabel.new(monoFont)
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
function Pong:onDraw()




     local x = screenRect.x +  screenRect.w + 10
     local y = screenRect.y + wallThickness
     label:setScale(0.5)
     label:setColor(color.gray)
     label:print(x, y, "OHMFLUX PONG SQUASH LUA EDITION ;)")
     y = y + 20
     label:print(x, y, string.format("Ball POS: %.2f,%.2f VEL: %.2f, %.2f", ball.x, ball.y, ballVel.x, ballVel.y))

     label:setScale(1.0)
     label:setColor(color.white)
     y = y + 40
     label:print(x, y, string.format("SCORE: %2d    HI: %2d", score, hiscore))

     if (gameOver) then
         label:setScale(1.2)
         y = y + 30
         label:setColor(color.red)
         label:print(x, y, "G A M E   O V E R")

         label:setScale(0.5)
         y = y + 20
         label:setColor(color.skyblue)
         label:print(x, y, "SPACE = Start, Cursor: Up / Down ")

     end


end

-- --------- score up  -----------
function Pong:doScore(  )
    score = score + 1;
    if score > hiscore then hiscore = score end
    ballSpeed = ballSpeed + 0.15;
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
                    self:doScore()


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

    -- update render objects
    paddleObj:setRectF(paddle)
    ballObj:setRectF(ball)




end
-- ----------------------------------------------------------------------------
function Pong:onKeyEvent(key, isDown, alt, ctrl, shift)
if (isDown) then print(key, "pressed") end
    self.keys = self.keys or {}
    self.keys[key] = isDown

    if (self.keys["Space"]) then
        print (" HAAAAAAAALOOOOOOOO?!", gameOver)
        if gameOver then
            print("new round ...")
            score = 0
            ballSpeed = 1.0
            gameOver = false;
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
