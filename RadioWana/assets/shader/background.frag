#version 330 core
out vec4 FragColor;

uniform float u_time;
uniform float u_rmsL;
uniform float u_rmsR;
uniform vec2 u_res;
uniform float u_freqs[32];

// Converts Hue, Saturation, Value to RGB
vec3 hsv2rgb(vec3 c) {
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main() {

    vec2 uv = (gl_FragCoord.xy * 2.0 - u_res.xy) / min(u_res.y, u_res.x);

    // Use low frequencies (Bass) to shake the UV coordinates
    // Index 0-3 are usually the kick/bass regions
    float bassImpact = (u_freqs[0] + u_freqs[1] + u_freqs[2]) * 0.5;
    uv *= 1.0 + bassImpact * 0.05; // Subtle pulse effect on the whole scene


    // Calculate shifting hues over time (one full cycle every ~20s)
    float hueL = fract(u_time * 0.05 + bassImpact * 0.1); // Bass shifts color slightly
//     float hueL = fract(u_time * 0.05);
    float hueR = fract(u_time * 0.05 + 0.5); // Opposite color for contrast

    // Create colors (Hue, Saturation 0.6, Value 0.5 for a subtle look)
    vec3 colorL = hsv2rgb(vec3(hueL, 0.6, 0.5));
    vec3 colorR = hsv2rgb(vec3(hueR, 0.6, 0.5));


    // points ---->
//     vec2 gridUv = uv;
//     gridUv.y = 1.0 / (gridUv.y + 1.1);
//     float grid = sin(gridUv.x * 20.0) * sin(gridUv.y * 20.0 + u_time);
//     colorL += smoothstep(0.98, 1.0, grid) * vec3(0.2, 0.4, 0.8) * (u_rmsL + 0.2);
//     colorR += smoothstep(0.98, 1.0, grid) * vec3(0.2, 0.4, 0.8) * (u_rmsR + 0.2);


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


    // 4. Draw Bars
    // Calculate local X coordinate for the spectrum area
    float aspect = u_res.x / u_res.y;
    float barAreaWidth = 1.6; // Width of the spectrum display
    float xPos = uv.x + (barAreaWidth * 0.5);

    if (xPos >= 0.0 && xPos <= barAreaWidth) {
        int idx = int((xPos / barAreaWidth) * 31.0);
        float val = u_freqs[idx];

        // Scale bar height (adjust 0.8 to your liking)
        float h = val * 0.8;

        // Smooth bar shape (vertical line)
        float bar = smoothstep(h, h - 0.02, uv.y + 0.85); // 0.85 is vertical offset
        bar *= step(-0.9, uv.y); // Bottom crop

        // Add color based on frequency index (low=red/purple, high=blue/cyan)
        vec3 barColor = hsv2rgb(vec3(float(idx)/32.0 * 0.3 + 0.5, 0.7, 0.8));
        finalColor += barColor * bar * 0.4;
    }



    FragColor = vec4(finalColor, 1.0);

}


// #version 330 core
// out vec4 FragColor;
//
// uniform float u_time;
// uniform float u_rmsL;
// uniform float u_rmsR;
// uniform vec2 u_res;
//
// void main() {
//     vec2 uv = (gl_FragCoord.xy * 2.0 - u_res.xy) / min(u_res.y, u_res.x);
//
//     float avgRms = (u_rmsL + u_rmsR) * 0.5;
//
//     vec3 color = vec3(0.02, 0.01, 0.05);
//
//     vec2 gridUv = uv;
//     gridUv.y = 1.0 / (gridUv.y + 1.1);
//     float grid = sin(gridUv.x * 20.0) * sin(gridUv.y * 20.0 + u_time);
//     color += smoothstep(0.98, 1.0, grid) * vec3(0.2, 0.4, 0.8) * (avgRms + 0.2);
//
//     float distL = length(uv + vec2(0.6, 0.0));
//     float distR = length(uv - vec2(0.6, 0.0));
//
//     float glowL = 0.8 / (distL + 0.4);
//     float glowR = 0.8 / (distR + 0.4);
//
//     color += vec3(0.8, 0.2, 0.5) * glowL * (u_rmsL * 0.5); // * 0.5 dämpft die Spitze
//     color += vec3(0.2, 0.8, 0.9) * glowR * (u_rmsR * 0.5);
//
//
//     float noise = fract(sin(dot(uv, vec2(12.9898, 78.233))) * 43758.5453);
//     color += noise * 0.02;
//
//     FragColor = vec4(color, 1.0);
// }

// #version 330 core
// out vec4 FragColor;
//
// uniform float u_time;
// uniform float u_rmsL;
// uniform float u_rmsR;
// uniform vec2 u_res;
//
// void main() {
//     vec2 uv = (gl_FragCoord.xy - 0.5 * u_res.xy) / min(u_res.y, u_res.x);
//
//     vec3 baseColor = vec3(0.05, 0.05, 0.1);
//
//     float distL = length(uv + vec2(0.4, 0.0));
//     float distR = length(uv - vec2(0.4, 0.0));
//
//     float glowL = 0.15 * u_rmsL / (distL + 0.5);
//     float glowR = 0.15 * u_rmsR / (distR + 0.5);
//
//     vec2 gridUV = uv * 4.0;
//     gridUV.y += sin(u_time * 0.5 + gridUV.x) * 0.2; // Wellenbewegung
//     float grid = sin(gridUV.x * 10.0) * sin(gridUV.y * 10.0);
//     grid = smoothstep(0.95, 1.0, grid) * 0.1;
//
//     vec3 finalColor = baseColor + vec3(0.6, 0.2, 0.8) * glowL + vec3(0.2, 0.6, 0.8) * glowR;
//     finalColor += grid * (u_rmsL + u_rmsR + 0.1);
//
//     FragColor = vec4(finalColor, 1.0);
// }
