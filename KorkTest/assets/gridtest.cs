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
    %this.grid.addRandomMud();

    %this.path = %this.grid.findPath( %this.grid.start, %this.grid.end, true);
    if ( isObject(%this.path) ) %this.add( %this.path );
}


function Grid::addRandomMud(%this) {
    %cnt = %this.getNodeCount();
    for( %i = 0; %i < %cnt / 1.5; %i++ ) {
        %n = getRandom(%cnt - 1);
        %w = getRandom(50,255);
        %this.setWeightByNodeId(%n, %w);
    }
}


function GridTest::onRender(%this,%dt) {
	%this.writeText(5,20, "Grid + Pathfinding Demo -" SPC getFPS() @ "fps",  $align::left, "0.5 0.2 0.5" );
    %vert = %this.grid.getNodeCountY();
    %hor  = %this.grid.getNodeCountX();
    // echo("GRID IS:" SPC %hor SPC "x" SPC %vert );
    %idx = 0;
    for (%i = 0; %i < %vert; %i++) {
        %line = "";
        for (%j=0; %j < %hor; %j++) {
        	
            // %line = %line SPC byteToHex(%this.grid.getWeightByNodeId( %idx ));
            %weight = %this.grid.getWeightByNodeId( %idx );
            %this.writeText(65 + 50 * %j,%i*30 + 50,byteToHex(%weight), $align::left , %weight SPC "64 64");
            %idx++;
        }
        // echo (%line);
        // %this.writeText(50,%i*30 + 50,%line, $align::left );
    }
    // path
    if ( isObject(%this.path) ) {
        %cnt = %this.path.getDynamicFieldCount();
        for (%i = 0; %i < %cnt; %i++) {
            %pos = %this.path.getFieldValue("node" @ %i);
            if ( %pos !$= "" ) {
                //FIXME calc pos to text position
                %this.writeText(getWord(%pos,0) * 50  + 52, getWord(%pos,1) * 30 + 50,"x", $align::left, "0 0 255" );
            }
        }
    }
}



function GridTest::onRemove(%this) {
    %this.deleteObjects();
}



// -------- --------- ----------
$game = new GameCtrl() { class = "GridTest"; };
$game.init();
CleanupSet.add($Game);



// -----------------------------------------------------------------------------
//FIXME for a real test i need to render a grid with weight, path, neighbour check and so on
function walk(%path)
{
    if (!isObject(%path))
        return false;

    %posId = %path.pos;
    if (%posId $= "") %posId = 0; //1
    %posName = "node" @ %posId;

    %dest = %path.getFieldValue(%posName);

    if (%dest $= "") //done!
    {
        %path.delete();
        return true;
    }

    echo("**WALK TO " SPC %dest);
    %path.pos = %posId+1;


    return true;
}
function TestGrid() {
    $g = new Grid();
    %x = 0; %y = 0;
    %w = 9; %h = 9;
    $start = %x SPC %y;
    $end   = %x + %w -1 SPC %y + %w - 1;
    $size  = %w SPC %h;
    echo("INIT:" SPC $g.init($start SPC $size, 1));
    echo("NODECOUNT " SPC $g.getNodeCount());
    $g.getinfo();

    //random mud
    for( $i = 0; $i < $g.getNodeCount() / 3; $i++ ) {
        $n = getRandom($g.getNodeCount() - 1);
        $w = getRandom(50,255);
        // echo("set NODE" SPC $n SPC "to" SPC byteToHex($w));
        $g.setWeightByNodeId($n, $w);
    }

    // draw grid ...
    $vert = $g.getNodeCountY();
    $hor  = $g.getNodeCountX();
    echo("GRID IS:" SPC $hor SPC "x" SPC $vert );
    $idx = 0;
    for ($i = 0; $i < $vert; $i++) {
        $line = "";
        for ($j=0; $j < $hor; $j++) {
            $line = $line SPC byteToHex($g.getWeightByNodeId( $idx ));
            $idx++;
        }
        echo ($line);
    }

    // path test .......
    $path = $g.findPath($start, $end, true);
    if (!isObject($path)) error("PATH FAILED FROM " SPC $start SPC "TO" SPC $end);


    while (isObject($path)) walk($path);

    //...... cleanup
    $g.delete();
}

 // TestGrid();
 echo("HELLO EDITOR!");




