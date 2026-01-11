#pragma once
#ifndef _ERRORLOG_H
#define _ERRORLOG_H

#include "core/fluxGlobals.h"

#ifdef WIN32																// If We're Under MSVC
#include <windows.h>														// We Need The Windows Header
#else																		// Otherwhise
#include <stdio.h>															// We're Including The Standard IO Header
#include <stdlib.h>															// And The Standard Lib Header
#include <string.h>															// And The String Lib Header
#endif																		// Then...

#include <format>
#include <string_view>


// dLog
#ifdef FLUX_DEBUG
#define dLog(fmt, ...) Log(fmt, ##__VA_ARGS__)
#else
#define dLog(fmt, ...) ((void)0)
#endif

bool InitErrorLog(const char* log_file, const char* app_name, const char* app_version) ;	// Initializes The Error Log
void CloseErrorLog(void);									// Closes The Error Log
int  Log(const char *, ...) PRINTF_CHECK(1, 2);										// Uses The Error Log :)

template<typename... Args>
int LogFMT(std::string_view fmt, Args&&... args) {
    try {
        // Modern C++20 formatting
        std::string s = std::vformat(fmt, std::make_format_args(args...));

        // Pass the already-formatted string to your existing C-style Log
        // This avoids duplicating the file/console logging logic
        return Log("%s", s.c_str());
    } catch (const std::format_error& e) {
        return Log("LogFMT Error: %s", e.what());
    }
}

#endif //_ERRORLOG_H
