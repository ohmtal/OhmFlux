#pragma once
#include "math/mMathFn.h"
#include "core/fluxGlobals.h"
#ifndef _DYNAMIC_CONSOLETYPES_H_
#include "console/dynamicTypes.h"
#endif

#ifndef _ENGINEPRIMITIVES_H_
#include "console/enginePrimitives.h"
#endif

#ifndef _ENGINESTRUCTS_H_
#include "console/engineStructs.h"
#endif



DECLARE_STRUCT(Color4F);
DECLARE_STRUCT( RectI );
DECLARE_STRUCT( RectF );
DECLARE_STRUCT(Point2I);
DECLARE_STRUCT( Point2F );
DECLARE_STRUCT( Point3F );

DefineConsoleType( TypeColorF, Color4F )
DefineConsoleType( TypeRectI, RectI )
DefineConsoleType( TypeRectF, RectF )
DefineConsoleType( TypePoint2I, Point2I )
DefineConsoleType( TypePoint2F, Point2F )
DefineConsoleType( TypePoint3F, Point3F )

