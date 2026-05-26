#include "console/console.h"
#include <platform/platformString.h>
#include "appMain.h"
#include "core/fluxGlue.h"
#include <string>

namespace KorkFlux {
ConsoleFunctionGroupBegin(App, "App Functions: getFPS, ...");

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


//------------------------------------------------------------------------------

ConsoleFunctionGroupEnd(App)
}
