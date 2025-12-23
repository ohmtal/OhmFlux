//-----------------------------------------------------------------------------
// Copyright (c) 2025 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#include "fluxShader.h"

#include "fluxGL.h"
#include "errorlog.h"

//-------------------------------------------------------------------------------
bool FluxShader::load(const char* vSource, const char* fSource)
{
    GLuint sVertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(sVertex, 1, &vSource, nullptr);
    glCompileShader(sVertex);
    if (!checkCompileErrors(sVertex, "VERTEX")) return false;

    GLuint sFragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(sFragment, 1, &fSource, nullptr);
    glCompileShader(sFragment);
    if (!checkCompileErrors(sFragment, "FRAGMENT")) return false;

    mProgram = glCreateProgram();
    glAttachShader(mProgram, sVertex);
    glAttachShader(mProgram, sFragment);
    glLinkProgram(mProgram);

    bool linked = checkCompileErrors(mProgram, "PROGRAM");

    glDeleteShader(sVertex);
    glDeleteShader(sFragment);
    return linked;
}
//-------------------------------------------------------------------------------
void FluxShader::use()
{
    if (mProgram) glUseProgram(mProgram);
}
//-------------------------------------------------------------------------------
void FluxShader::unload()
{
    if (mProgram) {
        glDeleteProgram(mProgram);
        mProgram = 0;
    }
    mUniformLocs.clear();
}
//-------------------------------------------------------------------------------
GLint FluxShader::getLoc(const std::string& name)
{
    if (mUniformLocs.count(name)) return mUniformLocs[name];
    GLint loc = glGetUniformLocation(mProgram, name.c_str());
    mUniformLocs[name] = loc;
    return loc;
}
//-------------------------------------------------------------------------------
void FluxShader::setMat4(const std::string& name, const float* matrix)
{
    glUniformMatrix4fv(getLoc(name), 1, GL_FALSE, matrix);
}
//-------------------------------------------------------------------------------
void FluxShader::setVec4(const std::string& name, const Color4F& color)
{
    glUniform4f(getLoc(name), color.r, color.g, color.b, color.a);
}
//-------------------------------------------------------------------------------
void FluxShader::setVec2(const std::string& name, float x, float y)
{
    glUniform2f(getLoc(name), x, y);
}
//-------------------------------------------------------------------------------
void FluxShader::setFloat(const std::string& name, float value)
{
    glUniform1f(getLoc(name), value);
}
//-------------------------------------------------------------------------------
bool FluxShader::checkCompileErrors(GLuint shader, std::string type)
{
    GLint success;
    GLchar infoLog[1024];
    if (type != "PROGRAM") {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
            Log("SHADER_COMPILATION_ERROR of type: %s\n%s", type.c_str(), infoLog);
        }
    } else {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader, 1024, nullptr, infoLog);
            Log("PROGRAM_LINKING_ERROR of type: %s\n%s", type.c_str(), infoLog);
        }
    }
    return success;
}
