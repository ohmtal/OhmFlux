/*
Flying Text 
 XXTH 2024-02-08
 based on the flying_text resouce on  garagegames.com

 2026-06-05 ported from auteria
    OK timinging seams to be wrong
    	It's much slower than original
    	Math ?!  OGE/Torque2D sin/con/tan is in degrees not radians!
    OK create fragments of textures
    OK cleanup does not work
    FIXME should textrues only once!


exec("common/client/flyingText.cs");


--------------------------------------------------------------------------------
//short 
$test = new ScriptGroup() {class = "FlyingText"; color = -1; text = "Hello World!\nMY FRIEND";};
~~~~
// Appearance Methods (appearanceMethod)
// ==================
// -1 - Random Method (any but 'None' method)
//  0 - None
//  1 - Rise Up
//  2 - Zoom from the left
//  3 - Drop Down
//  4 - Zoom from the right
//  5 - Diagonal to straight
//  6 - Four Corners
//  7 - Circle in from right
//
//
// Show Methods (showMethod) (bits set - more than one method can be selected)
// ============
// -1 - Random Method (any but 'None' method) 
//  0 - Steady
//  1 - Sin bounce
//  2 - Cos side-to-side
//  4 - Whole word sin bounce
//  8 - Whole word cos side-to-side


$test = new ScriptGroup() {
   class = "FlyingText";
   text = "Hello World!\nMY FRIEND";
   parent="PlayGui";
   //appearance
   appearance=-1; 
   appearanceTime = 600;
   //showing
   showMethod = -1;
   showTime   = 2000;
   showCycle  = 900;
   
   color = "0.2 0.5 1.0 0.9";
   fontPath = "common/ui/letters/default/"; // traling slash!! 
   charSpaceing = 0;

   onReset = ""; //callback
   deleteOnDone = true; //DEFAULT - else you can use it again
   startOnAdd   = true; //DEFAULT - else you can use .start
}; 
~~~~
with callback :

function newTestText() { $i++; $test = new ScriptGroup() {class = "FlyingText"; color = -1; text = "Hello World!\n" @ $i; onReset="newTestText();";};}
newTestText();

~~~~
with wrapper

FlyingText_wrapper("HELLO WORLD");


*/


//~~~~~~~~~~ DEFAULT GLOBALS :: can be overwritten after exec! ~~~~~~~~~~~~~~~~~
// COLORS: red green blue yellow
$flyingText::colorCount = 7;
$flyingText::color[0] = "1.0 0.0 0.0 1.0"; //red
$flyingText::color[1] = "0.0 1.0 0.0 1.0"; //green
$flyingText::color[2] = "0.0 0.0 1.0 1.0"; //blue
$flyingText::color[3] = "1.0 1.0 0.0 1.0"; //yellow
$flyingText::color[4] = "1.0 1.0 1.0 1.0"; //white
$flyingText::color[5] = "0.0 1.0 1.0 1.0"; //cyan
$flyingText::color[6] = "1.0 0.0 1.0 1.0"; //magenta

// NOTE: all files must be .png and lowercase !!!!!!!!!!!
$flyingText::LetterImages = "assets/texture/letters/default/";



function ft(%text) {

    // $flyingText::LetterImages = "assets/texture/letters/nice/";
    %result  = new ScriptGroup() {
        class = "FlyingText";
        color = %color;
        text = %text $= "" ? "Hello World" : %text;
        appearance = 1;
        showMethod = 1;
        showTime = 2000;
        color = "-2";

    };
    return %result;
}


//------------------------------------------------------------------------------
// CONST 
$PI = 3.14159265;
//------------------------------------------------------------------------------
//simple STATIC  wrapper  
function FlyingText_wrapper(%text, %color, %time)
{
   %result  = new ScriptGroup() {
      class = "FlyingText"; 
      color = %color; 
      text = %text;
      showTime = %time;
   };
   return %result;
}

