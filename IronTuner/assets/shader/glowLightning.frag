precision mediump float;

out vec4 FragColor;

uniform float u_time;
uniform float u_rmsL;
uniform float u_rmsR;
uniform vec2 u_res;
uniform float u_freqCount;
uniform float u_freqs[32];


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

    finalColor += glowColor;

    // -------------------- LIGHTNING BOLT (between L and R) --------------------
    // Define the two centers (matching your Glow positions)
    vec2 posL = vec2(-u_rmsL * 2.0, 0.0);
    vec2 posR = vec2( u_rmsR * 2.0, 0.0);

     // Calculate distance to the line segment between posL and posR
    vec2 dir = posR - posL;
    float len = length(dir);
    vec2 normDir = dir / len;
    vec2 relP = uvGlow - posL;
    float projection = clamp(dot(relP, normDir), 0.0, len);
    vec2 closestPoint = posL + normDir * projection;
    float distToLine = length(uvGlow - closestPoint);

    float waveSum = 0.0;

     for (int i = 0; i < 32; i++) {
        float t = float(i) / 2.0;
        float freq =  u_freqs[i] * 0.1;
        float waveFreq = 0.5 + t * 10.0;
        float phase = 2.0; //u_time * 0.5 ; //* (1.0 + t) + t * //6.28;
        waveSum += sin(uv.x * waveFreq + phase) * freq;
    }

    float boltShape = abs(distToLine +  waveSum) ;
    float boltIntensity = smoothstep(0.2, 0.0, boltShape) * 2.0;
//     float boltIntensity = smoothstep(0.2, 0.0, distToLine + waveSum );

    float hue = fract(u_time * 0.2);
    vec3 colorDeep = hsv2rgb(vec3(hue, 0.5+energy*0.5, 1.0));

    vec3 boltColor = colorDeep * boltIntensity ;

    finalColor += boltColor * smoothstep(0.01, 0.1, energy) ;


    // --- darken ----
    float darken = 1.0 - length(uv - 0.5) * 0.8;
    finalColor *= clamp(darken, 0.0, 1.0);

    FragColor = vec4(finalColor, 1.0);
}
