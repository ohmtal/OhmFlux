//-----------------------------------------------------------------------------
// Copyright (c) 2012/2025 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#ifdef __EMSCRIPTEN__
#include <GLES3/gl3.h>
#else
#include <GL/glew.h>
#endif

#include <cmath>
#include <algorithm>

#include "fluxRender2D.h"
#include "fluxShaderSources.h"
#include "fluxMath.h"
#include "errorlog.h"
#include "misc.h"
#include "fluxLightManager.h"
//-------------------------------------------------------------------------------
bool FluxRender2D::init(U32 maxSprites)
{
    mMaxSprites = maxSprites;

    // 1. Setup GL States
    glDepthMask(GL_FALSE); // Optional: ensure they don't block each other
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // 2. Load Default Sprite Shader
    if (!mDefaultShader.load(vertexShaderSource, fragmentShaderSource)) {
        Log("FluxRender2D: Failed to load Default Sprite Shader");
        mShaderFailed = true;
        return false;
    }

    // 3. Load Flat Color Shader (for primitives)
    if (!mFlatShader.load(flatVertexShaderSource, flatFragmentShaderSource)) {
        Log("FluxRender2D: Failed to load Flat Color Shader");
        mShaderFailed = true;
        return false;
    }
    //--------

    // 4. Create the Batch Quad Mesh
    const U32 MAX_VERTICES = mMaxSprites * 4;
    const U32 MAX_INDICES = mMaxSprites * 6;

    // Allocate a large EMPTY VBO for vertices
    mQuadMesh.createEmpty(MAX_VERTICES);

    // Generate the repeating Index Pattern: 0,1,3, 1,2,3, 4,5,7, 5,6,7...
    std::vector<U32> quadIndices;
    quadIndices.reserve(MAX_INDICES);
    for (U32 i = 0; i < mMaxSprites; i++) {
        U32 offset = i * 4;
        quadIndices.push_back(offset + 0);
        quadIndices.push_back(offset + 1);
        quadIndices.push_back(offset + 3);
        quadIndices.push_back(offset + 1);
        quadIndices.push_back(offset + 2);
        quadIndices.push_back(offset + 3);
    }

    // Upload the Index Buffer once (it never changes)
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mQuadMesh.getEBO());
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, quadIndices.size() * sizeof(U32), quadIndices.data(), GL_STATIC_DRAW);

    //--------

    // Initialize the dynamic line mesh
    mLineMesh.createEmpty(10000);

    // // white pixel trick >>>>>
    // --------------------
    U32 whitePixel = 0xFFFFFFFF;
    glGenTextures(1, &mWhiteTextureHandle);
    glBindTexture(GL_TEXTURE_2D, mWhiteTextureHandle);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &whitePixel);

    // Set parameters (Nearest is best for 1x1)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // 2. Wrap it so the Batcher can use it
    mWhiteTextureWrapper = new FluxTexture();
    mWhiteTextureWrapper->setManual(mWhiteTextureHandle, 1, 1);

    //<<<< whitePixel trick

    mShaderFailed = false;
    Log("FluxRender2D: Initialized successfully.");
    return true;
}
//-------------------------------------------------------------------------------
void FluxRender2D::shutdown()
{
    // Only clean up if we aren't already shut down
    if (!mShaderFailed)
    {
        dLog("FluxRender2D: Cleaning up GPU resources...");

        // 1. Unload Shaders (calls glDeleteProgram internally)
        mDefaultShader.unload();
        mFlatShader.unload();

        // 2. Unload Meshes (calls glDeleteVertexArrays/Buffers internally)
        mQuadMesh.unload();

        // If you added a line mesh later:
        mLineMesh.unload();

        SAFE_DELETE(mDefaultCamera);
        mActiveCamera = nullptr;

        if (mWhiteTextureWrapper) {
            SAFE_DELETE(mWhiteTextureWrapper);
            mWhiteTextureHandle = 0; // Handled by FluxTexture destructor
        }

        dLog("FluxRender2D: Shutdown complete.");

        // 3. Mark as uninitialized
        mShaderFailed = true;
    }
}
//-------------------------------------------------------------------------------
void FluxRender2D::drawLine(F32 x1, F32 y1, F32 x2, F32 y2, const Color4F& color) {
    PrimitiveCommand cmd;
    cmd.type = PrimitiveCommand::LINE;
    cmd.p1 = {x1, y1, 0};
    cmd.p2 = {x2, y2, 0};
    cmd.color = color;
    mPrimitiveList.push_back(cmd);
}

