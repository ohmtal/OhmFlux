precision mediump float;
// precision highp float;

out vec4 FragColor;

uniform float u_time;
uniform float u_rmsL;
uniform float u_rmsR;
uniform vec2 u_res;
uniform float u_freqCount;     // Currently 16
uniform float u_freqs[32];     // Array of FFT data
uniform bool  u_scanlines;


vec3 hsv2rgb(vec3 c) {
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - vec3(K.w));
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main() {
    vec3 finalColor  = vec3(0.0,0.0,0.0);
    vec2 uv = gl_FragCoord.xy / u_res.xy;
    vec2 p = uv * 2.0 - 1.0;
    p.x *= u_res.x / u_res.y;

    float energy = (u_rmsL + u_rmsR) * 0.5;
    float slowTime =  u_time * 0.8;

    // Liquid deformation logic
    float waveSum = 0.0;


    for (int i = 0; i < int(u_freqCount); i++) {
        float t = float(i) / u_freqCount;
        float freq = u_freqs[i];
        if (i == 0) freq *= 0.85; //normalize first bass bar
        float waveFreq = 1.5 + t * 15.0;
        float phase = slowTime * (2.0 + t) + t * 6.28;
        waveSum += sin(p.x * waveFreq + phase) * freq * 0.15 ;
    }


     float hue = fract(u_time * 0.05);
     vec3 colorDeep = hsv2rgb(vec3(hue, 0.5+energy*0.2, 0.8)) * 3.0;

    vec3 colorMid  = vec3(0.1, 0.4, 0.6);    // Calm Teal
    vec3 colorHigh = vec3(0.4, 0.8, 0.9); // + (energy * 0.5) ;    // Bright Cyan/Aqua

    float height = p.y + waveSum * 1.5;
    float flow =  smoothstep(-0.5, 0.5, height) * 4.0;

    vec3 grad = mix(colorDeep, colorMid, flow );
    grad = mix(grad, colorHigh, smoothstep(0.3, 1.0, flow + waveSum * 0.5));
    finalColor += grad;


    // -------------------------

     float highlight = smoothstep(0.3, 1.0,  energy * 0.1 + 0.4);
     finalColor *= mix(finalColor, colorMid, highlight * 0.4);


    float darken = 1.0 - length(uv - 0.5) * 0.8;
    finalColor *= clamp(darken, 0.0, 1.0);

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

    FragColor = vec4(finalColor, 1.0);}