//------------------------------------------------------------------------------
function FlyingText::AddToQueue(%this, %text, %color)
{  
   if (%text $= "")
      return false;
      
   if (%color $= "")
      %color = %this.color;   
      
   %obj = new SimObject() { text = %text; color = %color; };
   %this.add(%obj); 
   return true;
}
//------------------------------------------------------------------------------
function FlyingText::onAdd(%this)
{
   dEcho(" ~~~~~~ onAdd ~~~~~~");


   %this.parentWidth = getScreenWidth();
   %this.parentHeight =  getScreenHeight();

   // ... appearance ...   
   if (%this.appearance $= "")    
      %this.appearance = -1;
      
   if (%this.appearance < 0)
      %this.appearance = getRandom(6) + 1;
      
      
   if (%this.appearanceTime $= "") 
      %this.appearanceTime = 800;
      
   if (%this.appearanceTime < 1)
      %this.appearanceTime = 1;
      
   // ... method ...   
   %this.setMethod(%this.showMethod, %this.showTime, %this.showCycle);
   // ... font ...
   if (%this.fontPath $= "")
      %this.fontPath = $flyingText::LetterImages;

   if (%this.charSpaceing $= "")
      %this.charSpaceing = 0;
      
   // ... text ...   
   %this.numChars = 0;
   %this.setText(%this.text, %this.color);   
   
   if (%this.deleteOnDone $= "")
      %this.deleteOnDone = true;
      
   if (%this.startOnAdd $= "") 
      %this.startOnAdd = true;
      
   if (%this.startOnAdd) 
      %this.start();
      
   dEcho("FlyingText::onAdd::"
      @ "\n %this.appearance=" SPC %this.appearance
      @ "\n %this.appearanceTime=" SPC %this.appearanceTime
      @ "\n %this.showMethod=" SPC %this.showMethod
      @ "\n %this.showTime=" SPC %this.showTime
      @ "\n %this.showCycle=" SPC %this.showCycle
   );

}
//------------------------------------------------------------------------------
function FlyingText::onRemove(%this)
{
   %this.removeControls();
   %this.cleanup.delete();
   dEcho("FlyingText::onRemove(%this)");
}
//------------------------------------------------------------------------------
function FlyingText::setMethod(%this, %method, %time, %cycle)
{

   %this.showMethod = %method;
   
   if (%this.showMethod $= "")
      %this.showMethod = -1;
      
   if (%this.showMethod < 0)
   {
      %randVal = getRandom(3);
      switch (%randVal) 
      {
        case 0:
            %this.showMethod = 1;
        case 1:
            %this.showMethod = 2;
        case 2:
            %this.showMethod = 4;
        default:
            %this.showMethod = 8;
      }
   }

   %this._showMethod = %this.showMethod & 15;
   %this._AlwaysShow = %this.showMethod & 16;

   %this.showTime = %time;
   if (%this.showTime $= "")
      %this.showTime = 3000;
      
   %this.showCycle = %cycle;   
   if (%this.showCycle $= "") 
      %this.showCycle = 900;
      
   dEcho("FlyingText::setMethod(%this, " SPC %this.showMethod  SPC "(" @ %this._showMethod @ ")" SPC %this.showTime SPC  %this.showCycle);
      
}
//------------------------------------------------------------------------------
function FlyingText::setText(%this, %text, %color)
{
   dEcho("FlyingText::setText \n" SPC %text @ "\n" @  %color);
   if ( %this.numChars  > 0)
   {
      %this.removeControls();
   }

   if (!isObject(%this.cleanup)) %this.cleanup = new SimSet();
   %this.text = %text;
   %this.color = %color;
   
   %this.numLines = getRecordCount(%this.text);
   %fixedTextToShow = "";
   for (%i = 0; %i < %this.numLines; %i++)
   {
      %fieldText = getRecord(%this.text,%i);
      %fixedTextToShow = %fixedTextToShow @ %fieldText;
      %nextLineChar[%i] = strlen(%fixedTextToShow)-1;
   }
      
   // Some initial setup
   %this.numChars = strlen(%fixedTextToShow);
   %curLine = 0;
   %this.totalWidth[0] = 0;
   
   // Create a BitmapCtrl for each non-space character
   for (%i=0;%i<%this.numChars;%i++)
   {
      %curChar = getSubStr(%fixedTextToShow,%i,1);
      
      // Skip all the spaces
      if (%curChar !$= " ")
      {
         // Check for special characters
         if (%curChar $= "!")
            %curChar = "ep";
         else if (%curChar $= ".")
            %curChar = "period";
         else if (%curChar $= "?")
            %curChar = "question";
         else if (%curChar $= ":")
            %curChar = "colon";
         else if (%curChar $= "-")
            %curChar = "minus";
         else if (%curChar $= ",")
            %curChar = "comma";
         else if (%curChar $= "\"")
            %curChar = "doublequote";
         else if (%curChar $= "=")
            %curChar = "equal";
         else if (%curChar $= "+")
            %curChar = "plus";
         else if (%curChar $= "\'")
            %curChar = "quote";
         else if (%curChar $= ";")
            %curChar = "semicolon";
         else
            %curChar = strlwr(%curChar);

         // All our alphanumeric bitmaps are in the folder
         %bitmap = %this.fontPath @  %curChar;

         %texture = new Texture() {
             fileName = %bitmap @ ".png";
         };



         %this.control[%i] = new Sprite() {
             Texture = %texture;
             x = -100;
             y = 0;
             z = 0.5;
             w = 64;
             h = 64;
         };


         %this.cleanup.add( %this.control[%i] );
         %this.cleanup.add( %texture );
            
         
         if (%this.color) 
         {
            if (getWordCount(%this.color) == 4 ) //onecolor
            {
               %this.control[%i].color = %this.color;
               
            } else if (%this.color $= "-2") //all random
            {
               %c = getRandomF(0.2,1) SPC getRandomF(0.2,1) SPC getRandomF(0.2,1) SPC 1;
               %this.control[%i].color = %c; 
            } else if (%this.color $= "-1" && $flyingText::colorCount * 1 > 0) //random from list
            {
               %this.control[%i].color = 
                  $flyingText::color[getRandom($flyingText::colorCount - 1)];
            }
            //else it's invalid keep default color
         }

         %this.control[%i].w = getWord(%this.control[%i].Texture.getSize(),0) + %this.charSpaceing;
         %this.control[%i].h = getWord(%this.control[%i].Texture.getSize(),1);
         
         
         %width = %this.control[%i].w;
         
      }
      else
      {
         // Give spaces a width of 40 pixels
         %width = 40;
         %this.control[%i] = 0;
      }

      // Update our total width
      %this.totalWidth[%curLine] += %width;
      if (%i == %nextLineChar[%curLine])
      {
         %curLine++;
         %this.totalWidth[%curLine] = 0;
      }
   }

   // Set our beginning x location so our text is centered on the screen
   %x = (%this.parentWidth / 2) - (%this.totalWidth[0] / 2);
   %y = (%this.parentHeight / 2) - 80 - (%this.numLines-1) * 60;

   // dError("parent size" SPC %this.parentWidth SPC %this.parentHeight);

   %curLine = 0;

   // Now that we know the total width we can set each chars final position
   for (%i=0;%i<%this.numChars;%i++)
   {
      if (%this.control[%i])
      {
         // PuPt2 is the final ending position
         %this.PuPt2x[%i] = %x;
         %this.PuPt2y[%i] = %y;

         // The starting position will depend upon our appearance method
         %this.setPuStartPos(%i);

         // Width of this bitmap
         %width = %this.control[%i].w;// getWord(%this.control[%i].extent,0);
      }
      else
      {
         // Spaces have a width of 40
         %width = 40;
      }

      // Set where our next character will be positioned
      %x += %width;
      if (%i == %nextLineChar[%curLine])
      {
         %curLine++;
         %x = (%this.parentWidth / 2) - (%this.totalWidth[%curLine] / 2);
         %y += 120;
      }
   }
}
//------------------------------------------------------------------------------
function FlyingText::start(%this)
{
   %this.startTime =  GetRealTime();  
   dEcho("~~~~~~~~~~~~~ START ~~~~~ Time:" SPC %this.starttime);

   // Schedule the removal of this message
   %this.removeSchedule = %this.schedule( %this.appearanceTime + %this.showTime
      , "reset");
   
   // Set all the chars of the message to their starting positions
   %this.updatePopupMessage();
}   
//------------------------------------------------------------------------------
function FlyingText::stop(%this)
{
   cancel(%this.removeSchedule);
   %this.removeSchedule = 0;
   cancel(%this.updateSchedule);
   %this.removeSchedule = 0;
}
//------------------------------------------------------------------------------
function FlyingText::removeControls(%this)
{
    if (isObject(%this.cleanup)) %this.cleanup.deleteObjects();

}
//------------------------------------------------------------------------------
function FlyingText::reset(%this, %ignoreDelete)
{
   dWarn("~~~~~ RESET - ignore:" SPC %ignoreDelete);
   %this.stop();
   %this.removeControls();
   
   //callback
   if (%this.onReset !$= "")
      eval(%this.onReset);
         
   //queue
   if (%this.getCount() > 0)
   {
      %text = %this.getObject(0).text;
      %color = %this.getObject(0).color;
      %this.getObject(0).delete();
      %this.setText(%text, %color);
      %this.start();
      return;
   }

   if (!%ignoreDelete && %this.deleteOnDone) {
      %this.schedule(0,"delete");
   }

      
}
//------------------------------------------------------------------------------
function FlyingText::updatePopupMessage(%this)
{
   %diffTime = GetRealTime() - %this.startTime;
   %appearancePercent = %diffTime / %this.appearanceTime;
   %showPercent = %diffTime / %this.showCycle;

   for (%i=0;%i<%this.numChars;%i++)
   {
      if (%this.control[%i])
      {
         %pos = %this.computePopupPosition(%i,%appearancePercent,%showPercent);
         %this.control[%i].x = getWord(%pos,0);
         %this.control[%i].y = getWord(%pos,1);
      }
   }
   %this.updateSchedule = %this.schedule( 16, "updatePopupMessage");
}
//------------------------------------------------------------------------------
function FlyingText::computePopupPosition( %this, %i, %percent, %percentShow )
{
   // Special case for the Circle in method
   if (%this.appearance == 7)
   {
      %percent -= %i * 0.04;
      if (%percent < 0)
         %percent = 0;
   }

   // Get our show adjust - %percent will always be set to 1.0 or
   // below after doing this code fragment
   if (%this._AlwaysShow & 16)
   {
      %adjust = %this.computePopupShowAdjust( %i, %percentShow, 1);
      if (%percent > 1)
         %percent = 1;
   }
   else
   {
      if (%percent > 1)
      {
         if (%percent > 1.2)
            %percentToUse = 1;
         else
            %percentToUse = (%percent - 1) / 0.2;

         %adjust = %this.computePopupShowAdjust( %i, %percentShow, %percentToUse);
         %percent = 1;
      }
      else
      {
         %adjust = 0;
      }
   }

   // Break out the x and y components of the adjust value
   %adjustX = getWord(%adjust,0);
   %adjustY = getWord(%adjust,1);

   // Special case for Circle in method
   if (%this.appearance == 7)
   {
      if (%percent <= 0.25)
      {
         // Adjust percent so it covers the range of 0.0 to 0.25
         %percent /= 0.25;

         // Get the interpolated position between pt1 and ptMid
         %workPtX = %this.interpolatePopup(%percent,%this.PuPt1x[%i],%this.PuPtMidx[%i]);
         %workPtY = %this.interpolatePopup(%percent,%this.PuPt1y[%i],%this.PuPtMidy[%i]);
      }
      else if (%percent <= 0.9)
      {
         // Adjust percent so it covers the range of 0.25 to 0.9
         %percent = (%percent - 0.25) / 0.65;
         %piVal = %percent * $PI * 2;
         %sinVal = mSin(%piVal);
         %cosVal = mCos(%piVal);

         // Get our position around the circle
         %workPtX = %this.parentWidth/2 - %sinVal * %this.PuPtMidy[%i];
         %workPtY = %this.parentHeight/2 - %cosVal * %this.PuPtMidy[%i];
      }
      else
      {
         // Adjust percent so it covers the range of 0.9 to 1.0
         %percent = (%percent - 0.9) / 0.1;

         // Get the interpolated position between ptMid and pt2
         %workPtX = %this.interpolatePopup(%percent,%this.PuPtMidx[%i],%this.PuPt2x[%i]);
         %workPtY = %this.interpolatePopup(%percent,%this.PuPtMidy[%i],%this.PuPt2y[%i]);
      }
   }
   else
   {
      // Get the interpolated position between pt1 and pt2
      %workPtX = %this.interpolatePopup(%percent,%this.PuPt1x[%i],%this.PuPt2x[%i]);
      %workPtY = %this.interpolatePopup(%percent,%this.PuPt1y[%i],%this.PuPt2y[%i]);
   }

   %workPtX = %workPtX+%adjustX;
   %workPtY = %workPtY+%adjustY;

   return (%workPtX SPC %workPtY);
}

