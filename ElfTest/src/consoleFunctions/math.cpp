
#include "console/console.h"
#include <platform/platformString.h>
#include "core/fluxMath.h"

ConsoleFunctionGroupBegin(Math, "Nath Functions: random numbers, ...");

/*! Gets a random integer number from min to max.
    @param min The minimum range of the random integer number.
    @param max The maximum range of the random integer number.
    @return A random integer number from min to max.
*/
ConsoleFunctionWithDocs(getRandom, ConsoleInt, 1, 3, ([ min ]?,[ max ]?))
{
    S32 min = dAtoi(argv[1]);
   if (argc == 2 && min > 0)
      return RandInt(0,min);
   else
   {
      if (argc == 3) 
      {

         S32 max = dAtoi(argv[2]);
         if (min > max) 
         {
            S32 t = min;
            min = max;
            max = t;
         }
         return RandInt(min, max);
      }
   }
   return RandInt(0, 100);
}

/*! Gets a random floating-point number from min to max.
    @param min The minimum range of the random floating-point number.
    @param max The maximum range of the random floating-point number.
    @return A random floating-point number from min to max.
*/
ConsoleFunctionWithDocs(getRandomF, ConsoleFloat, 1, 3, (min, max))
{
    if (argc == 3) {
          F32 min = dAtof(argv[1]);
        F32 max = dAtof(argv[2]);

        if ( min > max )
        {
            const F32 temp = min;
            min = max;
            max = temp;
        }

        return RandInRange(min, max);
    }

    return RandFloat();

}

//------------------------------------------------------------------------------

ConsoleFunctionGroupEnd(Math)

