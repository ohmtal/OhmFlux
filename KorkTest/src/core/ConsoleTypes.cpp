#include "core/fluxGlobals.h"
#include "sim/dynamicTypes.h"
#include <embed/internalApi.h>
#include <platform/platformString.h>
#include "Globals.h"
#include <console/console.h>

namespace KorkFlux {

    // Wrapper for Con::getReturnBuffer
    const char * getReturnString(const std::string& str, KorkApi::Vm* vmPtr) {
        KorkApi::ConsoleValue retV = Con::getReturnBuffer(str.length() + 1);
        char* ret = (char*)retV.evaluatePtr(vmPtr->getAllocBase());
        dStrcpy(ret, str.c_str());
        return ret;
    }
}
// ---------------------------- Point2F ---------------------------------------

ConsoleType( Point2F, TypePoint2F, sizeof(Point2F), sizeof(Point2F), "" )

ConsoleTypeOpDefault( TypePoint2F )

ConsoleGetType( TypePoint2F )
{
    const KorkApi::ConsoleValue* argv = nullptr;
    U32 argc = inputStorage ? inputStorage->data.argc : 0;
    bool directLoad = false;

    if (argc > 0 && inputStorage->data.storageRegister)
    {
        argv = inputStorage->data.storageRegister;
    }
    else
    {
        argc = 1;
        argv = &inputStorage->data.storageAddress;
        directLoad = true;
    }

    Point2F v = {0.f, 0.f};

    if (inputStorage->isField && directLoad)
    {
        const Point2F* src = (const Point2F*)inputStorage->data.storageAddress.evaluatePtr(vmPtr->getAllocBase());
        if (!src) return false;
        v = *src;
    }
    else
    {
        if (argc == 3)
        {
            v.x = (F32)argv[0].getFloat((F64)argv[0].getInt(0));
            v.y = (F32)argv[1].getFloat((F64)argv[1].getInt(0));
        }
        else if (argc == 1)
        {
            const char* s = vmPtr->valueAsString(argv[0]);
            if (!s) s = "";

            dSscanf(s, "%g %g", &v.x, &v.y);
        }
        else
        {
            // Not supported
            return false;
        }
    }

    // -> output

    if (requestedType == TypePoint2F  && outputStorage->isField)
    {
        Point2F* dstPtr = (Point2F*)outputStorage->data.storageAddress.evaluatePtr(vmPtr->getAllocBase());
        if (!dstPtr)
        {
            return false;
        }

        *dstPtr = v;

        if (outputStorage->data.storageRegister)
            *outputStorage->data.storageRegister = outputStorage->data.storageAddress;

        return true;
    }
    else if (requestedType == TypePoint2F || requestedType == KorkApi::ConsoleValue::TypeInternalString)
    {
        const U32 bufLen = 96;

        outputStorage->FinalizeStorage(outputStorage, bufLen);

        char* out = (char*)outputStorage->data.storageAddress.evaluatePtr(vmPtr->getAllocBase());
        if (!out) return false;

        dSprintf(out, bufLen, "%.9g %.9g", v.x, v.y);

        if (outputStorage->data.storageRegister)
            *outputStorage->data.storageRegister = outputStorage->data.storageAddress;

        return true;
    }
    else
    {
        KorkApi::ConsoleValue vals[2];
        vals[0] = KorkApi::ConsoleValue::makeNumber(v.x);
        vals[1] = KorkApi::ConsoleValue::makeNumber(v.y);

        KorkApi::TypeStorageInterface castInput =
        KorkApi::CreateRegisterStorageFromArgs(vmPtr->mInternal, 3, vals);

        return vmPtr->castValue(requestedType, &castInput, outputStorage, fieldUserPtr, flag);
    }
}

S32 dAtoPoint2F(Point2F& p,  const char * str) {
    return  dSscanf(str, "%g %g",&p.x, &p.y);
}


#ifdef FLUX_DEBUG
ConsoleFunction(test_Point2F, ConsoleVoid, 2, 2, "str point") {
    Point2F p = {0.f,0.f};
    S32 scanned = dAtoPoint2F(p, argv[1]);
    Con::errorf("test_Point2F RESULT: %s (%d)", p.to_string().c_str(), scanned);
}
#endif
// ---------------------------- Point3F ---------------------------------------
ConsoleType( Point3F, TypePoint3F, sizeof(Point3F), sizeof(Point3F), "" )

