#include "core/fluxGlobals.h"
#include "core/Globals.h"
#include <console/console.h>
#include "ConsoleTypes.h"
#include <core/strings/stringUnit.h>

// -----------------------------------------------------------------------------
// TypeColorF
// -----------------------------------------------------------------------------
IMPLEMENT_STRUCT( Color4F,
   Color4F,,
   "RGBA color quadruple in 32bit floating-point precision per channel." )

   FIELD( r, red, 1, "Red channel value." )
   FIELD( g, green, 1, "Green channel value." )
   FIELD( b, blue, 1, "Blue channel value." )
   FIELD( a, alpha, 1, "Alpha channel value." )

END_IMPLEMENT_STRUCT;

ConsoleType(Color4F, TypeColorF, Color4F, "")
ImplementConsoleTypeCasters( TypeColorF, Color4F )

ConsoleGetType( TypeColorF )
{
   // Fetch color.
   const Color4F* color = (Color4F*)dptr;

   // // Fetch stock color name.
   // StringTableEntry colorName = StringTable->insert("fixme color name"); //StockColor::name( *color );
   //
   // // Write as color name if was found.
   // if ( colorName != StringTable->EmptyString() )
   //    return colorName;

   // Format as color components.
   static const U32 bufSize = 256;
   char* returnBuffer = Con::getReturnBuffer(bufSize);
   dSprintf(returnBuffer, bufSize, "%g %g %g %g", color->r, color->g, color->b, color->a);
   return(returnBuffer);
}

ConsoleSetType( TypeColorF )
{
   Color4F *tmpColor = (Color4F *) dptr;
   if(argc == 1)
   {
      // Is only a single argument passed? should be hex value!
      if ( StringUnit::getUnitCount( argv[0], " " ) == 1 )
      {
         tmpColor->FromHex( dAtoi(argv[0]) );

         return;
      }

      tmpColor->FromHex(0x000000FF);
      F32 r,g,b,a;
      S32 args = dSscanf(argv[0], "%g %g %g %g", &r, &g, &b, &a);
      tmpColor->r = r;
      tmpColor->g = g;
      tmpColor->b = b;
      if (args == 4)
         tmpColor->a = a;
   }
   else if(argc == 3)
   {
      tmpColor->r    = dAtof(argv[0]);
      tmpColor->g  = dAtof(argv[1]);
      tmpColor->b   = dAtof(argv[2]);
      tmpColor->a  = 1.f;
   }
   else if(argc == 4)
   {
      tmpColor->r    = dAtof(argv[0]);
      tmpColor->g  = dAtof(argv[1]);
      tmpColor->b   = dAtof(argv[2]);
      tmpColor->a  = dAtof(argv[3]);
   }
   else
      Con::printf("Color must be set as { r, g, b [,a] }, { r g b [b] } or { hexValue  }");

    // it's float so i use it as float !
    // if (tmpColor->r > 1.f) tmpColor->r /= 255.f;
    // if (tmpColor->g > 1.f) tmpColor->g /= 255.f;
    // if (tmpColor->b > 1.f) tmpColor->b /= 255.f;
    // if (tmpColor->a > 1.f) tmpColor->a /= 255.f;
}
// -----------------------------------------------------------------------------
// TypePoint2I
// -----------------------------------------------------------------------------
IMPLEMENT_STRUCT( Point2I,
                  Point2I,,
                  "2 point integer" )

FIELD( x, y, 1, "x" )
FIELD( y, y, 1, "y" )

END_IMPLEMENT_STRUCT;

ConsoleType(Point2I, TypePoint2I, Point2I, "")
ImplementConsoleTypeCasters( TypePoint2I, Point2I )

ConsoleGetType( TypePoint2I )
{
    // Fetch .
    const Point2I* p = (Point2I*)dptr;

    // Format as color components.
    static const U32 bufSize = 256;
    char* returnBuffer = Con::getReturnBuffer(bufSize);
    dSprintf(returnBuffer, bufSize, "%d %d", p->x,p->y);
    return(returnBuffer);
}

