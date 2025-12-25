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
    // 1. For the GPU screen position, we need View and Projection
    gl_Position = projection * view * vec4(aPos, 1.0);

    // 2. For lighting, we need the STATIC world position.
    // If aPos is already world-space from the CPU, just pass it through:
    fragWorldPos = aPos;

    TexCoord = aTexCoord;
    TintColor = aColor;
}
)";


inline const char* fragmentShaderSource = GLSL_VERSION R"(
out vec4 FragColor;

in vec2 TexCoord;
in vec4 TintColor;
in vec3 fragWorldPos;

uniform sampler2D texture1;
uniform vec3 uAmbientColor; // New Uniform: RGB for color, Magnitude for intensity

struct Light {
    vec3 position;
    vec4 color;    // rgb, alpha for intensity
    float radius;
    vec2 direction;
    float cutoff;
};

#define MAX_LIGHTS 8
uniform Light uLights[MAX_LIGHTS];
uniform int uNumLights;
uniform bool uIsGui;
uniform float uExposure = 1.0;
uniform int uToneMappingType = 2; //0=none, 1=Reinhard, 2=Filmic

void main() {
    vec4 texColor = texture(texture1, TexCoord);

    if (uIsGui) {
        FragColor = texColor * TintColor;
    } else if ( uNumLights == 0)
    {
        texColor.rgb = pow(texColor.rgb, vec3(2.2));
        FragColor = texColor * TintColor;

    } else {
        texColor.rgb = pow(texColor.rgb, vec3(2.2));
        vec3 lightAccum = uAmbientColor;


        for (int i = 0; i < uNumLights; ++i) {
            vec2 lightToFrag = fragWorldPos.xy - uLights[i].position.xy;
            float dist = length(lightToFrag);

            if (dist < uLights[i].radius) {
                float intensity = 1.0; // Default for Omni-lights

                // Only calculate spotlight logic if it's NOT an Omni-light
                // AND we aren't exactly on top of the light source
                if (uLights[i].cutoff > -0.99 && dist > 0.001) {
                    vec2 normLightToFrag = normalize(lightToFrag);
                    float theta = dot(normLightToFrag, normalize(uLights[i].direction));

                    if (theta > uLights[i].cutoff) {
                        // Smooth the edge of the spotlight cone
                        float epsilon = 0.1;
                        intensity = clamp((theta - uLights[i].cutoff) / epsilon, 0.0, 1.0);
                    } else {
                        intensity = 0.0; // Outside the cone
                    }
                }

                if (intensity > 0.0) {
                    float attenuation = 1.0 - (dist / uLights[i].radius);
                    lightAccum += uLights[i].color.rgb * uLights[i].color.a * attenuation * intensity;
                }
            }
        } // Light loop



        vec3 result = texColor.rgb * lightAccum;
        result *= uExposure;

        switch (uToneMappingType)
        {
            case 1: // Reinhard Tonemapping
                result = result / (result + vec3(1.0));
                FragColor = vec4(pow(result, vec3(1.0/2.2)), texColor.a) * TintColor;
                break;
            case 2: // Filmic Tonemapping (Simplified ACES)
                vec3 x = result;
                result = (x * (6.2 * x + 0.5)) / (x * (6.2 * x + 1.7) + 0.06);
                FragColor = vec4(result, texColor.a * TintColor.a) * TintColor;
                break;
            default: //none
                // Clamp light to 1.0 to prevent over-exposure before final Tint
                lightAccum = min(lightAccum, vec3(1.0));
                texColor.rgb *= lightAccum;
                FragColor = texColor * TintColor;
                break;
        }
    } //<<< Lights

    // Linear to sRGB (Optional: recommended if your textures are sRGB)
    // texColor.rgb = pow(texColor.rgb, vec3(1.0/2.2));

    if (texColor.a < 0.1) {
        discard;
    }

}
)";


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
