#include "appMain.h"
#include "core/fluxGlue.h"
#include "core/Globals.h"

#include "console/engineAPI.h"
#include "console/script.h"

#include <string>
#include <core/volume.h>

namespace ElfFlux {


bool loadScript(String fileName) {
    if (!gLastScriptFile.isEmpty() && Con::isFunction("onLeaveScript")) {
      Con::executef("onLeaveScript",  gLastScriptFile);
    }
    if (Con::executeFile(fileName)) {
        if (Con::isFunction("onEnterScript")) Con::executef("onEnterScript",  fileName);
        gLastScriptFile = fileName;
        return true;
    }
    return false;
}


// --------------------------------------------------------------------------
ConsoleFunctionGroupBegin(App, "App Functions: getFPS, ...");

ConsoleFunction(getFullScreen, bool, 1,1, "") {
    return getScreenObject()->getFullScreen();
}
ConsoleFunction(setFullScreen, bool, 2,2, "bool value") {
    return getScreenObject()->setFullScreen(dAtob(argv[1]));
}

ConsoleFunction(getFrameTime, F32, 1,1, "") {
    return getFrameTime();
}

ConsoleFunction(getRealTime, S32, 1,1, "") {
    return Sim::getCurrentTime();
}

ConsoleFunction(getFPS, S32, 1,1, "") {
    return gMain->getFPS();
}

ConsoleFunction(getScreenWidth, S32, 1,1, "") {
    return getScreenObject()->getWidth();
}

ConsoleFunction(getScreenHeight, S32, 1,1, "") {
    return getScreenObject()->getHeight();
}

ConsoleFunction(setVSync, void, 2,2, "bool value") {
    return getScreenObject()->setVSync(dAtob(argv[1]));
}

DefineEngineFunction(getFullPath, String,(),, "get the current directory") {
    return Torque::FS::GetCwd().getFullPath();
}

// ----------------- include = exec with nocalls ----------------------

DefineEngineFunction(include,bool, (String fileName),, "include(fileName)" "exec a file without calls" ){
    return Con::executeFile(fileName, true);
}

// ----------------- debuglog ----------------------
//-----------------------------------------------------------------------------

DefineEngineStringlyVariadicFunction( dEcho, void, 2, 0, "debug echo ( string message... ) ")
{
#ifdef FLUX_DEBUG
    U32 len = 0;
    S32 i;
    for(i = 1; i < argc; i++)
        len += dStrlen(argv[i]);

    char *ret = Con::getReturnBuffer(len + 1);
    ret[0] = 0;
    for(i = 1; i < argc; i++)
        dStrcat(ret, argv[i], (U64)(len + 1));

    Con::printf("%s", ret);
    ret[0] = 0;
#endif
}

//-----------------------------------------------------------------------------

DefineEngineStringlyVariadicFunction( dWarn, void, 2, 0, "debug warn( string message... ) " )
{
#ifdef FLUX_DEBUG
    U32 len = 0;
    S32 i;
    for(i = 1; i < argc; i++)
        len += dStrlen(argv[i]);

    char *ret = Con::getReturnBuffer(len + 1);
    ret[0] = 0;
    for(i = 1; i < argc; i++)
        dStrcat(ret, argv[i], (U64)(len + 1));

    Con::warnf(ConsoleLogEntry::General, "%s", ret);
    ret[0] = 0;
#endif
}

//-----------------------------------------------------------------------------

DefineEngineStringlyVariadicFunction( dError, void, 2, 0, "(debug error  string message... ) ")
{
#ifdef FLUX_DEBUG
    U32 len = 0;
    S32 i;
    for(i = 1; i < argc; i++)
        len += dStrlen(argv[i]);

    char *ret = Con::getReturnBuffer(len + 1);
    ret[0] = 0;
    for(i = 1; i < argc; i++)
        dStrcat(ret, argv[i], (U64)(len + 1));

    Con::errorf(ConsoleLogEntry::General, "%s", ret);
    ret[0] = 0;
#endif
}

//------------------------------------------------------------------------------
// Mouse
//------------------------------------------------------------------------------
DefineEngineFunction(getMousePos, Point2F, (),," MousePositon relative to screenSize") {
    return (gAppStatus.getMousePos());
}
DefineEngineFunction(getRealMousePos, Point2F, (),,"real MousePositon reported by SDL") {
    return (gAppStatus.getRealMousePos());
}
DefineEngineFunction(getWorldMousePos, Point2F, (),,"MousePositon based on MousePos and relative to Camera Postion and Zoom") {
    return (gAppStatus.getWorldMousePos());
}

//------------------------------------------------------------------------------
// Rect stuff
//------------------------------------------------------------------------------
DefineEngineFunction(PointInRect, bool, (RectF rect, Point2F point),, "check a point is in a rect") {
    return rect.pointInRect(point);
}
DefineEngineFunction(RectInflate, RectF, (RectF rect, F32 x, F32 y),, "add a spacing to a Rect (inflate)") {
    rect.inflate(x, y);
    return rect;
}
DefineEngineFunction(RectContains, bool, (RectF rect, RectF other),, "check rect contains other") {
    return rect.contains(other);
}
DefineEngineFunction(RectIntersects, bool, (RectF rect, RectF other),, "check rect intersects other") {
    return rect.intersects(other);
}
DefineEngineFunction(RectGetCenter, Point2F, (RectF rect),, "get the center point of a rect") {
    return rect.getCenterPoint();
}
//------------------------------------------------------------------------------
// DEBUG / Test stuff
//------------------------------------------------------------------------------

#ifdef FLUX_DEBUG
// ---- Foo
static S32 myFoo = 0;
DefineEngineFunction(getFoo, S32, (), , "you will get a foo.") {
    return myFoo;
}

DefineEngineFunction(setFoo,  void, (S32 foo), , "you will set a foo.") {
    myFoo = foo;
}

DefineEngineFunction(getFooToGlobal, void, (const char* globalVarName), , "") {
    Con::setIntVariable(globalVarName, myFoo);
}

// ---- TestExec
DefineEngineFunction(testExec, void,() , , "Test an exec return") {
    Con::evaluate( R"(
            function onDebugTest() {
                %res = getRandom(1);
                echo("should be" SPC %res);
                return %res ? "true" : "false";
            }
        )"
    );
    ConsoleValue result = Con::executef("onDebugTest");
    Con::printf("Test result is %d, as string: %s", result.getBool(), result.getString());
}

// --------------------------------- VARIABLE TEST -----------------------------------
/*
   varTest(); echo($Global::IntVar); $Global::IntVar = 5; echo($PI);

*/

static S32 intVar = 4711;
constexpr F32 myPI = M_PI;
static F32 floatVar = 6.66f;
static void intVarChanged() {
    Con::warnf("IntVar changed: %d", intVar);
}


DefineEngineFunction(varTest, void, (),, "init the variable tests") {

    // Add a variable with pointer and callback!
    Con::addVariable(
        "$Global::IntVar",
        TypeS32,
        &intVar,
        "A integer Value with a pointer!"
    );
    Con::addVariableNotify( "$Global::IntVar", intVarChanged);

    // addConstant must be a variable!! - guess i cant use enum..
    // i also can use a non const variable so it may change
    Con::addConstant(
        "PI",
        TypeF32,
        &myPI,
        "A PI"
    );


}




#endif

ConsoleFunctionGroupEnd(App);
}
