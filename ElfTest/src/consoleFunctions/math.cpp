
#include "console/console.h"
#include "core/fluxMath.h"

#include "console/engineAPI.h"

ConsoleFunctionGroupBegin(Math, "Math Functions: random numbers, ...");

/*! Gets a random integer number from min to max.
    @param min The minimum range of the random integer number.
    @param max The maximum range of the random integer number.
    @return A random integer number from min to max.
*/
DefineEngineFunction(getRandom, S32, (S32 min, S32 max) ,(0), "getRandom interger value params min,max")
{
    if (min > max) {
        S32 t = min;
        min = max;
        max = t;
    }
    return RandInt(min,max);
};


/*! Gets a random floating-point number from min to max.
    @param min The minimum range of the random floating-point number.
    @param max The maximum range of the random floating-point number.
    @return A random floating-point number from min to max.
*/
DefineEngineFunction(getRandomF, F32, (F32 min, F32 max) ,(0.f), "getRandom float value params min,max")
{
    if (min > max) {
        F32 t = min;
        min = max;
        max = t;
    }
    return RandInRange(min,max);
};

//------------------------------------------------------------------------------

ConsoleFunctionGroupEnd(Math);

