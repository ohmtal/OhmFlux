echo("Hello from Test.cs");
//------------------------------
// NOTE KorkTest made on my i5 notebook running on battery
if (!isObject(CleanupSet)) new SimSet(CleanupSet);
else CleanupSet.deleteObjects();

new GameCtrl(TEST) {};
CleanupSet.add($Game);
setVSync(false);
//------------------------------

function rl(%mode) {
    exec("./test.cs");

    if (%mode !$= "") {
        $mode = %mode;
        $i = $times = 0;
    }
}


// benchmark
$foo = new TestObj() {
     bar = 0.0;
};

// $foo.dump();

function setMy() {
    $foo.myFloat = 2.2;
}



$mode = -1;

function SpeedTest() {
    %lF32 = 0.0;
    %lS32 = 0;

    //  --- count up a member without TYPEDEF ---
    // .. object as global var
    // KorkTest 200ms
    if ( $mode == 0 ) {
        for (%i =0 ; %i < 50000; %i++) {
            // $foo.bar += 0.1;
        }
    }
    else
    // .. object as local var
    // KorkTest 900ms
    if ( $mode == 1 ) {
        %foo = $foo;
        for (%i =0 ; %i < 50000; %i++) {
            %foo.bar += 0.1;
        }
    }
    else
    // .. object as local var + typed
    // KorkTest 1400ms

    if ( $mode == 2 ) {
        %foo = $foo;
        for (%i =0 ; %i < 50000; %i++) {
            //NOTE Why is + - slower than * / ??

            // %foo.myFloat = 0.1; // 65ms
            // %foo.myFloat += 0.1; // 90ms
            // %foo.myFloat *= 0.1; //70ms
            // %foo.myFloat /= 0.1; //70ms
            // %foo.myFloat -= 0.1; //90ms
            %foo.myFloat =  %foo.myFloat * 0.1; //86ms
        }
    }
    // ------ working on engine defined properties --------
    else
    // .. object as local var + set X
    // NOTE: OMG is this slow !!!!!!!

    // KorkTest 1500ms with parsing errors from time to time

    if ( $mode == 3 ) {
        %foo = $foo;
        for (%i =0 ; %i < 50000; %i++) {
            // THIS MUST BE A BUG !!! .x is slow like hell while myFloat is ok
            // -- x,y,z,w,r,g,b,a are defined in parser ---
            // 600 ms for this .... omg
            // %foo.x += 0.1; //NOTE VERY VERY SLOW

            // BUG testFloat is 600ms!!!
            // so its not the .x
            // %foo.testFloat *=   0.1;

            //NOTE Verified its not my ELFSCRIPT_STRICT_SLOT_TYPE

            // %testFloat = %foo.testFloat; //alone 60ms
            // %testFloat *= 0.1;
            // // !!!!!!!!!!!!1 THIS IT IS !!!!!!!!!!!!!!!!!!!!
            // %foo.testFloat = 4.2;

           // all three 87
            %foo.testFloat += 0.1; // 67 ms now !!!

            // %bar = %foo.testFloat * 1.001;
            // %foo.testFloat = %bar;
            //
            // if (%bar > 10000.0) %foo.testFloat = 0.0;
           //if ($i % 3 == 0)  echo(%foo.testFloat);

        }
    }
    else
    // KorkTest NOT IMPLEMENTED

    // .. object as local var + call walk
    if ( $mode == 4 ) {
        %foo = $foo;
        for (%i =0 ; %i < 50000; %i++) {
            %foo.walk(0.1, 0.4, 0.2); //NOTE FAST! 50-60ms
        }
    }
    // .. object as local var + call getPos to a local var
    // KorkTest not implemented

    if ( $mode == 5 ) {
        %foo = $foo;
        for (%i =0 ; %i < 50000; %i++) {
            %posVec = %foo.getPos(); //not bad 111ms
        }
    }
     // .. object as local var + call getx/y/z to a local vars
     // KorkTest 1400ms

    if ( $mode == 6 ) {
        %foo = $foo;
        for (%i =0 ; %i < 50000; %i++) {
            // NOTE a bit slower than getPos: 122ms
            %x = %foo.getX();
            %foo.testFloat = %x;
            // %y = %foo.getY();
            // %z = %foo.getZ();
        }
    }
    // KorkTest 2 ms :)
    if ($mode == 7) {
        $foo.testFloat += 0.1415  ;
        %a = $foo.testFloat;
        %a *= 1.001;
        $foo.testFloat = %a;
        // echo($foo.x);
    }
    // KorkTest 1300ms
    if ($mode == 8) {
        // $foo.testInt++;
        for (%i =0 ; %i < 50000; %i++) {
             // $foo.testInt += 1;
             $foo.testInt = 1;
             // $foo.testBool = 0;
             // $foo.testString = "HUHU";
             // %int = $foo.testInt;
             // $foo.testSlot[1] ++;
        }
    }
}


$i = 0; $times = 0;
// called from c++ mainloop
function TEST::onRender(%this,%dt) {

    if ($mode >= 0) {
        $i++;
        $times += GetFrameTime();
        if ($i >= 10) {
            $i -= 10;
            echo("Speedtest MODE:" SPC $mode SPC "avg time:" SPC $times * 100 SPC "testfloat/int/bool:" SPC $foo.testFloat SPC $foo.testInt SPC $foo.testBool);
            $times = 0;
        }
    }
    SpeedTest();

}
