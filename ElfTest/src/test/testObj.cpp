#include "console/engineAPI.h"

    // class EmptyObject : public SimObject
    // {
    //     typedef SimObject Parent;
    //
    // public:
    //     DECLARE_CONOBJECT(EmptyObject);
    // };
    // IMPLEMENT_CONOBJECT(EmptyObject);
//--------------

class TestObj: public SimObject
{
    typedef SimObject Parent;
public:
    DECLARE_CONOBJECT(TestObj);
    F32 mX, mY, mZ;
    F64 mDouble;
    S32 mIndex;
    bool mBool;
    StringTableEntry mTestName;
    S32 mSlots[3];

    TestObj() {
        mX = mY = mZ = 0.f;
        mDouble = 0.f;
        mIndex = 0;
        mBool = false;
        mTestName = StringTable->insert("NoName");
        for (S32 i = 0; i < 3; i++) mSlots[i]=0;
    }

    static void initPersistFields()
    {
        // Parent::initPersistFields();
        addField("x",     TypeF32,     Offset(mX, TestObj));
        addField("y",     TypeF32,     Offset(mY, TestObj));
        addField("z",     TypeF32,     Offset(mZ, TestObj));
        addField("testFloat", TypeF64,   Offset(mDouble, TestObj));
        addField("testInt",  TypeS32,    Offset(mIndex, TestObj));
        addField("testBool", TypeBool,   Offset(mBool, TestObj));
        addField("testString", TypeString, Offset(mTestName, TestObj));
        addField("testSlot", TypeS32, Offset(mSlots, TestObj), 3); //WARNING count is not validated!!!

    }

    void setPos(F32 x, F32 y, F32 z) {
        mX = x;
        mY = y;
        mZ = z;
    }
};

IMPLEMENT_CONOBJECT(TestObj);

DefineEngineMethod(TestObj, getPos, String, (), , "get the position") {
    // we do nothing special here
    StringBuilder str;
    str.format("%g %g %g", object->mX, object->mY, object->mZ);
    return Con::getStringArg(str.end());
}

DefineEngineMethod(TestObj, getX, F32, (), , "") {
  return object->mX;
}
DefineEngineMethod(TestObj, getY, F32, (), , "") {
    return object->mY;
}
DefineEngineMethod(TestObj, getZ, F32, (), , "") {
    return object->mZ;
}

DefineEngineMethod(TestObj, walk, void, (F32 x, F32 y, F32 z), , "Set point where to walk to") {
    // we do nothing special here
    object->setPos(x,y,z);
}
