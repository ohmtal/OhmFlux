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
**************************************/


// Includes
#include <stdio.h>									// We Need The Standard IO Header
#include <stdlib.h>									// The Standard Library Header
#include <stdarg.h>									// And The Standard Argument Header For va_list
#include <errno.h>

#include "fluxGlobals.h"
#include <SDL3/SDL.h>
#include "errorlog.h"

// Globals
static FILE *ErrorLog;								// The File For Error Logging
static char gLogFilePath[512] = {0};                // Remember where we write

// Code
int Log(const char *szFormat, ...)						// Add A Line To The Log
{
	char targetStr[512];
	va_list Arg;									// We're Using The Same As The printf() Family, A va_list
													// To Substitute The Tokens Like %s With Their Value In The Output
	va_start(Arg,szFormat);							// We Start The List
	vsprintf(targetStr, szFormat, Arg);
	va_end(Arg);									// We End The List

	if(ErrorLog)									// If The Log Is Open
	{
		fprintf(ErrorLog, "%s", targetStr);			// We Use vprintf To Perform Substituctions
		fflush(ErrorLog);							// And Ensure The Line Is Written, The Log Must Be Quick
	}

	// SDL_Log("%s", targetStr);
	printf("%s\n", targetStr);

	return 0;										// And Return A Ok
}



bool InitErrorLog(const char* log_file, const char* app_name, const char* app_version)								// Initializes Error Logging
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
		app_name, app_version);						// We Print The Name Of The App In The Log
	if (gLogFilePath[0])
		SDL_Log("Writing ohmFlux log to: %s", gLogFilePath);

	return true;									// Otherwhise Return TRUE (Everything Went OK)
}

void CloseErrorLog(void)							// Closes Error Logging
{
	Log("-- Closing Log...");					// Print The End Mark

	if(ErrorLog)									// If The File Is Open
	{
		fclose(ErrorLog);							// Close It
	}

	return;											// And Return, Quite Plain Huh? :)
}

