//-----------------------------------------------------------------------------
// Copyright (c) 2025 Thomas Hühn (XXTH) 
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#include "platform/fluxGL.h"
#include "fluxMesh.h"
//-------------------------------------------------------------------------------
FluxMesh::FluxMesh() : mVAO(0), mVBO(0), mEBO(0), mIndexCount(0)
{
}

FluxMesh::~FluxMesh()
{
    unload();
}
//-------------------------------------------------------------------------------
void FluxMesh::unload()
{
    if (mVAO != 0) {
        glDeleteVertexArrays(1, &mVAO);
        mVAO = 0;
    }
    if (mVBO != 0) {
        glDeleteBuffers(1, &mVBO);
        mVBO = 0;
    }
    if (mEBO != 0) {
        glDeleteBuffers(1, &mEBO);
        mEBO = 0;
    }
    mIndexCount = 0;
    mMaxVertices = 0;
}
//-------------------------------------------------------------------------------
// ON GLES2 indices must be U16* !!!!
void FluxMesh::createStatic(Vertex2D* vertices, U32 vCount, void* indices, U32 iCount)
{
    mIndexCount = iCount;

    #ifndef FLUX_GLES2
    glGenVertexArrays(1, &mVAO);
    glBindVertexArray(mVAO);
    #endif

    glGenBuffers(1, &mVBO);
    glGenBuffers(1, &mEBO);

    // Vertex Data
    glBindBuffer(GL_ARRAY_BUFFER, mVBO);
    glBufferData(GL_ARRAY_BUFFER, vCount * sizeof(Vertex2D), vertices, GL_STATIC_DRAW);

    // Index Data (Unterscheidung U16 vs U32)
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEBO);
    #ifdef FLUX_GLES2
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, iCount * sizeof(U16), indices, GL_STATIC_DRAW);
    #else
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, iCount * sizeof(U32), indices, GL_STATIC_DRAW);
    #endif

    #ifndef FLUX_GLES2
    // Attribute nur im VAO speichern, wenn GLES3+
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex2D), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex2D), (void*)(3 * sizeof(F32)));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex2D), (void*)offsetof(Vertex2D, color));

    glBindVertexArray(0);
    #else
    // Buffer entbinden für GLES2
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    #endif
}

// void FluxMesh::createStatic(Vertex2D* vertices, U32 vCount, U32* indices, U32 iCount)
// {
//     mIndexCount = iCount;
//
//     glGenVertexArrays(1, &mVAO);
//     glGenBuffers(1, &mVBO);
//     glGenBuffers(1, &mEBO);
//
//     glBindVertexArray(mVAO);
//
//     // Upload Vertex Data
//     glBindBuffer(GL_ARRAY_BUFFER, mVBO);
//     glBufferData(GL_ARRAY_BUFFER, vCount * sizeof(Vertex2D), vertices, GL_STATIC_DRAW);
//
//     // Upload Index Data
//     glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEBO);
//     glBufferData(GL_ELEMENT_ARRAY_BUFFER, iCount * sizeof(U32), indices, GL_STATIC_DRAW);
//
//     // Attribute 0: Position (F32 position[3])
//     // The offset is 0
//     glEnableVertexAttribArray(0);
//     glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex2D), (void*)0);
//
//     // Attribute 1: TexCoord (F32 texCoord[2])
//     // The offset is the size of the position array (3 * 4 bytes = 12 bytes)
//     glEnableVertexAttribArray(1);
//     glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex2D), (void*)(3 * sizeof(F32)));
//
//     glEnableVertexAttribArray(2);
//     glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex2D), (void*)offsetof(Vertex2D, color));
//
//
//     glBindVertexArray(0);
// }
//-------------------------------------------------------------------------------
// 1. Initialize the "Storage" on the GPU
void FluxMesh::createEmpty(U32 maxVertices) {
    mMaxVertices = maxVertices;

    #ifndef FLUX_GLES2
    glGenVertexArrays(1, &mVAO);
    glBindVertexArray(mVAO);
    #endif

    glGenBuffers(1, &mVBO);
    glGenBuffers(1, &mEBO);

    // Vertex Buffer init
    glBindBuffer(GL_ARRAY_BUFFER, mVBO);
    glBufferData(GL_ARRAY_BUFFER, maxVertices * sizeof(Vertex2D), nullptr, GL_DYNAMIC_DRAW);

    // Index Buffer init
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEBO);
    // NOTE: EBO GLES 2.0 not saved in VAO

    #ifndef FLUX_GLES2
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex2D), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex2D), (void*)offsetof(Vertex2D, uv));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex2D), (void*)offsetof(Vertex2D, color));

    glBindVertexArray(0);
    #else
    //  GLES 2.0: use buffer
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    #endif
}

