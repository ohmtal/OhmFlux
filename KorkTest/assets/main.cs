// echo("Hello MAIN.CS");
//------------------------------
// ---- player test ------
if (!isObject($player)) {
  $player = new Player();
  echo("$player created");
}

function Player::bar(%this) {
  echo("getSimTime:" SPC getSimTime());
  %this.schedule(5000, foo);
}

function Player::foo(%this) {
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
