//-----------------------------------------------------------------------------
// Copyright (c) 2025 Thomas Hühn (XXTH) 
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#pragma once
#ifndef _FLUXSHADER_H_
#define _FLUXSHADER_H_

#include "platform/fluxGL.h"
#include "core/fluxGlobals.h"
#include <unordered_map>
#include <string>
#include <functional>

class FluxShader {
public:
    FluxShader() : mProgram(0) {}
    ~FluxShader() { unload(); }

    bool load(const char* vSource, const char* fSource);

    std::function<void(GLuint)> OnBeforeLinkShader = nullptr;


    void use();
    void unload();

    // Setters for Uniforms (C++17 optimized)
    void setMat4(const std::string& name, const float* matrix);
    void setVec4(const std::string& name, const Color4F& color);
    void setVec2(const std::string& name, float x, float y);
    void setVec3(const std::string& name, float x, float y, float z);
    void setFloat(const std::string& name, float value);
    void setInt(const std::string& name, float value);
    void setBool(const std::string& name, bool value);
    void setFloatArray(const std::string& name, const float* values, int count);

    void setVec2Array(const std::string& name, const float* values, int count);
    void setVec3Array(const std::string& name, const float* values, int count);
    void setVec4Array(const std::string& name, const float* values, int count);


private:
    GLuint mProgram;
    std::unordered_map<std::string, GLint> mUniformLocs;

    GLint getLoc(const std::string& name);
    bool checkCompileErrors(GLuint shader, std::string type);
};

#endif
