
//------------------------------

if (!isObject(CleanupSet)) new SimSet(CleanupSet);
else CleanupSet.deleteObjects();

//------------------------------
function testMe() {
	echo("testME!");
}
echo("empty loaded....");