ConsoleTypeOpDefault( TypePoint3F )

ConsoleGetType( TypePoint3F )
{
    const KorkApi::ConsoleValue* argv = nullptr;
    U32 argc = inputStorage ? inputStorage->data.argc : 0;
    bool directLoad = false;

    if (argc > 0 && inputStorage->data.storageRegister)
    {
        argv = inputStorage->data.storageRegister;
    }
    else
    {
        argc = 1;
        argv = &inputStorage->data.storageAddress;
        directLoad = true;
    }

    Point3F v = {0.f, 0.f, 0.f};

    if (inputStorage->isField && directLoad)
    {
        const Point3F* src = (const Point3F*)inputStorage->data.storageAddress.evaluatePtr(vmPtr->getAllocBase());
        if (!src) return false;
        v = *src;
    }
    else
    {
        if (argc == 3)
        {
            v.x = (F32)argv[0].getFloat((F64)argv[0].getInt(0));
            v.y = (F32)argv[1].getFloat((F64)argv[1].getInt(0));
            v.z = (F32)argv[2].getFloat((F64)argv[2].getInt(0));
        }
        else if (argc == 1)
        {
            const char* s = vmPtr->valueAsString(argv[0]);
            if (!s) s = "";

            dSscanf(s, "%g %g %g", &v.x, &v.y, &v.z);
        }
        else
        {
            // Not supported
            return false;
        }
    }

    // -> output

    if (requestedType == TypePoint3F  && outputStorage->isField)
    {
        Point3F* dstPtr = (Point3F*)outputStorage->data.storageAddress.evaluatePtr(vmPtr->getAllocBase());
        if (!dstPtr)
        {
            return false;
        }

        *dstPtr = v;

        if (outputStorage->data.storageRegister)
            *outputStorage->data.storageRegister = outputStorage->data.storageAddress;

        return true;
    }
    else if (requestedType == TypePoint3F || requestedType == KorkApi::ConsoleValue::TypeInternalString)
    {
        const U32 bufLen = 96;

        outputStorage->FinalizeStorage(outputStorage, bufLen);

        char* out = (char*)outputStorage->data.storageAddress.evaluatePtr(vmPtr->getAllocBase());
        if (!out) return false;

        dSprintf(out, bufLen, "%.9g %.9g %.9g", v.x, v.y, v.z);

        if (outputStorage->data.storageRegister)
            *outputStorage->data.storageRegister = outputStorage->data.storageAddress;

        return true;
    }
    else
    {
        KorkApi::ConsoleValue vals[3];
        vals[0] = KorkApi::ConsoleValue::makeNumber(v.x);
        vals[1] = KorkApi::ConsoleValue::makeNumber(v.y);
        vals[2] = KorkApi::ConsoleValue::makeNumber(v.z);

        KorkApi::TypeStorageInterface castInput =
        KorkApi::CreateRegisterStorageFromArgs(vmPtr->mInternal, 3, vals);

        return vmPtr->castValue(requestedType, &castInput, outputStorage, fieldUserPtr, flag);
    }
}

S32 dAtoPoint3F(Point3F& p,  const char * str) {
    return  dSscanf(str, "%g %g %g",&p.x, &p.y, &p.z);
}


#ifdef FLUX_DEBUG
ConsoleFunction(test_Point3F, ConsoleVoid, 2, 2, "str point") {
    Point3F p = {0.f,0.f,0.f};
    S32 scanned = dAtoPoint3F(p, argv[1]);
    Con::errorf("test_Point3F RESULT: %s (%d)", p.to_string().c_str(), scanned);
}
#endif
// ---------------------------- Color4F ---------------------------------------
ConsoleType( Color4F, TypeColor4F, sizeof(Color4F), sizeof(Color4F), "" )

ConsoleTypeOpDefault( TypeColor4F )