//------------------------------------------------------------------------------
function FlyingText::setPuStartPos( %this, %i )
{
   // Set our starting position based upon our appearance method
   if (%this.appearance == 0)
   {
      %this.PuPt1x[%i] = %this.PuPt2x[%i];
      %this.PuPt1y[%i] = %this.PuPt2y[%i];
   }
   else if (%this.appearance == 1)
   {
      %this.PuPt1x[%i] = %this.PuPt2x[%i];
      %this.PuPt1y[%i] = %this.parentHeight + (%i * 80);
   }
   else if (%this.appearance == 2)
   {
      //there is not totalwidth %pos = %this.PuPt2x[%i] - (%this.parentWidth/2) - (%this.totalWidth/2) - 40;
      %pos = %this.PuPt2x[%i] - (%this.parentWidth/2) - 40;
      %this.PuPt1x[%i] = %pos * 3;
      %this.PuPt1y[%i] = %this.PuPt2y[%i];
   }
   else if (%this.appearance == 3)
   {
      %this.PuPt1x[%i] = %this.PuPt2x[%i];
      %this.PuPt1y[%i] = 0 - (%i * 80) - 80;
   }
   else if (%this.appearance == 4)
   {
      //there is not totalwidth %pos = %this.PuPt2x[%i] - ((%this.parentWidth / 2) - (%this.totalWidth / 2));
      %pos = %this.PuPt2x[%i] - (%this.parentWidth / 2);
      %this.PuPt1x[%i] = %this.parentWidth + %pos * 3;
      %this.PuPt1y[%i] = %this.PuPt2y[%i];
   }
   else if (%this.appearance == 5)
   {
      %this.PuPt1x[%i] = (%this.parentWidth/2) - ((%this.parentWidth / 2) - %this.PuPt2x[%i]) * 2;
      %this.PuPt1y[%i] = %this.parentHeight - (%i * 80) - 80;
   }
   else if (%this.appearance == 6)
   {
      switch (%i % 8)
      {
         case 0:
            %this.PuPt1x[%i] = %this.parentWidth;
            %this.PuPt1y[%i] = %this.parentHeight - %this.control[%i].h; //getWord(%this.control[%i].extent,1);

         case 1:
            %this.PuPt1x[%i] = %this.parentWidth - %this.control[%i].w; //getWord(%this.control[%i].extent,0);
            %this.PuPt1y[%i] = %this.parentHeight;

         case 2:
            %this.PuPt1x[%i] = %this.parentWidth;
            %this.PuPt1y[%i] = 0;

         case 3:
            %this.PuPt1x[%i] = %this.parentWidth - %this.control[%i].w; //getWord(%this.control[%i].extent,0);
            %this.PuPt1y[%i] = 0 - %this.control[%i].h; //getWord(%this.control[%i].extent,1);

         case 4:
            %this.PuPt1x[%i] = 0;
            %this.PuPt1y[%i] = %this.parentHeight;

         case 5:
            %this.PuPt1x[%i] = 0 - %this.control[%i].w; //getWord(%this.control[%i].extent,0);
            %this.PuPt1y[%i] = %this.parentHeight - %this.control[%i].h; //getWord(%this.control[%i].extent,1);

         case 6:
            %this.PuPt1x[%i] = 0;
            %this.PuPt1y[%i] = 0 - %this.control[%i].h; //getWord(%this.control[%i].extent,1);

         case 7:
            %this.PuPt1x[%i] = 0 - %this.control[%i].w; //getWord(%this.control[%i].extent,0);
            %this.PuPt1y[%i] = 0;
      }
   }
   else if (%this.appearance == 7)
   {
      %this.PuPt1x[%i] = %this.parentWidth;
      %this.PuPt1y[%i] = %this.parentHeight/4;
      %this.PuPtMidx[%i] = %this.parentWidth/2;
      %this.PuPtMidy[%i] = %this.PuPt1y[%i];
   }
}


