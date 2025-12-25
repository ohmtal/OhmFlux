//-----------------------------------------------------------------------------
// Copyright (c) 2012/2025 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
//
// Singelton: Render2D
//
//-----------------------------------------------------------------------------
#pragma once
#ifndef _FLUXRENDER2D_H_
#define _FLUXRENDER2D_H_

#include "fluxGlobals.h"
#include "fluxTexture.h"
#include "fluxShader.h"
#include "fluxMesh.h"
#include "fluxCamera.h"



struct DrawParams2D {
    FluxTexture* image = nullptr;
    S32 imgId = 0;   // frame
    F32 x = 0.0f;
    F32 y = 0.0f;
    F32 z = 0.0f;  //layer
    S32 w = 0;
    S32 h = 0;
    F32 rot = 0.0f; //rotation
    bool flipX = false;
    bool flipY = false;
    F32 alpha = 0.1f;
    Color4F color = {1.0f, 1.0f, 1.0f, 1.0f};
    F32 horizontalScrollSpeed = 0.f;
    F32 verticalScollSpeed = 0.f;
    bool  isGuiElement = false; //not effected by camera

    // for TrueType Font >>>>
    bool  useUV = false;
    F32 u0 = 0.0f; F32 v0 = 0.0f;
    F32 u1 = 1.0f; F32 v1 = 1.0f;
    // <<<

    F32 getWidthF() const { return static_cast<F32>(w); }
    F32 getHeightF() const { return static_cast<F32>(h); }

    FluxTexture* getTexture() { return image; }
    F32 getFrame() const { return imgId; }
    F32 getLayer() const { return z; }

    RectF getRectF() const {
        return {
            x - (getWidthF() * 0.5f), // Shift from center to left
            y - (getHeightF() * 0.5f), // Shift from center to top
            getWidthF(),
            getHeightF()
        };
    }

    RectI getRectI() const {
        return {
            static_cast<S32>(x - (w * 0.5f)),
            static_cast<S32>(y - (h * 0.5f)),
            w,
            h
        };
    }
    DrawParams2D& setPositionAndLayer( Point3F value ) { x=value.x, y=value.y; z=value.z; return *this; }
};

//batch rendering
struct RenderCommand {
    GLuint textureHandle;
    bool isGui;
    // We store the data needed to calculate vertices later
    DrawParams2D params;
};

struct PrimitiveCommand {
    enum Type { LINE, CIRCLE, TRIANGLE };
    Type type;
    Point3F points[3]; // Used for Triangle corners
    Point3F p1, p2;    // Used for Lines
    F32 radius;      // For circles
    U32 segments;
    Color4F color;
    bool filled;       // NEW: Toggle for Triangle fill mode
};



static const F32 IDENTITY_MATRIX[16] = {
    1,0,0,0,
    0,1,0,0,
    0,0,1,0,
    0,0,0,1
};



class FluxRender2D {
private:
    bool mShaderFailed;

    //only render what is in the view ... cause z flicker if many objects overlap on same layer?!
    bool mUseCulling = true;

    FluxShader mDefaultShader;
    FluxShader mFlatShader;
    FluxMesh   mQuadMesh;
    FluxMesh   mLineMesh;



    FluxCamera* mActiveCamera = nullptr;
    FluxCamera* mDefaultCamera =  nullptr;;

    // orto and viewmatrix setup for screen
    //  alignas => matrix is aligned for SIMD operations (webGL performance)
    alignas(16) F32 mOrtho[16] = {0.0f};
    alignas(16) F32 mCurrentCameraViewMatrix[16] {0.0f};;


    // ------  RENDER BATCH -------
    //batch rendering
    U32 mMaxSprites = DEFAULT_MAX_SPRITES;

    std::vector<RenderCommand> mCommandList;
    std::vector<PrimitiveCommand> mPrimitiveList;
    FluxTexture* mWhiteTextureWrapper;

    GLuint mWhiteTextureHandle;
    void renderCurrentBuffer(std::vector<Vertex2D>& vertexBuffer, GLuint texture, bool isGui);
    void appendSpriteToBuffer(std::vector<Vertex2D>& buffer, const DrawParams2D& dp) ;


    std::vector<Vertex2D> _VertexBuffer; //

    // Lights
    Color4F mAmbientColor = { 0.4f,0.4f,0.4f, 1.f}; // cl_White; //only have effect when lights in scene
    F32 mLightExposure = 1.f; //only have effect when lights in scene
    S32 mToneMappingType = 2; //default=none, 1=Reinhard, 2=Filmic
    void renderLights();

public:
    static FluxRender2D& getInstance() {
        static FluxRender2D instance;
        return instance;
    }

    FluxRender2D(const FluxRender2D&) = delete;
    void operator=(const FluxRender2D&) = delete;

    bool init(U32 maxSprites = DEFAULT_MAX_SPRITES);
    void shutdown();

    // primitives
    void drawLine(F32 x1, F32 y1, F32 x2, F32 y2, const Color4F& color = cl_White);
    void executeDrawLine(const PrimitiveCommand& cmd);
    void drawRect(F32 x, F32 y, F32 w, F32 h, const Color4F& color = cl_White, bool filled = true);
    void drawCircle(F32 cx, F32 cy, F32 radius, const Color4F& color =cl_White, U32 segments = 32);
    void executeDrawCircle(const PrimitiveCommand& cmd);
    void drawTriangle(Point3F p1, Point3F p2, Point3F p3, const Color4F& color = cl_White, bool filled = true);
    void executeDrawTriangle(const PrimitiveCommand& cmd);

    //quad render
    // bool loadDefaultShader();
    void updateOrto(S32 width, S32 height);
    void beginFrame(); //FluxCamera* cam); //Camera

    // this batch the draws NOT draw
    bool drawSprite(const DrawParams2D& dp);
    void drawWithTransform(FluxTexture* texture, const Point3F& position, F32 rotation, F32 scale, const Color4F& color);
    void renderBatch();


    //wrapper for drawSprite with ugly call ... parameters got added and added over time
    bool uglyDraw2DStretch(
        FluxTexture* limg,
        const S32& limgId,
        const F32& lx,
        const F32& ly,
        const F32& lz,
        const S32& lw,
        const S32& lh,
        const F32& lrot,
        const bool& lflipX,
        const bool& lflipY,
        const F32& lAlpha = 0.1f,
        const bool& Obsolete_doBlend_ALLWAYS_TRUE=false,
        const Color4F& lColor = cl_White
    ) ;

    // FluxCamera

    void setCamera(FluxCamera* cam) { mActiveCamera = cam; }
    FluxCamera* getCamera() { return mActiveCamera; }

    void setViewMatrix(const F32* matrix) {
        memcpy(mCurrentCameraViewMatrix, matrix, sizeof(F32) * 16);
    }

    void setCulling(bool value) { mUseCulling = value; }
    bool getCulling() { return mUseCulling; }

    // for lighting
    void setAmbientColor( const Color4F& value ) { mAmbientColor = value; }
    const Color4F& getAmbientColor() const { return mAmbientColor; }

private:
    FluxRender2D() : mShaderFailed(true) {}
    ~FluxRender2D() { shutdown(); } // Fallback cleanup
};

#define Render2D FluxRender2D::getInstance()

#endif
