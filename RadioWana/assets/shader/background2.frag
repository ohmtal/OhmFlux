#version 330 core
out vec4 FragColor;

uniform float u_time;
uniform float u_rmsL;
uniform float u_rmsR;
uniform vec2 u_res;
uniform float u_freqCount; //MAX 32!
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

    // --- 4. Draw Bars (Wide Version) ---
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

    FragColor = vec4(finalColor, 1.0);

}

