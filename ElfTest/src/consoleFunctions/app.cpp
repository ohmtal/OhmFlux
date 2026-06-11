#include "appMain.h"
#include "core/fluxGlue.h"
#include "core/Globals.h"

#include "console/engineAPI.h"
#include "console/script.h"

#include <string>

namespace ElfFlux {


// --------------------------------------------------------------------------
void foo() {
    Con::setVariable("foo", "bar");
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

// ConsoleFunction(quit, void, 1,1, "") {
//     return gMain->TerminateApplication();
// }
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
//------------------------------------------------------------------------------
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
#endif

ConsoleFunctionGroupEnd(App);
}