void FluxRender2D::executeDrawLine(const PrimitiveCommand& cmd)
{
    // 1. Prepare Vertices from the command data
    Vertex2D lineVerts[2] = {
        { cmd.p1, {0.0f, 0.0f}, cmd.color },
        { cmd.p2, {0.0f, 0.0f}, cmd.color }
    };

    // 2. Setup Shader & State
    // Note: We use mDefaultShader to ensure Location 2 (Color) is active
    mDefaultShader.use();
    mDefaultShader.setMat4("projection", mOrtho);
    mDefaultShader.setMat4("view", mActiveCamera ? mActiveCamera->getViewMatrix() : IDENTITY_MATRIX);

    // 3. Bind the White Texture so the shader doesn't render black
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mWhiteTextureHandle);

    // 4. Update the VBO and Draw
    mLineMesh.updateDynamic(lineVerts, 2);

    // false = no indices, GL_LINES = primitive type
    mLineMesh.draw(2, false, GL_LINES);
}


//-------------------------------------------------------------------------------
void FluxRender2D::drawRect(F32 x, F32 y, F32 w, F32 h, const Color4F& color, bool filled) {
    if (!filled) {
        // Draw 4 lines using your existing drawLine
        drawLine(x, y, x + w, y, color);         // Top
        drawLine(x + w, y, x + w, y + h, color); // Right
        drawLine(x + w, y + h, x, y + h, color); // Bottom
        drawLine(x, y + h, x, y, color);         // Left
        return;
    }

    // 2025 High-Performance Approach:
    // Treat a filled rect as a sprite using the white pixel texture.
    DrawParams2D dp;
    dp.x = x + w * 0.5f; // Center X
    dp.y = y + h * 0.5f; // Center Y
    dp.z = 0.0f;         // Or a specific layer
    dp.w = w;
    dp.h = h;
    dp.color = color;

    // You need a dummy Image object that wraps mWhiteTextureHandle
    // If you don't have one, create a temporary Image wrapper
    dp.image = mWhiteTextureWrapper;
    dp.imgId = 0; // Usually 0 for 1x1 textures

    // This pushes it into the Big List automatically!
    drawSprite(dp);
}

//-------------------------------------------------------------------------------
void FluxRender2D::drawCircle(F32 cx, F32 cy, F32 radius, const Color4F& color, U32 segments) {
    if (mShaderFailed) return;

    PrimitiveCommand cmd;
    cmd.type = PrimitiveCommand::CIRCLE;
    cmd.p1 = { cx, cy, 0.0f }; // Use p1 for center
    cmd.radius = radius;
    cmd.segments = (segments > 128) ? 128 : segments; // Safety limit
    cmd.color = color;

    mPrimitiveList.push_back(cmd);
}

