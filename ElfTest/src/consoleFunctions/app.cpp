#include "appMain.h"
#include "core/fluxGlue.h"
#include "core/Globals.h"

#include "console/engineAPI.h"
#include "console/script.h"

#include <string>
#include <core/volume.h>

#include "console/consoleExtras.h"

// enums Box2D
#include "Box2D/Box2D.h"
#include <b2b/b2Objects.h>



namespace ElfFlux {
// --------------------------------------------------------------------------
constexpr F32 gPI = M_PI;
constexpr F32 g2PI = M_2PI;
constexpr F32 gPI2 = M_PI_2;

void init() {

    Con::addConstant("FrameTime", TypeF64, &gFrameTime, "current FrameTime");
    Con::addConstant("GameTime", TypeF64, &gGameTime, "current GameTime seconds since start");
    Con::addConstant("FPS", TypeF64, &gFPS, "current FPS");

    Con::addConstant("Paused", TypeBool, &gAppStatus.Paused, "game is paused");
    Con::addConstant("Focused", TypeBool, &gAppStatus.KeyboardFocus, "game is mouse focused");
    Con::addConstant("MousePos", TypePoint2F, &gAppStatus.MousePos, "mouse Postion (relative)");
    Con::addConstant("RealMousePos", TypePoint2F, &gAppStatus.RealMousePos, "mouse Postion (OS)");
    Con::addConstant("WorldMousePos", TypePoint2F, &gAppStatus.WorldMousePos, "mouse Postion in World");



    // Enum Helper Box2D
     Con::registerEnumS32<b2BodyType>("$box2d::", true);
     Con::registerEnumS32<Shape2bShapeTypes>("$box2d::", true);
     Con::registerEnumS32<b2JointType>("$box2d::", true);

     // Enum Font
     Con::registerEnumS32<FontAlign>("", true);

     // Math
     Con::addConstant("M_PI", TypeF32, &gPI,  "pi (3.14)");
     Con::addConstant("M_2PI", TypeF32, &g2PI,"pi * 2 (6.28)");
     Con::addConstant("M_PI2", TypeF32, &gPI2,"pi / 2 (1.57)");

     // Colors:
     Con::addConstant("Color::White", TypeColorF, &cl_White, "White");
     Con::addConstant("Color::Black", TypeColorF, &cl_Black, "Black");
     Con::addConstant("Color::Red", TypeColorF, &cl_Red, "Red");
     Con::addConstant("Color::Green", TypeColorF, &cl_Green, "Green");
     Con::addConstant("Color::Blue", TypeColorF, &cl_Blue, "Blue");
     Con::addConstant("Color::Yellow", TypeColorF, &cl_Yellow, "Yellow");
     Con::addConstant("Color::Cyan", TypeColorF, &cl_Cyan, "Cyan");
     Con::addConstant("Color::Magenta", TypeColorF, &cl_Magenta, "Magenta");
     Con::addConstant("Color::Gray", TypeColorF, &cl_Gray, "Gray");
     Con::addConstant("Color::LightGray", TypeColorF, &cl_LightGray, "LightGray");
     Con::addConstant("Color::DarkGray", TypeColorF, &cl_DarkGray, "DarkGray");
     Con::addConstant("Color::Orange", TypeColorF, &cl_Orange, "Orange");
     Con::addConstant("Color::Purple", TypeColorF, &cl_Purple, "Purple");
     Con::addConstant("Color::Brown", TypeColorF, &cl_Brown, "Brown");
     Con::addConstant("Color::Lime", TypeColorF, &cl_Lime, "Lime");
     Con::addConstant("Color::Pink", TypeColorF, &cl_Pink, "Pink");

     // Some more Colors
     Con::addConstant("Color::Crimson", TypeColorF, &cl_Crimson, "Crimson");
     Con::addConstant("Color::Emerald", TypeColorF, &cl_Emerald, "Emerald");
     Con::addConstant("Color::SkyBlue", TypeColorF, &cl_SkyBlue, "SkyBlue");
     Con::addConstant("Color::Slate", TypeColorF, &cl_Slate, "Slate");
     Con::addConstant("Color::Gold", TypeColorF, &cl_Gold, "Gold");
     Con::addConstant("Color::Transparent", TypeColorF, &cl_Transparent, "Transparent");
     Con::addConstant("Color::Aquamarine", TypeColorF, &cl_Aquamarine, "Aquamarine");
     Con::addConstant("Color::Coral", TypeColorF, &cl_Coral, "Coral");
     Con::addConstant("Color::DeepSea", TypeColorF, &cl_DeepSea, "DeepSea");

     Con::addConstant("Color::Seafoam", TypeColorF, &cl_Seafoam, "Bubbles/Foam");
     Con::addConstant("Color::Sand", TypeColorF, &cl_Sand, "Floor/Dirt");
     Con::addConstant("Color::Kelp", TypeColorF, &cl_Kelp, "Dark Underwater Plant");
     Con::addConstant("Color::NeonPink", TypeColorF, &cl_NeonPink, "NeonPink");
     Con::addConstant("Color::ElectricBlue", TypeColorF, &cl_ElectricBlue, "ElectricBlue");
     Con::addConstant("Color::AcidGreen", TypeColorF, &cl_AcidGreen, "AcidGreen");
     Con::addConstant("Color::Glass", TypeColorF, &cl_Glass, "Semi-transparent White");
     Con::addConstant("Color::Shadow", TypeColorF, &cl_Shadow, "Shadow Overlay");
     Con::addConstant("Color::Ghost", TypeColorF, &cl_Ghost, "Semi-transparent Blue");

}
// --------------------------------------------------------------------------
bool loadScript(String fileName) {
    if (!gLastScriptFile.isEmpty() && Con::isFunction("onLeaveScript")) {
      Con::executef("onLeaveScript",  gLastScriptFile);
    }
    if (Con::executeFile(fileName)) {
        if (Con::isFunction("onEnterScript")) Con::executef("onEnterScript",  fileName);
        gLastScriptFile = fileName;
        return true;
    }
    Log("[error] ElfTest:: FAILED TO LOAD SCRIPT: %s" , fileName.c_str());
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

DefineEngineFunction(getScreenRect, RectF, (), , "") {
    return {
        0.f, 0.f, //usually
        (F32)getScreenObject()->getWidth(),
        (F32)getScreenObject()->getHeight()
    };
}

DefineEngineFunction(getScreenCenterX, S32, (), , "") {
    return getScreenObject()->getCenterX();
}
DefineEngineFunction(getScreenCenterY, S32, (), , "") {
    return getScreenObject()->getCenterY();
}
DefineEngineFunction(getScreenCenter, Point2I, (), , "") {
    return getScreenObject()->getCenter();
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

DefineEngineFunction(RectIsValid, bool, (RectF rect),, "return if the rect is valid") {
    return rect.isValidRect();
}
//------------------------------------------------------------------------------
// Point/Vector stuff
//------------------------------------------------------------------------------
DefineEngineFunction(Vector2Add, Point2F, (Point2F a, Point2F b),,"a + b"){ return a + b;}
DefineEngineFunction(Vector2Sub, Point2F, (Point2F a, Point2F b),,"a - b"){ return a - b;}
DefineEngineFunction(Vector2Mul, Point2F, (Point2F a, Point2F b),,"a * b"){ return a * b;}
DefineEngineFunction(Vector2Div, Point2F, (Point2F a, Point2F b),,"a / b"){ return a / b;}

DefineEngineFunction(Vector2AddScalar, Point2F, (Point2F a, F32 b),,"a + b"){ return a + b;}
DefineEngineFunction(Vector2Subcalar, Point2F, (Point2F a, F32 b),,"a - b"){ return a - b;}
DefineEngineFunction(Vector2Scale, Point2F, (Point2F a, F32 b),,"a * b"){ return a * b;}
DefineEngineFunction(Vector2DivScalar, Point2F, (Point2F a, F32 b),,"a / b"){ return a / b;}

DefineEngineFunction(Vector2Dot, F32, (Point2F a, Point2F b),,"a.x * b.x + a.y * b.y"){ return a.dot ( b );}
DefineEngineFunction(Vector2Cross, F32, (Point2F a, Point2F b),,"a.x * b.x - a.y * b.y"){ return a.cross ( b );}
DefineEngineFunction(Vector2Distance, F32, (Point2F a, Point2F b),,"distance a b"){ return a.dist( b );}
DefineEngineFunction(Vector2Len, F32, (Point2F a),,"length of a"){ return a.len();}
DefineEngineFunction(Vector2Normalized, Point2F, (Point2F a),,"normalized a"){ return a.normalized();}



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

    Con::addConstant(
        "global::intvar",
        TypeS32,
        &intVar,
        ""
    );

}




#endif

ConsoleFunctionGroupEnd(App);
}


