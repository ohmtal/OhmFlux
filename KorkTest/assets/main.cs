// echo("Hello MAIN.CS");
//------------------------------
echo ("------------------------------");
echo ("STEPS for simple but usable:");
echo ("[X] basic Sprite and Texture ");
echo ("[X] fix dumpConsoleClasses cause segFault => vec[i]->getUsage() is dangling pointer! Namespace::mUsage was not initialized");
echo ("[ ] Add Sound - need my SoundTestBed finished!! or simply use the current.");
echo ("[ ]  add init/shutdown funcs -- mhh how to name  ");
echo ("[ ] Input Keyboard and Mouse");
echo ("[ ] write pong ...");
echo ("[ ] Add collision or better Box2D");
echo ("------------------------------");


//------------------------------

if (!isObject(CleanupSet)) new SimSet(CleanupSet);
else CleanupSet.deleteObjects();

//------------------------------

  $texBack = new Texture() {
    fileName = "assets/texture/nebulapurple_sky_back.png";

  };
  CleanupSet.add($texBack);

  $background = new Sprite() {
    Texture = $texBack;
    x = getScreenWidth() / 2.0;
    y = getScreenHeight() / 2.0;
    w = getScreenWidth();
    h = getScreenHeight();
  };
  CleanupSet.add($background);
  // ------
  $texFaces = new Texture() {
    fileName = "assets/texture/faces.png";
    TexCols = 13;
  };
  CleanupSet.add($texFaces);

  // $texFaces.dump();

  $sprite = new Sprite() {
    Texture = $texFaces;
    x = 100;
    y = 100;
    w = 64;
    h = 64;
    imgId = 3;
  };
  CleanupSet.add($sprite);
  // ------
  $sndPling = new AudioProfile(SndPling) { fileName = "assets/sound/pling.ogg"; };
  CleanupSet.add($sndPling);




function Sprite::bar(%this) {
  echo("getSimTime:" SPC getSimTime());

  %this.x += 5;

  %this.schedule(1000, foo);
}

function Sprite::foo(%this) {
  %this.y += 5;
  echo(%this@".foo!" SPC getSimTime());

}
//------------------------------
// TOTP (2FA):
// for testing script / platform: i copied my totp code from auteria :
function totP() {
    %skey =  "G26FPPD2YZZ2WHDG";
    echo ("CODE" SPC getTotpCode(%skey) SPC "VALIDATE:" SPC getTotpValidate(%skey,getTotpCode(%skey)));
}
//------------------------------
function bla() {
  $bla++;
  echo($bla);
}

//------------------------------
function help() {
  dumpConsoleFunctions();
  // dumpConsoleClasses();
}

function c(%p) {
  alxPlay(SndPling.getId());
  if (!%p)  %p = $sprite;
  if (!%p) return 0;
  // clone does NOT work with drawparams ?! %clone = %p.clone();
  %clone = new Sprite() {

      Texture = %p.Texture;
      w = %p.w;
      h = %p.h;

      imgId = getRandom(13);

      x = getRandom(1000) + %p.w;
      y = getRandom(400) + %p.h;
  };


  CleanupSet.add(%clone);


  // CloneSet.listObjects();
  echo("count:" SPC CloneSet.getCount() SPC "FPS:" SPC getFPS());

  return %clone;
 }
