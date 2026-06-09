//------------------------------------------------------------------------------
// InvaderGame ported from TGE to OGE To KorkFlux
//------------------------------------------------------------------------------
// NOTE: This is not a good example to demonstrate How to use ElfScript
//       using Sprite Objects is much better than GameCtrl with onRender callback
//       I added this script to test ElfScript itself.
//------------------------------------------------------------------------------
// FIXME  exec("common/highscore.cs");
$align::left = 0;
$align::center = 1;
$align::right = 2;


if (!isObject(CleanupSet)) new SimSet(CleanupSet);
else CleanupSet.deleteObjects();




function initInvader(%force) {

  $Game = new GameCtrl(invaderGame) {};
  CleanupSet.add($Game);

// if (%force)
//       playGui.delete();
 
//   if (isObject(invaderGame))
//         return;
// new GuiControl(PlayGui) {
//    Profile = "GuiContentProfile";
//    HorizSizing = "right";
//    VertSizing = "bottom";
//    position = "0 0";
//    Extent = "800 600";
//    MinExtent = "8 8";
//    Visible = "1";
//       applyFilterToChildren = "1";
//       noCursor = "1"; //disable mouse :P
//       cameraZRot = "0";
//       helpTag = "0";
//       forceFOV = "0";
//
//    new tom2DCtrl(invaderGame) {
//       canSaveDynamicFields = "0";
//       Profile = "PlayCanvasProfile";
//       HorizSizing = "width";
//       VertSizing = "height";
//       position = "0 0";
//       Extent = "800 600";
//       BaseExtent = "800 600";
//       MinExtent = "8 2";
//       canSave = "1";
//       Visible = "1";
//       hovertime = "1000";
//       applyFilterToChildren = "1";
//       cameraZRot = "0";
//       forceFOV = "30";
//    };
// };

// FIXME
  // %this = invaderGame;
  // %this.highScores = HighScore_create( getUserDataDirectory() @ "/Ohmtal Game Studio/OGE Invader/invhi.dta", 15 , true);
  // dError("ID:" SPC %this.highScores.getId());
//   if (! %this.highScores.loadHi() )
//   {
//         for (%i=0; %i<%this.highScores.maxScores ; %i++)
//                  %this.highScores.add("Ohm", 1998 + %i*666);
//                 //%this.highScores.add("Ohmtal", getRandom(50000,120000));
//         %this.highScores.saveHi();
//   }
//
//   invaderGame.hiscore = %this.highScores.getTopScore();


  loadGraphics();
  loadSounds();
  
  
  invaderGame.blockpos    = "120 280 440 600";
  invaderGame.blockStartY = 470; 
  invaderGame.playerY     = 520;

  invaderGame.limitX1     = 100;
  invaderGame.limitX2     = 700-16;
  invaderGame.limitX3     = 800-16;

  invaderGame.limitY1     = 10;
  invaderGame.limitY2     = 520+32;
  
  invaderGame.fastshoot = 1;
  invaderGame.enableBlockade = true;

  invaderGame.score   = 0;
  invaderGame.lives   = 0;
  //invaderGame.hiscore = 2000;
  
  invaderGame.gameover = true;
  invaderGame.wave    = 0;
  invaderGame.debug = "";
  invaderGame.aliendownstep   = 16;

  // FIXME invaderGame.hiFile = getUserDataDirectory() @ "/Ohmtal Game Studio/OGE Invader/invaderGoWild";
  //FIXME invaderGame.PrefsFile = getUserDataDirectory() @ "/Ohmtal Game Studio/OGE Invader/foo.cs";
  
  // FIXME read old
  // if ( exec(invaderGame.PrefsFile, true) )
  //       error("LAST START WAS ******************* " @ $foo::lastStart @ " *************************");
  // else
  //       error("********************* CANT READ fOO.cs ************************");
  
  // FIXME save new
  // $foo::lastStart=getLocalTimeFormated();
  // export("$foo::*", invaderGame.PrefsFile, false);
  
  // Canvas.setContent( PlayGui );
  // Canvas.setCursor("DefaultCursor");
  
  // invaderGame.setFirstResponder(); // MUST HAVE, else inputs does not work
}
//------------------------------------------------------------------------------
function addScore(%score) {
  invaderGame.score += %score;
  if (invaderGame.score > invaderGame.hiscore) {
    invaderGame.hiscore=invaderGame.score;
    
  }
}

