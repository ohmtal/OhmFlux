//------------------------------------------------------------------------------
function loadSound(%name,%filename,%looping) {

    // if (%looping $= "") %looping = false;
    %profile = new AudioProfile(%name)
    {
        filename = "assets/sound/" @ %filename;
        looping = %looping;
    };

}

//------------------------------------------------------------------------------
// function onEnterScript(%scriptFile) {
//     dWarn("onEnterScript" SPC %scriptFile);
//     if (%scriptFile $= "assets/pong.cs") $Main.setScreenSize("740 500");
// }

function onLeaveScript(%scriptFile) {
    // reset to default
    dError("CALLING ON LEAVE SCRIPT ....." SPC %scriptFile);
    // if (%scriptFile $= "assets/invaderGame.cs")
    $Main.setScreenSize("0 0");
}

//------------------------------------------------------------------------------
function Pong::init(%this) {
    $Main.setScreenSize("740 500");
    // --- config ----
    %this.rect = new RectObject() {
        x = 50;
        y = 50;
        w = 600;
        h = 400;
    };

    %this.thick = 10;

    %this.StartSpeed = 150;
    %this.IncSpeed = 3;
    %this.paddleHeight = 80;

    %this.score = 0;

    //--- borders ---
    %this.borderTop = new Sprite();
    %this.borderTop.setRectF( %this.rect.withHeight(%this.thick));

    %this.borderLeft = new Sprite();
    %this.borderLeft.setRectF( %this.rect.withWidth(%this.thick));

    %this.borderBottom = new Sprite();
    %tmpRect = new RectObject() { data = %this.rect.data; };
    %tmpRect.y = %this.rect.y + %this.rect.h - %this.thick / 2;
    %tmpRect.h = %this.thick;
    %rect = %this.x SPC %y SPC %this.w SPC %this.thick;
    %this.borderBottom.setRectF(%tmpRect.data);

    %this.borders = %this.borderTop SPC %this.borderLeft SPC %this.borderBottom;

    for (%i = 0; %i < getWordCount(%this.borders); %i++) {
        %b = getWord(%this.borders, %i);
        echo(%b);
        %b.Color = "0.5 0.7 0.5";
    }

    //--- paddle ---
    %this.paddle = new Sprite() {
        class = "Paddle";
        parent = %this;
        color  = "0 0 1";
        Speed  = 200;
    };

    %this.paddle.reset();

    // --- ball ----
    %this.ball = new Sprite() {
        class = "Ball";
        parent = %this;
        color = "1 0 0";
        // Speed = 100;
    };
    %this.ball.reset();


    // --- sounds -----
    loadSound(AlienMoveSound1,"alien_move_1.wav");
    loadSound(AlienMoveSound2,"alien_move_2.wav");
    loadSound(AlienMoveSound3,"alien_move_3.wav");
    loadSound(AlienMoveSound4,"alien_move_4.wav");

    loadSound(AlienExplosionSound,"alien_explosion.wav");

    loadSound(AlienShootSound,"alien_shoot.wav");
    loadSound(BonusAlienExplosionSound,"bonusalien_explosion.wav");
    loadSound(BonusAlienLoopSound,"bonusalien_loop.wav",true);

    loadSound(PlayerExplosionSound,"player_explosion.wav");
    loadSound(PlayerShootSound,"player_shoot.wav");


    %this.texBack =  new Texture() {
        fileName = "assets/texture/nebulapurple_sky_back.png";
    };
    %this.background = new Sprite() {
        Texture = %this.texBack;
        z = 1.0; //layer
    };
    %this.background.setRectF(getScreenRect());

    %this.fontJet = new Font() {
        fileName = "assets/font/JetBrainsMono-Regular.ttf";
    };

    %this.Label1 = new Label()  {
        Font = %this.fontJet;
        x = getScreenCenterX();
        y = 40;
        shadow=1;
        scale=1.2;
        align = 1;
        // color = "0.2 0.2 1";
        Caption = "Score:" SPC %this.score;
    };

    %tmpRect.delete();

}

function Pong::onInputEvent( %this, %deviceString, %actionString, %mouseX, %mouseY, %keyValue ) {
    if ( %deviceString $= "keyboard") {
        %this.paddle.dir = 0;
        if (%keyValue == true) {
            switch$ (%actionString) {
                case "Up":  %this.paddle.dir = -1;
                case "Down": %this.paddle.dir = 1;
                case "Space":
                    if (%this.ball.Speed == 0) {
                        %this.ball.Speed = %this.StartSpeed;
                        %this.score = 0;
                        %this.label1.caption = "Score" SPC %this.score;
                        alxPlay(BonusAlienExplosionSound);
                    }
            }
        } else {
           //Key up ...

        }
    }
    // echo("velo" SPC %this.paddle.Velocity SPC "speed:" SPC %this.paddle.Speed);
}

