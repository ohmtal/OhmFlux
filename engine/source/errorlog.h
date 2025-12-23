#pragma once
#ifndef _ERRORLOG_H
#define _ERRORLOG_H

#ifdef WIN32																// If We're Under MSVC
#include <windows.h>														// We Need The Windows Header
#else																		// Otherwhise
#include <stdio.h>															// We're Including The Standard IO Header
#include <stdlib.h>															// And The Standard Lib Header
#include <string.h>															// And The String Lib Header
#endif																		// Then...

// dLog
#ifdef FLUX_DEBUG
#define dLog(...) Log(__VA_ARGS__)
#else
#define dLog(...) (void)0
#endif

bool InitErrorLog(const char* log_file, const char* app_name, const char* app_version);	// Initializes The Error Log
void CloseErrorLog(void);									// Closes The Error Log
int  Log(const char *, ...);										// Uses The Error Log :)

#endif //_ERRORLOG_H