// void FluxMesh::createEmpty(U32 maxVertices)
// {
//     mMaxVertices = maxVertices;
//
//     glGenVertexArrays(1, &mVAO);
//     glGenBuffers(1, &mVBO);
//     glGenBuffers(1, &mEBO); // FIX 1: Generate the Index Buffer
//
//     glBindVertexArray(mVAO);
//
//     // Vertex Buffer
//     glBindBuffer(GL_ARRAY_BUFFER, mVBO);
//     glBufferData(GL_ARRAY_BUFFER, maxVertices * sizeof(Vertex2D), nullptr, GL_DYNAMIC_DRAW);
//
//     // FIX 2: Bind the Index Buffer to the VAO
//     // This allows glDrawElements to work later.
//     glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEBO);
//
//     // Attribute 0: Position
//     glEnableVertexAttribArray(0);
//     glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex2D), (void*)0);
//
//     // Attribute 1: UV
//     glEnableVertexAttribArray(1);
//     glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex2D), (void*)offsetof(Vertex2D, uv));
//
//     // Attribute 2: Color
//     glEnableVertexAttribArray(2);
//     glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex2D), (void*)offsetof(Vertex2D, color));
//
//     // FIX 3: Do NOT unbind GL_ELEMENT_ARRAY_BUFFER before unbinding the VAO
//     glBindVertexArray(0);
// }


void FluxMesh::updateDynamic(Vertex2D* vertices, U32 vCount)
{
    // mVBO must exist, mVAO can be ignored for the update
    if (mVBO == 0 || vertices == nullptr) return;

    U32 countToUpdate = (vCount > mMaxVertices) ? mMaxVertices : vCount;

    glBindBuffer(GL_ARRAY_BUFFER, mVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, countToUpdate * sizeof(Vertex2D), vertices);
    glBindBuffer(GL_ARRAY_BUFFER, 0); // Optional: Unbind für Sicherheit
}

// 2. Update the "Content" of that storage
// void FluxMesh::updateDynamic(Vertex2D* vertices, U32 vCount)
// {
//     if (mVAO == 0 || vertices == nullptr) return;
//
//     // Guard against buffer overflow
//     U32 countToUpdate = (vCount > mMaxVertices) ? mMaxVertices : vCount;
//
//     glBindBuffer(GL_ARRAY_BUFFER, mVBO);
//
//     // We use glBufferSubData because the memory was already
//     // allocated with GL_DYNAMIC_DRAW in createEmpty.
//     glBufferSubData(GL_ARRAY_BUFFER, 0, countToUpdate * sizeof(Vertex2D), vertices);
// }


//-------------------------------------------------------------------------------
void FluxMesh::draw(U32 count, bool useIndices, GLenum mode) {
    U32 elementsToDraw = (count > 0) ? count : mIndexCount;
    if (elementsToDraw == 0) return;

#ifdef FLUX_GLES2
    glBindBuffer(GL_ARRAY_BUFFER, mVBO);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex2D), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex2D), (void*)offsetof(Vertex2D, uv));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex2D), (void*)offsetof(Vertex2D, color));

    if (useIndices) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEBO);
        glDrawElements(mode, elementsToDraw, GL_UNSIGNED_SHORT, (void*)0);
    } else {
        glDrawArrays(mode, 0, elementsToDraw);
    }

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
#else
    // GLES 3.0
    glBindVertexArray(mVAO);
    if (useIndices) {
        glDrawElements(mode, elementsToDraw, GL_UNSIGNED_INT, (void*)0);
    } else {
        glDrawArrays(mode, 0, elementsToDraw);
    }
    glBindVertexArray(0);
#endif
}


// void FluxMesh::draw(U32 count, bool useIndices, GLenum mode) {
//     U32 elementsToDraw = (count > 0) ? count : mIndexCount;
//     if (mVAO == 0 || elementsToDraw == 0) return;
//
//     glBindVertexArray(mVAO);
//
//     if (useIndices) {
// #ifdef FLUX_GLES2
//         glDrawElements(mode, elementsToDraw, GL_UNSIGNED_SHORT, (void*)0);
// #else
//         glDrawElements(mode, elementsToDraw, GL_UNSIGNED_INT, (void*)0);
// #endif
//
//     } else {
//         glDrawArrays(mode, 0, elementsToDraw);
//     }
//
//     glBindVertexArray(0);
// }
