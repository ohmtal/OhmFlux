#version 330 core
precision mediump float;

// --------------------------------------------
// RadioWana Background Shader: liquid
// --------------------------------------------


uniform float u_time;
uniform float u_rmsL;
uniform float u_rmsR;
uniform vec2 u_res;
uniform float u_freqCount;     // Currently 16
uniform float u_freqs[32];     // Array of FFT data
uniform bool  u_scanlines;


vec3 hsv2rgb(vec3 c) {
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main() {
    // Normalize coordinates (0.0 to 1.0)
    vec2 uv = gl_FragCoord.xy / u_res.xy;

    // Center coordinates (-1.0 to 1.0) and fix aspect ratio
    vec2 p = uv * 2.0 - 1.0;
    p.x *= u_res.x / u_res.y;

    // Smooth average RMS for global intensity
    float rms = (u_rmsL + u_rmsR) * 0.5;

    // drifting time base
    float slowTime = u_time * 3.0;

    // Liquid deformation logic
    float wave = 0.0;

    // Sample a few bands from the u_freqs array (bass, mid, high)
    // to modulate the surface tension slowly
    int f1 = 0 ;
    float bass = u_freqs[f1] * 0.4;   // Low frequency influence
    int f2 = int(u_freqCount / 2.0) ;
    float mids = u_freqs[f2] * 0.3;   // Mid frequency influence
    int f3 = int(u_freqCount) - 1;
    float highs = u_freqs[f3] * 0.2; // High frequency influence


    float waveRand = fract(sin(dot(uv, vec2(12.9898, 78.233))) * 43758.5453);

    // Calculate fluid movement using layered sine waves
    // The frequency data subtly shifts the phase of the waves
    wave += sin(p.x * 2.5 + slowTime + bass) * 0.15;
    wave += sin(p.y * 1.8 - slowTime * 0.8 + mids) * 0.12 ;
    wave += cos(length(p * 0.8) - slowTime * 0.5 + highs) * waveRand * rms; //* 0.1;

    // Color palette: Deep, calm oceanic tones
//     vec3 colorDeep = vec3(0.02, 0.05, 0.15); // Deep Dark Blue
//     vec3 colorMid  = vec3(0.1, 0.4, 0.6);    // Calm Teal
//     vec3 colorHigh = vec3(0.4, 0.8, 0.9);    // Bright Cyan/Aqua

    vec3 colorDeep = vec3(f1 * 0.01, f2 * 0.01, f3 * 0.01);
    vec3 colorMid  = vec3(f3 * 0.02, f2 * 0.02, f1 * 0.02);
    vec3 colorHigh = hsv2rgb(vec3(f2 * 0.04, f1 * 0.04, f3 * 0.04));


    // Create a smooth gradient based on wave height and vertical position
    float flow = smoothstep(-0.4, 0.6, wave + p.y * 0.3);
    vec3 finalColor = mix(colorDeep, colorMid, flow);

    // Add a secondary highlight layer that reacts to RMS
    float highlight = smoothstep(0.3, 1.0, wave + rms * 0.5);
    finalColor = mix(finalColor, colorHigh, highlight * 0.4);

    // Soft "Audio-Glow": The whole scene breathes with the RMS volume
    finalColor += (rms * 0.01) * colorHigh;

    // Vignette for depth
//     float vignette = 1.0 - length(p * 0.4);
//     finalColor *= pow(vignette, 1.5) * 0.5;

    float vignette = distance(uv, vec2(0.5));
    float edgeStart =  0.7;
    float edgeEnd = clamp(0.4 + (rms * 0.2), 0.0, 0.75);
    finalColor *= smoothstep(edgeStart, edgeEnd, vignette);


    // -------------- --- CRT Scanline & Grain Layer --- -------------------
    if (u_scanlines) {
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



        // << CRT

    }


//     // Subtle grain noise to prevent color banding
//     float noise = fract(sin(dot(uv, vec2(12.9898, 78.233))) * 43758.5453);
//     finalColor  += noise * 0.015;



    gl_FragColor = vec4(finalColor, 1.0);
}
