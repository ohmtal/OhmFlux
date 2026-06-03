// --------- should be in engine but i'am testing TorqueScript ----------------

function byteToHex(%value)
{
    if (%value > 255 || %value < 0) return "?" @ %value;
    %value = %value & 0xFF;
    %map = "0123456789ABCDEF"; // Uppercase variant

    %high = getSubStr(%map, (%value >> 4), 1);
    %low  = getSubStr(%map, (%value & 0xF), 1);

    return %high @ %low;
}


function shiftColorHue(%color, %degreeShift)
{

    %r = getWord(%color, 0); %g = getWord(%color, 1); %b = getWord(%color, 2);
    if (%r > 1.0 || %g > 1.0 || %b > 1.0) { %r /= 255.0; %g /= 255.0; %b /= 255.0; }

    %max = (%r > %g) ? ((%r > %b) ? %r : %b) : ((%g > %b) ? %g : %b);
    %min = (%r < %g) ? ((%r < %b) ? %r : %b) : ((%g < %b) ? %g : %b);
    %delta = %max - %min;


    %v = %max;
    %s = (%max == 0) ? 0 : (%delta / %max);

    if (%delta == 0) %h = 0;
    else if (%max == %r) { %h = (%g - %b) / %delta; if (%h < 0) %h += 6; }
    else if (%max == %g) { %h = ((%b - %r) / %delta) + 2; }
    else                 { %h = ((%r - %g) / %delta) + 4; }
    %h *= 60;


    %h = (%h + %degreeShift) % 360;
    if (%h < 0) %h += 360;

    return hsvToRGB(%h, %s, %v);
}

function hsvToRGB(%h, %s, %v)
{
    // echo("hsvToRGB" SPC %h SPC  %s SPC  %v);
    if (%s == 0) return %v SPC %v SPC %v; // Graustufe

    %h /= 60;
    %i = mFloor(%h);
    %f = %h - %i;
    %p = %v * (1 - %s);
    %q = %v * (1 - (%s * %f));
    %t = %v * (1 - (%s * (1 - %f)));

    if (%i == 0)      return %v SPC %t SPC %p;
    else if (%i == 1) return %q SPC %v SPC %p;
    else if (%i == 2) return %p SPC %v SPC %t;
    else if (%i == 3) return %p SPC %q SPC %v;
    else if (%i == 4) return %t SPC %p SPC %v;
    else              return %v SPC %p SPC %q;
}
