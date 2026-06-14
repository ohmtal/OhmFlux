
function Game::LoadAssets(%this) {

}

function Game::onInputEvent( %this, %deviceString, %actionString, %mouseX, %mouseY, %keyValue ) {

}

function Game::onUpdate(%this,%dt) {

}

function Game::onRemove(%this) {
  // %this.deleteObjects();
}



//------------------------------
// Main
//------------------------------
GarbageCollectionSet.deleteObjects();
//------------------------------
$Game = new GameCtrl() {
  class = "Game";
};

$Game.LoadAssets();