//------------------------------------------------------------------------------
function FlyingText::interpolatePopup( %this, %percentage, %pt1, %pt2)
{
    %result = %pt1 + (%pt2 - %pt1) * %percentage;

    return %result;
}



//------------------------------------------------------------------------------
function FlyingText::computePopupShowAdjust( %this, %i, %percent, %percentToUse )
{
   // Default to adjusting neither 'x' nor 'y'
   %adjustX = 0;
   %adjustY = 0;

   if (%this._showMethod  & 3)
   {
      // Modify percent by char position
      %percent1 = %percent + (%i/10);

      if (%this._showMethod  & 1)
      {
         %offset = 12 * mSin($PI * 2 * %percent1);
         %adjustY = %offset * %percentToUse;
      }

      if (%this._showMethod  & 2)
      {
         %offset = 12 * mCos($PI * 2 * %percent1);
         %adjustX = %offset * %percentToUse;
      }
   }

   if (%this._showMethod  & 4)
   {
      %offset = 12 * mSin($PI * 2 * %percent);
      %adjustY += %offset * %percentToUse;
   }

   if (%this._showMethod  & 8)
   {
      %offset = 12 * mCos($PI * 2 * %percent);
      %adjustX += %offset * %percentToUse;
   }

   return (%adjustX SPC %adjustY);
}





