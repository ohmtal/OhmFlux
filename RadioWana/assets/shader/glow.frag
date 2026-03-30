#version 330 core
precision mediump float;

// -------------------------------------------
// RadioWana Background Shader: rain and Glow
// -------------------------------------------

uniform float u_time;
uniform float u_rmsL;
uniform float u_rmsR;
uniform vec2 u_res;
uniform float u_freqCount;
uniform float u_freqs[32];


vec3 hsv2rgb(vec3 c) {
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}


void main() {
//     vec2 uv = gl_FragCoord.xy / u_res.xy;
    vec2 uv = (gl_FragCoord.xy * 2.0 - u_res.xy) / min(u_res.y, u_res.x);

    // -------------------- GLOW --------------------------
    //rms values. silent with a little glow
    float rmsL = clamp(u_rmsL, 0.2, 1.0);
    float rmsR = clamp(u_rmsR, 0.2, 1.0);

    // Calculate shifting hues over time (one full cycle every ~20s)
    //     float hueL = fract(u_time * 0.05 + bassImpact * 0.1); // Bass shifts color slightly
    float hueL = fract(u_time * 0.05);
    float hueR = fract(u_time * 0.05 + 0.5); // Opposite color for contrast

    // Create colors (Hue, Saturation 0.6, Value 0.5 for a subtle look)
    vec3 colorL = hsv2rgb(vec3(hueL, 0.6, 0.5));
    vec3 colorR = hsv2rgb(vec3(hueR, 0.6, 0.5));

    // Distance from glow centers
    float distL = length(uv + vec2(rmsL * 2.5, 0.0));
    float distR = length(uv - vec2(rmsR * 2.5, 0.0));

    // Large, soft glow formula
    // Numerator controls size, addition in denominator prevents over-brightness
    float glowL = 1.4 / (distL + 0.7);
    float glowR = 1.4 / (distR + 0.7);

    // Deep dark blue background base
    vec3 finalColor = vec3(0.01, 0.01, 0.02);

    // Apply audio-reactive glow (dampened RMS to keep it calm)
    finalColor += colorL * glowL * (rmsL * 0.3);
    finalColor += colorR * glowR * (rmsR * 0.3);

    // Subtle grain noise to prevent color banding
    // ... fast random generator  ...
    float noise = fract(sin(dot(uv, vec2(12.9898, 78.233))) * 43758.5453);
    finalColor += noise * 0.012;




    gl_FragColor = vec4(finalColor, 1.0);
}
