// b2b Test ... unfinished !!!

include("./physicObjects/wall.cs");
include("./physicObjects/ball.cs");

function Game::Init(%this) {
  %this.rect = new RectObject() {
    x = 50;
    y = 50;
    w = 600;
    h = 400;

  };

  %this.thick = 10;

  %this.debugRender = true;

  // we need a world
  %this.world = new World2b();
  %this.world.setGravity("0 9.81");
  %this.world.setRatio(32);


  //------------------- WALLS ------------------------------------------

  %l = %this.Rect.x ;
  %t = %this.Rect.y ;
  %r = %l + %this.Rect.w ;
  %b = %t + %this.Rect.h ;

  // -------------- TOP Wall continue from wall %w/%h ------------------------

  %this.topWall = %this.CreateWallRect(%l SPC %t SPC %r SPC %t, "Wall");
  %this.bottomWall = %this.CreateWallRect(%l SPC %b SPC %r SPC %b, "BottomWall");
  %this.leftWall = %this.CreateWallRect(%l SPC %t SPC %l SPC %b, "Wall");
  %this.rightWall = %this.CreateWallRect(%r SPC %t SPC %r SPC %b, "Wall");

  // LoadBall
  %this.ball = %this.createBall(getScreenCenter());

  %this.balls = new SimSet();
  for (%i = 0; %i < 20; %i++) {
  	%b= %this.createBall(getScreenCenter());
  	%b.launch();
  	%this.balls.add(%b);
  }
 


  //LoadAssets
  %this.LoadAssets();
}
function Game::LoadAssets(%this) {

}

function Game::onInputEvent( %this, %deviceString, %actionString, %mouseX, %mouseY, %keyValue ) {
  if ( %deviceString $= "keyboard") {
    %this.paddle.dir = 0;
    if (%keyValue == true) {
      switch$ (%actionString) {
        case "Space":
          if (%this.ball.getactive()) %this.ball.stop();
          else %this.ball.launch();
        case "up":
        	for (%i = 0; %i < %this.balls.getCount(); %i++)
        		%this.balls.getObject(%i).imp();
      }
    } else {
      //Key up ...

    }
  }
}

function Game::onRender(%this,%dt) {
  %this.writeText(20, 30, "Box2D Test ....  unfinished!");

}

function Game::onUpdate(%this,%dt) {
  %this.world.step(%dt,8,2); //8,2 ??
}

function Game::onRemove(%this) {
  // %this.deleteObjects();
}



//------------------------------
// Main
//------------------------------
GarbageCollectionSet.deleteObjects();
//------------------------------
$Game = new GameCtrl() {
  class = "Game";
};

$Game.Init();


