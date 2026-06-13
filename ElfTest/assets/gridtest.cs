exec("./tools.cs"); 
// -----------------------------------------------------------------------------
// Demo cleanup
if (!isObject(CleanupSet)) new SimSet(CleanupSet);
else CleanupSet.deleteObjects();
// -----------------------------------------------------------------------------
function GridTest::Init(%this) {
    // Setup Grid
    %this.grid = new Grid();
    %x = 0; %y = 0;
    %w = 16; %h = 16;
    %this.grid.start = %x SPC %y;
    %this.grid.end  = %x + %w -1 SPC %y + %w - 1;
    %area  = %this.grid.start SPC %w SPC %h;
    %this.grid.squareSize = 1.0;
    echo("INIT:" SPC  %this.grid.init(%area, %this.grid.squareSize));
    %this.grid.getinfo();
    %this.add(%this.grid);


    %this.crazy  = false;
    %this.hideTerrain = false;
    %this.countX = %this.grid.getNodeCountY();
    %this.countY = %this.grid.getNodeCountX();
    %this.newTerrain();
}

function GridTest::newTerrain(%this) {
    %this.grid.addRandomMud();
    if ( isObject(%this.path) ) %this.path.delete();
    %this.path = %this.grid.findPath( %this.grid.start, %this.grid.end, false);
    if ( isObject(%this.path) ) %this.add( %this.path );
    // echo("PATH object is:" SPC %this.path);
}

function Grid::addRandomMud(%this) {
    %cnt = %this.getNodeCount();
    for( %i = 0; %i < %cnt / 1.5; %i++ ) {
        %n = getRandom(%cnt - 1);
        // %w = getRandom(50,250);
        // let think about 7 ground textures:
        // max is 224, 256 would be invalid btw
        // 255 would block completly 
        %w = getRandom(6) * 32 + 32;
        %this.setWeightByNodeId(%n, %w);
    }
   
}


function GridTest::onRender(%this,%dt) {


  %this.writeText(5,20, "Grid + Pathfinding Demo -" SPC getFPS() @ "fps",  $align::left, "0.5 0.2 0.5" );
   // %this.Line("5 35", "595 35", "0 1 0");

    if (!%this.hideTerrain) {
        %vert = %this.countX;
        %hor  = %this.countY;
        %idx = 0;
        %weight = 0;
        for (%i = 0; %i < %vert; %i++) {
            // %line = "";
            for (%j=0; %j < %hor; %j++) {
                %weight = %this.grid.getWeightByNodeId( %idx );
                // %this.writeText(65 + 50 * %j,%i*30 + 50,byteToHex(%weight), $align::left ,  %weight SPC "0.5 0.2");

                %this.rect(65 + 50 * %j SPC %i*30 + 40 SPC 35 SPC 28,   %weight SPC "0.5 0", true);

                %idx++;
            }
        }
    }

    // path
    %lastPoint = "";
    if ( isObject(%this.path) ) {
        %cnt = %this.path.getDynamicFieldCount();
        for (%i = 0; %i < %cnt; %i++) {
            %pos = %this.path.getFieldValue("node" @ %i);
            %realPos = getWord(%pos,0) * 50  + 85 SPC  getWord(%pos,1) * 30 + 55;
            if ( %lastPoint ) %this.Line(%lastPoint, %realPos  );
            %lastPoint = %realPos;

            // if ( %pos !$= "" ) {
            //     %this.writeText(getWord(%pos,0) * 50  + 52, getWord(%pos,1) * 30 + 50,"x", $align::left, "0 0 255" );
            // }
        }
    }
}

function GridTest::onUpdate(%this, %dt) {
    if (%this.crazy) %this.newTerrain();
}

function GridTest::onRemove(%this) {
    %this.deleteObjects();
}

// --------------- TEST update  all 32 ms --------
function GridTest::start(%this) {
    if (%this.crazy) return;
    %this.crazy = true;
    // %this.crazyLoop();
}
function GridTest::stop(%this) {
    %this.crazy = false;
}
// function GridTest::crazyLoop(%this) {
//     if (!%this.crazy) return;
//     %this.newTerrain();
//     %this.schedule(32, crazyLoop);
// }


// -------- --------- ----------
$game = new GameCtrl() { class = "GridTest"; };
$game.init();
CleanupSet.add($Game);

