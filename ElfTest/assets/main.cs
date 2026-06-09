// echo("Hello MAIN.CS");
include("./flyingText.cs"); // like a exec with no calls!
//------------------------------
echo ("------------------------------");
echo ("Porting from KorkTest to ElfTest");
echo ("[X] cleanup does not work! << it did?!");
echo ("[X] mouse does not work");
echo ("[X] cleanup on object (template) does not work. << onRemove is not called!!!!");
echo ("[X] flyingText does not work - fixed Point2I");
echo ("[X] include (noCalls) does not work. i actived it but now it does nowthing!! << fixed in compiledEval");
echo ("[X] add Box2D again: wild merge OGE+OGE3D+OhmFlux *haha*");
echo ("------------------------------");


//------------------------------

if (!isObject(CleanupSet)) new SimSet(CleanupSet);
else CleanupSet.deleteObjects();

//------------------------------


  $texBack = new Texture() {
    fileName = "assets/texture/nebulapurple_sky_back.png";

  };
  CleanupSet.add($texBack);

  $FontJetBrains = new Font() {
    fileName = "assets/font/JetBrainsMono-Regular.ttf";
    fontSize = 64.0;
  };
  CleanupSet.add($FontJetBrains);
  $Label1 = new Label()  {
    Font = $FontJetBrains;
    x = 20;
    y = 40;
    scale = 2.0;
    color = "0.8 0.8 0.1 1";
    Caption = "Hello World";
    shadowColor="0.1 0.1 0.1 1";
    shadowOffset = 0.9;
    shadow = true;
  };
  CleanupSet.add($Label1);

  $background = new Sprite() {
    Texture = $texBack;
    x = getScreenWidth() / 2.0;
    y = getScreenHeight() / 2.0;
    w = getScreenWidth();
    h = getScreenHeight();
    z = 1.0;
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
    z = 0.5;
    imgId = 3;
  };
  CleanupSet.add($sprite);
  // ------
  $sndPling = new AudioProfile(SndPling) { fileName = "assets/sound/pling.ogg"; Volume = 0.5; };
  CleanupSet.add($sndPling);

   $testWorld = new TestWorld(){
     x = 200;
     y = 100;
     w = 300;
     h = 300;
     // gravity = .0;
   };
   CleanupSet.add($testWorld);



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

 function test() {
    $foo = new ScriptObject();
    $bar = new ScriptObject();
    $foo.bar = $bar;
    $foo.bar.counter = 0; //OK
    $bar.counter++; //OK
    $foo.bar.counter = $foo.bar.counter + 1; //OK
    // $foo.bar.counter+=1; // cause parse error: token is opPLASN
    // $foo.bar.counter++; //cause parse error: token is opPLUSPLUS
    echo ("COUNTER IS:" SPC $foo.bar.counter SPC "/" SPC $bar.counter);
 }
//------------- try to find out why invaderGame crash ......

 // testing crash minimal setup
 // this was also before you latest changes!
 // crash at  StringStack::getArgcArgv << mNumFrames is 0 !
 function crash() {
    %player =  new ScriptObject() ;
    %player.FooBar(1,2,3,4,5,6,7,8,9,10); //OK
    %player.FooBar(1,2,3,4,5,6,7,8,9,10,11); //OK
    %player.FooBar(1,2,3,4,5,6,7,8,9,10,11,12); //OK
    %player.FooBar(1,2,3,4,5,6,7,8,9,10,11,12,13); //OK
    %player.FooBar(1,2,3,4,5,6,7,8,9,10,11,12,13,14); //OK
    echo ("HERE IT COMES!!!!!:");
    %player.FooBar(1,2,3,4,5,6,7,8,9,10,11,12,13,14,15); //crash
    // in my test project it crashed at the 10th parameter
 }

 // ParseError(
 function a() { echo ("a");totp(); $ashed = schedule(5000,0,"a"); }

 function b() { echo("b - a cancel"); cancel ($ashed);}


function realtime() {
	if ($time !$= "") echo("ellapsed:" SPC (GetRealTime()-$time) SPC "time:" SPC $time);
	$time = GetRealTime();
	schedule(1000,0,realtime);
}

schedule(1000,0,ft);
