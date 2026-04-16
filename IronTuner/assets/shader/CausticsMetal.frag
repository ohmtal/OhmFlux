// # Liquid Metal with bars
precision mediump float;

out vec4 FragColor;

uniform float u_time;
uniform vec2 u_res;
uniform float u_rmsL;
uniform float u_rmsR;

uniform float u_freqCount;  // frequencies (bands) count MAX 32!
uniform float u_freqs[32];  // frequency FFT data

// Converts Hue, Saturation, Value to RGB
vec3 hsv2rgb(vec3 c) {
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}



void main() {
    vec3 finalColor = vec3(0.0,0.0,0.0);
    vec2 uv = gl_FragCoord.xy / u_res.xy;
    vec2 p = uv * 3.0;

    float energy = (u_rmsL + u_rmsR) * 0.5;
    float time = u_time * 0.2 ;


    // ~~~~~~~~ LIQUID ~~~~~~~~~~
    float c = 0.0;

    for(float i = 1.0; i < 16.0; i++) {
        uv.x += 0.3 / i * sin(i * 3.0 * uv.y + time);
        uv.y += 0.4 / i * cos(i * 2.5 * uv.x + time);

        // was: 0.015
        float edgeSoftness = 0.015;
        c += (0.08 / (abs(uv.x) + edgeSoftness)) * i;
    }

//     c /= 256.0 * (energy + 0.1);
//     c =  5.0 - pow(c, 2.0);

    float highlights = smoothstep(0.6, 0.9, c);
    finalColor += highlights * ( energy * 0.2 ) ;

    // STARS
    for(float i = 0.0; i < 2.0; i++) {
        float depth = fract(i * 0.25 + time * 0.1);
        float scale = mix(20.0, 0.5, depth);
        float fade = depth * smoothstep(1.0, 0.8, depth);

        vec2 gridPtr = uv * scale + (i * 45.32);
        vec2 id = floor(gridPtr);
        vec2 gv = fract(gridPtr) - 0.5;

        float rand = fract(sin(dot(id, vec2(12.9898, 78.233) + i)) * 43758.5453);


        if(rand > 0.6)
        {
            float dist = length(gv);
            float star = 0.002 / dist;
            float glow = (0.05 * energy) / dist;
            float mask = smoothstep(0.5, 0.2, dist);
            vec3 color = vec3(0.7, 0.8, 1.0) + 0.3 * sin(vec3(0, 1, 2) + i);
             finalColor += color * (star + glow) * fade * mask;
        }
    }


    // -------------

    float vignette = 1.0 - length(uv - 0.5) * 1.2;
    finalColor *= clamp(vignette, 0.0, 1.0);

    FragColor = vec4(finalColor, 1.0);
}