ConsoleGetType( TypeColor4F )
{
    const KorkApi::ConsoleValue* argv = nullptr;
    U32 argc = inputStorage ? inputStorage->data.argc : 0;
    bool directLoad = false;

    if (argc > 0 && inputStorage->data.storageRegister)
    {
        argv = inputStorage->data.storageRegister;
    }
    else
    {
        argc = 1;
        argv = &inputStorage->data.storageAddress;
        directLoad = true;
    }

    Color4F v = {0.f, 0.f, 0.f, 1.f};

    if (inputStorage->isField && directLoad)
    {
        const Color4F* src = (const Color4F*)inputStorage->data.storageAddress.evaluatePtr(vmPtr->getAllocBase());
        if (!src) return false;
        v = *src;
    }
    else
    {
        if (argc == 3)
        {
            v.r = (F32)argv[0].getFloat((F64)argv[0].getInt(0));
            v.g = (F32)argv[1].getFloat((F64)argv[1].getInt(0));
            v.b = (F32)argv[2].getFloat((F64)argv[2].getInt(0));
            v.a = 1.0f;
        }
        else if (argc == 4)
        {
            v.r = (F32)argv[0].getFloat((F64)argv[0].getInt(0));
            v.g = (F32)argv[1].getFloat((F64)argv[1].getInt(0));
            v.b = (F32)argv[2].getFloat((F64)argv[2].getInt(0));
            v.a = (F32)argv[3].getFloat((F64)argv[2].getInt(0));
        }
        else if (argc == 1)
        {
            const char* s = vmPtr->valueAsString(argv[0]);
            if (!s) s = "";

            dSscanf(s, "%g %g %g %g", &v.r, &v.g, &v.b, &v.a);
        }
        else
        {
            // Not supported
            return false;
        }
    }

    // -> output

    if (requestedType == TypeColor4F && outputStorage->isField) // XXTH IMPORTANT isField
    {
        Color4F* dstPtr = (Color4F*)outputStorage->data.storageAddress.evaluatePtr(vmPtr->getAllocBase());
        if (!dstPtr)
        {
            return false;
        }

        *dstPtr = v;

        if (outputStorage->data.storageRegister)
            *outputStorage->data.storageRegister = outputStorage->data.storageAddress;

        return true;
    }
    else if (requestedType == TypeColor4F || requestedType == KorkApi::ConsoleValue::TypeInternalString)
    {
        const U32 bufLen = 128;

        outputStorage->FinalizeStorage(outputStorage, bufLen);

        char* out = (char*)outputStorage->data.storageAddress.evaluatePtr(vmPtr->getAllocBase());
        if (!out) return false;

        dSprintf(out, bufLen, "%.9g %.9g %.9g %.9g", v.r, v.g, v.b, v.a);

        if (outputStorage->data.storageRegister)
            *outputStorage->data.storageRegister = outputStorage->data.storageAddress;

        return true;
    }
    else
    {
        KorkApi::ConsoleValue vals[4];
        vals[0] = KorkApi::ConsoleValue::makeNumber(v.r);
        vals[1] = KorkApi::ConsoleValue::makeNumber(v.g);
        vals[2] = KorkApi::ConsoleValue::makeNumber(v.b);
        vals[3] = KorkApi::ConsoleValue::makeNumber(v.a);

        KorkApi::TypeStorageInterface castInput =
        KorkApi::CreateRegisterStorageFromArgs(vmPtr->mInternal, 4, vals);

        return vmPtr->castValue(requestedType, &castInput, outputStorage, fieldUserPtr, flag);
    }
}

S32 dAtoColor4F(Color4F& color,  const char * str) {
   S32 result = dSscanf(str, "%g %g %g %g", &color.r, &color.g, &color.b, &color.a);
   if (result > 0 ) color.normalize();
   return result;
}


#ifdef FLUX_DEBUG
ConsoleFunction(test_Color, ConsoleVoid, 2, 2, "str color") {
    Color4F myColor = cl_White;
    S32 scanned = dAtoColor4F(myColor, argv[1]);
    Con::errorf("TEST_COLOR RESULT: %s (%d)", myColor.to_string().c_str(), scanned);
}
#endif

// } //namespace
