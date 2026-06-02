//------------------------------

if (!isObject(CleanupSet)) new SimSet(CleanupSet);
else CleanupSet.deleteObjects();


// --------- should be in engine but i'am testing TorqueScript ----------------
function shiftColorHue(%color, %degreeShift)
{

  %r = getWord(%color, 0); %g = getWord(%color, 1); %b = getWord(%color, 2);
  if (%r > 1.0 || %g > 1.0 || %b > 1.0) { %r /= 255.0; %g /= 255.0; %b /= 255.0; }

  %max = (%r > %g) ? ((%r > %b) ? %r : %b) : ((%g > %b) ? %g : %b);
  %min = (%r < %g) ? ((%r < %b) ? %r : %b) : ((%g < %b) ? %g : %b);
  %delta = %max - %min;


  %v = %max;
  %s = (%max == 0) ? 0 : (%delta / %max);

  if (%delta == 0) %h = 0;
  else if (%max == %r) { %h = (%g - %b) / %delta; if (%h < 0) %h += 6; }
  else if (%max == %g) { %h = ((%b - %r) / %delta) + 2; }
  else                 { %h = ((%r - %g) / %delta) + 4; }
  %h *= 60;


  %h = (%h + %degreeShift) % 360;
  if (%h < 0) %h += 360;

  return hsvToRGB(%h, %s, %v);
}

function hsvToRGB(%h, %s, %v)
{
  // echo("hsvToRGB" SPC %h SPC  %s SPC  %v);
  if (%s == 0) return %v SPC %v SPC %v; // Graustufe

  %h /= 60;
  %i = mFloor(%h);
  %f = %h - %i;
  %p = %v * (1 - %s);
  %q = %v * (1 - (%s * %f));
  %t = %v * (1 - (%s * (1 - %f)));

  if (%i == 0)      return %v SPC %t SPC %p;
  else if (%i == 1) return %q SPC %v SPC %p;
  else if (%i == 2) return %p SPC %v SPC %t;
  else if (%i == 3) return %p SPC %q SPC %v;
  else if (%i == 4) return %t SPC %p SPC %v;
  else              return %v SPC %p SPC %q;
}

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
    x = getScreenWidth() / 2.0;
    y = 40;
    shadow=1;
    scale=2;
    align = 1;
    color = "1 0 0";
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

  %this.label1.shift = ( %this.label1.shift + 0.2) % 360;
  // modulo now rocks ;) if (%this.label1.shift >= 360) %this.label1.shift = 0;
  %this.Label1.color = shiftColorHue("0 1 0",%this.label1.shift );


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

function Game::ml(%this, $stop) {

  %this.label1.x =  (%this.label1.x + 1.4) % 1200 ;
  // echo("X is" SPC %this.label1.x);
  if (!%stop) %this.schedule(10,ml);
}

//FIXME class does NOT work!
$Game = new GameCtrl() {
  class = "Game";
};


$Game.LoadAssets();
CleanupSet.add($Game);
$game.ml(); //scrolling using schedule ..


