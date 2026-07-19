//------------------------------------------------------------------------------
// BALL
//------------------------------------------------------------------------------
// function Ball::onUpdate(%this,%fDt)
// {
//
//
//    %posAngle = %this.getTransform();
//    %pos = getWord(%posAngle,0) SPC getWord(%posAngle,1);
//    %angle = getWord(%posAngle,2);
//    %this.Velocity = VectorNormalize(%this.getLinearVelocity());
//    %this.Speed    = VectorLen(%this.getLinearVelocity());
//    /*  laaag ?! nöö */
//    $DEBUG::BallTransform =
//        "a:"@%this.getactive()
//        SPC "sp:"   @ tomRound(%this.speed,2)
//        SPC "pos:"  @ tomRoundWords(%posAngle,2)
//        SPC "velo:" @ tomRoundWords(%this.Velocity,3);
//
//    if (%this.getactive() == false)
//    {
//       %feelerSpeed = %this.player.feelerSpeed;
//       if (!%this.chooseDir)
//       {
//             if (!getRandom(1))
//                   %this.chooseDir = -1;
//             else
//                    %this.chooseDir = 1;
//       }
//       if (%this.chooseDir == 1)
//       {
//           if (%angle < -25)
//               %angle += %fDt * %feelerSpeed ;
//           else
//               %this.chooseDir = -1;
//
//       } else if (%this.chooseDir == -1) {
//           if (%angle > -155)
//               %angle -= %fDt * %feelerSpeed ;
//           else
//               %this.chooseDir = 1;
//       }
//
//       %this.setTransform(%pos SPC %angle);
//     } //active false
//     else {
//         // rare but can happen!!!
//         if (!%this.parent.rect.pointInRect(%pos))
//         {
//             %this.toSpawnPoint();
//             return;
//         }
//
//         //handle vertical or horizontal stuck
//         if (mRound(getWord(%this.Velocity,0)* 100) / 100 == 0) {
//
//             %xVelo=(getRandom(100) -50) / 100;
// //dError("BAD X VELOCITY OLD:" SPC %this.Velocity SPC  "NEWX" SPC %xVelo);
//             %this.ApplyLinearImpulseToCenter(%xVelo SPC "0", true);
//         }
//         else if (mRound(getWord(%this.Velocity,1)* 100) / 100 == 0) {
//
//             %xVelo=(getRandom(100) -50) / 100;
// //dError("BAD X VELOCITY OLD:" SPC %this.Velocity SPC  "NEWY" SPC %xVelo);
//             %this.ApplyLinearImpulseToCenter("0" SPC %xVelo, true);
//         }
//
//         //MINSPEED
//         if (%this.speed < %this.player.minSpeed) {
//         //dError("accelerate speed was: " SPC %this.speed);
//             %this.accelerate(0.1);
//         }
//         //MAXSPEED
//         else if (%this.speed > %this.player.maxSpeed) {
//         //dError("break speed was: " SPC %this.speed);
//             %this.accelerate(-0.1);
//         }
//
//
//     }
//
//
// }
//
// function Ball::onRender(%this,%dt)
// {
//
//   if (!%this.getactive())
//   {
//       %pointA = %this.x SPC %this.y;
//       %pointB = VectorAdd(%pointA, Vector2Scale(%this.getAxisVector(), %this.player.feelerLen));
//       %this.screen.primLine(%pointA,%pointB, "1 1 0 0.8");
//   }
//
// }
//------------------------------------------------------------------------------
function Ball::stop(%this)
{
   %this.SetLinearVelocity("0 0");
   %this.SetAngularVelocity("0");
   %this.setactive(false);
}
//------------------------------------------------------------------------------
function Ball::toSpawnPoint(%this)
{
   //hardcoded!!!

   dEcho("Ball to Spawnpoint" SPC %this.spawnPoint);
   %this.setTransform(%this.spawnPoint);
   %this.stop();   
}
//------------------------------------------------------------------------------
function Ball::accelerate(%this, %scale)
{
   %impulseVec = Vector2Scale(%this.Velocity,%scale);
   %this.ApplyLinearImpulseToCenter(%impulseVec,false);
}
//------------------------------------------------------------------------------
function Ball::imp(%this, %scale)
{
   if (%scale*1 == 0) %scale = 3;   
   %fV =  %this.getAxisVector();
   %fv = Vector2AddScalar(%fv, 0.001); // make sure its not 0 ?!
   %impulseVec = Vector2Scale(%fV,%scale);
   dEcho("launch: getAxisVector" SPC %fV SPC "impulse Vector:" SPC %impulseVec);
   %this.ApplyLinearImpulseToCenter(%impulseVec,false);
}
//------------------------------------------------------------------------------
function Ball::launch(%this)
{
   if (%this.getactive())
         return;
      
   %this.setactive(true);
   %this.imp();
   /*
   %fV =  %this.getAxisVector();
   %fv = Vector2AddScalar(%fv, 0.001); // make sure its not 0 ?!
   %impulseVec = Vector2Scale(%fV,%scale);
   dEcho("launch: getAxisVector" SPC %fV SPC "impulse Vector:" SPC %impulseVec);
   %this.ApplyLinearImpulseToCenter(%impulseVec,false);
   */
}
// //------------------------------------------------------------------------------
// function Ball::onCollision(%this,%obj, %normalX, %normalY, %posX, %posY)
// {
//    $DEBUG::LastBallCol = %this.getId() SPC "<=>" SPC %obj.getId()
//          SPC "normal:" SPC tomRoundWords(%normalX,3) SPC tomRound(%normalY,3)
//          SPC "pos:" SPC tomRound(%posX,3) SPC tomRound(%posY,3)
//    ;
//    if ( isObject(%obj) && %obj.getId() == %this.parent.player.paddle.getId())
//    {
// //nix!
//    }
//    else if (%obj.class $= "Brick") {
//          %obj.onBallCollide(%this);
//    }
//    else if (%obj.class $= "BottomWall") {
//        %this.player.ballOut();
//        //%this.schedule(20,toSpawnPoint);
//    }
// }
//
//
//
// function Ball::onCollisionEnd(%this,%obj)
// {
//
// }

