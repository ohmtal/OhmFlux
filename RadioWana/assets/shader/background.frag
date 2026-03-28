#version 330 core
out vec4 FragColor;

uniform float u_time;
uniform float u_rmsL;
uniform float u_rmsR;
uniform vec2 u_res;
// uniform float u_freqs[32];

// Converts Hue, Saturation, Value to RGB
vec3 hsv2rgb(vec3 c) {
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main() {

    vec2 uv = (gl_FragCoord.xy * 2.0 - u_res.xy) / min(u_res.y, u_res.x);


    // Calculate shifting hues over time (one full cycle every ~20s)
    float hueL = fract(u_time * 0.05);
    float hueR = fract(u_time * 0.05 + 0.5); // Opposite color for contrast

    // Create colors (Hue, Saturation 0.6, Value 0.5 for a subtle look)
    vec3 colorL = hsv2rgb(vec3(hueL, 0.6, 0.5));
    vec3 colorR = hsv2rgb(vec3(hueR, 0.6, 0.5));


    // points ---->
    vec2 gridUv = uv;
    gridUv.y = 1.0 / (gridUv.y + 1.1);
    float grid = sin(gridUv.x * 20.0) * sin(gridUv.y * 20.0 + u_time);
    colorL += smoothstep(0.98, 1.0, grid) * vec3(0.2, 0.4, 0.8) * (u_rmsL + 0.2);
    colorR += smoothstep(0.98, 1.0, grid) * vec3(0.8, 0.4, 0.2) * (u_rmsR + 0.2);


    // Distance from glow centers
    float distL = length(uv + vec2(0.6, 0.0));
    float distR = length(uv - vec2(0.6, 0.0));

    // Large, soft glow formula
    // Numerator controls size, addition in denominator prevents over-brightness
    float glowL = 1.4 / (distL + 0.7);
    float glowR = 1.4 / (distR + 0.7);

    // Deep dark blue background base
    vec3 finalColor = vec3(0.01, 0.01, 0.02);

    // Apply audio-reactive glow (dampened RMS to keep it calm)
    finalColor += colorL * glowL * (u_rmsL * 0.3);
    finalColor += colorR * glowR * (u_rmsR * 0.3);

    // Subtle grain noise to prevent color banding
    float noise = fract(sin(dot(uv, vec2(12.9898, 78.233))) * 43758.5453);
    finalColor += noise * 0.012;

    FragColor = vec4(finalColor, 1.0);

}
