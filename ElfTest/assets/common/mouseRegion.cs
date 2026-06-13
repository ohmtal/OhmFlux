// FIXME untested on ElfFlux!!!

//-----------------------------------------------------------------------------
// MouseRegion 
//-----------------------------------------------------------------------------
function MouseRegion_createRegions(%GameObj) { //STATIC!
  if (isObject(%GameObj.mouseRegions))
        %GameObj.mouseRegions.delete();
        
  %GameObj.mouseRegions = new SimGroup();
  %GameObj.mouseRegions.mouseOverObject = 0;
}

function MouseRegion_create(%GameObj,%x,%y,%w,%h, %lCallBack, %rCallBack, %mouseEnterCallback, %mouseLeaveCallback) { //STATIC
  
 %result = new ScriptObject() {
   class = MouseRegion;
   x = %x;
   y = %y; 
   w = %w;
   h = %h;
   down = 0;
   active = true;
   lCallback = %lCallback;
   rCallback = %rCallback;
   meCallback = %mouseEnterCallback;
   mlCallback = %mouseLeaveCallback;
   game = %GameObj;
 };
  
 return (%result);
}

function MouseRegion::onClick(%this, %button) 
{
  //dError("MouseRegion ONCLICK btn=" SPC %button);
   if (%button == 0)
        eval(%this.lCallBack);
   else if (%button == 1)
        eval(%this.rCallBack);
}

function MouseRegion::onMouseEnter(%this) 
{

 // dError("MouseRegion onMouseEnter obj=" SPC %this);
  if ( %this.meCallback !$= "")
        eval(%this.meCallBack);
}

function MouseRegion::onMouseLeave(%this) 
{
  // dError("MouseRegion onMouseLeave obj=" SPC %this);
  if ( %this.mlCallback !$= "")
        eval(%this.mlCallBack);
}



function MouseRegion::ProcessMouseAction(%GameObj, %button, %mouseX,%mouseY, %isDownEvent) //STATIC
{
    if (%button $= "button0"  )
    {
       if (%isDownEvent) 
                MouseRegion::onMouseDown(%GameObj,0,%mouseX,%mouseY);
       else
                MouseRegion::onMouseUp(%GameObj,0,%mouseX,%mouseY);
    } else if (%button $= "button1" ) {
       if (%isDownEvent) 
                MouseRegion::onMouseDown(%GameObj,1,%mouseX,%mouseY);
       else
                MouseRegion::onMouseUp(%GameObj,1,%mouseX,%mouseY);
    }

}


// *** STATIC ****
function MouseRegion::onMouseDown(%GameObj,%button,%x,%y) {
  %cnt = %GameObj.mouseRegions.getCount();
  for (%i=0; %i<%cnt; %i++) {
     %m = %GameObj.mouseRegions.getObject(%i);
     if (%m.active && PointInCenterRect(%x,%y , %m.x,%m.y,%m.w,%m.h)) {
        %m.down[%button]=1;
        return;
     }
  }
}

function MouseRegion::onMouseUp(%GameObj,%button,%x,%y) {

  %cnt = %GameObj.mouseRegions.getCount();
  for (%i=0; %i<%cnt; %i++) {
     %m = %GameObj.mouseRegions.getObject(%i);
     if (%m.down[%button])
     {
        if (%m.active &&  PointInCenterRect(%x,%y , %m.x,%m.y,%m.w,%m.h)) 
        {
            %m.down[%button]=0;
            %m.onClick(%button);
            // return;
        }
        if (!%m.locked)
               %m.down[%button] = 0;
    }
  }
}
//-----------------------------------------------------------------------------
// Mouse MOVE - different parameters than in usual event!
function MouseRegion::ProcessMouseMove(%GameObj, %x, %y)
{
 %cnt = %GameObj.mouseRegions.getCount();
 
 %foundObj = 0;
 
 
 
  for (%i=0; %i<%cnt; %i++) {
     %m = %GameObj.mouseRegions.getObject(%i);
     if (PointInCenterRect(%x,%y , %m.x,%m.y,%m.w,%m.h))
     {
        //found a object no action if its the current mouse over object 
        //else we send a leave/enter
        if (%m != %GameObj.mouseRegions.mouseOverObject )
        {
           if (isObject(%GameObj.mouseRegions.mouseOverObject)) 
                %GameObj.mouseRegions.mouseOverObject.onMouseLeave();
           %m.onMouseEnter();
           %GameObj.mouseRegions.mouseOverObject = %m;
           return;
        } else {
           return;
        }
     }
  } //for
  // we found no current object, if we have an curobject we leave here
  if (isObject(%GameObj.mouseRegions.mouseOverObject))
  {
        %GameObj.mouseRegions.mouseOverObject.onMouseLeave();
        %GameObj.mouseRegions.mouseOverObject = 0;
  }
}
//-----------------------------------------------------------------------------

