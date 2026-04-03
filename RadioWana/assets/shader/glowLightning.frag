// #version 330 core
precision mediump float;

// ------------------------------------------------------------
// RadioWana Background Shader: Glow and Lightning (not really)
// ------------------------------------------------------------

out vec4 FragColor;

uniform float u_time;
uniform float u_rmsL;
uniform float u_rmsR;
uniform vec2 u_res;
uniform float u_freqCount;
uniform float u_freqs[32];
uniform bool u_scanlines;

float hash(float x) {
    return fract(sin(x * 12.9898) * 43758.5453);
}


vec3 hsv2rgb(vec3 c) {
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}


void main() {
    vec2 uv = gl_FragCoord.xy / u_res.xy;
    vec2 uvGlow = (gl_FragCoord.xy * 2.0 - u_res.xy) / min(u_res.y, u_res.x);

    //rms values. silent with a little glow
    float rmsL = clamp(u_rmsL, 0.2, 1.0);
    float rmsR = clamp(u_rmsR, 0.2, 1.0);
    float rms = (u_rmsL + u_rmsR) * 0.5;

    // -------------------- GLOW --------------------------
    // Calculate shifting hues over time (one full cycle every ~20s)
    //     float hueL = fract(u_time * 0.05 + bassImpact * 0.1); // Bass shifts color slightly
    float hueL = fract(u_time * 0.05);
    float hueR = fract(u_time * 0.05 + 0.5); // Opposite color for contrast

    // Create colors (Hue, Saturation 0.6, Value 0.5 for a subtle look)
    vec3 colorL = hsv2rgb(vec3(hueL, 0.6, 0.5));
    vec3 colorR = hsv2rgb(vec3(hueR, 0.6, 0.5));

    // Distance from glow centers
    float distL = length(uvGlow + vec2(rmsL * 2.5, 0.0));
    float distR = length(uvGlow - vec2(rmsR * 2.5, 0.0));

    // Large, soft glow formula
    // Numerator controls size, addition in denominator prevents over-brightness
    float glowL = 1.4 / (distL + 0.7);
    float glowR = 1.4 / (distR + 0.7);

    // Deep dark blue background base
    vec3 glowColor = vec3(0.01, 0.01, 0.02);

    // Apply audio-reactive glow (dampened RMS to keep it calm)
//     glowColor += colorL * glowL * (rmsL * 0.3);
//     glowColor += colorR * glowR * (rmsR * 0.3);
    glowColor += colorL * glowL * (rmsL * 0.4);
    glowColor += colorR * glowR * (rmsR * 0.4);

    // ... fast random generator  ... moved to merge
    //     float noise = fract(sin(dot(uvGlow, vec2(12.9898, 78.233))) * 43758.5453);
    //     glowColor += noise * 0.012;


    // -------------------- LIGHTNING BOLT (between L and R) --------------------
    // Define the two centers (matching your Glow positions)
    vec2 posL = vec2(-u_rmsL * 1.5, 0.0);
    vec2 posR = vec2( u_rmsR * 1.5, 0.0);

    // Calculate distance to the line segment between posL and posR
    vec2 dir = posR - posL;
    float len = length(dir);
    vec2 normDir = dir / len;
    vec2 relP = uvGlow - posL;
    float projection = clamp(dot(relP, normDir), 0.0, len);
    vec2 closestPoint = posL + normDir * projection;
    float distToLine = length(uvGlow - closestPoint);

    // Create the "Electric" flicker using your noise & fast time
//     float lightningNoise = fract(sin(dot(uvGlow.yy, vec2(12.9898, 78.233))) * 43758.5453 + u_time * 20.0);
//     float wave = sin(uvGlow.x * 10.0 + u_time * 30.0) * 0.03 * (rmsL + rmsR);
//     float boltIntensity = smoothstep(0.05, 0.0, distToLine + wave * lightningNoise);

    int midBand = int(u_freqCount/2);

    float lightningNoise = fract(sin(dot(uvGlow.yy, vec2(12.9898, 78.233))) * 43758.5453 + u_time * 20.0);
    float wave = sin(uvGlow.x * 10.0 * u_freqs[0] + u_time * /*30.0 **/ u_freqs[1]) * 0.05 * (rmsL + rmsR);
    // The Bolt thickness
    float boltIntensity = smoothstep(0.03 * u_freqs[0], 0.0, distToLine + wave * lightningNoise);

    // Color the bolt (white core with cyan/purple edges)
//     vec3 boltColor = vec3(0.6, 0.8, 1.0) * boltIntensity * (rmsL + rmsR) * 2.0;
    vec3 boltColor = vec3(u_freqs[0], u_freqs[midBand], u_freqs[int(u_freqCount)]) * boltIntensity * (rmsL + rmsR) * 0.5;

//     boltColor += lightningNoise * 0.012;

    boltColor = boltColor * smoothstep(0.3, 0.6, rms) ;



    // -------- merge -------------
    // Add the bolt to your final color

    vec3 finalColor = glowColor + boltColor;

    // Subtle grain noise to prevent color banding
    float noise = fract(sin(dot(uvGlow, vec2(12.9898, 78.233))) * 43758.5453);
    finalColor  += noise * 0.012;

    // -------------- --- CRT Scanline & Grain Layer --- -------------------
    if (u_scanlines) {
        // Create moving scanlines based on screen height
        // We use a high multiplier (e.g., 3.0) for the density of lines
        float scanline = sin(uv.y * u_res.y * 1.5 - u_time * 5.0) * 0.1;

        // Subtle Grain (using your magic numbers)
        // We scale it down so it doesn't overpower the image
        float grain = fract(sin(dot(uv, vec2(12.9898, 78.233))) * 43758.5453);
        grain = (grain - 0.5) * 0.025; // range -0.025 to 0.025

        // Apply the effects to the final color
        // 'finalColor' is the vec3 result from your previous shader logic
        finalColor -= scanline; // Darkens every second/third pixel row
        finalColor += grain;    // Adds the organic noise
        // << CRT

        float vignette = distance(uv, vec2(0.5));
        float edgeStart =  0.8;
        float edgeEnd = clamp(0.4 + (rms * 0.3), 0.0, 0.75);
        finalColor *= smoothstep(edgeStart, edgeEnd, vignette);

    }


    FragColor = vec4(finalColor, 1.0);
}
