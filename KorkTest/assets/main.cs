// echo("Hello MAIN.CS");
//------------------------------
echo ("------------------------------");
echo ("STEPS for simple but usable:");
echo ("[X] basic sprite. ");
echo ("[ ] Input Keyboard and Mouse");
echo ("[ ] write pong ...");
echo ("[ ] Add Sound - need my SoundTestBed finished!! or simply use the current.");
echo ("[ ] Add collision or better Box2D");
echo ("------------------------------");

//------------------------------
// ---- player test ------
if (!isObject($sprite)) {
  $sprite = new Sprite() {
    Texture = "assets/texture/faces.png";
    TexCols = 13;
    x = 100;
    y = 100;
    w = 64;
    h = 64;
    imgId = 3;
  };

  echo("$sprite created");
}

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