void FluxRender2D::executeDrawCircle(const PrimitiveCommand& cmd)
{
    std::vector<Vertex2D> circleVerts;
    circleVerts.reserve(cmd.segments);

    // 1. Generate the circle points
    for (U32 i = 0; i < cmd.segments; i++) {
        F32 theta = 2.0f * 3.1415926f * (F32(i) / F32(cmd.segments));

        Vertex2D v;
        // cmd.p1 is our center (cx, cy)
        v.pos = { cmd.p1.x + cosf(theta) * cmd.radius,
            cmd.p1.y + sinf(theta) * cmd.radius, 0.0f };
            v.uv  = { 0.0f, 0.0f };
            v.color = cmd.color;
            circleVerts.push_back(v);
    }

    // 2. Setup Shader & Texture
    mDefaultShader.use();
    mDefaultShader.setMat4("projection", mOrtho);
    mDefaultShader.setMat4("view", mActiveCamera ? mActiveCamera->getViewMatrix() : IDENTITY_MATRIX);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mWhiteTextureHandle);

    // 3. Update VBO and Draw
    // Note: We use mLineMesh which was reserved for 10,000 vertices
    mLineMesh.updateDynamic(circleVerts.data(), (U32)circleVerts.size());

    // Use glDrawArrays (false) with GL_LINE_LOOP
    mLineMesh.draw((U32)circleVerts.size(), false, GL_LINE_LOOP);
}
//-------------------------------------------------------------------------------
void FluxRender2D::drawTriangle(Point3F p1, Point3F p2, Point3F p3, const Color4F& color, bool filled) {
    if (mShaderFailed) return;

    PrimitiveCommand cmd;
    cmd.type = PrimitiveCommand::TRIANGLE;
    cmd.points[0] = p1;
    cmd.points[1] = p2;
    cmd.points[2] = p3;
    cmd.color = color;
    cmd.filled = filled;

    mPrimitiveList.push_back(cmd);
}

void FluxRender2D::executeDrawTriangle(const PrimitiveCommand& cmd) {
    Vertex2D triVerts[3];
    for (int i = 0; i < 3; i++) {
        triVerts[i].pos = cmd.points[i];
        triVerts[i].uv  = { 0.0f, 0.0f };
        triVerts[i].color = cmd.color;
    }

    mDefaultShader.use();
    mDefaultShader.setMat4("projection", mOrtho);
    mDefaultShader.setMat4("view", mActiveCamera ? mActiveCamera->getViewMatrix() : IDENTITY_MATRIX);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mWhiteTextureHandle);

    mLineMesh.updateDynamic(triVerts, 3);

    if (cmd.filled) {
        // Filled: Draw as a single triangle
        mLineMesh.draw(3, false, GL_TRIANGLES);
    } else {
        // Outline: Draw as a connected loop
        mLineMesh.draw(3, false, GL_LINE_LOOP);
    }
}

//-------------------------------------------------------------------------------
void FluxRender2D::updateOrto(S32 width, S32 height)
{

    if (!mActiveCamera && !mDefaultCamera)
    {
        mDefaultCamera = new FluxCamera(width, height);
        mActiveCamera = mDefaultCamera;
    } else {
        if (mActiveCamera)
            mActiveCamera->onResize(width, height);
        else {
            //mhhh bad situation where is my camera ?
        }

    }

    createOrthoMatrix(0.0f, width, height, 0.0f, -100.0f, 100.0f, mOrtho);
}
//-------------------------------------------------------------------------------
bool FluxRender2D::uglyDraw2DStretch (
    FluxTexture* limg,
    const S32& limgId,
    const float& lx,
    const float& ly,
    const float& lz,
    const S32& lw,
    const S32& lh,
    const float& lrot,
    const bool& lflipX,
    const bool& lflipY,
    const F32& lAlpha,  	// = 0.1f,
    const bool& Obsolete_doBlend_ALLWAYS_TRUE ,  //=true,
    const Color4F& lColor   //= { 1.0f, 1.0f, 1.0f, 1.0f }
)
{
    DrawParams2D dp; // Creates the object
    dp.image = limg;
    dp.imgId = limgId;
    dp.x = lx;
    dp.y = ly;
    dp.z = lz;
    dp.w = lw;
    dp.h = lh;
    dp.rot = lrot;
    dp.flipX = lflipX;
    dp.flipY = lflipY;
    dp.alpha = lAlpha;
    dp.color = lColor;

    return drawSprite(dp);
}
//-------------------------------------------------------------------------------
void FluxRender2D::renderLights()
{
    const std::vector<FluxLight>& lights = LightManager.getLights();
    RectF view = Render2D.getCamera()->getVisibleWorldRect(false);

    int activeLightCount = 0;

    for (size_t i = 0; i < lights.size(); ++i) {
        //  Safety check: Stop if we've filled the shader's available slots
        if (activeLightCount >= MAX_LIGHTS) {
            break;
        }

        //  Frustum/Visibility Culling
        if (!checkAABBIntersectionF(view, lights[i].getRectF())) {
            continue;
        }

        //  Pass data using the activeLightCount index, NOT the loop index i
        std::string lightPrefix = "uLights[" + std::to_string(activeLightCount) + "]";

        mDefaultShader.setVec3((lightPrefix + ".position").c_str(), lights[i].position.x, lights[i].position.y, lights[i].position.z);
        mDefaultShader.setVec4((lightPrefix + ".color").c_str(), lights[i].color);
        mDefaultShader.setFloat((lightPrefix + ".radius").c_str(), lights[i].radius);
        mDefaultShader.setVec2((lightPrefix + ".direction").c_str(), lights[i].direction.x, lights[i].direction.y);
        mDefaultShader.setFloat((lightPrefix + ".cutoff").c_str(), lights[i].cutoff);

        activeLightCount++;
    }

    //  Tell the shader exactly how many lights were actually uploaded
    mDefaultShader.setInt("uNumLights", activeLightCount);

    // dLog("current Light count %d", activeLightCount);
}