ConsoleSetType( TypePoint2I )
{
    Point2I *p = (Point2I*)dptr;

    if(argc == 1)
    {

        S32 x,y;
        S32 args = dSscanf(argv[0], "%d %d", &x, &y);
        p->x = x;
        p->y = y;
    }
    else if(argc == 2)
    {
         p->x = dAtoi(argv[0]);
         p->y = dAtoi(argv[1]);
    }
    else
        Con::printf("Point2I must be set as { x,y } or \"x y\"");
}

// -----------------------------------------------------------------------------
// Rectangles
// -----------------------------------------------------------------------------

IMPLEMENT_STRUCT( RectI,
                  RectI, ,
                  "" )
END_IMPLEMENT_STRUCT;
IMPLEMENT_STRUCT( RectF,
                  RectF, ,
                  "" )
END_IMPLEMENT_STRUCT;


//-----------------------------------------------------------------------------
// TypeRectI
//-----------------------------------------------------------------------------
ConsoleType(RectI, TypeRectI, RectI, "")
ImplementConsoleTypeCasters( TypeRectI, RectI )

ConsoleGetType( TypeRectI )
{
    RectI *rect = (RectI *) dptr;
    static const U32 bufSize = 256;
    char* returnBuffer = Con::getReturnBuffer(bufSize);
    dSprintf(returnBuffer, bufSize, "%d %d %d %d", rect->x, rect->y,
             rect->w, rect->h);
    return returnBuffer;
}

ConsoleSetType( TypeRectI )
{
    if(argc == 1)
        dSscanf(argv[0], "%d %d %d %d", &((RectI *) dptr)->x, &((RectI *) dptr)->y,
                &((RectI *) dptr)->w, &((RectI *) dptr)->h);
        else if(argc == 4)
            *((RectI *) dptr) = RectI(dAtoi(argv[0]), dAtoi(argv[1]), dAtoi(argv[2]), dAtoi(argv[3]));
    else
        Con::printf("RectI must be set as { x, y, w, h } or \"x y w h\"");
}

//-----------------------------------------------------------------------------
// TypeRectF
//-----------------------------------------------------------------------------
ConsoleType(RectF, TypeRectF, RectF, "")
ImplementConsoleTypeCasters( TypeRectF, RectF )

ConsoleGetType( TypeRectF )
{
    RectF *rect = (RectF *) dptr;
    static const U32 bufSize = 256;
    char* returnBuffer = Con::getReturnBuffer(bufSize);
    dSprintf(returnBuffer, bufSize, "%g %g %g %g", rect->x, rect->y,
             rect->w, rect->h);
    return returnBuffer;
}

ConsoleSetType( TypeRectF )
{
    if(argc == 1)
        dSscanf(argv[0], "%g %g %g %g", &((RectF *) dptr)->x, &((RectF *) dptr)->y,
                &((RectF *) dptr)->w, &((RectF *) dptr)->h);
        else if(argc == 4)
            *((RectF *) dptr) = RectF(dAtof(argv[0]), dAtof(argv[1]), dAtof(argv[2]), dAtof(argv[3]));
    else
        Con::printf("RectF must be set as { x, y, w, h } or \"x y w h\"");
}

//-----------------------------------------------------------------------------
// Point2F
//-----------------------------------------------------------------------------

IMPLEMENT_STRUCT( Point2F,
                  Point2F, ,
                  "" )

FIELD( x, x, 1, "X coordinate." )
FIELD( y, y, 1, "Y coordinate." )
END_IMPLEMENT_STRUCT;
//-----------------------------------------------------------------------------
// TypePoint2F
//-----------------------------------------------------------------------------
ConsoleType(Point2F, TypePoint2F, Point2F, "")
ImplementConsoleTypeCasters( TypePoint2F, Point2F )

ConsoleGetType( TypePoint2F )
{
    Point2F *pt = (Point2F *) dptr;
    static const U32 bufSize = 256;
    char* returnBuffer = Con::getReturnBuffer(bufSize);
    dSprintf(returnBuffer, bufSize, "%g %g", pt->x, pt->y);
    return returnBuffer;
}

