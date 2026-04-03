// #version 330 core
precision mediump float;

// ------------------------------------
// RadioWana Background Shader: rain
// ------------------------------------

out vec4 FragColor;

uniform float u_time;
uniform float u_rmsL;
uniform float u_rmsR;
uniform vec2 u_res;
uniform float u_freqCount;
uniform float u_freqs[32];
uniform bool  u_scanlines;

// Helper to create a pseudo-random value based on column index
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

    // Average RMS for overall intensity
    float rms = (u_rmsL + u_rmsR) * 0.5;

    // Grid settings: 16 columns matching your u_freqCount
    float columns = u_freqCount;
    float colIndex = floor(uv.x * columns);
    float xPos = fract(uv.x * columns); // Relative x within the column

    // Get frequency data for this specific column
    // We cast colIndex to int to access the array
    float freqValue = u_freqs[int(colIndex)];

    // Animation: Speed is constant but boosted by RMS peaks
    float speed = (u_time * 0.1) + (hash(colIndex) * 10.0);
//     float speed = (u_time * 0.4) + (hash(colIndex) * 10.0);
//     not break: speed += rms * 0.5;

    // Vertical "rain" effect using modulo on y-coordinate
    float yPos = uv.y + speed * (0.5 + hash(colIndex + 123.0) * 0.5);
    float cellY = fract(yPos * 5.0); // Sub-segments of the rain
    float rowID = floor(yPos * 5.0);

    vec3 whiteCore = vec3(0.8, 1.0, 0.9); // Brighter tip
    float heightLimit = smoothstep(uv.y - 0.2, uv.y, freqValue * 2.5);


    // --------------------------------------
    //     float drop = 1.0 - fract(yPos);
    //     drop = pow(drop, 3.0); // Sharpen the head of the drop

    // --- Motion Blur / Trail Simulation ---
    // Instead of a simple power function, we create a longer,
    // more organic tail that reacts to the volume (RMS).
    // Higher RMS = longer, more blurred trails.
    float trailLength = 2.0 + (rms * 4.0);
    float drop = 1.0 - fract(yPos);
    drop = pow(drop, trailLength);

    // To simulate "Ghosting", we add a secondary,
    // slightly offset and fainter drop layer.
    float ghosting = 1.0 - fract(yPos - 0.1);
    ghosting = pow(ghosting, trailLength * 1.5) * 0.5;

    float combinedDrop = max(drop, ghosting);

    // Combine with your frequency height limit
    float intensity = combinedDrop * heightLimit;

    // Soften the vertical edges to make it look "blurry"
    intensity *= smoothstep(0.0, 0.1, fract(yPos));

    // <<<<<<<<< blur ....
    // fire like stream smoothstep:
    intensity *= smoothstep(0.20, 0.05, abs(xPos - 0.5));
    // make it colorized
    float hue = fract(float(colIndex) / columns + u_time * 0.05);
    vec3 barColor = hsv2rgb(vec3(hue, 0.7, 0.9));
    // Neon-Glow
    vec3 glow = barColor * pow(intensity, 2.0) * (2.0 + rms * 5.0);
    vec3 finalColor = (barColor * intensity) + glow + (whiteCore * pow(intensity, 10.0));
    finalColor += whiteCore * pow(intensity, 10.0) * (1.0 + rms);
    // Flicker effect
    finalColor *= 0.8 + 0.2 * sin(u_time * 20.0 + colIndex);

    // -------------- --- CRT Scanline & Grain Layer --- -------------------
    if ( u_scanlines ) {
        // Create moving scanlines based on screen height
        // We use a high multiplier (e.g., 3.0) for the density of lines
        float scanline = sin(uv.y * u_res.y * 1.5 - u_time * 5.0) * 0.1;

        // Subtle Grain (using your magic numbers)
        // We scale it down so it doesn't overpower the image
        float grain = fract(sin(dot(uv, vec2(12.9898, 78.233))) * 43758.5453);
        grain = (grain - 0.5) * 0.05; // range -0.025 to 0.025

        // Apply the effects to the final color
        // 'finalColor' is the vec3 result from your previous shader logic
        finalColor -= scanline; // Darkens every second/third pixel row
        finalColor += grain;    // Adds the organic noise
    }
    // << CRT


    FragColor = vec4(finalColor, 1.0);
}
