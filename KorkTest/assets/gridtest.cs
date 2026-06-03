exec("assets/tools.cs"); //need absolute path! FIXME? nah

//FIXME for a real test i need to render a grid with weight, path, neighbour check and so on

$g = new Grid();
$start = "1 1"; // "0 0" is a problem on path ?!
$end   = "120 240";
echo("INIT:" SPC $g.init($start SPC $end, 8));
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
for ($i = 0; $i < $vert; $i++) {
    $line = "";
    for ($j=0; $j < $hor; $j++) {
        $line = $line SPC byteToHex($g.getWeightByNodeId( $i * $j));
    }
    echo ($line);
}



// path test .......
$path = $g.findPath($start, $end, true);
if (!isObject($path)) error("PATH FAILED FROM " SPC $start SPC "TO" SPC $end);
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

while (isObject($path)) walk($path);

//...... cleanup
$g.delete();
// $path.delete();