ConsoleSetType( TypePoint2F )
{
    if(argc == 1)
        dSscanf(argv[0], "%g %g", &((Point2F *) dptr)->x, &((Point2F *) dptr)->y);
    else if(argc == 2)
        *((Point2F *) dptr) = Point2F(dAtof(argv[0]), dAtof(argv[1]));
    else
        Con::printf("Point2F must be set as { x, y } or \"x y\"");
}

//-----------------------------------------------------------------------------
// Point3F
//-----------------------------------------------------------------------------


IMPLEMENT_STRUCT( Point3F,
                  Point3F, ,
                  "" )

FIELD( x, x, 1, "X coordinate." )
FIELD( y, y, 1, "Y coordinate." )
FIELD( z, z, 1, "Z coordinate." )

END_IMPLEMENT_STRUCT;
//-----------------------------------------------------------------------------
// TypePoint3F
//-----------------------------------------------------------------------------
ConsoleType(Point3F, TypePoint3F, Point3F, "")
ImplementConsoleTypeCasters(TypePoint3F, Point3F)

ConsoleGetType( TypePoint3F )
{
    Point3F *pt = (Point3F *) dptr;
    static const U32 bufSize = 256;
    char* returnBuffer = Con::getReturnBuffer(bufSize);
    dSprintf(returnBuffer, bufSize, "%g %g %g", pt->x, pt->y, pt->z);
    return returnBuffer;
}

ConsoleSetType( TypePoint3F )
{
    if(argc == 1)
        dSscanf(argv[0], "%g %g %g", &((Point3F *) dptr)->x, &((Point3F *) dptr)->y, &((Point3F *) dptr)->z);
    else if(argc == 3)
        *((Point3F *) dptr) = Point3F(dAtof(argv[0]), dAtof(argv[1]), dAtof(argv[2]));
    else
        Con::printf("Point3F must be set as { x, y, z } or \"x y z\"");
}


// =============================================================================
//FIXME not sure about this works........
// namespace PropertyInfo
// {
//     using namespace ElfMath; //mFloor
//
//     bool default_scan(const String &data, Point2I & result)
//     {
//         // Handle passed as floating point from script
//         if(data.find('.') != String::NPos)
//         {
//             Point2F tempResult;
//             dSscanf(data.c_str(),"%f %f",&tempResult.x,&tempResult.y);
//             result.x = ElfMath::mFloor(tempResult.x);
//             result.y = ElfMath::mFloor(tempResult.y);
//         }
//         else
//             dSscanf(data.c_str(),"%d %d",&result.x,&result.y);
//         return true;
//     }
//
//     bool default_print(String & result, Point2I const & data)
//     {
//         result = String::ToString("%d %d",data.x,data.y);
//         return true;
//     }
//
//
//     //-----------------------------------------------------------------------------
//     // Math - Rectangles and boxes
//     //-----------------------------------------------------------------------------
//     bool default_scan( const String &data, RectI & result )
//     {
//         // Handle passed as floating point from script
//         if(data.find('.') != String::NPos)
//         {
//             RectF tempResult;
//             dSscanf(data.c_str(),"%f %f %f %f",&tempResult.x,&tempResult.y,&tempResult.w,&tempResult.h);
//             result.x = mFloor(tempResult.x);
//             result.y = mFloor(tempResult.y);
//             result.w = mFloor(tempResult.w);
//             result.h = mFloor(tempResult.h);
//         }
//         else
//             dSscanf(data.c_str(),"%d %d %d %d",&result.x,&result.y,&result.w,&result.h);
//         return true;
//     }
//     bool default_print( String & result, const RectI & data )
//     {
//         result = String::ToString("%i %i %i %i",data.x,data.y,data.w,data.h);
//         return true;
//     }
//
//     bool default_scan(const String &data, RectF & result)
//     {
//         dSscanf(data.c_str(),"%g %g %g %g",&result.x,&result.y,&result.w,&result.h);
//         return true;
//     }
//
//     bool default_print(String & result, const RectF & data)
//     {
//         result = String::ToString("%g %g %g %g",data.x,data.y,data.w,data.h);
//         return true;
//     }
// --------------------------------------------------------------------------------------------------
// } //namespace
