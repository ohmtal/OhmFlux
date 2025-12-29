//-----------------------------------------------------------------------------
// Copyright (c) 2025 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#pragma once


#if defined(__EMSCRIPTEN__) || defined(__ANDROID__)
#include <GLES3/gl3.h>
#else
#include <GL/glew.h>
#endif

#include <SDL3/SDL_opengl.h>

#include "errorlog.h"

//----------------------------------------------------------------------------
//  OpenGL Debug
//----------------------------------------------------------------------------
//
// HOWTO:
//     // Enable it after glewInit()
//     if (glDebugMessageCallback) {
//         glEnable(GL_DEBUG_OUTPUT);
//         glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); // Ensures error is reported exactly when it happens
//         glDebugMessageCallback(glDebugOutput, nullptr);
//         glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
//     }
#ifndef APIENTRY
#ifdef _WIN32
#define APIENTRY __stdcall
#else
#define APIENTRY
#endif
#endif

inline void APIENTRY glDebugOutput(GLenum source, GLenum type, GLuint id, GLenum severity,
                                   GLsizei length, const GLchar* message, const void* userParam) {
    // Ignore non-significant error codes
    if(id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

    Log( "--------------- [OpenGL Debug Message] ---------------");
    Log( "Message: %s", message);
    Log( "Source: %x | Type: %x | Severity: %x", source, type, severity);
    Log( "------------------------------------------------------");

}
//-------------------------------------------------------------------------------
inline void checkGLError(const char* file, int line) {
    GLenum err = glGetError();
    while (err != GL_NO_ERROR) {
        Log( "[OpenGL Error] %s:%d GL Error Code: %d", file, line, err);
        err = glGetError(); // Clear all errors in the queue
    }
}
