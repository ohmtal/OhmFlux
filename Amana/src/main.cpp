#include <stdio.h>
#include "amanaGame.h"
#include <SDL3/SDL_main.h> //<<< Android! and Windows need that
//--------------------------------------------------------------------------------------
// int main(int argc, char **argv)
int main(int argc, char *argv[])
{

   AmanaGameMainLoop();

    exit(0);
    return 0;
}
