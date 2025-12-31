/**************************************
*                                     *
*   Jeff Molofee's Basecode Example   *
*   SDL porting by Fabio Franchello   *
*          nehe.gamedev.net           *
*                2001                 *
*                                     *
***************************************
*                                     *
*   Basic Error Handling Routines:    *
*                                     *
*   InitErrorLog() Inits The Logging  *
*   CloseErrorLog() Stops It          *
*   Log() Is The Logging Funtion,     *
*   It Works Exactly Like printf()    *
*                                     *
***************************************
* XXTH 2012,2025:                     *
* Some fixed and modernisations added *
**************************************/


// Includes
#include <stdio.h>									// We Need The Standard IO Header
#include <stdlib.h>									// The Standard Library Header
#include <stdarg.h>									// And The Standard Argument Header For va_list
#include <errno.h>


#include "core/fluxGlobals.h"
#include <SDL3/SDL.h>
#include "utils/errorlog.h"

// Globals
static FILE *ErrorLog;								// The File For Error Logging
static char gLogFilePath[512] = {0};                // Remember where we write
//-----------------------------------------------------------------------------
int Log(const char *szFormat, ...)
{
	if (!szFormat) return -1;

	char targetStr[1024];
	va_list Arg;

	va_start(Arg, szFormat);
	SDL_vsnprintf(targetStr, sizeof(targetStr), szFormat, Arg);
	va_end(Arg);

	targetStr[sizeof(targetStr) - 1] = '\0';

	if (ErrorLog)
	{
		if (fprintf(ErrorLog, "%s\n", targetStr) < 0) {
			fclose(ErrorLog);
			ErrorLog = nullptr;
		}
		fflush(ErrorLog);
	}

#ifdef __EMSCRIPTEN__
	printf("%s\n", targetStr);
#else
	SDL_Log("%s", targetStr);
#endif

	return 0;
}

//-----------------------------------------------------------------------------
bool InitErrorLog(const char* log_file, const char* app_name, const char* app_version)
{
	// try current working directory first
	ErrorLog = fopen(log_file, "w");
	if (!ErrorLog) {
		SDL_Log("Can't open log file '%s' (%s). Trying SDL_GetPrefPath fallback.", log_file, strerror(errno));

		char* prefPath = SDL_GetPrefPath("ohmFlux", app_name ? app_name : "ohmFlux");
		if (prefPath) {
			snprintf(gLogFilePath, sizeof(gLogFilePath), "%s%s", prefPath, log_file);
			SDL_free(prefPath);

			ErrorLog = fopen(gLogFilePath, "w");
			if (!ErrorLog) {
				SDL_Log("Still failed to open log file at '%s' (%s). Logging to stdout only.",
				        gLogFilePath, strerror(errno));
			}
		} else {
			SDL_Log("SDL_GetPrefPath failed: %s", SDL_GetError());
		}
	} else {
		snprintf(gLogFilePath, sizeof(gLogFilePath), "%s", log_file);
	}

	Log("%s V%s -- Log Init...",
		app_name, app_version);
	if (gLogFilePath[0])
		SDL_Log("Writing ohmFlux log to: %s", gLogFilePath);

	return true;
}
//-----------------------------------------------------------------------------
void CloseErrorLog(void)
{
	if(ErrorLog)
	{
		Log("-- Closing Log...");
		fclose(ErrorLog);
		ErrorLog = nullptr; // Safety: prevent further write attempts
	}
}
