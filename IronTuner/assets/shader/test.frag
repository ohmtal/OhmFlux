precision mediump float;

out vec4 FragColor;

uniform float u_time;
uniform float u_rmsL;
uniform float u_rmsR;
uniform vec2 u_res;
uniform float u_freqCount;
uniform float u_freqs[32];


const float speed = 5.0;

float hash(float x) {
    return fract(sin(x * 12.9898) * 43758.5453);
}


vec3 hsv2rgb(vec3 c) {
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - vec3(K.w));
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}


void main() {
    vec3 finalColor = vec3(0.0,0.0,0.0);
    vec2 uv = gl_FragCoord.xy / u_res.xy;
    vec2 uvGlow = (gl_FragCoord.xy * 2.0 - u_res.xy) / min(u_res.y, u_res.x);

    //rms values. silent with a little glow
    float rmsL = clamp(u_rmsL, 0.2, 1.0);
    float rmsR = clamp(u_rmsR, 0.2, 1.0);
    float energy = (u_rmsL + u_rmsR) * 0.5;

    // -------------------- GLOW --------------------------

    vec3 colorL = vec3(0.4,0.01,0.02);
    vec3 colorR = vec3(0.01,0.01,0.4);

    float distL = length(uvGlow + vec2(rmsL * 2.5, 0.0));
    float distR = length(uvGlow - vec2(rmsR * 2.5, 0.0));

    float glowL = 1.4 / (distL + 0.7);
    float glowR = 1.4 / (distR + 0.7);

    vec3 glowColor = vec3(0.1, 0.1, 0.2);

    glowColor += colorL * glowL * rmsL;
    glowColor += colorR * glowR * rmsR;

//     finalColor += glowColor;

    // -------------------- LIGHTNING BOLT --------------------

   float waveSum = 0.0;
/*
    for (int i = 0; i < 32; i++) {
        float t = float(i) / 32.0;
        float freq =  u_freqs[i];
        float amplitude = freq * (0.1 + t * 0.2);
        float waveFreq = 50.0 + t * 10.0;

        waveSum += sin(uv.x * waveFreq + u_time * speed + waveSum * 2.0) * amplitude * 0.75;
    }
*/

       for (int i = 0; i < 32; i++) {
        float t = float(i) / 32.0;
        float freq = u_freqs[i];
        if (i == 0) freq *= 0.85; //normalize first bass bar
        float waveFreq = 30 + t * 10.0;
        float phase = u_time * speed * (2.0 + t) + t * 6.28;
        waveSum += sin(uv.x * waveFreq + phase) * freq * 0.15 ;
    }



    float boltShape = abs(uvGlow.y +  waveSum);
    float boltIntensity = smoothstep(0.3, 0.0, boltShape);
    const float glowMulti = 0.3;
    vec3 boltGlow = vec3(0.3, 0.5, 1.0) * (glowMulti / (boltShape + 0.04));

    float hue = fract(u_time * 0.5);
    vec3 colorDeep = hsv2rgb(vec3(hue, 0.2 + energy * 0.5, 0.2));
    vec3 boltColor = colorDeep * (boltIntensity + boltGlow) ;
    // check energy
    finalColor += boltColor *  smoothstep(0.05, 0.5, energy);

    // --- darken ----
    float darken = 1.0 - length(uv - 0.5) * 0.8;
    finalColor *= clamp(darken, 0.0, 1.0);

    FragColor = vec4(finalColor, 1.0);
}
