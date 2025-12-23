//-----------------------------------------------------------------------------
// Copyright (c) 2025 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#pragma once
#ifndef _FLUXSHADERSOURCES_H_
#define _FLUXSHADERSOURCES_H_

#ifdef __EMSCRIPTEN__
#define GLSL_VERSION "#version 300 es\nprecision highp float;\n"
#else
#define GLSL_VERSION "#version 330 core\n"
#endif

// --- Default Sprite Shader ---
inline const char* vertexShaderSource = GLSL_VERSION R"(
layout (location = 0) in vec3 aPos;      // Final position calculated on CPU
layout (location = 1) in vec2 aTexCoord; // Final UV (handled flipping/scrolling)
layout (location = 2) in vec4 aColor;    // Per-sprite Tint and Alpha

out vec2 TexCoord;
out vec4 TintColor; // Pass to Fragment Shader
out vec3 fragWorldPos; // Pass world position to fragment shader

uniform mat4 view;       // Camera View Matrix (Shared by batch)
uniform mat4 projection; // Ortho Matrix (Shared by batch)

void main() {
    // Note: 'model' is gone! aPos is already transformed.
    gl_Position = projection * view * vec4(aPos, 1.0);

    TexCoord = aTexCoord;
    TintColor = aColor;
}
)";

// hackfest
// inline const char* fragmentShaderSource = GLSL_VERSION R"(
// out vec4 FragColor;
//
// in vec2 TexCoord;
// in vec4 TintColor; // Received from Vertex Shader
//
// uniform sampler2D texture1;
//
// void main() {
//     // Multiply texture color by the vertex tint (TintColor)
//     vec4 texColor = texture(texture1, TexCoord);
//     FragColor = texColor * TintColor;
// }
// )";

inline const char* fragmentShaderSource = GLSL_VERSION R"(
out vec4 FragColor;

in vec2 TexCoord;
in vec4 TintColor;

uniform sampler2D texture1;

void main() {
    // 1. Sample the texture (UVs are already calculated on CPU now)
    vec4 texColor = texture(texture1, TexCoord);

    // 2. 2025 Standard: Gamma Correction (sRGB to Linear)
    // Your old shader used this; without it, colors look wrong.
    texColor.rgb = pow(texColor.rgb, vec3(2.2));

    // 3. Alpha Threshold (Replaces uAlphaThreshold)
    // We use a fixed small value or pass it via TintColor.a
    if (texColor.a < 0.1) {
        discard;
    }

    // 4. Final Color (Multiply by the vertex color/tint)
    FragColor = texColor * TintColor;
}
)";


//-----------------------------------------------------------
// pre batch renamed to vertexShaderSourceDirectDraw2D
// inline const char* vertexShaderSourceDirectDraw2D = GLSL_VERSION R"(
// layout (location = 0) in vec3 aPos;
// layout (location = 1) in vec2 aTexCoord;
//
// out vec2 TexCoord;
//
// uniform mat4 model;
// uniform mat4 view; //Camera
// uniform mat4 projection;
//
// void main() {
//     gl_Position = projection * view * model * vec4(aPos, 1.0);
//     TexCoord = aTexCoord;
// }
// )";
//
// // pre batch renamed to vertexShaderSourceDirectDraw2D
// inline const char* fragmentShaderSourceDirectDraw2D = GLSL_VERSION R"(
// out vec4 FragColor;
// in vec2 TexCoord;
//
// uniform sampler2D ourTexture;
// uniform vec4 uTint;
// uniform float uAlphaThreshold;
// uniform vec2 uTexOffset;
// uniform vec2 uTexSize;
// uniform vec2 uFlip;
// uniform float uTime;
//
// void main() {
//     // Calculate final UV with tile offset and size
//     vec2 finalUV = uTexOffset + (TexCoord * uTexSize);
//
//     // Apply horizontal scrolling
//     finalUV.x += uTime;
//
//     // Apply UV Flip logic
//     finalUV = mix(finalUV, 1.0 - finalUV, step(uFlip, vec2(0.0)));
//
//     vec4 texColor = texture(ourTexture, finalUV);
//
//     // Gamma Correction: sRGB to Linear
//     texColor.rgb = pow(texColor.rgb, vec3(2.2));
//
//     if (texColor.a <= uAlphaThreshold) {
//         discard;
//     }
//
//     FragColor = texColor * uTint;
// }
// )";

// --- Flat Color Shader (for Primitives) ---
inline const char* flatVertexShaderSource = GLSL_VERSION R"(
layout (location = 0) in vec3 aPos;
layout (location = 2) in vec4 aColor; // MUST match Location 2 in Vertex2D

out vec4 vColor;

uniform mat4 model;      // Keep for line transformations
uniform mat4 view;
uniform mat4 projection;

void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    vColor = aColor; // Pass the vertex color to fragment
}
)";


inline const char* flatFragmentShaderSource = GLSL_VERSION R"(
out vec4 FragColor;
in vec4 vColor; // Received from vertex shader

void main() {
    FragColor = vColor; // No texture sampling needed here
}
)";

#endif
