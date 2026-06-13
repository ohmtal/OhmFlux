
//------------------------------
GarbageCollectionSet.deleteObjects();
//------------------------------

function MemTest::onRender(%this,%dt) {
	%red = 128;
	%this.writeText(20, 49,"Testing memory. Never do something like that in production ;)"
	     , $align::left 
	     ,  %red SPC "0.2 0.2"
	);
    for (%i = 0; %i < 100; %i++){
    	 %bla = 1; // no mem raise
    	 $foo++; // no mem raise
         setFoo( $foo * 1 ); // no mem raise

         // no mem lost so far ....

         //NOTE after the _STK clean macro
         //NOTE after ConsoleValue::resetConversionBuffer();

         // only this =>  nothing so far
         %this.fooVal = getFoo();

         // only this =>  nothing so far
         %foo = getFoo();

         // only this =>  nothing so far
    	  $foo = getFoo();

          // only this =>  only this still nothing
        //!!!!!! when i set $foo = 1000000000; in console
        // it start eating memory !!!!!!!!!!!!!!!
         // now 6MB / sec ?! .....
          getFooToGlobal("$foo");
    } 

}

function MemTest::onUpdate(%this,%dt) {

}

$Game = new GameCtrl() {
    class = "MemTest";
};