//-------------------------------------------------------------------------------
void FluxRender2D::beginFrame() //FluxCamera* cam)
{

    mActiveCamera->update();
    setViewMatrix(mActiveCamera->getViewMatrix());

    // Bind the shader once
    mDefaultShader.use();
    mDefaultShader.setMat4("projection", mOrtho);

    if (mActiveCamera) {
        mDefaultShader.setMat4("view", mActiveCamera->getViewMatrix());
    } else {
        float identity[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
        mDefaultShader.setMat4("view", identity);
    }

    // Pass Light Data to Shader
    mDefaultShader.setVec3("uAmbientColor", mAmbientColor.r, mAmbientColor.g, mAmbientColor.b);
    mDefaultShader.setFloat("uExposure", mLightExposure);
    mDefaultShader.setInt("uToneMappingType", mToneMappingType);

    renderLights();
    // ---

    if (mCommandList.capacity() < mMaxSprites) {
        mCommandList.reserve(mMaxSprites);
    }
    mCommandList.clear();
}
//-------------------------------------------------------------------------------
// renamed from >> draw2D <<
bool FluxRender2D::drawSprite(const DrawParams2D& dp)
{
    if (!dp.image || mShaderFailed) return false;

    // --- SAFETY CHECK ---
    // If we hit our hard limit, flush immediately to clear space
    if (mCommandList.size() >= mMaxSprites) {
        renderBatch();
    }

    // only render sprites which are in the view
    //FIXME z flicker
    if  ( !dp.isGuiElement && mUseCulling )
    {
        RectF view = Render2D.getCamera()->getVisibleWorldRect(false);
        // view.inflate(32.f,32.f);
        if ( !checkAABBIntersectionF (view, dp.getRectF()) )
            return true;
    }


    RenderCommand cmd;
    cmd.textureHandle = dp.image->getHandle();
    cmd.isGui = dp.isGuiElement;
    cmd.params = dp;
    mCommandList.push_back(cmd);

    return true;
}
//-------------------------------------------------------------------------------

void FluxRender2D::renderCurrentBuffer(std::vector<Vertex2D>& vertexBuffer, GLuint texture, bool isGui) {
    if (vertexBuffer.empty()) return;

    mDefaultShader.use();
    mDefaultShader.setMat4("projection", mOrtho);
    mDefaultShader.setMat4("view", isGui ? IDENTITY_MATRIX : mCurrentCameraViewMatrix);
    mDefaultShader.setInt("uIsGui", isGui);

    // 1. Upload vertices (This stays here as the Renderer owns the CPU data)
    mQuadMesh.updateDynamic(vertexBuffer.data(), (U32)vertexBuffer.size());

    // 2. State & Texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    // 3. The Clean Draw
    // We pass true for indices and GL_TRIANGLES is the default
    U32 indicesToDraw = (U32)(vertexBuffer.size() / 4) * 6;
    mQuadMesh.draw(indicesToDraw, true);
}

//-------------------------------------------------------------------------------
void FluxRender2D::appendSpriteToBuffer(std::vector<Vertex2D>& buffer, const DrawParams2D& dp)
{
    float umin, vmin, umax, vmax;

    // Get UVs
    if (dp.u0 != 0.0f || dp.v0 != 0.0f || dp.u1 != 1.0f || dp.v1 != 1.0f) {
        umin = dp.u0; vmin = dp.v0; umax = dp.u1; vmax = dp.v1;
    } else {
        Point2F lTexPos, lTexSize;
        dp.image->getTextureRectById(dp.imgId, lTexPos, lTexSize);
        umin = lTexPos.x; vmin = lTexPos.y;
        umax = lTexPos.x + lTexSize.x; vmax = lTexPos.y + lTexSize.y;
    }

    // Flip Logic
    if (dp.flipX) std::swap(umin, umax);
    if (dp.flipY) std::swap(vmin, vmax);

    // Vertex Positions (Top-Left 0,0 system)
    float halfW = dp.w * 0.5f;
    float halfH = dp.h * 0.5f;

    // In a Y-Down system:
    // -halfH is ABOVE the center (smaller Y)
    // +halfH is BELOW the center (larger Y)
    Point2F corners[4] = {
        { -halfW, -halfH }, // 0: Top Left
        {  halfW, -halfH }, // 1: Top Right
        {  halfW,  halfH }, // 2: Bottom Right
        { -halfW,  halfH }  // 3: Bottom Left
    };

    float cosR = cosf(dp.rot);
    float sinR = sinf(dp.rot);

    size_t startSize = buffer.size();
    buffer.resize(startSize + 4);

    for (int i = 0; i < 4; i++) {
        float rx = corners[i].x * cosR - corners[i].y * sinR;
        float ry = corners[i].x * sinR + corners[i].y * cosR;
        buffer[startSize + i].pos = { rx + dp.x, ry + dp.y, -dp.z };
        buffer[startSize + i].color = dp.color;
    }

    // Scroll
    float timeOffsetX = (dp.horizontalScrollSpeed != 0.f) ? (getGameTime() * dp.horizontalScrollSpeed / 1000.f) : 0.0f;
    umin += timeOffsetX; umax += timeOffsetX;
    float timeOffsetY = (dp.verticalScollSpeed != 0.f) ? (getGameTime() * dp.verticalScollSpeed / 1000.f) : 0.0f;
    vmin += timeOffsetY; vmax += timeOffsetY;


    // Match Top-Texture (vmin) to Top-Vertices
    // This assumes your texture was loaded so that v=0 is the top
    buffer[startSize + 0].uv = { umin, vmin }; // Top Left
    buffer[startSize + 1].uv = { umax, vmin }; // Top Right
    buffer[startSize + 2].uv = { umax, vmax }; // Bottom Right
    buffer[startSize + 3].uv = { umin, vmax }; // Bottom Left
}

// pre UV for TrueType
// void FluxRender2D::appendSpriteToBuffer(std::vector<Vertex2D>& buffer, const DrawParams2D& dp)
// {
//     Point2F lTexPos, lTexSize;
//     dp.image->getTextureRectById(dp.imgId, lTexPos, lTexSize);
//
//     float umin = lTexPos.x;
//     float vmin = lTexPos.y;
//     float umax = lTexPos.x + lTexSize.x;
//     float vmax = lTexPos.y + lTexSize.y;
//
//     if (dp.flipX) std::swap(umin, umax);
//     if (dp.flipY) std::swap(vmin, vmax);
//
//     float timeOffset = (dp.horizontalScrollSpeed != 0.f) ? (getGameTime() * dp.horizontalScrollSpeed) : 0.0f;
//     umin += timeOffset; umax += timeOffset;
//
//     float halfW = dp.w * 0.5f;
//     float halfH = dp.h * 0.5f;
//
//     Point2F corners[4] = {
//         { -halfW,  halfH }, // Oben Links
//         {  halfW,  halfH }, // Oben Rechts
//         {  halfW, -halfH }, // Unten Rechts
//         { -halfW, -halfH }  // Unten Links
//     };
//
//     float cosR = cosf(dp.rot);
//     float sinR = sinf(dp.rot);
//
//     Vertex2D v[4];
//     for (int i = 0; i < 4; i++) {
//         float rx = corners[i].x * cosR - corners[i].y * sinR;
//         float ry = corners[i].x * sinR + corners[i].y * cosR;
//         v[i].pos = { rx + dp.x, ry + dp.y, -dp.z };
//         v[i].color = dp.color;
//     }
//
//     v[0].uv = { umin, vmax };
//     v[1].uv = { umax, vmax };
//     v[2].uv = { umax, vmin };
//     v[3].uv = { umin, vmin };
//
//     for (int i = 0; i < 4; i++) {
//         buffer.push_back(v[i]);
//     }
// }

//-------------------------------------------------------------------------------
void FluxRender2D::drawWithTransform(FluxTexture* texture, const Point3F& position, float rotation, float scale, const Color4F& color)
{
    if (!texture || mShaderFailed) return;

    DrawParams2D dp;
    dp.image = texture;
    dp.imgId = 0; // Assuming particles use the whole texture or first frame
    dp.x = position.x;
    dp.y = position.y;
    dp.z = position.z;
    dp.w = static_cast<S32>(texture->getWidth() * scale);
    dp.h = static_cast<S32>(texture->getHeight() * scale);
    dp.rot = rotation;
    dp.flipX = false; // Default
    dp.flipY = false; // Default
    dp.alpha = color.a; // Use the alpha from the provided color
    dp.color = color; // Use the provided color
    dp.isGuiElement = false; // Particles are usually in world space
    dp.horizontalScrollSpeed = 0.0f; // Default

    drawSprite(dp);
}

//-------------------------------------------------------------------------------
void FluxRender2D::renderBatch()
{

    // pre view filtering.
    if (mCommandList.empty()) return;

    std::sort(mCommandList.begin(), mCommandList.end(), [](const RenderCommand& a, const RenderCommand& b) {
        if (a.isGui != b.isGui) return a.isGui < b.isGui;

        // Use an epsilon or integer layers speed up sort function
        S32 layerA = static_cast<S32>(a.params.z * 1000);
        S32 layerB = static_cast<S32>(b.params.z * 1000);
        if (layerA != layerB) return layerA > layerB;

        return a.textureHandle < b.textureHandle;
    });

    _VertexBuffer.clear();
    if (_VertexBuffer.capacity() < 16000)
        _VertexBuffer.reserve(16000);

    // std::vector<Vertex2D> vertexBuffer;
    _VertexBuffer.reserve(16000); // 4000 sprites per internal flush

    GLuint currentTex = 0;
    bool currentGuiMode = false;

    for (auto& cmd : mCommandList)
    {
        bool bufferFull = (_VertexBuffer.size() >= 16000);
        // If texture or GUI mode changes, we MUST draw the current accumulated vertices
        if ( cmd.textureHandle != currentTex
             || cmd.isGui != currentGuiMode
             || bufferFull
            )
        {
            renderCurrentBuffer(_VertexBuffer, currentTex, currentGuiMode);
            _VertexBuffer.clear();
            currentTex = cmd.textureHandle;
            currentGuiMode = cmd.isGui;
        }

        // Calculate the 4 vertices for this sprite (apply rotation, scale, flip, UVs here)
        appendSpriteToBuffer(_VertexBuffer, cmd.params);
    }

    // Final flush
    renderCurrentBuffer(_VertexBuffer, currentTex, currentGuiMode);
    mCommandList.clear();

    // ------------ Primitives ------------------
    // 2. Render all Scheduled Primitives (Lines, Circles)
    // Because this happens AFTER the sprites, they appear ON TOP.

    for (const auto& prim : mPrimitiveList) {
        switch(prim.type) {
            case PrimitiveCommand::LINE:     executeDrawLine(prim);     break;
            case PrimitiveCommand::CIRCLE:   executeDrawCircle(prim);   break;
            case PrimitiveCommand::TRIANGLE: executeDrawTriangle(prim); break;
        }
    }
    mPrimitiveList.clear(); // Clear for next frame

}

