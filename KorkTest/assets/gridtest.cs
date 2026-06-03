function byteToHex(%value)
{
    %value = %value & 0xFF;
    %map = "0123456789ABCDEF"; // Uppercase variant

    %high = getSubStr(%map, (%value >> 4), 1);
    %low  = getSubStr(%map, (%value & 0xF), 1);

    return %high @ %low;
}





//FIXME for a real test i need to render a grid with weight, path, neighbour check and so on
$g = new Grid();
echo("INIT:" SPC $g.init("0 0 512 512", 32));// 32 =>16x16
echo("NODECOUNT " SPC $g.getNodeCount());
$g.getinfo();

//random mud
for( $i = 0; $i < 50; $i++ ) {
    $n = getRandom($g.getNodeCount() - 1);
    $w = getRandom(50,255);
    // echo("set NODE" SPC $n SPC "to" SPC byteToHex($w));
    $g.setWeightByNodeId($n, $w);
}

// draw grid
for ($i = 0; $i < 16; $i++) {
    $line = "";
    for ($j=0; $j<16; $j++) {
        $line = $line SPC byteToHex($g.getWeightByNodeId( $i * $j));
    }
    echo ($line);
}



// path test .......
$path = $g.findPath("16 16", "500 500", true);
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