// function Pong::onRender(%this,%dt) {
//     //DEBUG
//     %this.writeText(20,40, %this.ball.Velocity SPC %this.ball.speed SPC %this.score);
// }

// wallID 0 == paddle ;)
function Pong::hit(%this, %wallId) {
    if (%wallId == 0) {
        %this.score ++;
        %this.label1.caption = "Score" SPC %this.score;
    }
    //FIXME alxPlay()
    %snd = PlayerShootSound;
    if (%wallId > 0) %snd = ("AlienMoveSound" @ %wallId); //.getid();
    alxPlay(%snd);

    %this.ball.speed += %this.IncSpeed;
}

function Pong::onUpdate(%this,%dt) {
    // paddle
    %pad = %this.paddle;
    if (%pad.dir == -1 &&  %pad.y > %pad.top ) %pad.y -= %pad.speed * %dt;
    else if (%pad.dir == 1 &&  %pad.y < %pad.bottom ) %pad.y += %pad.speed * %dt;
    else %pad.dir = 0;


    // Ball NOTE: nicht besonders elegant ;)
    %b = %this.ball;
    if (%b.speed == 0) return;  //WARNING

    if (!isObject(%b.V)) %b.V = new Vector2Object();
    %b.V.data = %b.Velocity;

    // left border check
    if (%b.x <= %b.left) {
        if (%b.V.x <= 0) {
            %this.hit(1);
            %b.V.x *= -1;
             %b.Velocity = %b.V.data;
        }
    }
    if (%b.y <= %b.top) {
        if (%b.V.y  <= 0) {
            %this.hit(2);
            %b.V.y *= -1;
             %b.Velocity = %b.V.data;
        }
    }
    if (%b.y >= %b.bottom) {
        if (%b.V.y  >= 0) {
            %this.hit(3);
            %b.V.y *= -1;
             %b.Velocity = %b.V.data;
        }
    }

    if (%b.x >= %b.paddleRight ) {
        if (%b.V.x >= 0) {
            if (RectIntersects(%this.paddle.getRectF(), %b.getRectF())) {
                %b.V.x *= -1;
                if (%this.paddle.dir != 0)  %b.V.y  = %b.V.y + %this.paddle.dir *  0.2;
                else %b.V.y =%b.V.y * 0.5 + getRandomF(-0.125,0.125);
                if (%b.V.y > 1) %b.V.y = 1;
                else if (%b.V.y < -1) %b.V.y = -1;

                %b.Velocity = %b.V.data;
                %this.hit(0);
            } else {
                // %ball.reset(); //missed
                echo("MISSED ?!");
            }
        }
    }

    if (%b.x > %b.right) {
            %b.reset();
            echo("Game Over");
            alxPlay(PlayerExplosionSound);
    }


}

// ------------------- PADDLE -----------------------------
function Paddle::reset(%this) {
    %p = %this.parent;
    %this.x = %p.rect.x + %p.rect.w - %p.thick;
    %this.y = %p.rect.y + %p.rect.h / 2;
    %this.w = %p.thick;
    %this.h = %p.paddleHeight;

    %this.top = %p.rect.y + %p.thick + %this.h / 2;
    %this.bottom = %p.rect.y + %p.rect.h - %this.h / 2 - %p.thick / 2;

    echo(%this.getRectF());
}

// ------------------- Ball -----------------------------
function Ball::reset(%this) {
    %p = %this.parent;

    %this.x = %p.rect.x + %p.rect.w / 2;
    %this.y = %p.rect.y + %p.rect.h / 2;
    %this.w = %p.thick;
    %this.h = %p.thick;

    %this.top    = %p.rect.y + %p.thick + %this.h / 2;
    %this.bottom = %p.rect.y + %p.rect.h - %this.h;
    %this.left   = %p.rect.x + %p.thick + %p.thick / 2;
    //let it fly a bit longer (thick * 3)
    %this.right  = %p.rect.x + %p.rect.w + %p.thick * 3;

    %this.paddleRight = %p.paddle.x - %p.thick ;

    %this.speed = 0;
    %this.velocity = 1 SPC getRandomF(-0.5,0.5);


    echo(%this.getRectF());
}
// ------------------- MAIN -----------------------------
//------------------------------
GarbageCollectionSet.deleteObjects();
//------------------------------
$Game = new GameCtrl() {
    class = "Pong";
};

$Game.init();
