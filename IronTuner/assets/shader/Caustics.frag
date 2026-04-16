precision mediump float;

uniform float u_time;
uniform vec2 u_res;
uniform float u_rmsL;
uniform float u_rmsR;
uniform float u_freqCount; // 32.0
uniform float u_freqs[32];


vec3 hsv2rgb(vec3 c) {
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - vec3(K.w));
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}


void main() {
    vec2 static_uv = gl_FragCoord.xy / u_res.xy;
    vec2 p = static_uv * 4.0 - 20.0;

    // Frequenz-Analyse für das Wasser:
    float bass = u_freqs[1] * 2.5;
    float mids = u_freqs[15] * 3.0;
    float highs = u_freqs[30] * 5.0;

    float time = u_time * 0.2;

    vec2 i = vec2(p);
    float c = 1.0;
    float intensity = 0.005;

    for (int n = 0; n < 50; n++) {
        float t =  time * (1.0 - (3.0 / float(n + 1)));
        i = p + vec2(cos(t - i.x) + sin(t + i.y), sin(t - i.y) + cos(t + i.x));
        c += 0.1 / length(vec2(p.x / (sin(i.x + t) / intensity), p.y / (cos(i.y + t) / intensity)));
    }

    c /= 5.0;
    c = 1.5 - pow(c, 0.5);

    float hue = fract(u_time * 0.05);
    float dynamicSaturation = 0.5 + (bass * 0.1);
    vec3 color = hsv2rgb(vec3(hue, dynamicSaturation, 0.5));
//      vec3 color = vec3(0.05, 0.15, 0.3);

    float sparkle = pow(abs(c), 8.0);
    vec3 causticColor = vec3(0.2, 0.6, 0.7) * sparkle;

    float energy = (u_rmsL + u_rmsR) * 0.5;
    vec3 finalColor = color + causticColor + (causticColor /** highs*/ * 2.0);
//     finalColor += energy * 0.15 ; //* bass;

    // Vignette
    float vignette = 1.0 - length(static_uv - 0.5) * 1.1;
    finalColor *= clamp(vignette, 0.0, 1.0);

//     float vignette = distance(static_uv, vec2(0.5));
//     float edgeStart =  0.7;
//     float edgeEnd = clamp(0.4 + (energy * 0.2), 0.0, 0.75);
//     finalColor *= smoothstep(edgeStart, edgeEnd, vignette);



    gl_FragColor = vec4(finalColor, 1.0);
}
