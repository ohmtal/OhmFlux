#include "core/fluxGlobals.h"
#include "core/Globals.h"
#include "console/engineAPI.h"
#include "console/script.h"


namespace ElfFlux {


    class Struct: public SimObject {
        typedef SimObject Parent;
    public:
        Color4F mColor = {1.f,1.f,1.f,1.f};
        Point2F mVector2f = {0.f,0.f};
        Point3F mVector3f = {0.f,0.f,0.f};
        // Point2I mVector2i = {0,0};
        RectF mRectF = {0.f,0.f,0.f,0.f};
        // RectI mRectI = {0,0,0,0}
        static void initPersistFields();
        DECLARE_CONOBJECT(Struct);
    };
    IMPLEMENT_CONOBJECT(Struct);
    void Struct::initPersistFields() {
        addField("Color", TypeColorF, Offset(mColor, Struct ), "floating point color ");
        addField("Point2F", TypePoint2F, Offset(mVector2f, Struct ), "floating point struct (x,y)");
        addField("Point3F", TypePoint3F, Offset(mVector3f, Struct ), "floating point struct (x,y)");
        addField("RectF", TypeRectF, Offset(mRectF, Struct ), "floating point struct (x,y,w,h)");
    };

    // -------------------- Vector2Object -----------------------------------------
    class Vector2Object: public SimObject {
        typedef SimObject Parent;
    public:
        Point2F mVector;
        static void initPersistFields();
        DECLARE_CONOBJECT(Vector2Object);
    };
    IMPLEMENT_CONOBJECT(Vector2Object);
    void Vector2Object::initPersistFields() {
        addField("data", TypePoint2F, Offset(mVector, Vector2Object ), "Point2F");
        addField("x", TypeF32, Offset(mVector.x, Vector2Object ), "");
        addField("y", TypeF32, Offset(mVector.y, Vector2Object ), "");
    }


    DefineEngineMethod(Vector2Object, add, Point2F, (Point2F b),,"+ b")
    {
        object->mVector += b;
        return object->mVector;
    }
    DefineEngineMethod(Vector2Object, sub, Point2F, (Point2F b),,"- b")
    {
        object->mVector -= b;
        return object->mVector;
    }
    DefineEngineMethod(Vector2Object, mul, Point2F, (Point2F b),,"* b")
    {
        object->mVector *= b;
        return object->mVector;
    }

    DefineEngineMethod(Vector2Object, div, Point2F, (Point2F b),,"/ b")
    {
        object->mVector /= b;
        return object->mVector;
    }

    DefineEngineMethod(Vector2Object, addScalar, Point2F, (F32 b),,"+ b")
    {
        object->mVector.x += b;
        object->mVector.y += b;
        return object->mVector;
    }

    DefineEngineMethod(Vector2Object, subScalar, Point2F, (F32 b),,"- b")
    {
        object->mVector.x -= b;
        object->mVector.y -= b;
        return object->mVector;
    }

    DefineEngineMethod(Vector2Object, scale, Point2F, (F32 b),,"* b")
    {
        object->mVector *= b;
        return object->mVector;
    }

    DefineEngineMethod(Vector2Object, divScalar, Point2F, (F32 b),,"/ b")
    {
        object->mVector /= b;
        return object->mVector;
    }

    DefineEngineMethod(Vector2Object, dot, F32, (Point2F b),,"a.x * b.x + a.y * b.y")
    {
        return object->mVector.dot(b);
    }

    DefineEngineMethod(Vector2Object, cross, F32, (Point2F b),,"a.x * b.y - a.y * b.x")
    {
        return object->mVector.cross(b);
    }

    DefineEngineMethod(Vector2Object, distance, F32, (Point2F b),,"distance to b")
    {
        return object->mVector.dist(b);
    }

    DefineEngineMethod(Vector2Object, len, F32, (),,"length of vector")
    {
        return object->mVector.len();
    }

    DefineEngineMethod(Vector2Object, normalize, Point2F, (),,"normalizes the vector")
    {
        object->mVector.normalize();
        return object->mVector;
    }

    DefineEngineMethod(Vector2Object, toPoint2I, Point2I, (),,"return integer values")
    {
        return object->mVector.toPoint2I();
    }


    // -------------------- RectObject -----------------------------------------
    class RectObject: public SimObject {
        typedef SimObject Parent;
    public:
        RectF mRect;
        static void initPersistFields();
        DECLARE_CONOBJECT(RectObject);
    };
    IMPLEMENT_CONOBJECT(RectObject);
    void RectObject::initPersistFields() {
        addField("data", TypeRectF, Offset(mRect, RectObject ), "RectF");
        addField("x", TypeF32, Offset(mRect.x, RectObject ), "");
        addField("y", TypeF32, Offset(mRect.y, RectObject ), "");
        addField("w", TypeF32, Offset(mRect.w, RectObject ), "");
        addField("h", TypeF32, Offset(mRect.h, RectObject ), "");
    }


     DefineEngineMethod(RectObject, isValid, bool, ( ),, "check rect is valid") {
         return object->mRect.isValidRect();
     }

    DefineEngineMethod(RectObject, PointInRect, bool, ( Point2F point),, "check a point is in a rect") {
        return object->mRect.pointInRect(point);
    }
    DefineEngineMethod(RectObject, Inflate, RectF, ( F32 x, F32 y),, "add a spacing to a Rect (inflate)") {
        object->mRect.inflate(x, y);
        return object->mRect;
    }
    DefineEngineMethod(RectObject, Contains, bool, ( RectF other),, "check rect contains other") {
        return object->mRect.contains(other);
    }
    DefineEngineMethod(RectObject, Intersects, bool, ( RectF other),, "check rect intersects other") {
        return object->mRect.intersects(other);
    }
    DefineEngineMethod(RectObject, GetCenter, Point2F, (),, "get the center point of a rect") {
        return object->mRect.getCenterPoint();
    }

    // return a modified rect
    DefineEngineMethod(RectObject, withHeight, RectF, (F32 h),, "return the object with modified height") {
        RectF result = object->mRect;
        result.h = h;
        return result;
    }
    DefineEngineMethod(RectObject, withWidth, RectF, (F32 w),, "return the object with modified width") {
        RectF result = object->mRect;
        result.w = w;
        return result;
    }


} //ElfFlux
