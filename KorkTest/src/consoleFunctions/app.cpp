#include "console/console.h"
#include <platform/platformString.h>
#include "appMain.h"
#include "core/fluxGlue.h"
#include <string>
#include "core/Globals.h"

namespace KorkFlux {
ConsoleFunctionGroupBegin(App, "App Functions: getFPS, ...");

ConsoleFunction(getFullScreen, ConsoleBool, 1,1, "") {
    return getScreenObject()->getFullScreen();
}
ConsoleFunction(setFullScreen, ConsoleBool, 2,2, "bool value") {
    return getScreenObject()->setFullScreen(dAtob(argv[1]));
}

ConsoleFunction(getFrameTime, ConsoleFloat, 1,1, "") {
    return getFrameTime();
}

ConsoleFunction(getFPS, ConsoleInt, 1,1, "") {
    return gMain->getFPS();
}

ConsoleFunction(getScreenWidth, ConsoleInt, 1,1, "") {
    return getScreenObject()->getWidth();
}

ConsoleFunction(getScreenHeight, ConsoleInt, 1,1, "") {
    return getScreenObject()->getHeight();
}

ConsoleFunction(setVSync, void, 2,2, "bool value") {
    return getScreenObject()->setVSync(dAtob(argv[1]));
}

ConsoleFunction(quit, void, 1,1, "") {
    return gMain->TerminateApplication();
}


#ifdef FLUX_DEBUG
ConsoleFunction(testString, ConsoleString, 1,1,"TEST lazy string"){
    // return getReturnString("Hello String", vmPtr);
    char rbuf[256] = {0};
    dSprintf(rbuf, 256, "%d %d",815, 4711);
    return getReturnString(rbuf, vmPtr);
}
#endif
//------------------------------------------------------------------------------

ConsoleFunctionGroupEnd(App)
}
