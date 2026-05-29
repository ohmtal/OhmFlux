//------------------------------

if (!isObject(CleanupSet)) new SimSet(CleanupSet);
else CleanupSet.deleteObjects();

//------------------------------
function Game::LoadAssets(%this) {
  echo("GAME ON ADD.....");
  // ------
  %this.texBack =  new Texture() {
    fileName = "assets/texture/nebulapurple_sky_back.png";
  };
  %this.add(%this.texBack);
  // ------
  %this.fontJet = new Font() {
    fileName = "assets/font/JetBrainsMono-Regular.ttf";
  };
  %this.add(%this.fontJet);
  // ------
 %this.Label1 = new Label()  {
    Font = %this.fontJet;
    x = 20;
    y = 40;
    Caption = "Hello World";
  };
  %this.add(%this.Label1);
  // ------
 %this.background = new Sprite() {
    Texture = %this.texBack;
    x = getScreenWidth() / 2.0;
    y = getScreenHeight() / 2.0;
    z = 1.0; //layer
    w = getScreenWidth();
    h = getScreenHeight();
  };
  %this.add(%this.background);
  // ------
  $texFaces = new Texture() {
    fileName = "assets/texture/faces.png";
    TexCols = 13;
  };
  %this.add($texFaces);
  // ------
  %this.sprite = new Sprite() {
    Texture = $texFaces;
    x = 100;
    y = 100;
    z = 0.5;
    w = 64;
    h = 64;
    imgId = 3;
  };
  %this.add(%this.sprite);
  // ------
  %this.sndPling = new AudioProfile(SndPling) { fileName = "assets/sound/pling.ogg"; Volume = 0.2; };
  %this.add(%this.sndPling);

}

function Game::onInputEvent( %this, %deviceString, %actionString, %mouseX, %mouseY, %keyValue ) {
  if (%keyValue ) {
    echo("ACTION=" SPC %actionString);
    if (%actionString $= "button3" ) %this.c(%mouseX, %mouseY);  // )%this.schedule(0,c); //defered
  }

}


function Game::onUpdate(%this,%dt) {
  %this.Label1.Caption = getFPS() SPC "FPS";
}

function Game::c(%this, %mx, %my, %p) {
  alxPlay(SndPling.getId());
  if (!%p)  %p = %this.sprite;
  if (!%p) { error("No object %this.sprite!"); return 0; }
  // clone does NOT work with drawparams ?! %clone = %p.clone();

  %layer =  getRandom(1, 999) / 1000;
  // echo("LAYER:" SPC %layer);

  %clone = new Sprite() {

      Texture = %p.Texture;
      w = %p.w;
      h = %p.h;

      imgId = getRandom(13);

      x = %mx; //getRandom(1000) + %p.w;
      y = %my; //getRandom(400) + %p.h;
      z = %layer;
  };


  %this.add(%clone);


  return %clone;
 }

function Game::onRemove(%this) {
  error("GAME ONREMOVE!");
  %this.deleteObjects();
}

//FIXME class does NOT work!
$Game = new GameCtrl() {
  class = "Game";
};
$Game.LoadAssets();
CleanupSet.add($Game);