//------------------------------------------------------------------------------
function loadTexture(%filename) {
  %result = new Texture() {
    fileName = "assets/texture/" @ %filename @ ".png";
  };
  CleanupSet.add(%result);
  return %result;
  
}
//------------------------------------------------------------------------------
function loadGraphics() {

  invaderGame.img_back = loadTexture("back");
  
  // aliens with alternative image
  invaderGame.img_alien1_0 = loadTexture("alien1_0");
  invaderGame.img_alien1_1 = loadTexture("alien1_1");
  invaderGame.img_alien2_0 = loadTexture("alien2_0");
  invaderGame.img_alien2_1 = loadTexture("alien2_1");
  invaderGame.img_alien3_0 = loadTexture("alien3_0");
  invaderGame.img_alien3_1 = loadTexture("alien3_1");


  // alien explosion
  invaderGame.img_alien_e = loadTexture("alien_explosion");

  // bonus alien
  invaderGame.img_alien4_0 = loadTexture("alien4");
  
  // blocks
  invaderGame.img_block_0  = loadTexture("block_0");
  invaderGame.img_block_1  = loadTexture("block_1");
  invaderGame.img_block_2  = loadTexture("block_2");
  invaderGame.img_block_3  = loadTexture("block_3");
  invaderGame.img_block_4  = loadTexture("block_4");
  
  // player
  invaderGame.img_player     = loadTexture("player");
  invaderGame.img_player_e_0 = loadTexture("player_explosion_0");
  invaderGame.img_player_e_1 = loadTexture("player_explosion_1");
  invaderGame.img_shield     = loadTexture("shield");
  
  
  // missile
  invaderGame.img_missile    = loadTexture("shoot");
  invaderGame.img_missileAlien    = loadTexture("shoot_a");

}
//------------------------------------------------------------------------------
function loadSound(%name,%filename,%looping) {

  // if (%looping $= "") %looping = false;
  %profile = new AudioProfile(%name)
  {
    filename = "assets/sound/" @ %filename;
    looping = %looping;
  };
  CleanupSet.add(%profile);

}
//------------------------------------------------------------------------------
function loadSounds() {

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

  //loadSound(,"");

}
//------------------------------------------------------------------------------
// invaderGame
//------------------------------------------------------------------------------
function invaderGame::drawHigh(%this,%dt)
{

 %screen=%this;
  %x = 200;
  %y = 150;
        
        // 0=left, 1=middle, 2=right, 3=center
// %screen.writeText(%x+50,%y,"Highscores Top" SPC %this.highScores.maxScores, $textAlignLeft,LCDTextProfile,0);
        
//         %y+=35;
//         for (%i=0; %i<%this.highScores.maxScores ; %i++)
//         {
//             %l = %this.highScores.getPlace(%i);
//             // * score has format: [score] TAB [localtime] TAB [name]
//             switch (%i)
//             {
//                 case 0: %pText= "ST";
//                 case 1: %pText= "ND";
//                 case 2: %pText= "RD";
//                 default: %pText= "TH";
//             }
//             %screen.writeText(%x,%y,fillLeadingChar(%i+1,2," ") @ %pText, $textAlignRight,LCD2TextProfile, 0);
//             // %screen.writeText(%x,%y,fillLeadingChar(%i+1,2,"0"), $textAlignLeft,LCD2TextProfile, 0);
//             %screen.writeText(%x+15,%y,getField(%l,1), $textAlignLeft,StatusTextProfile20,0);
//             // name %screen.writeText(%x+200,%y,getField(%l,2), $textAlignLeft,LCD2TextProfile, 0);
//             %screen.writeText(%x+280,%y,getField(%l,0), $textAlignRight,LCD2TextProfile,0);
//             %screen.writeText(%x+320,%y,getField(%l,2), $textAlignLeft);
//             %y+=25;
//         }
//
}


