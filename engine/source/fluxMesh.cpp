//-----------------------------------------------------------------------------
// Copyright (c) 2025 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#include "fluxGL.h"
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

void FluxMesh::createStatic(Vertex2D* vertices, U32 vCount, U32* indices, U32 iCount)
{
    mIndexCount = iCount;

    glGenVertexArrays(1, &mVAO);
    glGenBuffers(1, &mVBO);
    glGenBuffers(1, &mEBO);

    glBindVertexArray(mVAO);

    // Upload Vertex Data
    glBindBuffer(GL_ARRAY_BUFFER, mVBO);
    glBufferData(GL_ARRAY_BUFFER, vCount * sizeof(Vertex2D), vertices, GL_STATIC_DRAW);

    // Upload Index Data
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, iCount * sizeof(U32), indices, GL_STATIC_DRAW);

    // Attribute 0: Position (F32 position[3])
    // The offset is 0
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex2D), (void*)0);

    // Attribute 1: TexCoord (F32 texCoord[2])
    // The offset is the size of the position array (3 * 4 bytes = 12 bytes)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex2D), (void*)(3 * sizeof(F32)));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex2D), (void*)offsetof(Vertex2D, color));


    glBindVertexArray(0);
}
//-------------------------------------------------------------------------------
// 1. Initialize the "Storage" on the GPU
void FluxMesh::createEmpty(U32 maxVertices)
{
    mMaxVertices = maxVertices;

    glGenVertexArrays(1, &mVAO);
    glGenBuffers(1, &mVBO);
    glGenBuffers(1, &mEBO); // FIX 1: Generate the Index Buffer

    glBindVertexArray(mVAO);

    // Vertex Buffer
    glBindBuffer(GL_ARRAY_BUFFER, mVBO);
    glBufferData(GL_ARRAY_BUFFER, maxVertices * sizeof(Vertex2D), nullptr, GL_DYNAMIC_DRAW);

    // FIX 2: Bind the Index Buffer to the VAO
    // This allows glDrawElements to work later.
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEBO);

    // Attribute 0: Position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex2D), (void*)0);

    // Attribute 1: UV
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex2D), (void*)offsetof(Vertex2D, uv));

    // Attribute 2: Color
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex2D), (void*)offsetof(Vertex2D, color));

    // FIX 3: Do NOT unbind GL_ELEMENT_ARRAY_BUFFER before unbinding the VAO
    glBindVertexArray(0);
}

// 2. Update the "Content" of that storage
void FluxMesh::updateDynamic(Vertex2D* vertices, U32 vCount)
{
    if (mVAO == 0 || vertices == nullptr) return;

    // Guard against buffer overflow
    U32 countToUpdate = (vCount > mMaxVertices) ? mMaxVertices : vCount;

    glBindBuffer(GL_ARRAY_BUFFER, mVBO);

    // We use glBufferSubData because the memory was already
    // allocated with GL_DYNAMIC_DRAW in createEmpty.
    glBufferSubData(GL_ARRAY_BUFFER, 0, countToUpdate * sizeof(Vertex2D), vertices);
}

//-------------------------------------------------------------------------------
void FluxMesh::draw(U32 count, bool useIndices, GLenum mode) {
    U32 elementsToDraw = (count > 0) ? count : mIndexCount;
    if (mVAO == 0 || elementsToDraw == 0) return;

    glBindVertexArray(mVAO);

    // 2025 Standard: Transparency handling inside the draw call
    // Since this is a 2D mesh, we usually don't want it to block other sprites
    // glDepthMask(GL_FALSE);

    if (useIndices) {
        glDrawElements(mode, elementsToDraw, GL_UNSIGNED_INT, (void*)0);
    } else {
        glDrawArrays(mode, 0, elementsToDraw);
    }

    // glDepthMask(GL_TRUE); // Always reset state for the next clear/call
    glBindVertexArray(0);
}

// void FluxMesh::draw(U32 count, bool useIndices, GLenum mode)
// {
//     U32 elementsToDraw = (count > 0) ? count : mIndexCount;
//
//     if (mVAO != 0 && elementsToDraw > 0) {
//         glBindVertexArray(mVAO);
//
//         if (useIndices) {
//             // Default path: Sprites/Quads using EBO
//             glDrawElements(mode, elementsToDraw, GL_UNSIGNED_INT, (void*)0);
//         } else {
//             // Manual path: Lines/Circles using glDrawArrays
//             glDrawArrays(mode, 0, elementsToDraw);
//         }
//
//         glBindVertexArray(0);
//     }
// }


//-------------------------------------------------------------------------------
// update for batch rendering aproch
// void FluxMesh::createEmpty()
// {
//     glGenVertexArrays(1, &mVAO);
//     glGenBuffers(1, &mVBO);
//     // No EBO needed for glDrawArrays (Lines/Circles)
//
//     glBindVertexArray(mVAO);
//     glBindBuffer(GL_ARRAY_BUFFER, mVBO);
//
//     // batched attempt  glBufferData(GL_ARRAY_BUFFER, maxVertices * sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW);
//
//
//     // Set up the same attributes as createStatic
//     glEnableVertexAttribArray(0);
//     glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
//     glEnableVertexAttribArray(1);
//     glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3 * sizeof(F32)));
//
//     glBindVertexArray(0);
// }

// void FluxMesh::updateDynamic(Vertex* vertices, U32 vCount, GLenum usage) {
//     if (mVAO == 0) return;
//     glBindBuffer(GL_ARRAY_BUFFER, mVBO);
//     // Use glBufferSubData or glBufferData to update the GPU memory
//     glBufferData(GL_ARRAY_BUFFER, vCount * sizeof(Vertex), vertices, usage);
// }
// //-------------------------------------------------------------------------------
// void FluxMesh::draw()
// {
//     if (mVAO != 0) {
//         glBindVertexArray(mVAO);
//         glDrawElements(GL_TRIANGLES, mIndexCount, GL_UNSIGNED_INT, 0);
//         glBindVertexArray(0);
//     }
// }
