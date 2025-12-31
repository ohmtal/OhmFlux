//-----------------------------------------------------------------------------
// Copyright (c) 2025 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#pragma once
#ifndef _FLUXMESH_H_
#define _FLUXMESH_H_

#include "platform/fluxGL.h"
#include "core/fluxGlobals.h"

/**
 * @class FluxMesh
 * @brief Manages OpenGL Vertex Array Objects (VAO) and Buffer Objects (VBO/EBO).
 *
 * This class encapsulates the GPU resources required to render geometry
 * using the global 'Vertex' structure defined in fluxGlobals.h.
 */
class FluxMesh {
public:
    FluxMesh();
    ~FluxMesh();

    // Disable copy constructor and assignment operator to prevent
    // accidental duplication of GPU handles.
    FluxMesh(const FluxMesh&) = delete;

    void operator=(const FluxMesh&) = delete;

    void createStatic(Vertex2D* vertices, U32 vCount, U32* indices, U32 iCount);

    void updateDynamic(Vertex2D* vertices, U32 vCount); //, GLenum usage = GL_DYNAMIC_DRAW);

    void draw(U32 count = 0, bool useIndices = true, GLenum mode = GL_TRIANGLES);


    void unload();

    // Initializer for dynamic meshes (creates handles without data)
    // void createEmpty();
    void createEmpty(U32 maxVertices);

    // Getter for the VAO handle
    GLuint getVAO() const { return mVAO; }
    GLuint getVBO() const { return mVBO; }
    GLuint getEBO() const { return mEBO; }

private:
    GLuint mVAO;         ///< Handle for the Vertex Array Object.
    GLuint mVBO;         ///< Handle for the Vertex Buffer Object (Vertex data).
    GLuint mEBO;         ///< Handle for the Element Buffer Object (Index data).
    U32    mIndexCount;  ///< Number of indices to draw.
    U32    mMaxVertices; ///< maxVertices guessed or known for the VBO
};

#endif