function invaderGame::onRender(%this,%dt) {
  // compat
  %dt *= 1000;
  //drawBackground
  %this.drawstretch(invaderGame.img_back,0,400,300,99,800,600,0,false,false);

  %this.writeText(5,5,"Score:" SPC invaderGame.score, $align::left );
  %this.writeText(400,5,"HI:" SPC  invaderGame.hiscore, $align::center );
  %this.writeText(795,5,"LIVES:" SPC invaderGame.lives, $align::right);
    
  if (!invaderGame.GameOver) {
    invaderGame.player.process(%dt);

    //FIXME TEST
    processAliens(%dt);
    processbonusAlien(%dt);
    processMissiles(%dt);

    drawBlocks();
    drawAliens();
  
  } else {
  
    %this.writeText(400,100,"Game Over",$align::center, "0.9 0.1 0.1 0.7");
    %this.drawHigh(%this,%dt);
    
  }
  /*
  if (isDebugBuild())
    %this.writeText(5,580,"DT:" SPC %dt,0);
  */
    
  if (invaderGame.escapeHit == 1)
        %this.writeText(400,200,"Hit again to quit the game.",$align::center);
}

//------------------------------------------------------------------------------
function invaderGame::onInputEvent( %this, %deviceString, %actionString, %mouseX, %mouseY, %keyValue ) {

  // FIXME !!!

 dEcho("invaderGame::onInputEvent Device:" SPC %deviceString SPC "action:" SPC %actionString SPC "mx/my:" SPC %mouseX SPC %mouseY SPC "DOWN:" SPC %keyValue);



 if (%deviceString $= "keyboard") {
   invaderGame.Keys[%actionString] = %keyValue;
   
   if (!%keyValue) {//keyup
     if (%actionString $= "escape") {
                invaderGame.escapeHit ++;
                if (invaderGame.escapeHit>1)
                {
                        alxStopAll();
                        quit();
                } else {
                  %this.schedule(3000,resetEscapeHit);
                }
     } else  if (%actionString $= "space" ) {
        if (!invaderGame.GameOver)
                invaderGame.Player.loaded = false;
        else
                resetGame();
     }
   }
   
 }  else if (%deviceString $= "joystick0" || %deviceString $= "accelerometer") {

   /*
     Notes:
      1.) This needs a small hack in ActionMap::buildActionString to get the axis of a joystick 
      2.) On diagonal movement this is called two time with each axis, keep in mind!
      3.) Axis value is the %keyValue value! -1,0,1
      4.) Only tested with a digital joystick, so far
   */
   // joystick 
   if (%actionString $= "xaxis" ) {
   
      invaderGame.Keys["left"] = 0;
      invaderGame.Keys["right"] = 0;
      if (%keyValue > 0.4) { //right
        invaderGame.Keys["right"] = 1;
      } else if (%keyValue < -0.4) {//right
        invaderGame.Keys["left"] = 1;
      }
   }
   
   // phone
   if (%actionString $= "accely" ) {
   
      invaderGame.Keys["left"] = 0;
      invaderGame.Keys["right"] = 0;
      if (%keyValue > 0.4) { //right
        invaderGame.Keys["right"] = 1;
      } else if (%keyValue < -0.4) {//right
        invaderGame.Keys["left"] = 1;
      }
   }
   
        switch$ (%actionString)
        {
                case "button0":   
                        invaderGame.Keys["space"] = %keyValue;
                        if (!invaderGame.GameOver)
                                    invaderGame.Player.loaded = false;

                case "button1":
                        if (%keyValue)
                        {
                                if (!invaderGame.GameOver)
                                        invaderGame.Player.loaded = false;
                                else
                                        resetGame();
                        }

                case "rpov":      invaderGame.Keys["right"] = %keyValue;
                case "lpov":      invaderGame.Keys["left"] = %keyValue; 
        }
        
 
 } //joystick
 

}
//------------------------------------------------------------------------------
// common stuff:
//------------------------------------------------------------------------------
function resetGame(%wave) {

  invaderGame.wave = %wave;
  resetPlayer();
  if (%wave  == 0) 
        resetBlockade();
  resetAliens();
  resetMissiles();

  if (isObject(invaderGame.bonusAlien))
        invaderGame.bonusAlien.schedule(0,delete);
        
  
  
 if (%wave == 0) {
   invaderGame.score = 0;
   invaderGame.lives = 3;
   invaderGame.gameOver = false;
 }
}
//-------------------------------------------------------------
function nextWave() {
  alxStopAll();
  resetGame(invaderGame.wave+1);
}
//-------------------------------------------------------------
function doGameOver() {
  alxStopall();
  
  if (isObject(invaderGame.blockades))
        invaderGame.blockades.schedule(0,delete);
  if (isObject(invaderGame.player))
        invaderGame.player.schedule(0,delete);
  if (isObject(invaderGame.bonusAlien))
        invaderGame.bonusAlien.schedule(0,delete);
  if (isObject(invaderGame.aliens))
        invaderGame.aliens.schedule(0,delete);
  if (isObject(invaderGame.missiles))
        invaderGame.missiles.schedule(0,delete);
  
  if ( invaderGame.score >= invaderGame.hiscore)
    saveHi();
    
  %this = invaderGame;  
  if (%this.highScores.add("FooBar", invaderGame.score))
  {
    %this.highScores.saveHi();
  }
    
        
  invaderGame.lives = 0;
  invaderGame.gameOver = true;
}
//------------------------------------------------------------------------------
// player stuff:
//------------------------------------------------------------------------------

