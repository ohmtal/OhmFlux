// #version 330 core
precision mediump float;

// --------------------------------------------
// RadioWana Background Shader: Bars
// --------------------------------------------

out vec4 FragColor;

uniform float u_time;       // timer (dt)
uniform float u_rmsL;       // left RMS
uniform float u_rmsR;       // right RMS
uniform vec2 u_res;         // screen resolution
uniform float u_freqCount;  // frequencies (bands) count MAX 32!
uniform float u_freqs[32];  // frequency FFT data
uniform bool  u_scanlines;

// Converts Hue, Saturation, Value to RGB
vec3 hsv2rgb(vec3 c) {
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main() {



    vec2 uv = (gl_FragCoord.xy * 2.0 - u_res.xy) / min(u_res.y, u_res.x);
    vec3 finalColor = vec3(0.01, 0.01, 0.02);


    // Calculate shifting hues over time (one full cycle every ~20s)
    //260329 clamp rms values. silent with a little glow
//     float rmsL = clamp(u_rmsL, 0.2, 1.0);
//     float rmsR = clamp(u_rmsR, 0.2, 1.0);
//     float hueL = fract(u_time * 0.05);
//     float hueR = fract(u_time * 0.05 + 0.5); // Opposite color for contrast
//
//     // Create colors (Hue, Saturation 0.6, Value 0.5 for a subtle look)
//     vec3 colorL = hsv2rgb(vec3(hueL, 0.6, 0.5));
//     vec3 colorR = hsv2rgb(vec3(hueR, 0.6, 0.5));
//
//     // Distance from glow centers
//     float distL = length(uv + vec2(rmsL * 2.5, 0.0));
//     float distR = length(uv - vec2(rmsR * 2.5, 0.0));
//
//     // Large, soft glow formula
//     // Numerator controls size, addition in denominator prevents over-brightness
//     float glowL = 1.4 / (distL + 0.7);
//     float glowR = 1.4 / (distR + 0.7);
//
//
//     // Apply audio-reactive glow (dampened RMS to keep it calm)
//     finalColor += colorL * glowL * (rmsL * 0.3);
//     finalColor += colorR * glowR * (rmsR * 0.3);
//
//     // Subtle grain noise to prevent color banding
//     // ... fast random generator  ...
//     float noise = fract(sin(dot(uv, vec2(12.9898, 78.233))) * 43758.5453);
//     finalColor += noise * 0.012;


    // --- Bars (Wide Version) ---
    float aspect = u_res.x / u_res.y;
    float barAreaWidth = 1.8; // Total width of the spectrum display
    float xPos = uv.x + (barAreaWidth * 0.5);

    if (xPos >= 0.0 && xPos <= barAreaWidth) {
        // Current bar index (0 to 31 (default)
        float barCount = u_freqCount;
        int idx = int((xPos / barAreaWidth) * (barCount - 0.01));
        float val = u_freqs[idx];

        // --- Horizontal Bar Width Logic ---
        // Get local X coordinate within a single bar (0.0 to 1.0)
        float localX = fract((xPos / barAreaWidth) * barCount);

        // Gap size: 0.1 means 10% gap, 90% bar width.
        // Increase 0.1 to 0.3 if you want more space between bars.
        float gap = 0.15;
        float horizontalMask = smoothstep(0.0, 0.05, localX - gap) *
        smoothstep(1.0, 0.95, localX);

        // --- Vertical Height Logic ---
        float h = val * 0.9; // Scale height
        float verticalMask = smoothstep(h, h - 0.02, uv.y + 0.85);
        verticalMask *= step(-0.92, uv.y); // Bottom crop

        // Combine masks
        float finalBarMask = horizontalMask * verticalMask;

        // --- Dynamic Color ---
        // Full rainbow shift from left to right + time animation
        float hue = fract(float(idx) / barCount + u_time * 0.05);
        vec3 barColor = hsv2rgb(vec3(hue, 0.7, 0.9));

        // Add to final color with a bit of "bloom/glow"
        finalColor += barColor * finalBarMask * 0.7;
    }

    // -------------- --- CRT Scanline & Grain Layer --- -------------------
    if ( u_scanlines ) {
        float rms = (u_rmsL + u_rmsR) * 0.5;
        // Create moving scanlines based on screen height
        float scanline = sin(uv.y * u_res.y * 1.5 - u_time * 2.0) * 0.05;

        // Subtle Grain
        float grain = fract(sin(dot(uv, vec2(12.9898, 78.233))) * 43758.5453);
        //     grain = (grain - 0.5) * 0.05; // range -0.025 to 0.025
        grain = (grain - 0.5) * 0.025; // range -0.025 to 0.025

        // Apply the effects to the final color
        // 'finalColor' is the vec3 result from your previous shader logic
        finalColor -= scanline; // Darkens every second/third pixel row
        finalColor += grain;    // Adds the organic noise

//         float vignette = distance(uv, vec2(0.5));
//         float edgeStart =  0.8;
//         float edgeEnd = clamp(0.4 + (rms * 0.3), 0.0, 0.75);
//         finalColor *= smoothstep(edgeStart, edgeEnd, vignette);

        // << CRT
    }


    FragColor = vec4(finalColor, 1.0);

}