//------------------------------------------------------------------------------
// Game class must have:
//  %this.world <<
function Game::CreateBall(%this, %position, %class, %radius )
{

 if (%class $= "") %class = "ball";
 if (%radius $= "") %radius = 10;


      
 %bodyDef = new BodyDef2b() {type = $box2d::b2_dynamicBody;};
 %bodyDef.setPosition(%position);
 %bodyDef.allowSleep=false; //sleeping is bad for balls

 // %bodyDef.gravityScale = 0; //0 = no gravity;
 %bodyDef.linearDamping=-0.05; //reduce the linear velocity :: NEGATIV 
 
 
 %ball = new Body2b()
 {
      class = %class;
      w  = %radius * 2;
      h  = %radius * 2;
      layer = 50;
      SendCollision = true;
      spawnPoint = %position;
 };
 %ball.CreateBody(%this.world,%bodyDef); 
 
 %shape = new Shape2b(); 
 %shape.shapeType = $box2d::CircleShape;  // $PolygonShape;
 %shape.setRadius(%radius);
 
 %fixtureDef = new FixtureDef2b();  
 %fixtureDef.setShape(%shape);
 
 
 // 1, 0.5, 1 bouncing ball with bit of roll
 // 1, 0.5, 0.5 rolling ball with bit of bounce
 %fixtureDef.density = 1; //1; //mass multi
 %fixtureDef.friction = 0.1;  //glue on collision
 %fixtureDef.restitution = 0.8; //bounce from collision
   
 %ball.CreateFixture(%fixtureDef);

 %VeloX = getRandom(20) - 10; 
 %ball.SetLinearVelocity(%VeloX SPC "-2");
 
 

 %ball.debugrender = %this.debugrender;
 %ball.visible = true;
 
 %ball.toSpawnPoint();
 
  // ~~~ cleanup ~~~
  %bodyDef.delete();
  %shape.delete();
  %fixtureDef.delete();

  return %ball;  

}