function InvPlayer::process(%this,%dt) {

  if (%this.damage == 0) {
        if (invaderGame.Keys["left"] && %this.x > invaderGame.limitX1)
                %this.x = %this.x - 0.15 * %dt;
        else if (invaderGame.Keys["right"] && %this.x < invaderGame.limitX2)
                %this.x = %this.x + 0.15 * %dt;
                
                 
                
        if (invaderGame.fastshoot && %this.Shootdelay > 0)
        {
                %this.Shootdelay -= %dt;
        }        
        if (invaderGame.Keys["space"]) 
        {
           invaderGame.Keys["space"] = 0; //phone!
           if (invaderGame.fastshoot ) {//shooting on press/release
             if (!%this.loaded && %this.Shootdelay <= 0)
             {
                %this.Shootdelay=300;
                %this.missile=launchMissile(%this);
                %this.loaded = true;
              }
           } else if (!isObject(%this.missile)) {//classic shooting
                %this.missile=launchMissile(%this);
           }
        }
        %img = %this.img;        
   } else {
     if (%this.damage > 200) {
       invaderGame.lives--;
       if (invaderGame.lives < 0 ) {
         doGameOver();
         return;
       }
       resetMissiles();
       resetPlayer();
       return;
     } else if (%this.damage % 5 == 0) {
       %img = %this.exImg1;
     } else { 
       %img = %this.exImg0;
     }
     %this.damage += 0.07 * %dt;  
   }
   
   // echo("CALL PLAYER DRAW");

  %this.draw(%img);

  // echo("done player draw, call shield draw:");


  //ALSO CRASH: %this.drawShield(%dt);
}
//-------------------------------------------------------------
function resetPlayer() {
  if (isObject(invaderGame.player))
        invaderGame.player.schedule(0,delete);
        
  invaderGame.player=new ScriptObject() {
    class = InvPlayer;
    img   = invaderGame.img_player;
    exImg0= invaderGame.img_player_e_0;
    exImg1= invaderGame.img_player_e_1;
    shieldImg = invaderGame.img_shield;
    damage = 0;
    r = 16;
    x = 400-16;
    y = invaderGame.playerY;
    layer = 10;
    
  };
        
}
//-------------------------------------------------------------
function invPlayer::draw(%this, %img) {

// echo("layer is " SPC %this.layer);
  invaderGame.drawstretch(%img ,0,%this.x,%this.y,%this.layer,32,32 ,0, false,false);


}
//-------------------------------------------------------------
function invPlayer::drawShield(%this, %dt) {
  
  //(tom2DTexture,imgId,x,y,layer,w,h,
  // [rotation,flipX,flipY, alpha channel default 0.1], optimizetransparent)
  %this.shieldRotate += %dt * 0.1 ;
  if (%this.shieldRotate > 360)
      %this.shieldRotate = 0;
   
  invaderGame.drawstretch(%this.shieldImg ,0,%this.x,%this.y,%this.layer,60,60,
   %this.shieldRotate, false,false,0,true);
   
   
  //invaderGame.writeText(%this.x,%this.y-50,"ROT:" SPC %this.shieldRotate SPC "DT" SPC %dt,1); 
}

