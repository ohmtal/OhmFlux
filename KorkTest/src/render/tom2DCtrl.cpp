#include "tom2DCtrl.h"

#include "appMain.h"
#include <platform/platformString.h>
#include "Texture.h"

namespace KorkFlux {

    IMPLEMENT_CONOBJECT(tom2DCtrl);

//------------------------------------------------------------------------------
// ConsoleMethod( tom2DCtrl, draw, void, 6, 6, "(Texture,x,y,layer)"
// "draw a image, Layer 1-99 possible.")
// {
//     Texture* simTex = dynamic_cast<Texture*>(Sim::findObject(consoleobject));
//     F32 z = dAtof(argv[5]) / HS2D_MAXLAYERS;
//     Render2D.uglyDraw2DStretch(simTex, dAtof(argv[3]),dAtof(argv[4]),z);
// }

// ConsoleMethod( tom2DCtrl, drawstretch, void, 9, 14, "(tom2DTexture,imgId,x,y,layer,w,h,[rotation,flipX,flipY, alpha channel default 0.1], optimizetransparent)"
// "draw a image, Layer 1-99 possible. WARNING LAYER DOES NOT WORK ANYMORE!!(iPhone compat)")
// {
//     tom2DTexture* obj = (tom2DTexture*)Sim::findObject(dAtoi(argv[2]));
//     F32 z = dAtof(argv[6]) / HS2D_MAXLAYERS;
//
//     F32  rot   = 0;
//     bool flipX = false;
//     bool flipY = false;
//     F32  lAlpha = 0.1f;
//     bool ldoBlend=false;
//     if (argc > 9)
//         rot = dAtof(argv[9]);
//     if (argc > 10)
//         flipX = dAtob(argv[10]);
//     if (argc > 11)
//         flipY = dAtob(argv[11]);
//     if (argc > 12)
//         lAlpha = dAtof(argv[12]);
//     if (argc > 13)
//         ldoBlend = dAtob(argv[13]);
//
//     object->drawStretch(obj, dAtoi(argv[3]), Point3F(dAtof(argv[4]),dAtof(argv[5]),z), Point2I(dAtoi(argv[7]),dAtoi(argv[8])),rot , flipX, flipY, lAlpha, ldoBlend);
// }
// //---------
// ConsoleMethod( tom2DCtrl, drawRect, void, 10, 11, "(tom2DTexture,imgId,x,y,layer,w,h,srcRect,(optimizetransparent)"
// "draw a image from srcRect, Layer 1-99 possible.")
// {
//     tom2DTexture* obj = (tom2DTexture*)Sim::findObject(dAtoi(argv[2]));
//     F32 z = dAtof(argv[6]) / HS2D_MAXLAYERS;
//
//     F32  rot   = 0;
//     bool flipX = false;
//     bool flipY = false;
//     F32  lAlpha = 0.1f;
//     bool ldoBlend=false;
//     if (argc > 10)
//         ldoBlend = dAtob(argv[10]);
//
//     RectF lsrcRect;
//     dSscanf(argv[9],"%g %g %g %g",
//             &lsrcRect.point.x,&lsrcRect.point.y,&lsrcRect.extent.x,&lsrcRect.extent.y);
//
//
//     // void tom2DCtrl::drawRect(tom2DTexture* img, U32 imgId, Point3F worldPos,RectF lSrc, Point2I dimension, bool doBlend
//     object->drawRect(obj, dAtoi(argv[3]), Point3F(dAtof(argv[4]),dAtof(argv[5]),z), lsrcRect, Point2I(dAtoi(argv[7]),dAtoi(argv[8])),ldoBlend);
// }
//
//
//
// ConsoleMethod( tom2DCtrl, writeText, void, 6, 8, "(x,y,string, align (0=left,1=middle,2=right,3=center), profile, fontcolortype (0=normal,1=NA,2=HL)"
// "write text on screen.")
// {
//     GuiControlProfile *lProfile =  object->mProfile;
//     if (argc > 6)
//     {
//         SimObject *obj = Sim::findObject(argv[6]);
//         if(obj)
//             lProfile = static_cast<GuiControlProfile*>(obj);
//     }
//     U8 lFontColorType = 0;
//     if (argc > 7)
//         lFontColorType = dAtoi( argv[7] );
//
//     object->writeText(argv[4],lProfile, dAtoi(argv[2]),dAtoi(argv[3]),dAtoi(argv[5]), lFontColorType);
// }
// //------------------------------------------------------------------------------
//
// ConsoleMethod( tom2DCtrl, writeCustomText, void, 9, 9, "(x,y,text, align (0=left,1=middle,2=right,3=center), fontName, fontSize, fontColor ==> like 200 200 200 [255])"
// "write text on screen with font / fontsize and fontcolor.")
// {
//     //void tom2DCtrl::writeCustomText(const char* text, U32 x, U32 y, U32 align, const char* fontFace, U32 fontSize, const char* FontColorString)
//     object->writeCustomText(argv[4],dAtoi(argv[2]),dAtoi(argv[3]),dAtoi(argv[5]),argv[6],dAtoi(argv[7]),argv[8]);
//
// }
//
// //------------------------------------------------------------------------------
// // 2.24  writeColoredText
// ConsoleMethod(tom2DCtrl, writeColoredText, void, 8, 8,
//               "(x,y,text, align (0=left,1=middle,2=right,3=center), profile, fontColor ==> like 200 200 200 [255])"
//               "write text on screen with  fontcolor.")
// {
//     GuiControlProfile* lProfile = object->mProfile;
//     SimObject* obj = Sim::findObject(argv[6]);
//     if (obj)
//         lProfile = static_cast<GuiControlProfile*>(obj);
//
//
//     object->writeColoredText(argv[4], lProfile, dAtoi(argv[2]), dAtoi(argv[3]), dAtoi(argv[5]), object->scanColorText(argv[7]));
// }
// //------------------------------------------------------------------------------
// // 2.24  writeColoredText
// ConsoleMethod(tom2DCtrl, writeColoredShadowText, void, 11, 11,
//               "(x,y,text, align (0=left,1=middle,2=right,3=center), profile, xoffset,yoffset, fontColor ==> like 200 200 200 [255], shadowcolor)"
//               "write text on screen with  fontcolor.")
// {
//     GuiControlProfile* lProfile = object->mProfile;
//     SimObject* obj = Sim::findObject(argv[6]);
//     if (obj)
//         lProfile = static_cast<GuiControlProfile*>(obj);
//
//     int x = dAtoi(argv[2]);
//     int y = dAtoi(argv[3]);
//     int xoffset = dAtoi(argv[7]);
//     int yoffset = dAtoi(argv[8]);
//     ColorI fontColor = object->scanColorText(argv[9]);
//     ColorI shadowColor = object->scanColorText(argv[10]);
//
//     object->writeColoredText(argv[4], lProfile, x+xoffset,y+yoffset, dAtoi(argv[5]), shadowColor);
//     object->writeColoredText(argv[4], lProfile, x, y, dAtoi(argv[5]), fontColor);
// }
//------------------------------------------------------------------------------
} //namespace
