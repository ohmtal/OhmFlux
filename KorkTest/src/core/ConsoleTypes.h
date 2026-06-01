#pragma once
#ifndef _DYNAMIC_CONSOLETYPES_H_
#include "sim/dynamicTypes.h"
#endif

DefineConsoleType( TypePoint2F )
struct Point2F;
S32 dAtoPoint2F(Point2F& p,  const char * str);

DefineConsoleType( TypePoint3F )
struct Point3F;
S32 dAtoPoint3F(Point3F& p,  const char * str);

DefineConsoleType( TypeColor4F )
struct Color4F;
// return the param count scanned
S32 dAtoColor4F(Color4F& color,  const char * str);