//------------------------------------------------------------------------------
// blockade stuff:
//------------------------------------------------------------------------------
function resetBlockade() {
//echo("ENTER resetBlockade " SPC invaderGame.blockades);
  if (isObject(invaderGame.blockades)) {
        invaderGame.blockades.schedule(0,delete);
  }

  if (!invaderGame.enableBlockade)
	return;

  invaderGame.blockades = new SimGroup();

  for (%i=0; %i<4; %i++) {
     createBlock(getWord(invaderGame.blockpos,%i));    
  }

}
//-------------------------------------------------------------
function createBlock(%pos) {
  %xpos = %pos;
  %ypos = invaderGame.blockStartY;
  
  for (%i=0;%i<4;%i++) {
        %xpos = %pos;
        for (%j=0;%j<9;%j++) {
          %block = new ScriptObject() {
            class = "invBlock";
            x = %xpos;
            y = %ypos;
            damage = 0;
            r = 5;
            layer = 10;
          };
          invaderGame.blockades.add(%block);
          %xpos += 8;
        }
        %ypos += 8;
  }
}
//-------------------------------------------------------------
function drawBlocks() {
  if (!isObject(invaderGame.blockades))
        return;
  for (%i=0;%i<invaderGame.blockades.getCount();%i++) {
     %b = invaderGame.blockades.getObject(%i);
     // Move Missile
     %img = invaderGame.img_block_0;
     switch (%b.damage) {
       case 80: %img = invaderGame.img_block_4;
       case 60: %img = invaderGame.img_block_3;
       case 40: %img = invaderGame.img_block_2;
       case 20: %img = invaderGame.img_block_1;
     }
     invaderGame.draw(%img,%b.x,%b.y,%b.layer);
  }
}
//------------------------------------------------------------------------------
// Aliens
//------------------------------------------------------------------------------
function resetAliens() {
  if (isObject(invaderGame.aliens))
        invaderGame.aliens.schedule(0,delete);
  invaderGame.aliens = new SimGroup();
  invaderGame.aliens.lastalienmove = 0;
  invaderGame.aliens.idxsnd = 0;
  %startx = 96 + invaderGame.wave * invaderGame.aliendownstep * 2;
  if (%startx>= 304) {
    %startx = 304 ;
  }
  
  %starty = 184;

  createAlienRow(invaderGame.img_alien3_0,invaderGame.img_alien3_1,%starty,%startx, 40);
  createAlienRow(invaderGame.img_alien2_0,invaderGame.img_alien2_1,%starty,%startx+32, 20);
  createAlienRow(invaderGame.img_alien2_0,invaderGame.img_alien2_1,%starty,%startx+64, 20);
  createAlienRow(invaderGame.img_alien1_0,invaderGame.img_alien1_1,%starty,%startx+96, 10);
  createAlienRow(invaderGame.img_alien1_0,invaderGame.img_alien1_1,%starty,%startx+128, 10);

}
//-------------------------------------------------------------
function createAlienRow(%img0,%img1,%xstart,%y,%score){
  %x = %xstart;
  for (%i=0; %i< 9; %i++) {
    %a = new ScriptObject() {
      class = "invAlien";
      x = %x; 
      y = %y;
      anim = 0;
      img0 = %img0;
      img1 = %img1;
      exImg= invaderGame.img_alien_e;
      damage = 0;
      score = %score;
      direction = 0;
      r = 16;
      layer = 10;
    };
    invaderGame.aliens.add(%a);
    %x += 48;
  }
}
//-------------------------------------------------------------
function drawAliens(){
  if (!isObject(invaderGame.aliens))
        return;
  for (%i=0;%i<invaderGame.aliens.getCount();%i++) {
     %a = invaderGame.aliens.getObject(%i);
     // Move Missile
     %img = "";
     if (%a.damage >= 100) {
       %img = %a.exImg;
       %a.damage += 50;
     } else if (%a.anim == 0) {
       %img = %a.img0;
     } else {
       %img = %a.img1;
     }
     
     invaderGame.draw(%img,%a.x,%a.y,%a.layer);
  }

}
//-------------------------------------------------------------
function processAliens(%dt){

  if (!isObject(invaderGame.aliens)) return;
  //move speed 
  invaderGame.aliens.lastalienmove =  invaderGame.aliens.lastalienmove + %dt;
  %cnt = invaderGame.aliens.getCount();
  
  if (%cnt == 0) {
    nextWave();
    return;
  }
  
  %moveinterval = 25 * %cnt;
  //echo( "LAST: " SPC invaderGame.aliens.lastalienmove SPC "INTERVALL:" SPC %moveinterval); 
  if (invaderGame.aliens.lastalienmove < %moveinterval) 
        return;
        
  //sound
  invaderGame.aliens.idxsnd = invaderGame.aliens.idxsnd + 1;
  if (invaderGame.aliens.idxsnd > 4) 
        invaderGame.aliens.idxsnd = 1;
  
  alxStop(invaderGame.aliens.snd);
  %snd = ("AlienMoveSound" @ invaderGame.aliens.idxsnd); //.getid();
//echo("SOUND = " SPC %snd);  
  invaderGame.aliens.snd = alxPlay(%snd);
  
  //may we spawn a bonus alien ?! 
  if (!isObject(invaderGame.bonusAlien)) {
      %dospawn = getrandom(1, 30 ); 
      if (%dospawn == 1)
        spawnBonusAlien(); 
  }

  //move em and check for destroyed
  invaderGame.aliens.lastalienmove = invaderGame.aliens.lastalienmove - %moveinterval;
  %setNextDir = -1;
  %speed = 16;
  for (%i = 0; %i<%cnt; %i++) {
    %a = invaderGame.aliens.getObject(%i); 
    if (%a.damage > 200) {
      %a.schedule(0,delete);
      continue;
    }
    
    if (%a.direction == 0) { //move right
      %a.x+=%speed;
      if (%a.x >= invaderGame.limitX2) 
        %setNextDir = 1;
    
    } else if (%a.direction == 1 || %a.direction == 2 ) { //move down then left or right
      %a.y += invaderGame.aliendownstep;
      if (%setNextDir == -1)
        %setNextDir = %a.direction == 1 ? 3 : 0;
        
      if (%a.y >= invaderGame.playerY - 16  ) { //game over ?
          doGameOver();
          return;
      } else if (%a.y >= invaderGame.blockStartY-4 *8 && isObject(invaderGame.blockades)) { //remove blockades!
         invaderGame.blockades.schedule(0,delete);
      }
    } else {   //move LEFT
      %a.x-=%speed;
      if (%a.x <= invaderGame.limitX1) 
        %setNextDir = 2;
    }
    
    //animation
    if (%a.anim == 0)  
        %a.anim = 1;
    else
        %a.anim = 0;
    
    //echo("set alien anim:" SPC %a.anim);    
    
    //shoot a missile ? 
    if (getRandom(0,%cnt * 10) == %i) {
      launchMissile(%a);
    }
  } //for loop

  //change direction if needed
  if (%setNextDir != -1) 
        setAlienDirection(%setNextDir);


}
//-------------------------------------------------------------
function setAlienDirection(%newDirection) {
  %cnt = invaderGame.aliens.getCount();
  for (%i = 0; %i<%cnt; %i++) {
    %a = invaderGame.aliens.getObject(%i);
    %a.direction = %newDirection;
  }
}
//------------------------------------------------------------------------------
// Bonus Alien
//------------------------------------------------------------------------------
function spawnBonusAlien() {

  if (isObject(invaderGame.bonusAlien))
        invaderGame.bonusAlien.schedule(0,delete);

  %score = getrandom(1, (invaderGame.wave + 1) * 4 ) * 50;   
  
  invaderGame.bonusAlien=new ScriptObject() {
    class = InvBonusAlien;
    img   = invaderGame.img_alien4_0;
    dead = 0;
    x = 16;
    y = 40;
    damage = 0;
    score = %score;
    layer = 10;
    r = 16;
  };
  invaderGame.bonusAlienSound =  alxPlay(BonusAlienLoopSound);
}
//-------------------------------------------------------------
function processbonusAlien(%dt) {
  if (!isObject(invaderGame.bonusAlien))
  {
        invaderGame.bonusAlien = 0;
        return;
  }
  
   %a = invaderGame.bonusAlien;
  if (%a.damage >= 100) {
         if (alxIsPlaying(invaderGame.bonusAlienSound))
                alxStop(invaderGame.bonusAlienSound);

         %a.damage += 0.05 * %dt;
         if ( %a.damage> 200) {
             %a.schedule(0,delete);
             return;
         }
        invaderGame.writeText(%a.x,%a.y,%a.score,$align::center);
        return;
  }
  
  //move it, removeit 
  %a.x += %dt * 0.05;
  if ( %a.x > invaderGame.limitX3) {
    %a.schedule(0,delete);
    if (alxIsPlaying(invaderGame.bonusAlienSound)) 
           alxStop(invaderGame.bonusAlienSound);
    return;
  }
  
  //draw it
  invaderGame.draw(%a.img,%a.x,%a.y, %a.layer);
        
}
//------------------------------------------------------------------------------
// Missiles
//------------------------------------------------------------------------------
function resetMissiles() {
  if (isObject(invaderGame.missiles))
        invaderGame.missiles.schedule(0,delete);
  invaderGame.missiles = new SimGroup();
}
//-------------------------------------------------------------
function launchMissile(%owner) {


  switch$ (%owner.class) { 
       case "invPlayer":
                alxPlay(PlayerShootSound);
                %img = invaderGame.img_missile; 

       case "invAlien":
                alxPlay(AlienShootSound);
                %img = invaderGame.img_missileAlien;

       default: //huh wrong owner ?!
                return;
  }


  %result = new ScriptObject() {
     class = "invMissile";
     owner = %owner;
     ownerclass = %owner.class; 
     img = %img;
     layer = 5;
     r = 4;
     x = %owner.x; //center shoot +ownerwidth/2-missilewidth/2
     y = %owner.y;
     
  };
  invaderGame.missiles.add(%result);
  
  
  return %result;
}
//-------------------------------------------------------------
function processMissiles(%dt) {
  if (!isObject(invaderGame.missiles))
        return;
  for (%i=0;%i<invaderGame.missiles.getCount();%i++) {
     %m = invaderGame.missiles.getObject(%i);
     if (%m.disabled) continue;
     // Move Missile
     switch$ (%m.ownerclass) { 
       case "invPlayer":
                     %m.y = %m.y - 0.3 * %dt;
                     if (%m.y <=invaderGame.limitY1) {
                       if (isObject(%m.owner))
                            %m.owner.missile = "";
                       error("DELETE Missile AT Y:" SPC %m.y);
                       %m.schedule(0,delete);
                     }
       case "invAlien":
                     %m.y = %m.y + 0.3 * %dt;
                     if (%m.y >=invaderGame.limitY2) {
                       if (isObject(%m.owner))
                            %m.owner.missile = "";
                       %m.schedule(0,delete);
                     }
     }
     //collision Check 
     if (%m.collideCheck()) {
//XXTH22
       error("Missile collide! owner:" SPC %m.owner SPC " ownerclass " SPC %m.ownerclass SPC " collisionobject = " SPC %m.lastCollision SPC "colType" SPC %m.dings);
       %m.disabled = true;
       if (isObject(%m.owner))
            %m.owner.missile = "";
            %m.schedule(0,delete);
     }

     //draw
     invaderGame.draw(%m.img,%m.x,%m.y, %m.layer);     
  
  } //loop
}
//-------------------------------------------------------------
function invMissile::colCheck(%this, %obj) {

  
   if (%obj.damage < 100    
       && %this.x > %obj.x - %obj.r   
       && %this.x < %obj.x + %obj.r 
       && %this.y > %obj.y - %obj.r   
       && %this.y < %obj.y + %obj.r
       )
        return true;
  
  
   return false;
}
//-------------------------------------------------------------
function invMissile::collideCheck(%this) {

  %this.lastCollision = 0;
  %this.dings="";

  // 1. all like to collide with a block, we only check if it's in the y range of a block
  if (isObject(invaderGame.blockades) && %this.y > invaderGame.blockStartY -4 && %this.y < invaderGame.blockStartY+28)
      for (%i=0; %i<invaderGame.blockades.getCount(); %i++){
        %o=invaderGame.blockades.getObject(%i);
        if (%this.colCheck(%o)) {
            %o.damage += 40;
            if (%o.damage >= 80) 
                    %o.schedule(0,delete);
            %this.lastCollision = %o;
            %this.dings="blockade";
            return true;
        }
      }
  
 //2. players missle like to collide with alien
   if (%this.ownerclass $= "invPlayer") {
      //usual aliens   
      for (%i=0; %i<invaderGame.aliens.getCount(); %i++){
        %o=invaderGame.aliens.getObject(%i);
        if (%this.colCheck(%o)) {
            %o.damage = 100;
            addScore( %o.score );
            alxPlay(AlienExplosionSound);
            %this.lastCollision = %o;
            %this.dings="alienship";
            return true;
        }
      }
      
      //bonus alien
      %o=invaderGame.bonusAlien;
      if (isObject(%o) && %this.colCheck(%o)) {
            %o.damage = 100;
            addScore(%o.score);
             if (alxIsPlaying(invaderGame.bonusAlienSound)) 
                alxStop(invaderGame.bonusAlienSound);
            alxPlay(BonusAlienExplosionSound);
            %this.lastCollision = %o;
            %this.dings="bonusalienship";
            return true;
      }
      
   } else
     // 3. alien missle like to collide with player   
     if (%this.ownerclass $= "invAlien") {  
        %o=invaderGame.player;
        if (%this.colCheck(%o)) {
            %o.damage = 100;
            alxPlay(PlayerExplosionSound);
            %this.lastCollision = %o;
            %this.dings="playership";
            return true;
        }
   }
 

   
  return false;
}

//------------------------------------------------------------------------------
function cheatme() {

  invaderGame.fastshoot = !invaderGame.fastshoot;
  if (invaderGame.fastshoot) {
     echo("CHEAT ON");
     invaderGame.lives = 99;
  } else {
     echo("CHEAT OFF");
  }

}

// //------------------------------------------------------------------------------
// function onAndroidResignActive()
// {
// //DONE IN ENGINE NOW!  OpenALShutdownDriver();
//
// }
// //------------------------------------------------------------------------------
// function onAndroidBecomeActive()
// {
// //DONE IN ENGINE NOW!	OpenALInitDriver();
//   // HERE GO AND START LOOPING SOUND LIKE MUSIC AGAIN!!!
//
//
// }
//
// //------------------------------------------------------------------------------
// function androidBackButton(%down)
// {
//   invaderGame.onInputEvent("keyboard","escape",0,0,%down);
// }


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
initInvader();

